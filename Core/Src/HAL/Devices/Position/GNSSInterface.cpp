#include "GNSSInterface.h"


#include "Utils/StringManipulation.h"
#include "Devices/Communication/Interfaces/UartCommunicationInterface.h"

#include <cstring>

using HAL::Utils::StringManipulation;

namespace HAL
{
namespace Devices
{
namespace Position
{

GNSSInterface::GNSSInterface( std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> gnss_uart ) : 
                            TaskWrapper("GnssInterface", 500, nullptr, 2),
                            DebugInterface("GnssInterface"),
                            gnss_uart_(gnss_uart),
                            rx_buffer_pos_(0),
                            is_callback_executing_(false)
{
	gnss_uart_->ListenRxIT([this](const uint8_t *data, uint16_t size){UartCallBack(data, size);});

	for (auto &cb : NMEACallbacks)
		cb = nullptr;

	RegisterCallback( GNSSInterface::NMEAMessageType::GGA, [this](const std::string &msg){ return this->GGACallback(msg); } );
	RegisterCallback( GNSSInterface::NMEAMessageType::GLL, [this](const std::string &msg){ return this->GLLCallback(msg); } );
	RegisterCallback( GNSSInterface::NMEAMessageType::GSA, [this](const std::string &msg){ return this->GSACallback(msg); } );
	RegisterCallback( GNSSInterface::NMEAMessageType::GSV, [this](const std::string &msg){ return this->GSVCallback(msg); } );
	RegisterCallback( GNSSInterface::NMEAMessageType::MSS, [this](const std::string &msg){ return this->MSSCallback(msg); } );
	RegisterCallback( GNSSInterface::NMEAMessageType::RMC, [this](const std::string &msg){ return this->RMCCallback(msg); } );
	RegisterCallback( GNSSInterface::NMEAMessageType::VTG, [this](const std::string &msg){ return this->VTGCallback(msg); } );
}

GNSSInterface::~GNSSInterface()
{

}

void GNSSInterface::RegisterCallback( NMEAMessageType message_type, NMEAParserFunc nmea_func )
{
	if( message_type >= GNSSInterface::NMEAMessageType::MAXMessagesTypes )
	{
		return;
	}

	NMEACallbacks[message_type] = nmea_func;
}

void GNSSInterface::Task(void *params)
{
	while(1)
	{
		if( CanProcessMessage() )
		{
			std::string nmea_message( reinterpret_cast<char*>(uart_buffer_receive_), rx_buffer_pos_ );
			ProcessNMEAMessage(nmea_message);

			rx_buffer_pos_ = 0;
			std::memset(uart_buffer_receive_, 0, kRxBufferSize);
		}

		TaskDelay(200);
	}
}

bool GNSSInterface::ProcessNMEAMessage( const std::string &nmea_message )
{
	if( nmea_message.size() == 0 )
	{
		return false;
	}

	GNSSInterface::NMEAMessageType message_type = GetNMEAMessageType( nmea_message );

	if( message_type == GNSSInterface::NMEAMessageType::MAXMessagesTypes )
	{
		return false;
	}

	if( NMEACallbacks[message_type] == nullptr)
	{
		return false;
	}

	return NMEACallbacks[message_type](nmea_message);
}

GNSSInterface::NMEAMessageType GNSSInterface::GetNMEAMessageType( const std::string &nmea_message )
{
	GNSSInterface::NMEAMessageType message_type;
	StringManipulation strManipulation;

	std::vector<std::string> fields = strManipulation.SplitString(nmea_message, ',');
	if(nmea_message.size() == 0)
	{
		message_type = GNSSInterface::NMEAMessageType::MAXMessagesTypes;
	}
	else
	{
		message_type = ToMessagetypeFromStr(fields.at(0));
	}

	return message_type;
}

GNSSInterface::NMEAMessageType GNSSInterface::ToMessagetypeFromStr( const std::string str_message_type )
{
	if( str_message_type.find("GGA") != std::string::npos )
	{
		return GNSSInterface::NMEAMessageType::GGA;	
	}
	else if( str_message_type.find("GLL") != std::string::npos )
	{
		return GNSSInterface::NMEAMessageType::GLL;	
	}
	else if( str_message_type.find("GSA") != std::string::npos )
	{
		return GNSSInterface::NMEAMessageType::GSA;	
	}
	else if( str_message_type.find("GSV") != std::string::npos )
	{
		return GNSSInterface::NMEAMessageType::GSV;	
	}	
	else if( str_message_type.find("MSS") != std::string::npos )
	{
		return GNSSInterface::NMEAMessageType::MSS;	
	}
	else if( str_message_type.find("RMC") != std::string::npos )
	{
		return GNSSInterface::NMEAMessageType::RMC;	
	}
	else if( str_message_type.find("VTG") != std::string::npos )
	{
		return GNSSInterface::NMEAMessageType::VTG;	
	}
	else 
	{
		return GNSSInterface::NMEAMessageType::MAXMessagesTypes;
	}
}

bool GNSSInterface::GGACallback(const std::string &nmea_msg)
{
	return true;
}

bool GNSSInterface::GLLCallback(const std::string &nmea_msg)
{
	return true;
}

bool GNSSInterface::GSACallback(const std::string &nmea_messages)
{
	return true;
}

bool GNSSInterface::GSVCallback(const std::string &nmea_msg)
{
	return true;
}

bool GNSSInterface::MSSCallback(const std::string &nmea_msg)
{
	return true;
}

bool GNSSInterface::RMCCallback(const std::string &nmea_msg)
{
	return true;
}

bool GNSSInterface::VTGCallback(const std::string &nmea_msg)
{
	return true;
}

void GNSSInterface::UartCallBack( const uint8_t *data, uint16_t size )
{
	if(data == nullptr)
	{
		return;
	}

	if(size >= (kRxBufferSize - rx_buffer_pos_) )
	{
		size = ((kRxBufferSize - rx_buffer_pos_) - 1);
	}

	is_callback_executing_ = true;
	std::memcpy(uart_buffer_receive_ + rx_buffer_pos_, data, size);
	rx_buffer_pos_ += size;
	is_callback_executing_ = false;
}

bool GNSSInterface::CanProcessMessage()
{	
	if(rx_buffer_pos_ == 0)
		return false;

	return (((uart_buffer_receive_[rx_buffer_pos_ - 1] == '\n' || uart_buffer_receive_[rx_buffer_pos_ - 1] == '\r') ) 
			  && is_callback_executing_ == false);
}

}
}
}

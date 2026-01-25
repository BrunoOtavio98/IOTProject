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

}

GNSSInterface::~GNSSInterface()
{

}

void GNSSInterface::Task(void *params)
{
	while(1)
	{
		if( CanProcessMessage() )
		{	
			std::string nmea_message( reinterpret_cast<char*>(uart_buffer_receive_), rx_buffer_pos_ );
			ProcessNMEAMessage(nmea_message);
		}

		TaskDelay(200);
	}
}

bool GNSSInterface::ProcessNMEAMessage( const std::string &nmea_message )
{
	bool status = true;

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
	return (((uart_buffer_receive_[rx_buffer_pos_ - 1] == '\n' || uart_buffer_receive_[rx_buffer_pos_ - 1] == '\r') ) 
			  && is_callback_executing_ == false);
}

}
}
}

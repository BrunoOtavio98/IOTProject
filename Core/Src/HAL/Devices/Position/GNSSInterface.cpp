#include "GNSSInterface.h"

#include "NMEAParser.h"
#include "Utils/StringManipulation.h"
#include "Devices/Communication/Interfaces/UartCommunicationInterface.h"

#include <iostream>
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
                            is_callback_executing_(false),
                            nmea_parser_(std::make_unique<NMEAParser>())
{
	gnss_uart_->ListenRxIT([this](const uint8_t *data, uint16_t size){UartCallBack(data, size);});
}

GNSSInterface::~GNSSInterface()
{

}

void GNSSInterface::Task(void *params)
{   
    NMEAParser::NMEA_Data nmea_data;
	while(1)
	{
		if( CanProcessMessage() )
		{
			std::string nmea_message( reinterpret_cast<char*>(uart_buffer_receive_), rx_buffer_pos_ );

            //nmea_parser_->ProcessNMEAMessage( nmea_message, nullptr );

			rx_buffer_pos_ = 0;
			std::memset(uart_buffer_receive_, 0, kRxBufferSize);
		}

		TaskDelay(200);
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
	if(rx_buffer_pos_ == 0)
		return false;

	return (((uart_buffer_receive_[rx_buffer_pos_ - 1] == '\n' || uart_buffer_receive_[rx_buffer_pos_ - 1] == '\r') ) 
			  && is_callback_executing_ == false);
}

}
}
}

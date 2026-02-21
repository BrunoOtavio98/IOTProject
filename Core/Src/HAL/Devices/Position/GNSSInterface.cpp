#include "GNSSInterface.h"

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
                            nmea_parser_(std::make_unique<NMEAParser>())
{
	gnss_uart_->ListenRxIT([this](const uint8_t *data, uint16_t size){UartCallBack(data, size);});

	active_buffer_.store(&buffer_a_);
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

            if( nmea_parser_->ProcessNMEAMessage( nmea_message, nmea_data ) )
            {
                UpdateGNSSData( nmea_data );
            }

			rx_buffer_pos_ = 0;
			std::memset(uart_buffer_receive_, 0, kRxBufferSize);
		}

		TaskDelay(10);
	}
}

void GNSSInterface::UpdateGNSSData( NMEAParser::NMEA_Data &nmea_data )
{
	BasicGNSSData* active = active_buffer_.load(std::memory_order_acquire);
    BasicGNSSData* inactive = (active == &buffer_a_) ? &buffer_b_ : &buffer_a_;

	*inactive = *active;

	switch( nmea_data.nmea_type )
	{
		case NMEAParser::GGA:
			{
				float raw_lat = nmea_data.payload_received.gga_data.latitude;
				char ns = nmea_data.payload_received.gga_data.NSIndicator;

				float raw_lon = nmea_data.payload_received.gga_data.longitude;
				char ew = nmea_data.payload_received.gga_data.EWIndicator;

				inactive->lat = LatToDeg( raw_lat, ns );
				inactive->lon = LonToDeg(raw_lon, ew);

				inactive->alt = nmea_data.payload_received.gga_data.mslAltitude;
				inactive->hour = nmea_data.payload_received.gga_data.hour;
				inactive->minutes = nmea_data.payload_received.gga_data.minutes;
				inactive->seconds = nmea_data.payload_received.gga_data.seconds;
			}
			break;
	
		case NMEAParser::GSA:
			inactive->pdop = nmea_data.payload_received.gsa_data.pdop;
			inactive->hdop = nmea_data.payload_received.gsa_data.hdop;
			inactive->vdop = nmea_data.payload_received.gsa_data.vdop;
			break;

		case NMEAParser::RMC:
			inactive->year = nmea_data.payload_received.rmc_data.year;
			inactive->month = nmea_data.payload_received.rmc_data.month;
			inactive->day = nmea_data.payload_received.rmc_data.day;
			break;

		case NMEAParser::VTG:
			inactive->speedKmh = nmea_data.payload_received.vtg_data.speedKmh;
			inactive->courseDeg = nmea_data.payload_received.vtg_data.trueTrack;
			break;
		
		default:
			break;
	}

	active_buffer_.store(inactive, std::memory_order_release);
}

void GNSSInterface::GetUpdatedGnssData( BasicGNSSData &gnss_data )
{
	BasicGNSSData *current = active_buffer_.load(std::memory_order_acquire);
	gnss_data = *current;
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

	TaskEnterCriticalSection();
	std::memcpy(uart_buffer_receive_ + rx_buffer_pos_, data, size);
	rx_buffer_pos_ += size;
	TaskExitCriticalSection();
}

bool GNSSInterface::CanProcessMessage()
{	
    bool can_process = false;

    TaskEnterCriticalSection();
    if (rx_buffer_pos_ > 0)
    {
        uint16_t pos = rx_buffer_pos_;
        char last_char = uart_buffer_receive_[pos - 1];

        if (last_char == '\n' || last_char == '\r')
        {
            can_process = true;
        }
    }
	TaskExitCriticalSection();

    return can_process;
}

float GNSSInterface::LatToDeg( float raw, char ns)
{
    int degrees = static_cast<int>(raw / 100);
    double minutes = raw - (degrees * 100);

    float decimal = degrees + (minutes / 60.0);

    if (ns == 'S')
        decimal = -decimal;

    return decimal;
}

float GNSSInterface::LonToDeg( float raw, char ew )
{
    int degrees = static_cast<int>(raw / 100);
    double minutes = raw - (degrees * 100);

    float decimal = degrees + (minutes / 60.0);

    if (ew == 'W')
        decimal = -decimal;

    return decimal;
}

}
}
}

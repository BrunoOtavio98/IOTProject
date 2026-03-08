#include "GNSSInterface.h"

#include "Utils/StringManipulation.h"
#include "Devices/Communication/Interfaces/UartCommunicationInterface.h"
#include "DebugController/DebugController.h"

#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>

using HAL::Utils::StringManipulation;
using HAL::DebugController::DebugController;

namespace HAL
{
namespace Devices
{
namespace Position
{

GNSSInterface::GNSSInterface( std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> gnss_uart, const std::shared_ptr<HAL::DebugController::DebugController> &debug_controler ) : 
                            TaskWrapper("GNSS", 1250, nullptr, 2),
                            DebugInterface("GNSS"),
                            gnss_uart_(gnss_uart),
							debug_controller_(debug_controler),
                            insert_buffer_ctrl_(0),
							process_buffer_ctrl_(0),
							uart_buffer_receive_(),
							empty_buffer_space_(kRxBufferSize),
                            nmea_parser_(std::make_unique<NMEAParser>()),
							buffer_a_(),
							buffer_b_()
{
	debug_controller_->RegisterModuleToDebug(this);
	gnss_uart_->ListenRxIT([this](const uint8_t *data, uint16_t size){UartCallBack(data, size);});
	active_buffer_.store(&buffer_a_);
}

GNSSInterface::~GNSSInterface()
{

}

void GNSSInterface::Task(void *params)
{   
	NMEAParser::NMEA_Data nmea_data = {};
	std::string nmea_message = "";

	while(1)
	{
		if( GetNMEAFrame(nmea_message) )
		{
            if( nmea_parser_->ProcessNMEAMessage( nmea_message, nmea_data ) )
            {
                UpdateGNSSData( nmea_data );

				BasicGNSSData gnss_dta;
				GetUpdatedGnssData( gnss_dta );

				nmea_message.clear();
            }
	
			TaskDelay(1);
		}
		else
		{
			TaskDelay(5);
		}
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

    char buffer1[120];
    int lat_int = (int)(gnss_data.lat * 10000);
    int lon_int = (int)(gnss_data.lon * 10000);
    int alt_int = (int)(gnss_data.alt * 10000);
    std::snprintf(buffer1, sizeof(buffer1), "lat: %d.%04d lon: %d.%04d alt: %d.%04d\n",
                  lat_int / 10000, abs(lat_int) % 10000,
                  lon_int / 10000, abs(lon_int) % 10000,
                  alt_int / 10000, abs(alt_int) % 10000);
    debug_controller_->PrintInfo(this, std::string(buffer1), true);

    char buffer2[100];
    int speed_int = (int)(gnss_data.speedKmh * 10000);
    int course_int = (int)(gnss_data.courseDeg * 10000);
    std::snprintf(buffer2, sizeof(buffer2), "Speed: %d.%04d course: %d.%04d\n",
                  speed_int / 10000, abs(speed_int) % 10000,
                  course_int / 10000, abs(course_int) % 10000);
    debug_controller_->PrintInfo(this, std::string(buffer2), true);
}

void GNSSInterface::UartCallBack( const uint8_t *data, uint16_t size )
{
	if(data == nullptr || (empty_buffer_space_ < size) )
	{
		return;
	}

	std::string std_data( reinterpret_cast<const char*>(data), size );
	debug_controller_->PrintInfo(this, std_data, true);

	if(size >= (kRxBufferSize - insert_buffer_ctrl_) )
	{
		TaskEnterCriticalSection();
		uint16_t empty_space_end = kRxBufferSize - insert_buffer_ctrl_;
		std::memcpy(uart_buffer_receive_ + insert_buffer_ctrl_, data, empty_space_end);
		insert_buffer_ctrl_ = 0;
		TaskExitCriticalSection();
		
		data += empty_space_end;
		size = size - empty_space_end;
		empty_buffer_space_ -= empty_space_end;
	}

	TaskEnterCriticalSection();
	std::memcpy(uart_buffer_receive_ + insert_buffer_ctrl_, data, size);
	insert_buffer_ctrl_ += size;
	empty_buffer_space_ -= size;
	TaskExitCriticalSection();
}

bool GNSSInterface::GetNMEAFrame( std::string &nmea_frame )
{
	TaskEnterCriticalSection();
	uint16_t safe_insert_buffer_ctrl = insert_buffer_ctrl_;
	uint8_t safe_uart_buffer[kRxBufferSize] = "";
	std::memcpy(safe_uart_buffer, uart_buffer_receive_, kRxBufferSize);
	TaskExitCriticalSection();

	int16_t first_char = -1;
	for(int i = process_buffer_ctrl_; i < kRxBufferSize; i++)
	{
		if( safe_uart_buffer[i] == '\r' )
		{
			first_char = i;
			break;
		}
	}

	if(first_char == -1)
	{
		for(int i = 0; i < safe_insert_buffer_ctrl; i++)
		{
			if( safe_uart_buffer[i] == '\r' )
			{
				first_char  = i;
				break;
			}
		}

		if(first_char == -1)
		{
			return false;
		}

		nmea_frame.assign( reinterpret_cast<const char*>(safe_uart_buffer + process_buffer_ctrl_), (kRxBufferSize - process_buffer_ctrl_) );
		nmea_frame.append( reinterpret_cast<const char*>(safe_uart_buffer), first_char );
		first_char++;

		if(safe_uart_buffer[first_char] == '\n')
		{
			first_char++;
		}

		empty_buffer_space_ += (kRxBufferSize - process_buffer_ctrl_) + first_char;

		process_buffer_ctrl_ = first_char;
	}
	else
	{
		nmea_frame.assign( reinterpret_cast<const char*>(safe_uart_buffer + process_buffer_ctrl_), (first_char - process_buffer_ctrl_) );
		first_char++;

		if(safe_uart_buffer[first_char] == '\n')
		{
			first_char++;
		}

		empty_buffer_space_ += first_char - process_buffer_ctrl_;
		process_buffer_ctrl_ = first_char;
	}
	return true;
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

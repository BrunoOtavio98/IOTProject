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

GNSSInterface::GNSSInterface( std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> gnss_uart, 
							  const std::shared_ptr<HAL::DebugController::DebugController> &debug_controler ) : 
                            TaskWrapper("GNSS", 1250, nullptr, 2),
                            DebugInterface("GNSS"),
                            gnss_uart_(gnss_uart),
							debug_controller_(debug_controler),
                            insert_buffer_ctrl_(0),
							process_buffer_ctrl_(0),
							uart_buffer_receive_(),
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
				nmea_message.clear();
            }
	
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
	if(data == nullptr )
	{
		return;
	}

	int i = 0;
	TaskEnterCriticalSection();
	while( i != size )
	{
		uart_buffer_receive_[insert_buffer_ctrl_] = data[i];

		int next_write = GetNextIndex(insert_buffer_ctrl_);
		if( next_write == process_buffer_ctrl_ )
		{
			process_buffer_ctrl_ = GetNextIndex(process_buffer_ctrl_);
		}
		insert_buffer_ctrl_ = next_write;

		i++;
	}
	TaskExitCriticalSection();
}

bool GNSSInterface::GetNMEAFrame( std::string &nmea_frame )
{	
	nmea_frame.clear();

	TaskEnterCriticalSection();
	if( process_buffer_ctrl_ == insert_buffer_ctrl_ )
	{	
		TaskExitCriticalSection();
		return false;
	}

	bool end_of_frame_found = false;
	uint16_t tmp_process_buffer_ctrl = process_buffer_ctrl_;	

	while( tmp_process_buffer_ctrl != insert_buffer_ctrl_ )
	{
		char c =  uart_buffer_receive_[tmp_process_buffer_ctrl];

		if( c == '\r' )
		{	
			end_of_frame_found = true;
			tmp_process_buffer_ctrl = GetNextIndex(tmp_process_buffer_ctrl);

			c = uart_buffer_receive_[tmp_process_buffer_ctrl];
			if( c == '\n' ) 
			{
				tmp_process_buffer_ctrl = GetNextIndex(tmp_process_buffer_ctrl);
			}

			break;
		}
	
		nmea_frame += c;
		tmp_process_buffer_ctrl = GetNextIndex( tmp_process_buffer_ctrl );
	}
	
	if( end_of_frame_found == true )
	{
		process_buffer_ctrl_ = tmp_process_buffer_ctrl;
	}

	TaskExitCriticalSection();
	return end_of_frame_found;
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

int GNSSInterface::GetNextIndex(int index)
{
	int new_index = index + 1;

	if( new_index == kRxBufferSize )
	{
		new_index = 0;
	}

	return new_index;
}

}
}
}

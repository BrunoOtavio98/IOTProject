#include "GNSSInterface.h"


#include "Utils/StringManipulation.h"
#include "Devices/Communication/Interfaces/UartCommunicationInterface.h"

#include <cstring>
#include <iostream>

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
  	GNSSInterface::NMEA_GGA gga_msg = {0};
    StringManipulation strManipulation;
    std::vector<std::string> fields = strManipulation.SplitString(nmea_msg, ',');
    if( fields.size() < 15 )
    {	
		std::cout << "Wrong number of fields\n";
        return false;
    }

    if( !ValidateCheckSum(nmea_msg) )
    {	
		std::cout << "wrong checksum\n";
        return false;
    }

    // Field 0: Message ID ($GPGGA)
    if( fields[0].size() >= kMaxMessageIdSize )
        return false;
    std::strncpy(reinterpret_cast<char*>(gga_msg.messageID.data()), fields[0].c_str(), kMaxMessageIdSize - 1);

    // Field 1: UTC Time (HHMMSS.SS) - Optional
    if( !fields[1].empty() )
    {
        if( fields[1].size() >= 9 )
		{	
			std::cout << "Wrong utc field size\n";
            return false;
		}
        try 
        {
            gga_msg.hour = std::stoi(fields[1].substr(0, 2));
            gga_msg.minutes = std::stoi(fields[1].substr(2, 2));
            gga_msg.seconds = std::stoi(fields[1].substr(4, 2));
        }
        catch (...) 
        {
			std::cout << "Failed at utc trycatch\n";
            return false;
        }
    }

    // Field 2: Latitude - Optional
    if( !fields[2].empty() )
    {
        try
        {
            gga_msg.latitude = std::stof(fields[2]);
        }
        catch (...) 
        {
			std::cout << "failed at latitude trycatch\n";
            return false;
        }
    }

    // Field 3: N/S Indicator - Optional
    if( !fields[3].empty() )
    {
        if( fields[3].size() != 1 || (fields[3][0] != 'N' && fields[3][0] != 'S') )
            return false;
        gga_msg.NSIndicator = fields[3][0];
    }

    // Field 4: Longitude - Optional
    if( !fields[4].empty() )
    {
        try 
        {
            gga_msg.longitude = std::stof(fields[4]);
        } 
        catch (...)
        {
            return false;
        }
    }

    // Field 5: E/W Indicator - Optional
    if( !fields[5].empty() )
    {
        if( fields[5].size() != 1 || (fields[5][0] != 'E' && fields[5][0] != 'W') )
            return false;
        gga_msg.EWIndicator = fields[5][0];
    }

    // Field 6: Position Fix Indicator - Optional
    if( !fields[6].empty() )
    {
        try 
        {
            uint8_t fix_indicator = std::stoi(fields[6]);
            if( fix_indicator >= DEAD_RECKONING + 1 )
                return false;
            gga_msg.positionFixIndicator = static_cast<PositionFixIndicator>(fix_indicator);
        } 
        catch (...) 
        {
            return false;
        }
    }

    // Field 7: Number of Satellites (skipping for now)

    // Field 8: HDOP (Horizontal Dilution of Precision) - Optional
    if( !fields[8].empty() )
    {
        try
        {
            gga_msg.hdop = std::stof(fields[8]);
        } 
        catch (...)
        {
            return false;
        }
    }

    // Field 9: MSL Altitude - Optional
    if( !fields[9].empty() )
    {
        try 
        {
            gga_msg.mslAltitude = std::stof(fields[9]);
        } 
        catch (...) 
        {
            return false;
        }
    }

    // Field 10: Altitude Units (M = meters, skip validation)

    // Field 11: Geoid Separation - Optional
    if( !fields[11].empty() )
    {
        try
        {
            gga_msg.geoidSeparation = std::stof(fields[11]);
        } 
        catch (...) 
        {
            return false;
        }
    }

    return true;
}

bool GNSSInterface::GLLCallback(const std::string &nmea_msg)
{
    GNSSInterface::NMEA_GLL gll_msg = {0};
    StringManipulation strManipulation;
    std::vector<std::string> fields = strManipulation.SplitString(nmea_msg, ',');
    
    if( fields.size() < 6 )
    {
        std::cout << "Wrong number of fields for GLL\n";
        return false;
    }

    if( !ValidateCheckSum(nmea_msg) )
    {
        std::cout << "Wrong checksum for GLL\n";
        return false;
    }

    // Field 0: Message ID
    if( fields[0].size() >= kMaxMessageIdSize )
        return false;
    std::strncpy(reinterpret_cast<char*>(gll_msg.messageID.data()), fields[0].c_str(), kMaxMessageIdSize - 1);

    // Field 1: Latitude
    if( !fields[1].empty() )
    {
        try 
        {
            gll_msg.latitude = std::stof(fields[1]);
        } 
        catch (...)
        {
            return false;
        }
    }

    // Field 2: N/S Indicator
    if( !fields[2].empty() )
    {
        if( fields[2].size() != 1 || (fields[2][0] != 'N' && fields[2][0] != 'S') )
            return false;
        gll_msg.NSIndicator = fields[2][0];
    }

    // Field 3: Longitude
    if( !fields[3].empty() )
    {
        try 
        {
            gll_msg.longitude = std::stof(fields[3]);
        } 
        catch (...) 
        {
            return false;
        }
    }

    // Field 4: E/W Indicator
    if( !fields[4].empty() )
    {
        if( fields[4].size() != 1 || (fields[4][0] != 'E' && fields[4][0] != 'W') )
            return false;
        gll_msg.EWIndicator = fields[4][0];
    }

    // Field 5: UTC Time
    if( !fields[5].empty() )
    {   
        std::cout << "utc len: " << fields[5].size() << std::endl;
        if( fields[5].size() != 6 )
            return false;
        try 
        {
            gll_msg.hour = std::stoi(fields[5].substr(0, 2));
            gll_msg.minutes = std::stoi(fields[5].substr(2, 2));
            gll_msg.seconds = std::stoi(fields[5].substr(4, 2));
        } 
        catch (...) 
        {
            return false;
        }
    }

    return true;
}

bool GNSSInterface::GSACallback(const std::string &nmea_msg)
{
    GNSSInterface::NMEA_GSA gsa_msg = {0};
    StringManipulation strManipulation;
    std::vector<std::string> fields = strManipulation.SplitString(nmea_msg, ',');
    
    if( fields.size() < 18 )
    {
        return false;
    }

    if( !ValidateCheckSum(nmea_msg) )
    {
        return false;
    }

    // Field 0: Message ID
    if( fields[0].size() >= kMaxMessageIdSize )
    {   
        return false;
    }
    std::strncpy(reinterpret_cast<char*>(gsa_msg.messageID.data()), fields[0].c_str(), kMaxMessageIdSize - 1);

    // Field 1: Mode Selection (A=Auto, M=Manual)
    if( !fields[1].empty() )
    {
        if( fields[1].size() != 1 || (fields[1][0] != 'A' && fields[1][0] != 'M') )
        {   
            return false;
        }
        gsa_msg.modeSelection = fields[1][0];
    }

    // Field 2: Mode (1=Fix not available, 2=2D, 3=3D)
    if( !fields[2].empty() )
    {
        try 
        {
            gsa_msg.mode = std::stoi(fields[2]);
            if( gsa_msg.mode < 1 || gsa_msg.mode > 3 )
            {
                return false;
            }
        } 
        catch (...) 
        {
            return false;
        }
    }

    // Field 15: PDOP
    if( !fields[15].empty() )
    {
        try 
        {
            gsa_msg.pdop = std::stof(fields[15]);
        }
        catch (...) 
        {
            return false;
        }
    }

    // Field 16: HDOP
    if( !fields[16].empty() )
    {
        try 
        {
            gsa_msg.hdop = std::stof(fields[16]);
        }
        catch (...) 
        {
            return false;
        }
    }

    // Field 17: VDOP
    if( !fields[17].empty() )
    {
        try 
        {
            gsa_msg.vdop = std::stof(fields[17]);
        } 
        catch (...)
        {
            return false;
        }
    }

    return true;
}

bool GNSSInterface::GSVCallback(const std::string &nmea_msg)
{
	return true;
}

bool GNSSInterface::MSSCallback(const std::string &nmea_msg)
{
    GNSSInterface::NMEA_MSS mss_msg = {0};
    StringManipulation strManipulation;
    std::vector<std::string> fields = strManipulation.SplitString(nmea_msg, ',');
    
    if( fields.size() < 4 )
    {
        std::cout << "Wrong number of fields for MSS\n";
        return false;
    }

    if( !ValidateCheckSum(nmea_msg) )
    {
        std::cout << "Wrong checksum for MSS\n";
        return false;
    }

    // Field 0: Message ID
    if( fields[0].size() >= kMaxMessageIdSize )
        return false;
    std::strncpy(reinterpret_cast<char*>(mss_msg.messageID.data()), fields[0].c_str(), kMaxMessageIdSize - 1);

    // Field 1: Signal Strength
    if( !fields[1].empty() )
    {
        try {
            mss_msg.signalStrength = std::stoi(fields[1]);
        } catch (...) {
            return false;
        }
    }

    // Field 2: Signal-to-Noise Ratio
    if( !fields[2].empty() )
    {
        try {
            mss_msg.snr = std::stoi(fields[2]);
        } catch (...) {
            return false;
        }
    }

    return true;
}

bool GNSSInterface::RMCCallback(const std::string &nmea_msg)
{
    GNSSInterface::NMEA_RMC rmc_msg = {0};
    StringManipulation strManipulation;
    std::vector<std::string> fields = strManipulation.SplitString(nmea_msg, ',');
    
    if( fields.size() < 12 )
    {
        std::cout << "Wrong number of fields for RMC\n";
        return false;
    }

    if( !ValidateCheckSum(nmea_msg) )
    {
        std::cout << "Wrong checksum for RMC\n";
        return false;
    }

    // Field 0: Message ID
    if( fields[0].size() >= kMaxMessageIdSize )
        return false;
    std::strncpy(reinterpret_cast<char*>(rmc_msg.messageID.data()), fields[0].c_str(), kMaxMessageIdSize - 1);

    // Field 1: UTC Time
    if( !fields[1].empty() )
    {
        if( fields[1].size() != 6 )
            return false;
        try {
            rmc_msg.hour = std::stoi(fields[1].substr(0, 2));
            rmc_msg.minutes = std::stoi(fields[1].substr(2, 2));
            rmc_msg.seconds = std::stoi(fields[1].substr(4, 2));
        } catch (...) {
            return false;
        }
    }

    // Field 2: Status (A=Valid, V=Invalid)
    if( !fields[2].empty() )
    {
        if( fields[2].size() != 1 || (fields[2][0] != 'A' && fields[2][0] != 'V') )
            return false;
        rmc_msg.Status = (fields[2][0] == 'A');
    }

    // Field 3: Latitude
    if( !fields[3].empty() )
    {
        try {
            rmc_msg.latitude = std::stof(fields[3]);
        } catch (...) {
            return false;
        }
    }

    // Field 4: N/S Indicator
    if( !fields[4].empty() )
    {
        if( fields[4].size() != 1 || (fields[4][0] != 'N' && fields[4][0] != 'S') )
            return false;
        rmc_msg.NSIndicator = fields[4][0];
    }

    // Field 5: Longitude
    if( !fields[5].empty() )
    {
        try {
            rmc_msg.longitude = std::stof(fields[5]);
        } catch (...) {
            return false;
        }
    }

    // Field 6: E/W Indicator
    if( !fields[6].empty() )
    {
        if( fields[6].size() != 1 || (fields[6][0] != 'E' && fields[6][0] != 'W') )
            return false;
        rmc_msg.EWIndicator = fields[6][0];
    }

    // Field 7: Speed over ground (knots)
    if( !fields[7].empty() )
    {
        try {
            rmc_msg.speedOverGround = std::stof(fields[7]);
        } catch (...) {
            return false;
        }
    }

    // Field 8: Course over ground (degrees)
    if( !fields[8].empty() )
    {
        try {
            rmc_msg.courseOverGround = std::stof(fields[8]);
        } catch (...) {
            return false;
        }
    }

    // Field 9: Date (DDMMYY)
    if( !fields[9].empty() )
    {
        if( fields[9].size() != 6 )
            return false;
        try {
            rmc_msg.day = std::stoi(fields[9].substr(0, 2));
            rmc_msg.month = std::stoi(fields[9].substr(2, 2));
            rmc_msg.year = std::stoi(fields[9].substr(4, 2));
        } catch (...) {
            return false;
        }
    }

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

bool GNSSInterface::ValidateCheckSum( const std::string &nmea_msg )
{
    size_t start = nmea_msg.find('$');
    size_t end = nmea_msg.find('*');
    
    if( start == std::string::npos || end == std::string::npos )
    {	
		std::cout << "wrong start and end\n";
        return false;
    }

    start++; // Move past the '$'
    
    // XOR all characters between $ and *
    uint8_t calculated_checksum = 0;
    for( size_t i = start; i < end; i++ )
    {
        calculated_checksum ^= static_cast<uint8_t>(nmea_msg[i]);
    }

    // Extract checksum from message (after *)
    std::string received_checksum_str = nmea_msg.substr(end + 1);

    // Remove any trailing characters like \r or \n
    size_t checksum_end = received_checksum_str.find_first_not_of("0123456789ABCDEFabcdef");
    if( checksum_end != std::string::npos )
    {
        received_checksum_str = received_checksum_str.substr(0, checksum_end);
    }

    if( received_checksum_str.empty() )
    {	
		std::cout << "received checksum empty\n";
        return false;
    }

    // Convert received checksum from hex string to uint8_t
    try {
        uint8_t received_checksum = static_cast<uint8_t>(std::stoi(received_checksum_str, nullptr, 16));
        
		std::cout << "received: " << static_cast<int>(received_checksum) << " calculated: " << static_cast<int>(calculated_checksum) << std::endl;
		return calculated_checksum == received_checksum;
    } catch (...) {
		std::cout << "failed on converting checksum\n";
        return false;
    }
}

}
}
}

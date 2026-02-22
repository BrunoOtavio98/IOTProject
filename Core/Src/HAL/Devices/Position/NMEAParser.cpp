#include "NMEAParser.h"

#include "Utils/StringManipulation.h"
#include "Utils/SafeParse.h"

#include <cstring>

using HAL::Utils::StringManipulation;
using HAL::Utils::SafeParse;

namespace HAL
{
namespace Devices
{
namespace Position
{
NMEAParser::NMEAParser()
{
    for (auto &cb : NMEACallbacks)
    cb = nullptr;

	RegisterCallback( NMEAParser::NMEAMessageType::GGA, [this](const std::string &msg, NMEA_Data &parsed_data){ return this->GGACallback(msg, parsed_data); } );
	RegisterCallback( NMEAParser::NMEAMessageType::GLL, [this](const std::string &msg, NMEA_Data &parsed_data){ return this->GLLCallback(msg, parsed_data); } );
	RegisterCallback( NMEAParser::NMEAMessageType::GSA, [this](const std::string &msg, NMEA_Data &parsed_data){ return this->GSACallback(msg, parsed_data); } );
	RegisterCallback( NMEAParser::NMEAMessageType::GSV, [this](const std::string &msg, NMEA_Data &parsed_data){ return this->GSVCallback(msg, parsed_data); } );
	RegisterCallback( NMEAParser::NMEAMessageType::MSS, [this](const std::string &msg, NMEA_Data &parsed_data){ return this->MSSCallback(msg, parsed_data); } );
	RegisterCallback( NMEAParser::NMEAMessageType::RMC, [this](const std::string &msg, NMEA_Data &parsed_data){ return this->RMCCallback(msg, parsed_data); } );
	RegisterCallback( NMEAParser::NMEAMessageType::VTG, [this](const std::string &msg, NMEA_Data &parsed_data){ return this->VTGCallback(msg, parsed_data); } );
}

NMEAParser::~NMEAParser()
{
}

void NMEAParser::RegisterCallback( NMEAMessageType message_type, NMEAParserFunc nmea_func )
{
	if( message_type >= NMEAParser::NMEAMessageType::MAXMessagesTypes )
	{
		return;
	}

	NMEACallbacks[message_type] = nmea_func;
}

bool NMEAParser::ProcessNMEAMessage( const std::string &nmea_message, NMEA_Data &data_received )
{
	if( nmea_message.size() == 0 )
	{
		return false;
	}

	NMEAParser::NMEAMessageType message_type = GetNMEAMessageType( nmea_message );

	if( message_type == NMEAParser::NMEAMessageType::MAXMessagesTypes )
	{
		return false;
	}

	if( NMEACallbacks[message_type] == nullptr)
	{
		return false;
	}

    data_received.nmea_type = message_type;

	return NMEACallbacks[message_type](nmea_message, data_received);
}

NMEAParser::NMEAMessageType NMEAParser::GetNMEAMessageType( const std::string &nmea_message )
{
	NMEAParser::NMEAMessageType message_type;
	StringManipulation strManipulation;

	std::vector<std::string> fields = strManipulation.SplitString(nmea_message, ',');
	if(nmea_message.size() == 0)
	{
		message_type = NMEAParser::NMEAMessageType::MAXMessagesTypes;
	}
	else
	{
		message_type = ToMessagetypeFromStr(fields.at(0));
	}

	return message_type;
}

NMEAParser::NMEAMessageType NMEAParser::ToMessagetypeFromStr( const std::string str_message_type )
{
	if( str_message_type.find("GGA") != std::string::npos )
	{
		return NMEAParser::NMEAMessageType::GGA;	
	}
	else if( str_message_type.find("GLL") != std::string::npos )
	{
		return NMEAParser::NMEAMessageType::GLL;	
	}
	else if( str_message_type.find("GSA") != std::string::npos )
	{
		return NMEAParser::NMEAMessageType::GSA;	
	}
	else if( str_message_type.find("GSV") != std::string::npos )
	{
		return NMEAParser::NMEAMessageType::GSV;	
	}	
	else if( str_message_type.find("MSS") != std::string::npos )
	{
		return NMEAParser::NMEAMessageType::MSS;	
	}
	else if( str_message_type.find("RMC") != std::string::npos )
	{
		return NMEAParser::NMEAMessageType::RMC;	
	}
	else if( str_message_type.find("VTG") != std::string::npos )
	{
		return NMEAParser::NMEAMessageType::VTG;	
	}
	else 
	{
		return NMEAParser::NMEAMessageType::MAXMessagesTypes;
	}
}

std::string NMEAParser::StripNMEAChecksum(const std::string &field)
{
	size_t asterisk_pos = field.find('*');
	if (asterisk_pos != std::string::npos)
	{
		return field.substr(0, asterisk_pos);
	}
	return field;
}

bool NMEAParser::GGACallback(const std::string &nmea_msg, NMEA_Data &parsed_data)
{	
  	NMEAParser::NMEA_GGA gga_msg = {0};
    StringManipulation strManipulation;
    std::vector<std::string> fields = strManipulation.SplitString(nmea_msg, ',');
    if( fields.size() < 15 )
    {	    
        return false;
    }

    if( !ValidateCheckSum(nmea_msg) )
    {	
        return false;
    }

    // Field 0: Message ID ($GPGGA)
    if( fields[0].size() >= NMEAParser::kMaxMessageIdSize )
        return false;
    std::strncpy(reinterpret_cast<char*>(gga_msg.messageID.data()), fields[0].c_str(), NMEAParser::kMaxMessageIdSize - 1);

    // Field 1: UTC Time (HHMMSS.SS) - Optional
    if (!fields[1].empty())
    {
        if (fields[1].size() >= 9)
            return false;

        std::string tmp;
        tmp = fields[1].substr(0, 2);
        if (!SafeParse::ToUint8t(tmp, gga_msg.hour)) 
        {
            return false;
        }

        tmp = fields[1].substr(2, 2);
        if (!SafeParse::ToUint8t(tmp, gga_msg.minutes))
        {   
            return false;
        }

        tmp = fields[1].substr(4, 2);
        if (!SafeParse::ToUint8t(tmp, gga_msg.seconds))
        {
            return false;
        }
    }

    // Field 2: Latitude - Optional
    if(!fields[2].empty())
    {
        if (!SafeParse::ToFloat(fields[2], gga_msg.latitude))
        {   
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
    if(!fields[4].empty())
    {
        if (!SafeParse::ToFloat(fields[4], gga_msg.longitude))
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
    if(!fields[6].empty())
    {
        uint8_t fix_indicator;
        if (!SafeParse::ToUint8t(fields[6], fix_indicator))
        {
            return false;
        }

        if (fix_indicator >= DEAD_RECKONING + 1)
        {
            return false;
        }
        
        gga_msg.positionFixIndicator = static_cast<PositionFixIndicator>(fix_indicator);
    }

    // Field 7: Number of Satellites (skipping for now)

    // Field 8: HDOP (Horizontal Dilution of Precision) - Optional
    if(!fields[8].empty())
    {
        if (!SafeParse::ToFloat(fields[8], gga_msg.hdop))
        {
            return false;
        }
    }

    // Field 9: MSL Altitude - Optional
    if(!fields[9].empty())
    {
        if (!SafeParse::ToFloat(fields[9], gga_msg.mslAltitude))
        {
            return false;
        }
    }

    // Field 10: Altitude Units (M = meters, skip validation)

    // Field 11: Geoid Separation - Optional
    if(!fields[11].empty())
    {
        std::string geoid_str = StripNMEAChecksum(fields[11]);
        if (!SafeParse::ToFloat(geoid_str, gga_msg.geoidSeparation))
        {
            return false;
        }
    }

    std::memcpy( &parsed_data.payload_received.gga_data, &gga_msg, sizeof(gga_msg) );
    return true;
}

bool NMEAParser::GLLCallback(const std::string &nmea_msg, NMEA_Data &parsed_data)
{
    NMEAParser::NMEA_GLL gll_msg = {0};
    StringManipulation strManipulation;
    std::vector<std::string> fields = strManipulation.SplitString(nmea_msg, ',');
    
    if( fields.size() < 6 )
    {
        return false;
    }

    if( !ValidateCheckSum(nmea_msg) )
    {
        return false;
    }

    // Field 0: Message ID
    if( fields[0].size() >= NMEAParser::kMaxMessageIdSize )
        return false;
    std::strncpy(reinterpret_cast<char*>(gll_msg.messageID.data()), fields[0].c_str(), NMEAParser::kMaxMessageIdSize - 1);

    // Field 1: Latitude
    if( !fields[1].empty() )
    {
        if (!SafeParse::ToFloat(fields[1], gll_msg.latitude))
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
        if (!SafeParse::ToFloat(fields[3], gll_msg.longitude))
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
        if( fields[5].size() != 6 )
            return false;

       if ( !SafeParse::ToUint8t(fields[5].substr(0,2), gll_msg.hour) ||
            !SafeParse::ToUint8t(fields[5].substr(2,2), gll_msg.minutes) ||
            !SafeParse::ToUint8t(fields[5].substr(4,2), gll_msg.seconds))
        {
            return false;
        }
    }

    std::memcpy( &parsed_data.payload_received.gll_data, &gll_msg, sizeof(gll_msg) );
    return true;
}

bool NMEAParser::GSACallback(const std::string &nmea_msg, NMEA_Data &parsed_data)
{
    NMEAParser::NMEA_GSA gsa_msg = {0};
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
    if( fields[0].size() >= NMEAParser::kMaxMessageIdSize )
    {   
        return false;
    }
    std::strncpy(reinterpret_cast<char*>(gsa_msg.messageID.data()), fields[0].c_str(), NMEAParser::kMaxMessageIdSize - 1);

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
        if (!SafeParse::ToUint8t(fields[2], gsa_msg.mode))
        {
            return false;
        }

        if( gsa_msg.mode < 1 || gsa_msg.mode > 3 )
        {
            return false;
        }
    }

    // Field 15: PDOP
    if( !fields[15].empty() )
    { 
        if (!SafeParse::ToFloat(fields[15], gsa_msg.pdop))
       {
            return false;
       }
    }

    // Field 16: HDOP
    if( !fields[16].empty() )
    {
        if (!SafeParse::ToFloat(fields[16], gsa_msg.hdop))
        {
            return false;
        }
    }

    // Field 17: VDOP
    if( !fields[17].empty() )
    {
        std::string vdop_str = StripNMEAChecksum(fields[17]);
        if (!SafeParse::ToFloat(vdop_str, gsa_msg.vdop))
       {
            return false;
       }
    }

    std::memcpy( &parsed_data.payload_received.gsa_data, &gsa_msg, sizeof(gsa_msg) );
    return true;
}

bool NMEAParser::GSVCallback(const std::string &nmea_msg, NMEA_Data &parsed_data)
{
    NMEAParser::NMEA_GSV gsv_msg = {0};
    StringManipulation strManipulation;
    std::vector<std::string> fields = strManipulation.SplitString(nmea_msg, ',');
    
    if( fields.size() < 8 )
    {
        return false;
    }

    if( !ValidateCheckSum(nmea_msg) )
    {
        return false;
    }

    // Field 0: Message ID
    if( fields[0].size() >= NMEAParser::kMaxMessageIdSize )
        return false;
    std::strncpy(reinterpret_cast<char*>(gsv_msg.messageID.data()), fields[0].c_str(), NMEAParser::kMaxMessageIdSize - 1);

    // Field 1: Total number of messages
    if( !fields[1].empty() )
    {
        if( !SafeParse::ToUint8t(fields[1], gsv_msg.numberOfMessages) )
        {
            return false;
        }
    }

    // Field 2: Message number
    if( !fields[2].empty() )
    {   
        if( !SafeParse::ToUint8t(fields[2], gsv_msg.messageNumber) )
        {
            return false;
        }
    }

    // Field 3: Total satellites in view
    if( !fields[3].empty() )
    {
        if( !SafeParse::ToUint8t(fields[3], gsv_msg.satellitesInView) )
        {
            return false;
        }
    }

    // Fields 4-7: Satellite data (up to 4 satellites per message)
    // for( int i = 0; i < 4 && (4 + i * 4 + 3) < fields.size(); i++ )
    // {
    //     // Satellite ID
    //     if( !fields[4 + i * 4].empty() )
    //     {
    //         try {
    //             gsv_msg.satellites[i].id = std::stoi(fields[4 + i * 4]);
    //         } catch (...) {
    //             return false;
    //         }
    //     }

    //     // Elevation
    //     if( !fields[5 + i * 4].empty() )
    //     {
    //         try {
    //             gsv_msg.satellites[i].elevation = std::stoi(fields[5 + i * 4]);
    //         } catch (...) {
    //             return false;
    //         }
    //     }

    //     // Azimuth
    //     if( !fields[6 + i * 4].empty() )
    //     {
    //         try {
    //             gsv_msg.satellites[i].azimuth = std::stoi(fields[6 + i * 4]);
    //         } catch (...) {
    //             return false;
    //         }
    //     }

    //     // SNR (Signal-to-Noise Ratio)
    //     if( !fields[7 + i * 4].empty() )
    //     {
    //         try {
    //             gsv_msg.satellites[i].snr = std::stoi(fields[7 + i * 4]);
    //         } catch (...) {
    //             return false;
    //         }
    //     }
    // }

    std::memcpy( &parsed_data.payload_received.gsv_data, &gsv_msg, sizeof(gsv_msg) );
    return true;
}

bool NMEAParser::MSSCallback(const std::string &nmea_msg, NMEA_Data &parsed_data)
{
    NMEAParser::NMEA_MSS mss_msg = {0};
    StringManipulation strManipulation;
    std::vector<std::string> fields = strManipulation.SplitString(nmea_msg, ',');
    
    if( fields.size() < 4 )
    {
        return false;
    }

    if( !ValidateCheckSum(nmea_msg) )
    {
        return false;
    }

    // Field 0: Message ID
    if( fields[0].size() >= NMEAParser::kMaxMessageIdSize )
        return false;
    std::strncpy(reinterpret_cast<char*>(mss_msg.messageID.data()), fields[0].c_str(), NMEAParser::kMaxMessageIdSize - 1);

    // Field 1: Signal Strength
    if( !fields[1].empty() )
    {
        if (!SafeParse::ToUint8t(fields[1], mss_msg.signalStrength))
        {
            return false;
        }
    }

    // Field 2: Signal-to-Noise Ratio
    if( !fields[2].empty() )
    {
        if (!SafeParse::ToUint8t(fields[2], mss_msg.snr))
        {
            return false;
        }
    }

    // Field 3: Beacon frequency
    if( !fields[3].empty() )
    {
        if (!SafeParse::ToFloat(fields[3], mss_msg.beaconFrequency))
        {
            return false;
        }
    }

    // Field 4: Beacon bit rate - may contain checksum
    if( !fields[4].empty() )
    {
        std::string bitrate_str = StripNMEAChecksum(fields[4]);
        if (!SafeParse::ToUint8t(bitrate_str, mss_msg.beaconBitRate))
        {
            return false;
        }
    }

    std::memcpy( &parsed_data.payload_received.mss_data, &mss_msg, sizeof(mss_msg) );
    return true;
}

bool NMEAParser::RMCCallback(const std::string &nmea_msg, NMEA_Data &parsed_data)
{
    NMEAParser::NMEA_RMC rmc_msg = {0};
    StringManipulation strManipulation;
    std::vector<std::string> fields = strManipulation.SplitString(nmea_msg, ',');
    
    if( fields.size() < 12 )
    {
        return false;
    }

    if( !ValidateCheckSum(nmea_msg) )
    {
        return false;
    }

    // Field 0: Message ID
    if( fields[0].size() >= NMEAParser::kMaxMessageIdSize )
        return false;
    std::strncpy(reinterpret_cast<char*>(rmc_msg.messageID.data()), fields[0].c_str(), NMEAParser::kMaxMessageIdSize - 1);

    // Field 1: UTC Time
    if( !fields[1].empty() )
    {
        if( fields[1].size() != 6 )
            return false;

        std::string tmp;
        tmp = fields[1].substr(0, 2);
        if (!SafeParse::ToUint8t(tmp, rmc_msg.hour)) 
        {
            return false;
        }

        tmp = fields[1].substr(2, 2);
        if (!SafeParse::ToUint8t(tmp, rmc_msg.minutes))
        {   
            return false;
        }

        tmp = fields[1].substr(4, 2);
        if (!SafeParse::ToUint8t(tmp, rmc_msg.seconds))
        {
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
        if (!SafeParse::ToFloat(fields[3], rmc_msg.latitude))
        {
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
        if (!SafeParse::ToFloat(fields[5], rmc_msg.longitude))
        {
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
        if (!SafeParse::ToFloat(fields[7], rmc_msg.speedOverGround))
        {
            return false;
        }
    }

    // Field 8: Course over ground (degrees)
    if( !fields[8].empty() )
    {
        if (!SafeParse::ToFloat(fields[8], rmc_msg.courseOverGround))
        {
            return false;
        }
    }

    // Field 9: Date (DDMMYY)
    if( !fields[9].empty() )
    {
        if( fields[9].size() != 6 )
            return false;

        std::string day_str = fields[9].substr(0, 2);
        std::string month_str = fields[9].substr(2, 2);
        std::string year_str = fields[9].substr(4, 2);
        
        if ( !SafeParse::ToUint8t(day_str, rmc_msg.day) || 
             !SafeParse::ToUint8t(month_str, rmc_msg.month) || 
             !SafeParse::ToUint8t(year_str, rmc_msg.year) )
        {
            return false;
        }
    }

    // Field 10: Magnetic variation
    if( !fields[10].empty() )
    {
        if (!SafeParse::ToFloat(fields[10], rmc_msg.magneticVariation))
        {
            return false;
        }
    }

    // Field 11: mode - may contain checksum
    if( !fields[11].empty() )
    {
        std::string mode_str = StripNMEAChecksum(fields[11]);
        if (!mode_str.empty())
        {
            rmc_msg.mode = mode_str[0];
        }
    }

    std::memcpy( &parsed_data.payload_received.rmc_data, &rmc_msg, sizeof(rmc_msg) );
    return true;
}

bool NMEAParser::VTGCallback(const std::string &nmea_msg, NMEA_Data &parsed_data)
{
    NMEAParser::NMEA_VTG vtg_msg = {0};
    StringManipulation strManipulation;
    std::vector<std::string> fields = strManipulation.SplitString(nmea_msg, ',');
    
    if( fields.size() < 9 )
    {
        return false;
    }

    if( !ValidateCheckSum(nmea_msg) )
    {
        return false;
    }

    // Field 0: Message ID
    if( fields[0].size() >= NMEAParser::kMaxMessageIdSize )
        return false;
    std::strncpy(reinterpret_cast<char*>(vtg_msg.messageID.data()), fields[0].c_str(), NMEAParser::kMaxMessageIdSize - 1);

    // Field 1: True track made good (degrees)
    if( !fields[1].empty() )
    {
        if (!SafeParse::ToFloat(fields[1], vtg_msg.trueTrack))
        {
            return false;
        }
    }

    // Field 3: Magnetic track (degrees)
    if( !fields[3].empty() )
    {
        if (!SafeParse::ToFloat(fields[3], vtg_msg.magneticTrack))
        {
            return false;
        }
    }

    // Field 5: Speed over ground (knots)
    if( !fields[5].empty() )
    {
        if (!SafeParse::ToFloat(fields[5], vtg_msg.speedKnots))
        {
            return false;
        }
    }

    // Field 7: Speed over ground (km/h) - may contain checksum
    if( !fields[7].empty() )
    {
        std::string speedkmh_str = StripNMEAChecksum(fields[7]);
        if (!SafeParse::ToFloat(speedkmh_str, vtg_msg.speedKmh))
        {
            return false;
        }
    }

    std::memcpy( &parsed_data.payload_received.vtg_data, &vtg_msg, sizeof(vtg_msg) );
    return true;
}

bool NMEAParser::ValidateCheckSum( const std::string &nmea_msg )
{
    size_t start = nmea_msg.find('$');
    size_t end = nmea_msg.find('*');

    if( start == std::string::npos || end == std::string::npos )
    {	
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
        return false;
    }

    // Convert received checksum from hex string to uint8_t using SafeParse
    uint8_t received_checksum;
    if (!SafeParse::ToUint8tHex(received_checksum_str, received_checksum))
    {
        return false;
    }

    return calculated_checksum == received_checksum;
}

}
}
}

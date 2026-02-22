#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H

#include <string>
#include <array>
#include <functional>
#include <cstdint>

namespace HAL
{
namespace Devices
{
namespace Position
{

class NMEAParser
{
public:
    static const uint8_t kMaxMessageIdSize = 10;
    static const uint8_t kMaxCheckSumSize  = 5;
    
    enum PositionFixIndicator
    {
        NOT_AVAILABLE,
        GPS_SPS,
        DIFF_GPS_SPS,
        NOT_SUPPORTED,
        DEAD_RECKONING
    };

    enum NMEAMessageType
    {
        GGA,
        GLL,
        GSA,
        GSV,
        MSS,
        RMC,
        VTG,
        MAXMessagesTypes
    };

    typedef struct
    {
        std::array<char, kMaxMessageIdSize> messageID;
        uint8_t hour;
        uint8_t minutes;
        uint8_t seconds;
        float latitude;
        char  NSIndicator;
        float longitude;
        char EWIndicator;
        PositionFixIndicator positionFixIndicator;
        float hdop;
        float mslAltitude;
        float geoidSeparation;
        std::array<char, kMaxCheckSumSize> checkSum;
    } NMEA_GGA;

    typedef struct
    {
        std::array<char, kMaxMessageIdSize> messageID;
        float latitude;
        char NSIndicator;
        float longitude;
        char EWIndicator;
        uint8_t hour;
        uint8_t minutes;
        uint8_t seconds; 
        std::array<char, kMaxCheckSumSize> checkSum;
    } NMEA_GLL;

    typedef struct 
    {
        std::array<char, kMaxMessageIdSize> messageID;
        char modeSelection;
        uint8_t mode;
        float pdop;
        float hdop;
        float vdop;
        std::array<char, kMaxCheckSumSize> checkSum;
    } NMEA_GSA;

    typedef struct
    {
        std::array<char, kMaxMessageIdSize> messageID;
        uint8_t numberOfMessages;
        uint8_t messageNumber;
        uint8_t satellitesInView;
        uint8_t satelliteId;
        uint8_t elevation;
        uint16_t azimuth;
        uint8_t snr;
        std::array<char, kMaxCheckSumSize> checkSum;
    } NMEA_GSV;

    typedef struct
    {
        std::array<char, kMaxMessageIdSize> messageID;
        uint8_t signalStrength;
        uint8_t snr;
        float beaconFrequency;
        uint8_t beaconBitRate;
        uint8_t channelNumber;
        std::array<char, kMaxCheckSumSize> checkSum;
    } NMEA_MSS;

    typedef struct 
    {
        std::array<char, kMaxMessageIdSize> messageID;
        uint8_t hour;
        uint8_t minutes;
        uint8_t seconds; 
        bool Status;
        float latitude;
        char NSIndicator;
        float longitude;
        char EWIndicator;
        float speedOverGround;
        float courseOverGround;
        uint8_t day;
        uint8_t month;
        uint8_t year; 
        float magneticVariation;
        char mode;
        std::array<char, kMaxCheckSumSize> checkSum;
    } NMEA_RMC;

    typedef struct
    {
        std::array<char, kMaxMessageIdSize> messageID;
        float trueTrack;
        float magneticTrack;
        float speedKnots;
        float speedKmh;
        std::array<char, kMaxCheckSumSize> checkSum;
    } NMEA_VTG;

    typedef union
    {
        NMEA_GGA gga_data;
        NMEA_GLL gll_data;
        NMEA_GSA gsa_data;
        NMEA_GSV gsv_data;
        NMEA_MSS mss_data;
        NMEA_RMC rmc_data;
        NMEA_VTG vtg_data;
    } NMEA_Payload;

    typedef struct
    {
        NMEAMessageType nmea_type;
        NMEA_Payload payload_received;
    } NMEA_Data;

    NMEAParser();
    ~NMEAParser();

    bool ProcessNMEAMessage( const std::string &nmea_message, NMEA_Data &data_received );

protected:
    using NMEAParserFunc = std::function<bool(const std::string &, NMEA_Data &)>;

    NMEAParserFunc NMEACallbacks[NMEAParser::MAXMessagesTypes];

    void RegisterCallback( NMEAParser::NMEAMessageType message_type, NMEAParserFunc nmea_func );

    virtual bool GGACallback(const std::string &nmea_messages, NMEA_Data &parsed_data);
    virtual bool GLLCallback(const std::string &nmea_messages, NMEA_Data &parsed_data);
    virtual bool GSACallback(const std::string &nmea_messages, NMEA_Data &parsed_data);
    virtual bool GSVCallback(const std::string &nmea_messages, NMEA_Data &parsed_data);
    virtual bool MSSCallback(const std::string &nmea_messages, NMEA_Data &parsed_data);
    virtual bool RMCCallback(const std::string &nmea_messages, NMEA_Data &parsed_data);
    virtual bool VTGCallback(const std::string &nmea_messages, NMEA_Data &parsed_data);
    bool ValidateCheckSum( const std::string &nmea_msg );

    NMEAParser::NMEAMessageType GetNMEAMessageType( const std::string &nmea_message );
    NMEAParser::NMEAMessageType ToMessagetypeFromStr( const std::string str_message_type );
    std::string StripNMEAChecksum(const std::string &field);

};

}
}
}

#endif // NMEA_PARSER_H

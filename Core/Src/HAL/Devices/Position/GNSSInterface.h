#ifndef GNSS_INTERFACE_H
#define GNSS_INTERFACE_H

#include <memory>
#include <cstdint>
#include <array>
#include <functional>

#include "RTOSWrappers/TaskWrapper.h"
#include "DebugController/DebugInterface.h"

namespace HAL
{
namespace Devices
{
namespace Communication
{
namespace Interfaces
{
class UartCommunicationInterface;
}
}
}
}

namespace HAL
{
namespace Devices
{
namespace Position
{

class GNSSInterface : public RtosWrappers::TaskWrapper,
                      public DebugController::DebugInterface
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
        float utcTime;
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
        float utcTime;
        bool Status;
        char Mode;
        std::array<char, kMaxCheckSumSize> checkSum;
    } NMEA_GLL;

    typedef struct 
    {
        std::array<char, kMaxMessageIdSize> messageID;
        char mode1;
        uint8_t mode2;
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
        uint8_t sinalStrength;
        uint8_t snr;
        float beaconFrequency;
        uint8_t beaconBitRate;
        uint8_t channelNumber;
        std::array<char, kMaxCheckSumSize> checkSum;
    } NMEA_MSS;

    typedef struct 
    {
        std::array<char, kMaxMessageIdSize> messageID;
        float utcTime;
        bool Status;
        float latitude;
        char NSIndicator;
        float longitude;
        char EWIndicator;
        float speedOverGround;
        float courseOverGround;
        int date;
        float magneticVariation;
        char mode;
        std::array<char, kMaxCheckSumSize> checkSum;
    } NMEA_RMC;

    typedef struct
    {
        std::array<char, kMaxMessageIdSize> messageID;
        float courseTrue;
        float courseMagnetic;
        float speedKnots;
        float speedKmh;
        std::array<char, kMaxCheckSumSize> checkSum;
    } NMEA_VTG;

    GNSSInterface( std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> gnss_uart );
    ~GNSSInterface();

protected:

    using NMEAParserFunc = std::function<bool(const std::string &)>;

    std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> gnss_uart_;
    NMEAParserFunc NMEACallbacks[MAXMessagesTypes];

    void Task(void *params) override;
    bool ProcessNMEAMessage( const std::string &nmea_message );
    NMEAMessageType GetNMEAMessageType( const std::string &nmea_message );
    NMEAMessageType ToMessagetypeFromStr( const std::string str_message_type );
    void RegisterCallback( NMEAMessageType message_type, NMEAParserFunc nmea_func );

    virtual bool GGACallback(const std::string &nmea_messages);
    virtual bool GLLCallback(const std::string &nmea_messages);
    virtual bool GSACallback(const std::string &nmea_messages);
    virtual bool GSVCallback(const std::string &nmea_messages);
    virtual bool MSSCallback(const std::string &nmea_messages);
    virtual bool RMCCallback(const std::string &nmea_messages);
    virtual bool VTGCallback(const std::string &nmea_messages);

private:
    static const int kRxBufferSize = 256;

    void UartCallBack( const uint8_t *data, uint16_t data_size );
    bool CanProcessMessage();

    uint16_t rx_buffer_pos_;
    uint8_t uart_buffer_receive_[kRxBufferSize];
    bool is_callback_executing_;
};

}
}
}

#endif // GNSS_INTERFACE
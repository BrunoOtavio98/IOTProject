#ifndef GNSS_INTERFACE_H
#define GNSS_INTERFACE_H

#include <cstdint>
#include <atomic>
#include <memory>

#include "RTOSWrappers/TaskWrapper.h"
#include "DebugController/DebugInterface.h"
#include "NMEAParser.h"

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
namespace DebugController 
{
class DebugController;
}
}

namespace HAL
{
namespace Devices
{
namespace Position
{

class NMEAParser;

class GNSSInterface : public RtosWrappers::TaskWrapper,
                      public DebugController::DebugInterface
{
public:

    typedef struct
    {
        float lat, lon, alt;
        float pdop, hdop, vdop;
        float speedKmh, courseDeg;
        uint8_t hour, minutes, seconds;
        uint8_t day, month, year;
    } BasicGNSSData;

    GNSSInterface( std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> gnss_uart,
                   const std::shared_ptr<HAL::DebugController::DebugController> &debug_controler );
    ~GNSSInterface();

    void GetUpdatedGnssData( BasicGNSSData &gnss_data );

protected:
    std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> gnss_uart_;
    std::shared_ptr<HAL::DebugController::DebugController> debug_controller_;

    void Task(void *params) override;

    float LatToDeg( float raw, char ns);
    float LonToDeg( float raw, char ew );
    void UpdateGNSSData( NMEAParser::NMEA_Data &nmea_data );
private:
    static const int kRxBufferSize = 256;

    void UartCallBack( const uint8_t *data, uint16_t data_size );
    bool CanProcessMessage();

    uint16_t rx_buffer_pos_;
    uint8_t uart_buffer_receive_[kRxBufferSize];
    std::unique_ptr<NMEAParser> nmea_parser_;
    
    BasicGNSSData buffer_a_;
    BasicGNSSData buffer_b_;
    std::atomic<BasicGNSSData*> active_buffer_;
};

}
}
}

#endif // GNSS_INTERFACE

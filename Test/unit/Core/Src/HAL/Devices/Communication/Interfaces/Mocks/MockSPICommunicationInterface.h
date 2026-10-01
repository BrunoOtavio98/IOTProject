#ifndef MOCK_SPI_COMM_INTERFACE_H
#define MOCK_SPI_COMM_INTERFACE_H

#include <gmock/gmock.h>
#include <functional>
#include "Devices/Communication/Interfaces/SPIInterface.h"

namespace HAL
{
namespace Devices
{
namespace Communication
{

namespace Interfaces
{

class MockSPICommunicationInterface : public SPIInterface
{
public:
    MockSPICommunicationInterface() : SPIInterface(SPIConfiguration{}){};
    ~MockSPICommunicationInterface() = default;

    MOCK_METHOD2(WriteData, bool(const uint8_t *data, uint16_t data_size));
    MOCK_METHOD2(ReadData,  bool(uint8_t *data, uint16_t data_size));
    MOCK_METHOD3(WriteDataIT, bool(const uint8_t *data, uint16_t data_size, std::function<void(void)> callback_write_finish));
    MOCK_METHOD3(ReadDataIT,  bool(uint8_t *read_buffer, uint16_t data_size, std::function<void(void)> callback_read_finish));
    MOCK_METHOD3(WriteReadData, bool(const uint8_t *data_write, uint8_t *data_read, uint16_t data_size));
    MOCK_METHOD1(SetCSPin,  bool(uint8_t pin_value));
};

}
}
}
}

#endif // MOCK_SPI_COMM_INTERFACE_H

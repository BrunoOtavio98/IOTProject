#ifndef UART_COMMUNICATION_INTEFACE_H_
#define UART_COMMUNICATION_INTEFACE_H_

#include <gmock/gmock.h>
#include "Devices/Communication/Interfaces/UartCommunicationInterface.h"

namespace HAL {
namespace Devices {
namespace Communication {
namespace Interfaces {

class MockUartCommunicationInterface : public UartCommunicationInterface
{
public:
    MockUartCommunicationInterface() : UartCommunicationInterface(BaudRates::BAUD_115200, UartNumber::UART_1) {

    };

    MOCK_METHOD1(WriteData, bool(const std::string&));
    MOCK_METHOD1(ReadDataIT, bool(std::function<void(const uint8_t *, uint16_t)>));
};

}
}
}
}

#endif


#ifndef MOCK_DEBUG_CONTROLLER_H_
#define MOCK_DEBUG_CONTROLLER_H_

#include "DebugController/DebugController.h"

#include <memory>
#include <gmock/gmock.h>

#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockUartCommunicationInterface.h"

using HAL::Devices::Communication::Interfaces::MockUartCommunicationInterface;

namespace HAL {
namespace DebugController {

class MockDebugController : public DebugController
{
public:
    MockDebugController(std::shared_ptr<MockUartCommunicationInterface> uart_debug) :
                        DebugController(uart_debug) {

    }

    MOCK_METHOD2(PrintDebug, void(DebugInterface*, const std::string&));
    MOCK_METHOD2(PrintInfo, void(DebugInterface*, const std::string&));
    MOCK_METHOD2(PrintWarn, void(DebugInterface*, const std::string&));
    MOCK_METHOD2(PrintError, void(DebugInterface*, const std::string&));

    MOCK_METHOD1(RegisterModuleToDebug, void(DebugInterface*));
    MOCK_METHOD1(RegisterCallBackToReadMessages, void(std::function<void(const std::string&)>));
};

}
}

#endif

#include "Devices/IOT/Interfaces/ModemInterface.h"

#include <gtest/gtest.h>

#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockUartCommunicationInterface.h"
#include "Core/Src/HAL/DebugController/Mocks/MockDebugController.h"

using HAL::Devices::Communication::Interfaces::MockUartCommunicationInterface;
using HAL::DebugController::MockDebugController;

namespace HAL {
namespace Devices {
namespace IOT {
namespace Interfaces {

class ModemInterfaceHelper : public ModemInterface {
 public: 
    ModemInterfaceHelper(std::shared_ptr<MockUartCommunicationInterface> uart_modem_,
    std::shared_ptr<MockDebugController> debug_controller) : ModemInterface(uart_modem_, debug_controller) {

    }
};

class ModemInterfaceTests : testing::Test
{
public:
    ModemInterfaceTests() : 
    uart_debug_(std::make_shared<MockUartCommunicationInterface>()),
    uart_modem_(std::make_shared<MockUartCommunicationInterface>()),
    debug_controller_(std::make_shared<MockDebugController>(uart_debug_)),
    modem_(uart_modem_, debug_controller_){

    }

std::shared_ptr<MockUartCommunicationInterface> uart_debug_;
std::shared_ptr<MockUartCommunicationInterface> uart_modem_;
std::shared_ptr<MockDebugController> debug_controller_;
ModemInterfaceHelper modem_;
};

}
}
}
}
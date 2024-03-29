
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

    MOCK_METHOD1(MyMockableCallback, void(const std::string &command));
};

class ModemInterfaceTests : public testing::Test
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

TEST_F(ModemInterfaceTests, TestCommandFormat) {
   ModemInterface::ATCommands current_cmd = ModemInterface::ATCommands::COPS;
   ModemInterface::AtCommandTypes current_cmd_type = ModemInterface::AtCommandTypes::Test;

   std::list<std::string> list_of_parameters;

   std::string expect_msg_format = "AT+COPS=?";
   EXPECT_CALL(*uart_modem_, WriteData(expect_msg_format));
    
   modem_.SendCommand(current_cmd_type, current_cmd, list_of_parameters);
}

TEST_F(ModemInterfaceTests, ReadCommandFormat) {
   ModemInterface::ATCommands current_cmd = ModemInterface::ATCommands::ATeC;
   ModemInterface::AtCommandTypes current_cmd_type = ModemInterface::AtCommandTypes::Read;

   std::list<std::string> list_of_parameters;

   std::string expect_msg_format = "AT&C?";
   EXPECT_CALL(*uart_modem_, WriteData(expect_msg_format));
    
   modem_.SendCommand(current_cmd_type, current_cmd, list_of_parameters);
}

TEST_F(ModemInterfaceTests, WriteCommandFormat) {
   ModemInterface::ATCommands current_cmd = ModemInterface::ATCommands::ATI;
   ModemInterface::AtCommandTypes current_cmd_type = ModemInterface::AtCommandTypes::Write;

   std::list<std::string> list_of_parameters = {"1", "B", "C", "2"};

   std::string expect_msg_format = "ATI=1,B,C,2";
   EXPECT_CALL(*uart_modem_, WriteData(expect_msg_format));
    
   modem_.SendCommand(current_cmd_type, current_cmd, list_of_parameters);
}

TEST_F(ModemInterfaceTests, ExecuteCommandFormat) {
   ModemInterface::ATCommands current_cmd = ModemInterface::ATCommands::ATI;
   ModemInterface::AtCommandTypes current_cmd_type = ModemInterface::AtCommandTypes::Execute;

   std::list<std::string> list_of_parameters = {"1", "B", "C", "2"};

   std::string expect_msg_format = "ATI1,B,C,2";
   EXPECT_CALL(*uart_modem_, WriteData(expect_msg_format));
    
   modem_.SendCommand(current_cmd_type, current_cmd, list_of_parameters);
}

TEST_F(ModemInterfaceTests, TestCallingCallback) {
   ModemInterface::ATCommands current_cmd = ModemInterface::ATCommands::ATI;
   ModemInterface::AtCommandTypes current_cmd_type = ModemInterface::AtCommandTypes::Read;
   ModemInterface::ATCommandConfiguration cmd_config;
   cmd_config.timeout = 100;
   cmd_config.receive_callback = [&](const std::string &command) {
      modem_.MyMockableCallback(command);
   };
   std::list<std::string> list_of_parameters;

   modem_.RegisterCommand(current_cmd, cmd_config);
   modem_.SendCommand(current_cmd_type, current_cmd, list_of_parameters);

   std::string expect_msg_format = "OK";
   EXPECT_CALL(modem_, MyMockableCallback(expect_msg_format));

   modem_.ReceiveCommandCallBack(expect_msg_format);
}

}
}
}
}
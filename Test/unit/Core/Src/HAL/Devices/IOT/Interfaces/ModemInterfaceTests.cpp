
#include "Devices/IOT/Interfaces/ModemInterface.h"

#include <gtest/gtest.h>

#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockUartCommunicationInterface.h"
#include "Core/Src/HAL/DebugController/Mocks/MockDebugController.h"
#include "Core/Src/HAL/RTOSWrappers/Mocks/MockQueueManager.h"
#include "RTOSWrappers/QueueWrapper.h"

using HAL::Devices::Communication::Interfaces::MockUartCommunicationInterface;
using HAL::DebugController::MockDebugController;
using HAL::RtosWrappers::MockQueueManager;

using testing::_;

namespace HAL {
namespace Devices {
namespace IOT {
namespace Interfaces {

class ModemInterfaceHelper : public ModemInterface {
 public: 
    ModemInterfaceHelper(std::shared_ptr<MockUartCommunicationInterface> uart_modem_,
                         std::shared_ptr<MockDebugController> debug_controller,
                         std::shared_ptr<MockQueueManager> queue_manager) :
                        ModemInterface(uart_modem_, debug_controller, queue_manager) {

    }

   using ModemInterface::kTaskDelayMs;
   using ModemInterface::kRxBufferSize;
   using ModemInterface::Task;
   using ModemInterface::modem_commands_;
   using ModemInterface::CurrentCmd;
   using ModemInterface::CanProcessUartMessage;

   MOCK_METHOD3(MyMockableCallback, void(const std::string &command, const ATCommands &cmd, const AtCommandTypes &type));

   void SetTaskShouldRun(bool should_run) {task_should_run_ = should_run;}
   void OnLoop() override {}
   uint16_t GetRxBufferPos() const { return rx_buffer_pos_; }
   void SetIsrExecuting(bool value) { is_isr_executing_ = value; }
   bool GetIsrExecuting() const { return is_isr_executing_; }

   
};

class ModemInterfaceTests : public testing::Test
{
   public:
   ModemInterfaceTests() : 
   uart_debug_(std::make_shared<MockUartCommunicationInterface>()),
   uart_modem_(std::make_shared<MockUartCommunicationInterface>()),
   debug_controller_(std::make_shared<MockDebugController>(uart_debug_)),
   queue_manager_(std::make_shared<MockQueueManager>()),
   modem_(uart_modem_, debug_controller_, queue_manager_){
      
   }
   
   std::shared_ptr<MockUartCommunicationInterface> uart_debug_;
   std::shared_ptr<MockUartCommunicationInterface> uart_modem_;
   std::shared_ptr<MockDebugController> debug_controller_;
   std::shared_ptr<MockQueueManager> queue_manager_;
   ModemInterfaceHelper modem_;
};

TEST_F(ModemInterfaceTests, TestRegisterCommand) {
   ModemInterface::ATCommands cops_cmd = ModemInterface::ATCommands::COPS;
   ModemInterface::ATCommandConfiguration cmd_config;
   cmd_config.timeout = 100;
   cmd_config.receive_callback = [](const std::string &command, const ModemInterface::ATCommands &cmd, const ModemInterface::AtCommandTypes &type) {
      // Do nothing
      return true;
   };

   modem_.RegisterCommand(cops_cmd, cmd_config);

   ModemInterface::ATCommands creg_cmd = ModemInterface::ATCommands::CREG;
   ModemInterface::ATCommandConfiguration cmd_config2;
   cmd_config2.timeout = 200;
   cmd_config2.receive_callback = [](const std::string &command, const ModemInterface::ATCommands &cmd, const ModemInterface::AtCommandTypes &type) {
      // Do nothing
      return true;
   };
   modem_.RegisterCommand(creg_cmd, cmd_config2);

   EXPECT_EQ(modem_.modem_commands_.size(), 2);
}

TEST_F(ModemInterfaceTests, TestCommandFormat) {
   ModemInterface::ATCommands current_cmd = ModemInterface::ATCommands::COPS;
   ModemInterface::AtCommandTypes current_cmd_type = ModemInterface::AtCommandTypes::Test;

   std::list<std::string> list_of_parameters;
   std::string expect_msg_format = "AT+COPS=?";

   ModemInterfaceHelper::CurrentCmd expected_cmd = {};  // Zero initialize
   expected_cmd.current_cmd = current_cmd;
   expected_cmd.current_cmd_type = current_cmd_type;
   strncpy(expected_cmd.raw_msg, expect_msg_format.c_str(), sizeof(expected_cmd.raw_msg) - 1);

   EXPECT_CALL(*uart_modem_, WriteData(expect_msg_format));
   EXPECT_CALL(*queue_manager_, QueueSend(_, _, _));
   EXPECT_CALL(*queue_manager_, QueueReceive(_, _ ,_)).WillOnce( [&](GenericQueueHandle queue, void* item, int timeout )
   {
      *((ModemInterfaceHelper::CurrentCmd *)item) = expected_cmd;
      return true;
   });

   modem_.SendCommand(current_cmd_type, current_cmd, list_of_parameters);
   modem_.SetTaskShouldRun(false);
   modem_.Task(nullptr);
}

TEST_F(ModemInterfaceTests, ReadCommandFormat) {
   ModemInterface::ATCommands current_cmd = ModemInterface::ATCommands::ATeC;
   ModemInterface::AtCommandTypes current_cmd_type = ModemInterface::AtCommandTypes::Read;

   std::list<std::string> list_of_parameters;

   std::string expect_msg_format = "AT&C?";
    
   ModemInterfaceHelper::CurrentCmd expected_cmd = {};  // Zero initialize
   expected_cmd.current_cmd = current_cmd;
   expected_cmd.current_cmd_type = current_cmd_type;
   strncpy(expected_cmd.raw_msg, expect_msg_format.c_str(), sizeof(expected_cmd.raw_msg) - 1);

   EXPECT_CALL(*uart_modem_, WriteData(expect_msg_format));
   EXPECT_CALL(*queue_manager_, QueueSend(_, _, _));
   EXPECT_CALL(*queue_manager_, QueueReceive(_, _ ,_)).WillOnce( [&](GenericQueueHandle queue, void* item, int timeout )
   {
      *((ModemInterfaceHelper::CurrentCmd *)item) = expected_cmd;
      return true;
   });

   modem_.SendCommand(current_cmd_type, current_cmd, list_of_parameters);
   modem_.SetTaskShouldRun(false);
   modem_.Task(nullptr);
}

TEST_F(ModemInterfaceTests, WriteCommandFormat) {
   ModemInterface::ATCommands current_cmd = ModemInterface::ATCommands::ATI;
   ModemInterface::AtCommandTypes current_cmd_type = ModemInterface::AtCommandTypes::Write;

   std::list<std::string> list_of_parameters = {"1", "B", "C", "2"};

   std::string expect_msg_format = "ATI=1,B,C,2";
   ModemInterfaceHelper::CurrentCmd expected_cmd = {};  // Zero initialize
   expected_cmd.current_cmd = current_cmd;
   expected_cmd.current_cmd_type = current_cmd_type;
   strncpy(expected_cmd.raw_msg, expect_msg_format.c_str(), sizeof(expected_cmd.raw_msg) - 1);

   EXPECT_CALL(*uart_modem_, WriteData(expect_msg_format));
   EXPECT_CALL(*queue_manager_, QueueSend(_, _, _));
   EXPECT_CALL(*queue_manager_, QueueReceive(_, _ ,_)).WillOnce( [&](GenericQueueHandle queue, void* item, int timeout )
   {
      *((ModemInterfaceHelper::CurrentCmd *)item) = expected_cmd;
      return true;
   });

   modem_.SendCommand(current_cmd_type, current_cmd, list_of_parameters);
   modem_.SetTaskShouldRun(false);
   modem_.Task(nullptr);
}

TEST_F(ModemInterfaceTests, ExecuteCommandFormat) {
   ModemInterface::ATCommands current_cmd = ModemInterface::ATCommands::ATI;
   ModemInterface::AtCommandTypes current_cmd_type = ModemInterface::AtCommandTypes::Execute;

   std::list<std::string> list_of_parameters = {"1", "B", "C", "2"};

   std::string expect_msg_format = "ATI1,B,C,2";
   ModemInterfaceHelper::CurrentCmd expected_cmd = {};  // Zero initialize
   expected_cmd.current_cmd = current_cmd;
   expected_cmd.current_cmd_type = current_cmd_type;
   strncpy(expected_cmd.raw_msg, expect_msg_format.c_str(), sizeof(expected_cmd.raw_msg) - 1);

   EXPECT_CALL(*uart_modem_, WriteData(expect_msg_format));
   EXPECT_CALL(*queue_manager_, QueueSend(_, _, _));
   EXPECT_CALL(*queue_manager_, QueueReceive(_, _ ,_)).WillOnce( [&](GenericQueueHandle queue, void* item, int timeout )
   {
      *((ModemInterfaceHelper::CurrentCmd *)item) = expected_cmd;
      return true;
   });

   modem_.SendCommand(current_cmd_type, current_cmd, list_of_parameters);
   modem_.SetTaskShouldRun(false);
   modem_.Task(nullptr);
}

TEST_F(ModemInterfaceTests, TestCallingCallback) {
   ModemInterface::ATCommands current_cmd = ModemInterface::ATCommands::ATI;
   ModemInterface::AtCommandTypes current_cmd_type = ModemInterface::AtCommandTypes::Read;
   ModemInterface::ATCommandConfiguration cmd_config;
   cmd_config.timeout = 100;
   cmd_config.receive_callback = [&](const std::string &command, const ModemInterface::ATCommands &cmd, const ModemInterface::AtCommandTypes &type) {
      modem_.MyMockableCallback(command, cmd, type);
      return true;
   };
   std::list<std::string> list_of_parameters = {};
   std::string expect_msg_format = "OK\r\n";
   ModemInterfaceHelper::CurrentCmd expected_cmd = {};  // Zero initialize
   expected_cmd.current_cmd = current_cmd;
   expected_cmd.current_cmd_type = current_cmd_type;
   strncpy(expected_cmd.raw_msg, expect_msg_format.c_str(), sizeof(expected_cmd.raw_msg) - 1);

   EXPECT_CALL(modem_, MyMockableCallback(expect_msg_format, current_cmd, current_cmd_type));
   EXPECT_CALL(*queue_manager_, QueueSend(_, _, _));
   EXPECT_CALL(*queue_manager_, QueueReceive(_, _ ,_)).WillOnce( [&](GenericQueueHandle queue, void* item, int timeout )
   {
      *((ModemInterfaceHelper::CurrentCmd *)item) = expected_cmd;
      return true;
   });
   EXPECT_CALL(*uart_modem_, WriteData(expect_msg_format));

   modem_.RegisterCommand(current_cmd, cmd_config);
   EXPECT_EQ(modem_.modem_commands_.size(), 1);

   modem_.SendCommand(current_cmd_type, current_cmd, list_of_parameters);

   modem_.ReceiveCommandCallBack((const uint8_t *)expect_msg_format.c_str(), expect_msg_format.size());
   modem_.SetTaskShouldRun(false);
   modem_.Task(nullptr);
}

TEST_F(ModemInterfaceTests, TestBufferOverflow) {
    // Create string larger than kRxBufferSize
    std::string large_data( modem_.kRxBufferSize + 10, 'A');
    
    modem_.ReceiveCommandCallBack(
        (const uint8_t*)large_data.c_str(), 
        large_data.size()
    );
    
    // Verify data was truncated
    EXPECT_EQ(modem_.GetRxBufferPos(), modem_.kRxBufferSize - 1);
}

TEST_F(ModemInterfaceTests, TestIsrProtection) {
    std::string msg1 = "AT+CSQ\r\n";
    std::string msg2 = "OK\r\n";
    
    // Simulate ISR started
    modem_.ReceiveCommandCallBack((const uint8_t*)msg1.c_str(), msg1.size());
    modem_.SetIsrExecuting(true);
    
    // Verify message not processed during ISR
    EXPECT_FALSE(modem_.CanProcessUartMessage());
    
    modem_.SetIsrExecuting(false);
    EXPECT_TRUE(modem_.CanProcessUartMessage());
}

TEST_F(ModemInterfaceTests, TestCommandTimeout) {
    ModemInterface::ATCommands cmd = ModemInterface::ATCommands::CSQ;
    ModemInterface::ATCommandConfiguration config;
    config.timeout = 100;
    config.receive_callback = [&](const std::string &resp, const ModemInterface::ATCommands &cmd,
                                const ModemInterface::AtCommandTypes &type) {
        modem_.MyMockableCallback(resp, cmd, type);
        return true;
    };
   
   modem_.RegisterCommand(cmd, config);

   ModemInterfaceHelper::CurrentCmd expected_cmd = {};  // Zero initialize
   expected_cmd.current_cmd = cmd;
   expected_cmd.current_cmd_type = ModemInterface::AtCommandTypes::Execute;
   strncpy(expected_cmd.raw_msg, "\r\nOK\r\n", sizeof(expected_cmd.raw_msg) - 1);
    
   EXPECT_CALL(*queue_manager_, QueueSend(_, _, _));
   EXPECT_CALL(*queue_manager_, QueueReceive(_, _ ,_)).WillOnce( [&](GenericQueueHandle queue, void* item, int timeout )
   {  
      *((ModemInterfaceHelper::CurrentCmd *)item) = expected_cmd;
      return true; // Simulate no response
   });
    
   // Send command but don't provide response
   std::list<std::string> params;
   modem_.SendCommand(ModemInterface::AtCommandTypes::Execute, cmd, params);
    
   // Expect timeout callback
   EXPECT_CALL(modem_, MyMockableCallback("\r\nTIMEOUT\r\n", cmd, 
                ModemInterface::AtCommandTypes::Execute));
    
   modem_.SetTaskShouldRun(false);
    // Run task enough times to trigger timeout
   for(int i = 0; i <= (config.timeout/modem_.kTaskDelayMs) + 1; i++) {
        modem_.Task(nullptr);
   }
}

}
}
}
}
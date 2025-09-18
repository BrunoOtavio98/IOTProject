#include "Devices/IOT/Modem/SIM800L.h"

#include <gtest/gtest.h>

#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockUartCommunicationInterface.h"
#include "Core/Src/HAL/DebugController/Mocks/MockDebugController.h"

using HAL::Devices::Communication::Interfaces::MockUartCommunicationInterface;
using HAL::DebugController::MockDebugController;
using HAL::Devices::IOT::Interfaces::ModemInterface;

using testing::_;

namespace HAL {
namespace Devices {
namespace IOT {
namespace Modem {

class SIM800LModemWrapper : public SIM800LModem 
{
  public:
    SIM800LModemWrapper(const std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> &uart_communication,
      const std::shared_ptr<HAL::DebugController::DebugController> &debug_controller) : SIM800LModem(uart_communication, debug_controller) 
      {}

    MOCK_METHOD3(SendCommand, bool(const AtCommandTypes &command_type, const ATCommands &command_to_execute, const std::list<std::string> &parameters));

    using SIM800LModem::kTimeToTestConnection;
    using SIM800LModem::kTaskDelayMs;

    using SIM800LModem::connection_completed_;
    using SIM800LModem::time_passed_keep_alive_;
    using SIM800LModem::modem_commands_;

    using SIM800LModem::KeepAliveControl;
    using SIM800LModem::SplitString;
};

class SIM800LTests : public testing::Test 
{
  public:
    SIM800LTests() : uart_modem_(std::make_shared<MockUartCommunicationInterface>()),
                    uart_debug_(std::make_shared<MockUartCommunicationInterface>()),
                    debug_controller_(std::make_shared<MockDebugController>(uart_debug_)),
                    modem_(uart_modem_, debug_controller_) 
    {
    }

   std::shared_ptr<MockUartCommunicationInterface> uart_modem_;
   std::shared_ptr<MockUartCommunicationInterface> uart_debug_;
   std::shared_ptr<MockDebugController> debug_controller_;
   SIM800LModemWrapper modem_;
};

TEST_F(SIM800LTests, TestSplitString) 
{
    std::string test_string = "+CREG: 1,2,\"1A2B\",\"1F2E\",7";
    std::vector<std::string> result = modem_.SplitString(test_string, ',');

    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], "+CREG: 1");
    EXPECT_EQ(result[1], "2");
    EXPECT_EQ(result[2], "\"1A2B\"");
    EXPECT_EQ(result[3], "\"1F2E\"");
    EXPECT_EQ(result[4], "7");
}

TEST_F(SIM800LTests, TestSplitStringNoDelimiter) 
{
    std::string test_string = "AT+CREG?";
    std::vector<std::string> result = modem_.SplitString(test_string, ',');

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "AT+CREG?");

}

TEST_F(SIM800LTests, TestSplitStringEmptyString) 
{
    std::string test_string = "";
    std::vector<std::string> result = modem_.SplitString(test_string, ',');

    ASSERT_EQ(result.size(), 0);
}

TEST_F(SIM800LTests, TestSplitStringWrongDelimiter) 
{
    std::string test_string = "AT+CREG,0";
    std::vector<std::string> result = modem_.SplitString(test_string, ';');

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "AT+CREG,0");

}

TEST_F(SIM800LTests, SIM800LTests_TestKeepAliveControl)
{
  modem_.connection_completed_ = true;
  uint32_t numberIterationTestConnection = (modem_.kTimeToTestConnection / modem_.kTaskDelayMs);

  EXPECT_CALL(modem_, SendCommand(_, _, _)).Times(1);

  for(int i = 0; i < numberIterationTestConnection; i++)
  {
    modem_.KeepAliveControl();
  }

  EXPECT_EQ(modem_.time_passed_keep_alive_, 0);
}

TEST_F(SIM800LTests, SIM800LTests_CommandsRegistered)
{
  EXPECT_EQ(modem_.modem_commands_.size(), 9);

  EXPECT_NE(modem_.modem_commands_.find(ModemInterface::ATE), modem_.modem_commands_.end());
  EXPECT_NE(modem_.modem_commands_.find(ModemInterface::CREG), modem_.modem_commands_.end());
  EXPECT_NE(modem_.modem_commands_.find(ModemInterface::CSQ), modem_.modem_commands_.end());
  EXPECT_NE(modem_.modem_commands_.find(ModemInterface::COPS), modem_.modem_commands_.end());
  EXPECT_NE(modem_.modem_commands_.find(ModemInterface::CGATT), modem_.modem_commands_.end());
  EXPECT_NE(modem_.modem_commands_.find(ModemInterface::CSTT), modem_.modem_commands_.end());
  EXPECT_NE(modem_.modem_commands_.find(ModemInterface::CIICR), modem_.modem_commands_.end());
  EXPECT_NE(modem_.modem_commands_.find(ModemInterface::CIFSR), modem_.modem_commands_.end());
  EXPECT_NE(modem_.modem_commands_.find(ModemInterface::CFUN), modem_.modem_commands_.end());

}

}
}
}
}
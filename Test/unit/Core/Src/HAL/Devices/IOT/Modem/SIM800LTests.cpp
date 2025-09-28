#include "Devices/IOT/Modem/SIM800L.h"

#include <gtest/gtest.h>

#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockUartCommunicationInterface.h"
#include "Core/Src/HAL/DebugController/Mocks/MockDebugController.h"

using HAL::Devices::Communication::Interfaces::MockUartCommunicationInterface;
using HAL::DebugController::MockDebugController;
using HAL::Devices::IOT::Interfaces::ModemInterface;

using testing::_;
using testing::Return;

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
    using SIM800LModem::current_cmd_state_;
    using SIM800LModem::next_cmd_to_execute_;
    using SIM800LModem::current_apn_;
    using SIM800LModem::current_username_;
    using SIM800LModem::current_password_;

    using SIM800LModem::modem_register_state;
    using SIM800LModem::modem_cmd_state;

    using SIM800LModem::KeepAliveControl;
    using SIM800LModem::SplitString;
    using SIM800LModem::ConnectStateMachine;
    using SIM800LModem::GenericCmdResponse;
    using SIM800LModem::CREGResponse;
    using SIM800LModem::CSQResponse;
    using SIM800LModem::COPSResponse;
    using SIM800LModem::CGATTResponse;
    using SIM800LModem::CSTTResponse;
    using SIM800LModem::CIICRResponse;
    using SIM800LModem::CIFSRResponse;
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

TEST_F(SIM800LTests, SIM800LTests_ConnectStateMachineInvalidState)
{
  modem_.connection_completed_ = false;
  modem_.current_cmd_state_ = modem_.modem_cmd_state::kError;
  modem_.next_cmd_to_execute_ = SIM800LModem::ATCommands::CREG;

  modem_.ConnectStateMachine("apn", "user", "pass");

  EXPECT_EQ(modem_.next_cmd_to_execute_, SIM800LModem::ATCommands::ATE);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kIdle);
}

// TEST_F(SIM800LTests, FullSequence_AdvancesThroughCommands_ToCompletion) 
// {
//     // Expected linear sequence based on implementation cases
//     const std::vector<SIM800LModemWrapper::ATCommands> seq = {
//         SIM800LModemWrapper::ATCommands::ATE,
//         SIM800LModemWrapper::ATCommands::CFUN,
//         SIM800LModemWrapper::ATCommands::CSQ,
//         SIM800LModemWrapper::ATCommands::COPS,
//         SIM800LModemWrapper::ATCommands::CREG,
//         SIM800LModemWrapper::ATCommands::CGATT,
//         SIM800LModemWrapper::ATCommands::CSTT,
//         SIM800LModemWrapper::ATCommands::CIICR,
//         SIM800LModemWrapper::ATCommands::CIFSR
//     };

//     // Allow SendCommand for any command during the Idle -> Waiting transition
//     EXPECT_CALL(modem_, SendCommand(_, _, _)).Times(testing::AtLeast(1)).WillRepeatedly(Return(true));

//     modem_.next_cmd_to_execute_ = SIM800LModemWrapper::ATCommands::ATE;

//     // Step through each command: first call should send, set waiting; then simulate success to advance
//     for (size_t i = 0; i < seq.size(); ++i) 
//     {
//         // Trigger sending
//         modem_.ConnectStateMachine("apn", "user", "pass");
//         EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kWaitingForResponse);

//         // Simulate response processed
//         modem_.current_cmd_state_ = modem_.modem_cmd_state::kLastCommandExecuted;
//         modem_.ConnectStateMachine("apn", "user", "pass");

//         if(modem_.next_cmd_to_execute_ == SIM800LModemWrapper::ATCommands::CREG)
//         {

//         }
//         else
//         {
//           // If not the last element, next_cmd should advance to the next in sequence
//           if (i + 1 < seq.size()) 
//           {
//               EXPECT_EQ(modem_.next_cmd_to_execute_, seq[i + 1]);
//               EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kIdle);
//           }
//           else
//           {
//               // After finishing the last sequence element, expect state machine to mark completion
//               EXPECT_EQ(modem_.next_cmd_to_execute_, SIM800LModemWrapper::ATCommands::Invalid);
//               EXPECT_TRUE(modem_.connection_completed_);
//           }
//       }
//     }
// }

TEST_F(SIM800LTests, SIM800LTests_GenericCmdResponseValid)
{
  std::string response = "ATE1\n\rOK";
  bool result = modem_.GenericCmdResponse(response, ModemInterface::ATCommands::ATE, ModemInterface::AtCommandTypes::Execute);

  EXPECT_TRUE(result);
}

TEST_F(SIM800LTests, SIM800LTests_GenericCmdResponseInvalid)
{
  std::string response = "ERROR\n\r";
  bool result = modem_.GenericCmdResponse(response, ModemInterface::ATCommands::ATE, ModemInterface::AtCommandTypes::Execute);

  EXPECT_FALSE(result); 
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kIdle);
}

TEST_F(SIM800LTests, SIM800LTests_GenericCmdResponseNoOk)
{
  std::string response = "ATE1\n\r";
  bool result = modem_.GenericCmdResponse(response, ModemInterface::ATCommands::ATE, ModemInterface::AtCommandTypes::Execute);

  EXPECT_FALSE(result); 
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kIdle);
}

TEST_F(SIM800LTests, SIM800LTests_GenericCmdResponseNoEcho)
{
  std::string response = "OK\n\r";
  bool result = modem_.GenericCmdResponse(response, ModemInterface::ATCommands::ATE, ModemInterface::AtCommandTypes::Execute);

  EXPECT_FALSE(result); 
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kIdle);
}

TEST_F(SIM800LTests, SIM800LTests_GenericCmdResponseTimeout)
{
  std::string response = "TIMEOUT\n\r";
  bool result = modem_.GenericCmdResponse(response, ModemInterface::ATCommands::ATE, ModemInterface::AtCommandTypes::Execute);

  EXPECT_FALSE(result); 
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kIdle);
}

TEST_F(SIM800LTests, SIM800LTests_CREGResponseValidRegistered)
{
  std::string response = "AT+CREG: 1,1,\"1A2B\",\"1F2E\"\n\rOK";
  bool result = modem_.CREGResponse(response, ModemInterface::ATCommands::CREG, ModemInterface::AtCommandTypes::Read);

  EXPECT_TRUE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kLastCommandExecuted);
}

TEST_F(SIM800LTests, SIM800LTests_CREGResponseValidRoaming)
{
  std::string response = "AT+CREG: 1,5,\"1A2B\",\"1F2E\"\n\rOK";
  bool result = modem_.CREGResponse(response, ModemInterface::ATCommands::CREG, ModemInterface::AtCommandTypes::Read);

  EXPECT_TRUE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kLastCommandExecuted);
}

TEST_F(SIM800LTests, SIM800LTests_CREGResponseInValidNotRegistered)
{
  std::string response = "AT+CREG: 1,0,\"1A2B\",\"1F2E\"\n\rOK";
  bool result = modem_.CREGResponse(response, ModemInterface::ATCommands::CREG, ModemInterface::AtCommandTypes::Read);

  EXPECT_FALSE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kError);
}

TEST_F(SIM800LTests, SIM800LTests_CREGResponseInvalidFormat)
{
  std::string response = "AT+CREG: 1,\"1A2B\",\"1F2E\"\n\rOK";
  bool result = modem_.CREGResponse(response, ModemInterface::ATCommands::CREG, ModemInterface::AtCommandTypes::Read);

  EXPECT_FALSE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kError);
}

TEST_F(SIM800LTests, SIM800LTests_CSQResponseValid)
{
  std::string response = "AT+CSQ: 15,99\n\rOK";
  bool result = modem_.CSQResponse(response, ModemInterface::ATCommands::CSQ, ModemInterface::AtCommandTypes::Execute);

  EXPECT_TRUE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kLastCommandExecuted);
}

TEST_F(SIM800LTests, SIM800LTests_CSQResponseInvalidFormat)
{
  std::string response = "AT+CSQ: 15\n\rOK";
  bool result = modem_.CSQResponse(response, ModemInterface::ATCommands::CSQ, ModemInterface::AtCommandTypes::Execute);

  EXPECT_FALSE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kError);
}

TEST_F(SIM800LTests, SIM800LTests_CSQResponseInvalidValues)
{
  std::string response = "AT+CSQ: XX,YY\n\rOK";
  bool result = modem_.CSQResponse(response, ModemInterface::ATCommands::CSQ, ModemInterface::AtCommandTypes::Execute);

  EXPECT_FALSE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kError);

}

TEST_F(SIM800LTests, SIM800LTests_COPSResponseValid)
{
  std::string response = "AT+COPS: 0,0,\"Operator\"\n\rOK";
  bool result = modem_.COPSResponse(response, ModemInterface::ATCommands::COPS, ModemInterface::AtCommandTypes::Read);

  EXPECT_TRUE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kLastCommandExecuted); 
}

TEST_F(SIM800LTests, SIM800LTests_COPSResponseInvalidFormat)
{
  std::string response = "AT+COPS: D0,\"Operator\"\n\rOK";
  bool result = modem_.COPSResponse(response, ModemInterface::ATCommands::COPS, ModemInterface::AtCommandTypes::Read);

  EXPECT_FALSE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kError); 
}

TEST_F(SIM800LTests, SIM800LTests_CGATTAttached)
{
  std::string response = "AT+CGATT: 1\n\rOK";
  bool result = modem_.CGATTResponse(response, ModemInterface::ATCommands::CGATT, ModemInterface::AtCommandTypes::Read);

  EXPECT_TRUE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kLastCommandExecuted); 
}

TEST_F(SIM800LTests, SIM800LTests_CGATTNotAttached)
{
  std::string response = "AT+CGATT: 0\n\rOK";
  bool result = modem_.CGATTResponse(response, ModemInterface::ATCommands::CGATT, ModemInterface::AtCommandTypes::Read);

  EXPECT_FALSE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kError); 
}

TEST_F(SIM800LTests, SIM800LTests_CGATTInvalidFormat)
{
  std::string response = "AT+CGATT: X\n\rOK";
  bool result = modem_.CGATTResponse(response, ModemInterface::ATCommands::CGATT, ModemInterface::AtCommandTypes::Read);

  EXPECT_FALSE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kError);
}

TEST_F(SIM800LTests, SIM800LTests_CSTTResponseValid)
{
  std::string response = "AT+CSTT:apn_test,usr_name_test,pass_test\r\nOK\r\n";
  bool result = modem_.CSTTResponse(response, ModemInterface::ATCommands::CSTT, ModemInterface::AtCommandTypes::Read);

  EXPECT_EQ(modem_.current_apn_, "apn_test");
  EXPECT_EQ(modem_.current_username_, "usr_name_test");
  EXPECT_EQ(modem_.current_password_, "pass_test");

  EXPECT_TRUE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kLastCommandExecuted); 
}

TEST_F(SIM800LTests, SIM800LTests_CSTTResponseInvalidFormat)
{
  std::string response = "AT+CSTT:apn_test,usr_name_test\r\nOK\r\n";
  bool result = modem_.CSTTResponse(response, ModemInterface::ATCommands::CSTT, ModemInterface::AtCommandTypes::Read);

  EXPECT_FALSE(result);
  EXPECT_EQ(modem_.current_cmd_state_, modem_.modem_cmd_state::kError); 
}

}
}
}
}
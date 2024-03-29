#include <gtest/gtest.h>

#include "DebugController/DebugController.h"
#include "Mocks/MockDebugInterface.h"
#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockUartCommunicationInterface.h"

#include <iostream>

using HAL::Devices::Communication::Interfaces::MockUartCommunicationInterface;
using ::testing::Return;

namespace HAL {
namespace DebugController {

class DebugControllerHelper : public DebugController {
  public:
  DebugControllerHelper(std::shared_ptr<MockUartCommunicationInterface> uart_communication_)
                        : DebugController(uart_communication_) {

  }

  int GetNumberOfModules() {
    return list_of_modules_.size();
  }

  using DebugController::CheckIfModuleCanLog;
};

class DebugControllerTests : public testing::Test {
  public:
    DebugControllerTests() : uart_communication_(std::make_shared<MockUartCommunicationInterface>()),
                            debug_controller_(uart_communication_)  {

    }

   std::shared_ptr<MockUartCommunicationInterface> uart_communication_;
   DebugControllerHelper debug_controller_;
};

TEST_F(DebugControllerTests, TestRegisterModuleCount) {

  MockDebugInterface debug_interface1;
  MockDebugInterface debug_interface2;
  MockDebugInterface debug_interface3;

  debug_controller_.RegisterModuleToDebug(&debug_interface1);
  debug_controller_.RegisterModuleToDebug(&debug_interface2);
  debug_controller_.RegisterModuleToDebug(&debug_interface3);
  debug_controller_.RegisterModuleToDebug(nullptr);

  EXPECT_EQ(debug_controller_.GetNumberOfModules(), 3);
}

TEST_F(DebugControllerTests, OkToLogWithEqualVerbosity) {

  MockDebugInterface debug_interface1;
  MockDebugInterface debug_interface2;
  MockDebugInterface debug_interface3;

  EXPECT_CALL(debug_interface1, GetCurrentVerbosity()).WillOnce(Return(DebugInterface::MessageVerbosity::INFO_MSG));

  debug_controller_.RegisterModuleToDebug(&debug_interface1);
  debug_controller_.RegisterModuleToDebug(&debug_interface2);
  debug_controller_.RegisterModuleToDebug(&debug_interface3);
  ASSERT_EQ(debug_controller_.GetNumberOfModules(), 3);

  EXPECT_TRUE(debug_controller_.CheckIfModuleCanLog(&debug_interface1, DebugInterface::MessageVerbosity::INFO_MSG));
}

TEST_F(DebugControllerTests, OkToLogWithBiggerVerbosity) {

  MockDebugInterface debug_interface1;
  MockDebugInterface debug_interface2;
  MockDebugInterface debug_interface3;

  EXPECT_CALL(debug_interface1, GetCurrentVerbosity()).WillOnce(Return(DebugInterface::MessageVerbosity::INFO_MSG));

  debug_controller_.RegisterModuleToDebug(&debug_interface1);
  debug_controller_.RegisterModuleToDebug(&debug_interface2);
  debug_controller_.RegisterModuleToDebug(&debug_interface3);
  ASSERT_EQ(debug_controller_.GetNumberOfModules(), 3);

  EXPECT_TRUE(debug_controller_.CheckIfModuleCanLog(&debug_interface1, DebugInterface::MessageVerbosity::ERROR_MSG));
}

TEST_F(DebugControllerTests, CantLogWithSmallerVerbosity) {

  MockDebugInterface debug_interface1;
  MockDebugInterface debug_interface2;
  MockDebugInterface debug_interface3;

  EXPECT_CALL(debug_interface1, GetCurrentVerbosity()).WillOnce(Return(DebugInterface::MessageVerbosity::DEBUG_MSG));

  debug_controller_.RegisterModuleToDebug(&debug_interface1);
  debug_controller_.RegisterModuleToDebug(&debug_interface2);
  debug_controller_.RegisterModuleToDebug(&debug_interface3);
  ASSERT_EQ(debug_controller_.GetNumberOfModules(), 3);

  EXPECT_FALSE(debug_controller_.CheckIfModuleCanLog(&debug_interface1, DebugInterface::MessageVerbosity::ERROR_MSG));
}

TEST_F(DebugControllerTests, CantLogUnregisteredModule) {

  MockDebugInterface debug_interface1;
  MockDebugInterface debug_interface2;
  MockDebugInterface debug_interface3;

  debug_controller_.RegisterModuleToDebug(&debug_interface1);
  debug_controller_.RegisterModuleToDebug(&debug_interface2);
  ASSERT_EQ(debug_controller_.GetNumberOfModules(), 2);

  EXPECT_FALSE(debug_controller_.CheckIfModuleCanLog(&debug_interface3, DebugInterface::MessageVerbosity::ERROR_MSG));
}

}
}
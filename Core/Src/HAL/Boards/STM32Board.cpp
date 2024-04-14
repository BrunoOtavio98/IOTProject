/*
 * STM32.cpp
 *
 *  Created on: Jan 3, 2024
 *      Author: BrunoOtavio
 */

#include "STM32Board.h"
#include <memory>

#include "Devices/Communication/STM32UartCommunication.h"
#include "Devices/IOT/Interfaces/ModemInterface.h"
#include "Devices/IOT/Modem/SIM7020E.h"
#include "DebugController/DebugController.h"
#include "RTOSWrappers/TaskWrapperManager.h"

#include "stm32f4xx_hal.h"

using HAL::Devices::Communication::STM32UartCommunication;
using HAL::Devices::Communication::Interfaces::UartCommunicationInterface;
using HAL::Devices::IOT::Interfaces::ModemInterface;
using HAL::Devices::IOT::Modem::SIM7020Modem;
using HAL::DebugController::DebugController;
using HAL::RtosWrappers::TaskWrapperManager;

namespace HAL {
namespace Boards {

STM32Board::STM32Board() {
	modem_uart_communication_ = std::make_shared<STM32UartCommunication>(UartCommunicationInterface::BAUD_115200, UartCommunicationInterface::UartNumber::UART_4);
	debug_uart_communication_ = std::make_shared<STM32UartCommunication>(UartCommunicationInterface::BAUD_115200, UartCommunicationInterface::UartNumber::UART_5);
	debug_controller_ = std::make_shared<DebugController::DebugController>(debug_uart_communication_);

	HAL_Init();
  rtos_task_manager_ = std::make_shared<TaskWrapperManager>();
  rtos_task_manager_->CreateTask(*debug_controller_);
}

STM32Board::~STM32Board() {

}

void STM32Board::InitPeripherals(AvailableModemInterfaces selected_modem) {
	SystemClockConfig();
	ConfigureModem(selected_modem);
}

void STM32Board::SystemClockConfig() {

  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
	Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
	Error_Handler();
  }
}

void STM32Board::ConfigureModem(AvailableModemInterfaces modem_interface) {

	switch (modem_interface) {
		case AvailableModemInterfaces::SIM_7020E:
			modem_interface_ = std::make_unique<SIM7020Modem>(modem_uart_communication_, debug_controller_);
			break;
		default:
			break;
	}
}

void STM32Board::Error_Handler() {
  __disable_irq();
  	while (1)
	{
	}
}

}
}

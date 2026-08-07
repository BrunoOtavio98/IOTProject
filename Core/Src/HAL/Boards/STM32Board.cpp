/*
 * STM32.cpp
 *
 *  Created on: Jan 3, 2024
 *      Author: BrunoOtavio
 */

#include "STM32Board.h"
#include <memory>

#include "Devices/Communication/STM32UartCommunication.h"
#include "Devices/Communication/STM32SPICommunication.h"
#include "Devices/IOT/Interfaces/ModemInterface.h"
#include "Devices/IOT/Modem/SIM7020E.h"
#include "Devices/Position/GNSSInterface.h"
#include "DebugController/DebugController.h"
#include "DebugController/DebugInterface.h"
#include "Storage/StorageInterface.h"
#include "RTOSWrappers/TaskWrapperManager.h"
#include "Storage/SDCard.h"

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

using HAL::Devices::Communication::STM32UartCommunication;
using HAL::Devices::Communication::STM32SPICommunication;
using HAL::Devices::Communication::Interfaces::SPIInterface;
using HAL::Devices::Communication::Interfaces::UartCommunicationInterface;
using HAL::Devices::IOT::Interfaces::ModemInterface;
using HAL::Devices::IOT::Modem::SIM7020Modem;
using HAL::DebugController::DebugController;
using HAL::DebugController::DebugInterface;
using HAL::RtosWrappers::TaskWrapperManager;
using HAL::Devices::Position::GNSSInterface;
using HAL::Storage::StorageInterface;
using HAL::Storage::SDCard;

namespace HAL {
namespace Boards {

STM32Board::STM32Board() : DebugInterface("STM32Board"), 
                           TaskWrapper("STM32Board", 500, nullptr, 2)
{
	//modem_uart_communication_ = std::make_shared<STM32UartCommunication>(UartCommunicationInterface::BAUD_115200, UartCommunicationInterface::UartNumber::UART_4, "modem_uart_task");
	debug_uart_communication_ = std::make_shared<STM32UartCommunication>(UartCommunicationInterface::BAUD_115200, UartCommunicationInterface::UartNumber::UART_1, "debug_uart_task");
  //gnss_uart_communication_ = std::make_shared<STM32UartCommunication>(UartCommunicationInterface::BAUD_9600, UartCommunicationInterface::UartNumber::UART_2, "modem_uart_task");
  rtos_task_manager_ = std::make_shared<TaskWrapperManager>();
  
	debug_controller_ = std::make_shared<DebugController::DebugController>(DebugInterface::MessageVerbosity::INFO_MSG, debug_uart_communication_);
  //gnss_interface_ = std::make_shared<GNSSInterface>(gnss_uart_communication_, debug_controller_);
	debug_controller_->RegisterModuleToDebug(this);

  rtos_task_manager_->CreateTask(*std::dynamic_pointer_cast<STM32UartCommunication>(debug_uart_communication_));
  rtos_task_manager_->CreateTask(*debug_controller_);

  ConfigureSDCard();

  //rtos_task_manager_->CreateTask(*std::dynamic_pointer_cast<STM32UartCommunication>(gnss_uart_communication_));
  //rtos_task_manager_->CreateTask(*gnss_interface_);

  //rtos_task_manager_->CreateTask(*std::dynamic_pointer_cast<STM32UartCommunication>(modem_uart_communication_));
  rtos_task_manager_->CreateTask(*this);
  
	HAL_Init();
	SystemClockConfig();
}

STM32Board::~STM32Board() 
{

}

void STM32Board::Task(void *params)
{
  //char buffer[580] = {0};
  //char test[] = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.";  // 21 chars, pad to 22

  while(1)
  {   
	  // if( !sd_spi_communication_->WriteReadData( (uint8_t*)test, (uint8_t *)buffer, 576 ) )
    //   {
    //       debug_controller_->PrintInfo(this, "Failed to complete SPI transaction\n", true);
    //   }
    //   else
    //   {
    //       debug_controller_->PrintInfo(this, buffer, true);
    //       debug_controller_->PrintInfo(this, "\n\n", true);
    //       buffer[0] = '\0';
    //   }

      TaskDelay(200);
  }
}

void STM32Board::InitPeripherals(AvailableModemInterfaces selected_modem) 
{
  rtos_task_manager_ = std::make_shared<TaskWrapperManager>();

  //ConfigureModem(selected_modem);

  osKernelStart();
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
      rtos_task_manager_->CreateTask(*modem_interface_);
			break;
		default:
			break;
	}
}

void STM32Board::ConfigureSDCard()
{
  SPIInterface::SPIConfiguration spi_config;
  spi_config.spi_number = SPIInterface::SPINumber::SPI_1;
  spi_config.spi_mode = SPIInterface::SPIMode::Master;
  spi_config.spi_data_size = SPIInterface::SPIDataSize::SPI_8Bits;
  spi_config.spi_timming_mode = SPIInterface::SPITimmingMode::CPOL0_CPHA0;
  spi_config.spi_baud_selector = SPIInterface::SPIBaudRatePrescaler::BaudratePrescaler_128;

  sd_spi_communication_ = std::make_shared<STM32SPICommunication>(spi_config);

  storage_interface_ = std::make_unique<SDCard>(sd_spi_communication_, debug_controller_);
  rtos_task_manager_->CreateTask( *std::dynamic_pointer_cast<SDCard>(storage_interface_) );
}

void STM32Board::Error_Handler() {
  __disable_irq();
  	while (1)
	{
	}
}

}
}

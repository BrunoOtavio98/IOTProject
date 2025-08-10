/*
 * STM32UartCommunication.cpp
 *
 *  Created on: 6 de jan de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_CPP_
#define SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_CPP_

#include "STM32UartCommunication.h"

#include <cstring>
#include <map>

using HAL::Devices::Communication::Interfaces::UartCommunicationInterface;;
using HAL::RtosWrappers::TaskWrapper;

namespace HAL {
namespace Devices {
namespace Communication {

std::map<UART_HandleTypeDef*, STM32UartCommunication*> group_of_uarts;

STM32UartCommunication::STM32UartCommunication(UartCommunicationInterface::BaudRates baud_rate, UartCommunicationInterface::UartNumber uart_number, const std::string &task_name) :
											   UartCommunicationInterface(baud_rate, uart_number, task_name,  500, nullptr, 0),
											   uart_handle_(std::make_unique<UART_HandleTypeDef>()),
											   rx_buffer_pos_(0),
											   enable_listen_rx_(false){
  EnableGPIOClk(uart_number);
  USART_TypeDef *hal_specific_uart_number = BaseUartToHalUartNumber(uart_number);
  uart_handle_->Instance = hal_specific_uart_number;
  uart_handle_->Init.BaudRate = static_cast<int>(baud_rate);
  uart_handle_->Init.WordLength = UART_WORDLENGTH_8B;
  uart_handle_->Init.StopBits = UART_STOPBITS_1;
  uart_handle_->Init.Parity = UART_PARITY_NONE;
  uart_handle_->Init.Mode = UART_MODE_TX_RX;
  uart_handle_->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  uart_handle_->Init.OverSampling = UART_OVERSAMPLING_16;

  group_of_uarts.insert({uart_handle_.get(), this});
  HAL_UART_Init(uart_handle_.get());
}

STM32UartCommunication::~STM32UartCommunication() {

}

void STM32UartCommunication::Task(void *params) {

	while(1) {
		
		if(callback_read_finish_ && rx_buffer_pos_ && !isr_executing_) {
			uint8_t temp_data[kRxBufferSize];
			std::memcpy(temp_data, rx_buffer_, rx_buffer_pos_);
			callback_read_finish_(temp_data, rx_buffer_pos_);
			rx_buffer_pos_ = 0;
		}
		TaskDelay(50);
	}
}

/**
 * @brief Write data to UART
 * 
 * @param data_to_write: data to be written into UART
 * @return true in case of success
 * @return false otherwise
 */
bool STM32UartCommunication::WriteData(const std::string &data_to_write) {
	const uint8_t *data = reinterpret_cast<const uint8_t *>(&data_to_write[0]);
	return (HAL_UART_Transmit(uart_handle_.get(), data, data_to_write.size(), kUartTimeoutTxCommunication) == HAL_OK);
}

/**
 * @brief Read data from UART using interrupt
 * 
 * @param callback_read_finish: callback to be called when data is received
 * @return true: in case of success of registering the interrupt
 * @return false: otherwise
 */
bool STM32UartCommunication::ReadDataIT(std::function<void(const uint8_t *data, uint16_t data_size)> callback_read_finish) {
	callback_read_finish_ = callback_read_finish;

	if(group_of_uarts.find(uart_handle_.get()) != group_of_uarts.end()) {
		STM32UartCommunication *current_uart = group_of_uarts[uart_handle_.get()];
		return (HAL_UARTEx_ReceiveToIdle_IT(current_uart->uart_handle_.get(), current_uart->current_chunk_, current_uart->kChunkSize) == HAL_OK);
	}
	return false;
}

/**
 * @brief Enable listening to RX interrupt
 * 
 * @param callback_read_finish: callback to be called when data is received
 * @return true in case of success of registering the interrupt
 * @return false otherwise
 */
bool STM32UartCommunication::ListenRxIT(std::function<void(const uint8_t *data, uint16_t data_size)> callback_read_finish) {
	enable_listen_rx_ = true;
	return ReadDataIT(callback_read_finish);
}

/**
 * @brief Disable listening to RX interrupt
 * 
 */
void STM32UartCommunication::DisableListenRxIt() {
	enable_listen_rx_ = false;
}

/**
 * @brief Convert a UartNumber enum to the corresponding USART_TypeDef
 * 
 * @param uart_number: UartNumber enum
 * @return USART_TypeDef*: corresponding USART_TypeDef
 */
USART_TypeDef *STM32UartCommunication::BaseUartToHalUartNumber(UartNumber uart_number) {

	switch (uart_number) {
		case UartNumber::UART_1:
			return USART1;
		case UartNumber::UART_2:
			return USART2;
		case UartNumber::UART_3:
			return USART3;
		case UartNumber::UART_4:
			return UART4;
		case UartNumber::UART_5:
			return UART5;
		case UartNumber::UART_6:
			return USART6;
		default:
			return UART4;
	}
}

/**
 * @brief Enable the GPIO clock for the corresponding UART
 * 
 * @param uart_number: UartNumber enum
 */
void STM32UartCommunication::EnableGPIOClk(UartCommunicationInterface::UartNumber uart_number) {

	switch (uart_number) {
		case UartNumber::UART_1:
			 __HAL_RCC_GPIOA_CLK_ENABLE();
			break;
		case UartNumber::UART_2:
			 __HAL_RCC_GPIOA_CLK_ENABLE();
			break;
		case UartNumber::UART_3:
			 __HAL_RCC_GPIOB_CLK_ENABLE();
			break;
		case UartNumber::UART_4:
			 __HAL_RCC_GPIOA_CLK_ENABLE();
			break;
		case UartNumber::UART_5:
			 __HAL_RCC_GPIOC_CLK_ENABLE();
			 __HAL_RCC_GPIOD_CLK_ENABLE();
			break;
		case UartNumber::UART_6:
			 __HAL_RCC_GPIOG_CLK_ENABLE();
			break;
		default:
			break;
	}
	return;
}

extern "C" {

	void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
		 auto it = group_of_uarts.find(huart);
		 if(it == group_of_uarts.end()) {
			 return;
		 }

		STM32UartCommunication *current_uart = it->second;

		if(Size >= (current_uart->kRxBufferSize - current_uart->rx_buffer_pos_)) {
			Size = (current_uart->kRxBufferSize - current_uart->rx_buffer_pos_) - 1;
		}
		current_uart->isr_executing_ = true;
		std::memcpy(current_uart->rx_buffer_ + current_uart->rx_buffer_pos_, current_uart->current_chunk_, Size);
		current_uart->rx_buffer_pos_ += Size;

		current_uart->isr_executing_ = false;
		if(current_uart->enable_listen_rx_) {
			HAL_UARTEx_ReceiveToIdle_IT(current_uart->uart_handle_.get(), current_uart->current_chunk_, current_uart->kChunkSize);
		}
	}
}

extern "C" {
	void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
		 auto it = group_of_uarts.find(huart);
		 if(it == group_of_uarts.end()) {
			return;
		 }

		STM32UartCommunication *current_uart = it->second;
		switch (huart->ErrorCode)
		{
		case HAL_UART_ERROR_ORE:
			/* code */
			__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE);
			if(current_uart->enable_listen_rx_) {
				HAL_UARTEx_ReceiveToIdle_IT(current_uart->uart_handle_.get(), current_uart->current_chunk_, current_uart->kChunkSize);
			}
			break;
		
		default:
			break;
		}
	}
}

extern "C" {
	void UART4_IRQHandler(void)
	{
		STM32UartCommunication *current_uart = nullptr;
		for(auto &uart : group_of_uarts) {
			if(uart.first->Instance == UART4) {
				current_uart = uart.second;
			}
		}

		if(current_uart == nullptr) {
			return;
		}

		HAL_UART_IRQHandler(current_uart->uart_handle_.get());
	}

	void UART5_IRQHandler(void) {

		STM32UartCommunication *current_uart = nullptr;
		for(auto &uart : group_of_uarts) {
			if(uart.first->Instance == UART5) {
				current_uart = uart.second;
			}
		}

		if(current_uart == nullptr) {
			return;
		}

		HAL_UART_IRQHandler(current_uart->uart_handle_.get());
	}
}

}
}
}

#endif /* SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_CPP_ */

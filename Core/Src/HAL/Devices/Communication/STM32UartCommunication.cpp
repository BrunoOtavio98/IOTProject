/*
 * STM32UartCommunication.cpp
 *
 *  Created on: 6 de jan de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_CPP_
#define SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_CPP_

#include "STM32UartCommunication.h"

#include <map>

using HAL::Devices::Communication::Interfaces::UartCommunicationInterface;;

namespace HAL {
namespace Devices {
namespace Communication {

std::map<UART_HandleTypeDef*, STM32UartCommunication*> group_of_uarts;

STM32UartCommunication::STM32UartCommunication(UartCommunicationInterface::BaudRates baud_rate, UartCommunicationInterface::UartNumber uart_number) :
											   UartCommunicationInterface(baud_rate, uart_number),
											   uart_handle_(std::make_unique<UART_HandleTypeDef>()),
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

bool STM32UartCommunication::WriteData(const std::string &data_to_write) {
	const uint8_t *data = reinterpret_cast<const uint8_t *>(&data_to_write[0]);
	return (HAL_UART_Transmit(uart_handle_.get(), data, data_to_write.size(), kUartTimeoutTxCommunication) == HAL_OK);
}

bool STM32UartCommunication::ReadDataIT(std::function<void(const std::string&)> callback_read_finish) {
	callback_read_finish_ = callback_read_finish;

	if(group_of_uarts.find(uart_handle_.get()) != group_of_uarts.end()) {
		STM32UartCommunication *current_uart = group_of_uarts[uart_handle_.get()];
		return (HAL_UART_Receive_IT(uart_handle_.get(), &current_uart->current_byte_, 1) == HAL_OK);
	}
	return false;
}

bool STM32UartCommunication::ListenRxIT(std::function<void(const std::string&)> callback_read_finish) {
	enable_listen_rx_ = true;
	return ReadDataIT(callback_read_finish);
}

void STM32UartCommunication::DisableListenRxIt() {
	enable_listen_rx_ = false;
}

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
	 void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

		 if(group_of_uarts.find(huart) == group_of_uarts.end()) {
			 return;
		 }

		 STM32UartCommunication *current_uart = group_of_uarts[huart];
		 if(current_uart->current_byte_ == '\n' || current_uart->current_byte_ == '\r') {
			 if(current_uart->callback_read_finish_) {
				 current_uart->callback_read_finish_(current_uart->rx_buffer_);
				 current_uart->rx_buffer_.clear();

				 if(current_uart->enable_listen_rx_) {
					HAL_UART_Receive_IT(current_uart->uart_handle_.get(), &current_uart->current_byte_, 1);
				 }

				 return;
			 }
		 }
		current_uart->rx_buffer_ += static_cast<char>(current_uart->current_byte_);
		HAL_UART_Receive_IT(current_uart->uart_handle_.get(), &current_uart->current_byte_, 1);
	}
}

extern "C" {
	void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
		//TODO
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

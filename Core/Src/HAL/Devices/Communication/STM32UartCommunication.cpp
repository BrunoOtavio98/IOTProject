/*
 * STM32UartCommunication.cpp
 *
 *  Created on: 6 de jan de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_CPP_
#define SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_CPP_

#include "STM32UartCommunication.h"

using HAL::Devices::Communication::Interfaces::UartCommunicationInterface;;


namespace HAL {
namespace Devices {
namespace Communication {

STM32UartCommunication *stm32_uart_communication = nullptr;

STM32UartCommunication::STM32UartCommunication(UartCommunicationInterface::BaudRates baud_rate, UartCommunicationInterface::UartNumber uart_number) :
											   UartCommunicationInterface(baud_rate, uart_number),
											   uart_handle_(std::make_unique<UART_HandleTypeDef>()),
											   enable_listen_rx_(false){
 stm32_uart_communication = this;
 __HAL_RCC_GPIOA_CLK_ENABLE();

  USART_TypeDef *hal_specific_uart_number = BaseUartToHalUartNumber(uart_number);
  uart_handle_->Instance = hal_specific_uart_number;
  uart_handle_->Init.BaudRate = static_cast<int>(baud_rate);
  uart_handle_->Init.WordLength = UART_WORDLENGTH_8B;
  uart_handle_->Init.StopBits = UART_STOPBITS_1;
  uart_handle_->Init.Parity = UART_PARITY_NONE;
  uart_handle_->Init.Mode = UART_MODE_TX_RX;
  uart_handle_->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  uart_handle_->Init.OverSampling = UART_OVERSAMPLING_16;
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
	return (HAL_UART_Receive_IT(uart_handle_.get(), &stm32_uart_communication->current_byte_, 1) == HAL_OK);
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
		default:
			return UART4;
	}
}


extern "C" {
	 void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

		 if(stm32_uart_communication->current_byte_ == '\n') {
			 if(stm32_uart_communication->callback_read_finish_) {
				 stm32_uart_communication->callback_read_finish_(stm32_uart_communication->rx_buffer_);
				 stm32_uart_communication->rx_buffer_.clear();

				 if(stm32_uart_communication->enable_listen_rx_) {
					HAL_UART_Receive_IT(stm32_uart_communication->uart_handle_.get(), &stm32_uart_communication->current_byte_, 1);
				 }

				 return;
			 }
		 }
		stm32_uart_communication->rx_buffer_ += static_cast<char>(stm32_uart_communication->current_byte_);
		HAL_UART_Receive_IT(stm32_uart_communication->uart_handle_.get(), &stm32_uart_communication->current_byte_, 1);
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
		HAL_UART_IRQHandler(stm32_uart_communication->uart_handle_.get());
	}
}

}
}
}

#endif /* SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_CPP_ */

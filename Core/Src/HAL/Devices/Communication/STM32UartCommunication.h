/*
 * STM32UartCommunication.h
 *
 *  Created on: 6 de jan de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_H_
#define SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_H_

#include "Interfaces/UartCommunicationInterface.h"
#include "RTOSWrappers/TaskWrapper.h"

#include <memory>
#include <string>
#include "stm32f4xx_hal.h"

namespace HAL {
namespace Devices {
namespace Communication {

class STM32UartCommunication : public HAL::Devices::Communication::Interfaces::UartCommunicationInterface,
							   public HAL::RtosWrappers::TaskWrapper {
public:
	static const int kRxBufferSize = 1024;
	static const int kChunkSize = 256;
	const int kUartTimeoutTxCommunication = 150;
	const int kUartTimeoutRxCommunication = 10000;

	STM32UartCommunication(UartCommunicationInterface::BaudRates baud_rate, UartCommunicationInterface::UartNumber uart_number, const std::string &task_name = "");
	virtual ~STM32UartCommunication();

	bool WriteData(const std::string &data_to_write) override;
	bool ReadDataIT(std::function<void(const uint8_t *data, uint16_t data_size)> callback_read_finish) override;
	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
	bool ListenRxIT(std::function<void(const uint8_t *data, uint16_t data_size)> callback_read_finish) override;
	void DisableListenRxIt() override;

	void Task(void *params) override;

	std::unique_ptr<UART_HandleTypeDef> uart_handle_;
	uint8_t current_chunk_[kChunkSize];
	uint8_t rx_buffer_[kRxBufferSize];
	uint16_t rx_buffer_pos_;
	bool enable_listen_rx_;
	bool isr_executing_;

private:
	void UART4_IRQHandler();
	void UART_IRQ();
	void EnableGPIOClk(UartCommunicationInterface::UartNumber uart_number);
	USART_TypeDef *BaseUartToHalUartNumber(UartNumber uart_number);

	std::function<void(const uint8_t *data, uint16_t data_size)> callback_read_finish_;
};

}
}
}



#endif /* SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_H_ */

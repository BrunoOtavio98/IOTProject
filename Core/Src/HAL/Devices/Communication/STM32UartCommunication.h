/*
 * STM32UartCommunication.h
 *
 *  Created on: 6 de jan de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_H_
#define SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_H_

#include "Interfaces/UartCommunicationInterface.h"

#include <memory>
#include <string>
#include "stm32f4xx_hal.h"

namespace HAL {
namespace Devices {
namespace Communication {

class STM32UartCommunication : public HAL::Devices::Communication::Interfaces::UartCommunicationInterface {
public:

	const int kUartTimeoutTxCommunication = 150;
	const int kUartTimeoutRxCommunication = 10000;

	STM32UartCommunication(UartCommunicationInterface::BaudRates baud_rate, UartCommunicationInterface::UartNumber uart_number);
	virtual ~STM32UartCommunication();

	bool WriteData(const std::string &data_to_write) override;
	bool ReadDataIT(std::function<void(const std::string&)> callback_read_finish) override;
	void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
	bool ListenRxIT(std::function<void(const std::string&)> callback_read_finish) override;
	void DisableListenRxIt() override;


	std::unique_ptr<UART_HandleTypeDef> uart_handle_;
	std::function<void(const std::string&)> callback_read_finish_;
	uint8_t current_byte_;
	std::string rx_buffer_;
	bool enable_listen_rx_;

private:
	void UART4_IRQHandler();
	void UART_IRQ();
	USART_TypeDef *BaseUartToHalUartNumber(UartNumber uart_number);
};

}
}
}



#endif /* SRC_HAL_DEVICES_COMMUNICATION_STM32UARTCOMMUNICATION_H_ */

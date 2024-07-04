/*
 * UartCommunication.h
 *
 *  Created on: 6 de jan de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEVICES_COMMUNICATION_UARTCOMMUNICATION_H_
#define SRC_HAL_DEVICES_COMMUNICATION_UARTCOMMUNICATION_H_

#include <stdint.h>
#include <functional>
#include <string>

namespace HAL {
namespace Devices {
namespace Communication {
namespace Interfaces {

class UartCommunicationInterface {
public:
	enum BaudRates {
		BAUD_4800 = 4800,
		BAUD_9600 = 9600,
		BAUD_19200 = 19200,
		BAUD_38400 = 38400,
		BAUD_57600 = 57600,
		BAUD_115200 = 115200,
	};

	enum UartNumber {
		UART_1,
		UART_2,
		UART_3,
		UART_4,
		UART_5,
		UART_6
	};

UartCommunicationInterface(BaudRates baud_rate, UartNumber uart_number) {
}

~UartCommunicationInterface() {

}

virtual bool WriteData(const std::string &data_to_write) = 0;
virtual bool ReadDataIT(std::function<void(const uint8_t *, uint16_t)> callback_read_finish) = 0;
virtual bool ListenRxIT(std::function<void(const uint8_t *, uint16_t)> callback_read_finish) {
	return true;
}
virtual void DisableListenRxIt() {
}

};

}

}
}
}

#endif /* SRC_HAL_DEVICES_COMMUNICATION_UARTCOMMUNICATION_H_ */

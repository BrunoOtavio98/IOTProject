/*
 * STM32Board.h
 *
 *  Created on: Jan 3, 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_BOARDS_STM32BOARD_H_
#define SRC_HAL_BOARDS_STM32BOARD_H_

#include "BoardInterface.h"

namespace HAL {
namespace Boards {

class STM32Board : public BoardInterface {
public:
	STM32Board();
	virtual ~STM32Board();

	void InitPeripherals(AvailableModemInterfaces modem_interface) override;

private:
	void SystemClockConfig();
	void Error_Handler();
	void ConfigureModem(AvailableModemInterfaces modem_interface);
};


}
}

#endif /* SRC_HAL_BOARDS_STM32BOARD_H_ */

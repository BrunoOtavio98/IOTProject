/*
 * STM32Board.h
 *
 *  Created on: Jan 3, 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_BOARDS_STM32BOARD_H_
#define SRC_HAL_BOARDS_STM32BOARD_H_

#include "BoardInterface.h"
#include "RTOSWrappers/TaskWrapper.h"
#include "DebugController/DebugInterface.h"

using HAL::RtosWrappers::TaskWrapper;
using HAL::DebugController::DebugInterface;

namespace HAL {
namespace Boards {

class STM32Board : public BoardInterface,
				   private TaskWrapper,
				   private DebugInterface {
public:
	STM32Board();
	virtual ~STM32Board();

	void InitPeripherals(AvailableModemInterfaces modem_interface) override;

private:
	void SystemClockConfig();
	void Error_Handler();
	void ConfigureModem(AvailableModemInterfaces modem_interface);
	void Task(void *params) override;

	AvailableModemInterfaces selected_modem_;
};

}
}

#endif /* SRC_HAL_BOARDS_STM32BOARD_H_ */

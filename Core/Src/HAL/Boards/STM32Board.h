/*
 * STM32Board.h
 *
 *  Created on: Jan 3, 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_BOARDS_STM32BOARD_H_
#define SRC_HAL_BOARDS_STM32BOARD_H_

#include "BoardInterface.h"
#include "DebugController/DebugInterface.h"
#include "RTOSWrappers/TaskWrapper.h"

using HAL::DebugController::DebugInterface;
using HAL::RtosWrappers::TaskWrapper;

namespace HAL {
namespace Boards {

class STM32Board : public BoardInterface,
				   public DebugInterface,
				   public TaskWrapper {
public:
	STM32Board();
	virtual ~STM32Board();

	void InitPeripherals(AvailableModemInterfaces modem_interface) override;
	void Task(void *params) override;

private:
	void SystemClockConfig();
	void Error_Handler();
	void ConfigureModem(AvailableModemInterfaces modem_interface);
	void SetSPIForSDCard();

};

}
}

#endif /* SRC_HAL_BOARDS_STM32BOARD_H_ */

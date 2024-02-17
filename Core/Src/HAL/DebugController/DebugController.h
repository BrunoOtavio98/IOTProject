/*
 * DebugController.h
 *
 *  Created on: 17 de fev de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEBUGCONTROLLER_DEBUGCONTROLLER_H_
#define SRC_HAL_DEBUGCONTROLLER_DEBUGCONTROLLER_H_

#include "DebugInterface.h"

#include <memory>

namespace HAL {
namespace Devices {
namespace Communication {
namespace Interfaces {
class UartCommunicationInterface;
}
}
}
}

namespace HAL {
namespace DebugController {

class DebugController {
 public:
	DebugController(std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> uart_communication);
	virtual ~DebugController();

	void PrintDebug(const std::string &msg);
	void PrintInfo(const std::string &msg);
	void PrintWarn(const std::string &msg);
	void PrintError(const std::string &msg);

 private:
	std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> uart_debug_;

	void PrintMessage(std::string message);
};

}
}

#endif /* SRC_HAL_DEBUGCONTROLLER_DEBUGCONTROLLER_H_ */

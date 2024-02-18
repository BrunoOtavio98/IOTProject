/*
 * DebugController.h
 *
 *  Created on: 17 de fev de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEBUGCONTROLLER_DEBUGCONTROLLER_H_
#define SRC_HAL_DEBUGCONTROLLER_DEBUGCONTROLLER_H_

#include "DebugInterface.h"

#include <vector>
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

	void PrintDebug(DebugInterface *module, const std::string &msg);
	void PrintInfo(DebugInterface *module, const std::string &msg);
	void PrintWarn(DebugInterface *module, const std::string &msg);
	void PrintError(DebugInterface *module, const std::string &msg);
	void RegisterModuleToDebug(DebugInterface *module);

 private:
	std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> uart_debug_;
	std::vector<DebugInterface *> list_of_modules_;

	bool CheckIfModuleCanLog(DebugInterface *module, const DebugInterface::MessageVerbosity &desired_verbosity);
	void PrintMessage(const std::string &module, const std::string &message);
};

}
}

#endif /* SRC_HAL_DEBUGCONTROLLER_DEBUGCONTROLLER_H_ */

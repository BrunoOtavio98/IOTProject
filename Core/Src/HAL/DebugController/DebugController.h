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
#include <functional>
#include <list>

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

	virtual void PrintDebug(DebugInterface *module, const std::string &msg);
	virtual void PrintInfo(DebugInterface *module, const std::string &msg);
	virtual void PrintWarn(DebugInterface *module, const std::string &msg);
	virtual void PrintError(DebugInterface *module, const std::string &msg);
	virtual void RegisterModuleToDebug(DebugInterface *module);
	virtual void RegisterCallBackToReadMessages(std::function<void(const std::string&)> callback);
	
 protected:
	std::vector<DebugInterface *> list_of_modules_;

	bool CheckIfModuleCanLog(DebugInterface *module, const DebugInterface::MessageVerbosity &desired_verbosity);
 private:
	std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> uart_debug_;
	std::list<std::function<void(const std::string&)>> callbacks_;

	std::string MessageTypeToStr(const DebugInterface::MessageVerbosity &verbosity);
	void PrintMessage(const DebugInterface::MessageVerbosity &msg_verbosity, const std::string &module, const std::string &message);
	void DispatchMessage(const std::string &message);
};

}
}

#endif /* SRC_HAL_DEBUGCONTROLLER_DEBUGCONTROLLER_H_ */

/*
 * DebugController.h
 *
 *  Created on: 17 de fev de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEBUGCONTROLLER_DEBUGCONTROLLER_H_
#define SRC_HAL_DEBUGCONTROLLER_DEBUGCONTROLLER_H_

#include "DebugInterface.h"

#include "RTOSWrappers/TaskWrapper.h"
#include "RTOSWrappers/QueueWrapper.h"

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
namespace RtosWrappers {
class QueueWrapper;
}
}

namespace HAL {
namespace DebugController {

class DebugController : public HAL::RtosWrappers::TaskWrapper {
 public:
	DebugController(std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> uart_communication);
	virtual ~DebugController();

	virtual void PrintDebug(DebugInterface *module, const std::string &msg, bool from_isr);
	virtual void PrintInfo(DebugInterface *module, const std::string &msg, bool from_isr);
	virtual void PrintWarn(DebugInterface *module, const std::string &msg, bool from_isr);
	virtual void PrintError(DebugInterface *module, const std::string &msg, bool from_isr);
	virtual void RegisterModuleToDebug(DebugInterface *module);
	virtual void RegisterCallBackToReadMessages(std::function<void(const std::string&)> callback);
	
 protected:
	std::vector<DebugInterface *> list_of_modules_;
	bool task_should_run_ = true;

	void Task(void *params) override;

	bool CheckIfModuleCanLog(DebugInterface *module, const DebugInterface::MessageVerbosity &desired_verbosity);
	void CallbackUartMsgReceived(const uint8_t *data, uint16_t size);
private:
	static const int kBufferSize = 1024;

	struct DebugData {
		DebugInterface::MessageVerbosity msg_verbosity;
		char msg[30];
		char module_name[10];
	}DataToLog;

	std::string MessageTypeToStr(const DebugInterface::MessageVerbosity &verbosity);
	void PrintMessage(const DebugInterface::MessageVerbosity &msg_verbosity, const std::string &module, const std::string &message);
	void DispatchMessage(const std::string &message);
	void InsertMsgIntoQueue(const DebugInterface::MessageVerbosity &msg_verbosity, const std::string &module, const std::string &message, bool from_isr);
	bool CanProcessMessage();

	std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> uart_debug_;
	std::list<std::function<void(const std::string&)>> callbacks_;
	std::shared_ptr<HAL::RtosWrappers::QueueWrapper> queue_manager_;
	GenericQueueHandle debug_msgs_queue_;
	uint8_t uart_buffer_receive_[kBufferSize];
	uint16_t rx_buffer_pos_;
	bool is_callback_executing_;
};

}
}

#endif /* SRC_HAL_DEBUGCONTROLLER_DEBUGCONTROLLER_H_ */

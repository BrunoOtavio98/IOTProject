/*
 * ModemInterface.h
 *
 *  Created on: 13 de jan de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEVICES_COMMUNICATION_INTERFACES_MODEMINTERFACE_H_
#define SRC_HAL_DEVICES_COMMUNICATION_INTERFACES_MODEMINTERFACE_H_

#include "DebugController/DebugInterface.h"

#include "RTOSWrappers/TaskWrapper.h"

#include <memory>
#include <functional>
#include <map>
#include <list>
#include <string>

namespace HAL{
namespace Devices {
namespace Communication {
namespace Interfaces{
class UartCommunicationInterface;
}
}
}
}

namespace HAL {
namespace DebugController {
class DebugController;
}
}

namespace HAL {
namespace Devices {
namespace IOT {
namespace Interfaces {

class ModemInterface : public HAL::DebugController::DebugInterface, public HAL::RtosWrappers::TaskWrapper {
public:
	enum ATCommands {
		ATE,
		ATI,
		ATL,
		ATM,
		ATN1,
		ATO,
		ATP,
		ATQ,
		ATS0,
		ATS1,
		ATS2,
		ATS3,
		ATS4,
		ATS5,
		ATS6,
		ATS7,
		ATS8,
		ATS10,
		ATS12,
		ATS25,
		ATS95,
		ATT,
		ATV,
		ATX,
		ATZ,
		ATeC,
		ATeD,
		ATeF,
		ATeK,
		ATeV,
		ATeW,
		AT_DR,
		CEER,
		CGMI,
		CGMM,
		CGMR,
		CGOI,
		CGSN,
		CIMI,
		CLCK,
		CMAR,
		CMEE,
		COPS,
		CPIN,
		CPWD,
		CREG,
		CPOL,
		CFUN,
		CCLK,
		CSIM,
		CGREG
	};

	enum AtCommandTypes {
		Test,
		Read,
		Write,
		Execute
	};

	enum AtReponses {
		OK,
		NO_CARRIER,
		NO_DIALTONE,
		BUSY,
		NO_ANSWER
	};

	struct ATCommandConfiguration {
		int16_t timeout;
		std::function<bool(const std::string&)> receive_callback;
	};

	ModemInterface(const std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> &uart_communication,
				   const std::shared_ptr<HAL::DebugController::DebugController> debug_controler);
    virtual ~ModemInterface();
    void RegisterCommand(const ATCommands &at_command, const ATCommandConfiguration &command_configuration);
    bool SendCommand(const AtCommandTypes &command_type, const ATCommands &command_to_execute, const std::list<std::string> &parameters);
    void ReceiveCommandCallBack(const uint8_t *data, uint16_t data_size);
    std::string EnumCommandToString(const ATCommands &command);
    void ForwardDebugUartMessage(const std::string &msg);

protected:
    void SendTestCommand(const std::string &command);
    void SendReadCommand(const std::string &command);
    void SendWriteCommand(const std::string &command, const std::list<std::string> &parameters);
    void SendExecutionCommand(const std::string &command, const std::list<std::string> &parameters);

	void Task(void *params) override;

	bool task_should_run_;
    std::shared_ptr<HAL::DebugController::DebugController> debug_controller_;
	std::map<ATCommands, ATCommandConfiguration> modem_commands_;
private:
	static const int kRxBufferSize = 1024;

	bool CanProcessUartMessage();

    std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> uart_communication_;
    std::shared_ptr<HAL::DebugController::DebugController> debug_controller_;
    std::map<ATCommands, ATCommandConfiguration> modem_commands_;
    ATCommands current_command_executed_;

	uint8_t rx_buffer_[kRxBufferSize];
	uint16_t rx_buffer_pos_;
	bool is_isr_executing_;
};

}
}
}
}

#endif /* SRC_HAL_DEVICES_COMMUNICATION_INTERFACES_MODEMINTERFACE_H_ */

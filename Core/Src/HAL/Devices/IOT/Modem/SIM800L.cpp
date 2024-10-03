
#include "SIM800L.h"
#include "DebugController/DebugController.h"
#include <string>
#include <sstream>

using HAL::Devices::Communication::Interfaces::UartCommunicationInterface;;
using HAL::DebugController::DebugController;

namespace HAL {
namespace Devices {
namespace IOT {
namespace Modem {

SIM800LModem::SIM800LModem(const std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> &uart_communication,
				 const std::shared_ptr<HAL::DebugController::DebugController> &debug_controller) : ModemInterface(uart_communication, debug_controller) 
{
	ATCommandConfiguration generic_at_config;
	generic_at_config.timeout = 100;
	generic_at_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute){return GenericCmdResponse(msg, command_to_execute);};
	RegisterCommand(ATCommands::ATE, generic_at_config);

	std::list<std::string> cmd_param = {"1"};	

	SendCommand(AtCommandTypes::Execute, ATCommands::ATE, cmd_param);
	SendCommand(AtCommandTypes::Execute, ATCommands::ATE, cmd_param);
	SendCommand(AtCommandTypes::Execute, ATCommands::ATE, cmd_param);
	SendCommand(AtCommandTypes::Execute, ATCommands::ATE, cmd_param);
	SendCommand(AtCommandTypes::Execute, ATCommands::ATE, cmd_param);
	SendCommand(AtCommandTypes::Execute, ATCommands::ATE, cmd_param);

}

SIM800LModem::~SIM800LModem()
{

}

bool SIM800LModem::GenericCmdResponse(const std::string &response, const ATCommands &command_to_execute)
{	
	debug_controller_->PrintDebug(this, "SIM800L Executing callback\n", true);
	debug_controller_->PrintInfo(this, response, true);
	
	std::stringstream ss(response);
	std::string response_cmd = "";

	std::getline(ss, response_cmd, '\r');
	std::getline(ss, response_cmd, '\r');

	debug_controller_->PrintDebug(this, "Reponse cmd: " + response_cmd, true);
	if(response_cmd.find("OK") != std::string::npos)
	{	
		debug_controller_->PrintDebug(this, "Response is ok\n", true);
		return true;
	}
	
	debug_controller_->PrintDebug(this, "Response if false\n", true);
	
	return false;
}

}
}
}
}
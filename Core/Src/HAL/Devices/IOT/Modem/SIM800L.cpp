
#include "SIM800L.h"
#include "DebugController/DebugController.h"
#include <string>

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
}

SIM800LModem::~SIM800LModem()
{

}

bool Connect(const std::string &apn, const std::string &username, const std::string &password) 
{
	return true;
}

bool SIM800LModem::GenericCmdResponse(const std::string &response, const ATCommands &command_to_execute)
{	
	debug_controller_->PrintInfo(this, "SIM800L: " + response, true);
	
	if(response.find("OK") != std::string::npos)
	{	
		debug_controller_->PrintDebug(this, "Response is ok\n", true);
		return true;
	}
	
	debug_controller_->PrintDebug(this, "Response is false\n", true);
	return false;
}

}
}
}
}
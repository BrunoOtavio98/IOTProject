
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
				 const std::shared_ptr<HAL::DebugController::DebugController> &debug_controller) : 
				 ModemInterface(uart_communication, debug_controller), 
				 connection_completed_(false),
				 last_cmd_status_(true),
				 number_of_expected_responses_(0),
				 number_of_received_responses_(0)
{
	ATCommandConfiguration generic_at_config;
	generic_at_config.timeout = 100;
	generic_at_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute){return GenericCmdResponse(msg, command_to_execute);};
	RegisterCommand(ATCommands::ATE, generic_at_config);

	ATCommandConfiguration creg_config;
	creg_config.timeout = 100;
	creg_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute){return CREGResponse(msg, command_to_execute);};
	RegisterCommand(ATCommands::ATE, creg_config);

	ATCommandConfiguration csq_config;
	csq_config.timeout = 100;
	csq_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute){return CSQRespoonse(msg, command_to_execute);};
	RegisterCommand(ATCommands::ATE, csq_config);

	ATCommandConfiguration cops_config;
	cops_config.timeout = 100;
	cops_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute){return COPSResponse(msg, command_to_execute);};
	RegisterCommand(ATCommands::ATE, cops_config);

	ATCommandConfiguration cgatt_config;
	cgatt_config.timeout = 100;
	cgatt_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute){return CGATTResponse(msg, command_to_execute);};
	RegisterCommand(ATCommands::ATE, cgatt_config);

	ATCommandConfiguration cstt_config;
	cstt_config.timeout = 100;
	cstt_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute){return CSTTResponse(msg, command_to_execute);};
	RegisterCommand(ATCommands::ATE, cstt_config);

	ATCommandConfiguration ciicr_config;
	ciicr_config.timeout = 100;
	ciicr_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute){return CIICRResponse(msg, command_to_execute);};
	RegisterCommand(ATCommands::ATE, ciicr_config);

	ATCommandConfiguration cifsr_config;
	cifsr_config.timeout = 100;
	cifsr_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute){return CIFSRResponse(msg, command_to_execute);};
	RegisterCommand(ATCommands::ATE, cifsr_config);
}

SIM800LModem::~SIM800LModem()
{

}

void SIM800LModem::OnLoop() 
{
	if(!connection_completed_)
	{
		Connect("", "", "");

	}

	if(number_of_expected_responses_ == number_of_received_responses_)
	{
		connection_completed_ = true;
	}

	if(connection_completed_) 
	{
		TestConnectionIsUp();
	}
}

bool SIM800LModem::Connect(const std::string &apn, const std::string &username, const std::string &password) 
{	
	SendCommand(AtCommandTypes::Write, ATCommands::ATE, {"0"});
	number_of_expected_responses_++;

	SendCommand(AtCommandTypes::Execute, ATCommands::CSQ, {});
	number_of_expected_responses_++;

	SendCommand(AtCommandTypes::Read, ATCommands::CREG, {});
	number_of_expected_responses_++;

	SendCommand(AtCommandTypes::Read, ATCommands::COPS, {});
	number_of_expected_responses_++;

	SendCommand(AtCommandTypes::Write, ATCommands::CGATT, {"1"});
	number_of_expected_responses_++;

	SendCommand(AtCommandTypes::Write, ATCommands::CSTT, {apn, username, password});
	number_of_expected_responses_++;

	SendCommand(AtCommandTypes::Execute, ATCommands::CIICR, {});
	number_of_expected_responses_++;

	SendCommand(AtCommandTypes::Execute, ATCommands::CIFSR, {});
	number_of_expected_responses_++;

	return true;
}

void SIM800LModem::TestConnectionIsUp() 
{
	SendCommand(AtCommandTypes::Execute, ATCommands::CREG, {});
}

bool SIM800LModem::GenericCmdResponse(const std::string &response, const ATCommands &command_to_execute)
{
	debug_controller_->PrintInfo(this, "SIM800L: " + response, true);
	
	if(response.find("OK") != std::string::npos)
	{	
		return true;
	}
	return false;
}

bool SIM800LModem::CREGResponse(const std::string &response, const ATCommands &command) {
	return true;
}

bool SIM800LModem::CSQRespoonse(const std::string &response, const ATCommands &command) {
	return true;
}

bool SIM800LModem::COPSResponse(const std::string &response, const ATCommands &command) {
	return true;
}

bool SIM800LModem::CGATTResponse(const std::string &response, const ATCommands &command) {
	return true;
}

bool SIM800LModem::CSTTResponse(const std::string &response, const ATCommands &command) {
	return true;
}

bool SIM800LModem::CIICRResponse(const std::string &response, const ATCommands &commmand) {
	return true;
}

bool SIM800LModem::CIFSRResponse(const std::string &response, const ATCommands &command) {
	return true;
}

}
}
}
}
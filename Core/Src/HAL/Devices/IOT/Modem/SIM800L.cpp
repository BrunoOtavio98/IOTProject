
#include "SIM800L.h"
#include "DebugController/DebugController.h"
#include <string>
#include <sstream>
#include <iostream>

using HAL::Devices::Communication::Interfaces::UartCommunicationInterface;;
using HAL::DebugController::DebugController;

namespace HAL {
namespace Devices {
namespace IOT {
namespace Modem {

SIM800LModem::SIM800LModem(const std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> &uart_communication,
				 const std::shared_ptr<HAL::DebugController::DebugController> &debug_controller) : 
				 ModemInterface(uart_communication, debug_controller), 
				 kTimeToTestConnection(kTaskDelayMs * 5),
				 next_cmd_to_execute_(ATCommands::Invalid),
				 connection_completed_(false),
				 time_passed_keep_alive_(0),
				 current_cmd_state_(modem_cmd_state::kIdle)
{
	ATCommandConfiguration generic_at_config;
	generic_at_config.timeout = 100;
	generic_at_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute, const AtCommandTypes &command_type){
												return GenericCmdResponse(msg, command_to_execute, command_type);};
	RegisterCommand(ATCommands::ATE, generic_at_config);

	ATCommandConfiguration creg_config;
	creg_config.timeout = 100;
	creg_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute, const AtCommandTypes &command_type){
												return CREGResponse(msg, command_to_execute, command_type);};
	RegisterCommand(ATCommands::CREG, creg_config);

	ATCommandConfiguration csq_config;
	csq_config.timeout = 100;
	csq_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute, const AtCommandTypes &command_type){
												return CSQRespoonse(msg, command_to_execute, command_type);};
	RegisterCommand(ATCommands::CSQ, csq_config);

	ATCommandConfiguration cops_config;
	cops_config.timeout = 1200;
	cops_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute, const AtCommandTypes &command_type){
												return COPSResponse(msg, command_to_execute, command_type);};
	RegisterCommand(ATCommands::COPS, cops_config);

	ATCommandConfiguration cgatt_config;
	cgatt_config.timeout = 1000;
	cgatt_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute, const AtCommandTypes &command_type){
												return CGATTResponse(msg, command_to_execute, command_type);};
	RegisterCommand(ATCommands::CGATT, cgatt_config);

	ATCommandConfiguration cstt_config;
	cstt_config.timeout = 100;
	cstt_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute, const AtCommandTypes &command_type){
												return CSTTResponse(msg, command_to_execute, command_type);};
	RegisterCommand(ATCommands::CSTT, cstt_config);

	ATCommandConfiguration ciicr_config;
	ciicr_config.timeout = 100;
	ciicr_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute, const AtCommandTypes &command_type){
												return CIICRResponse(msg, command_to_execute, command_type);};
	RegisterCommand(ATCommands::CIICR, ciicr_config);

	ATCommandConfiguration cifsr_config;
	cifsr_config.timeout = 100;
	cifsr_config.receive_callback = [this](const std::string &msg, const ATCommands &command_to_execute, const AtCommandTypes &command_type){
												return CIFSRResponse(msg, command_to_execute, command_type);};
	RegisterCommand(ATCommands::CIFSR, cifsr_config);
}

SIM800LModem::~SIM800LModem()
{

}

void SIM800LModem::OnLoop() 
{	
	if(!connection_completed_)
	{
		ConnectStateMachine("", "", "");
	}

	//KeepAliveControl();
}

void SIM800LModem::KeepAliveControl()
{
	if(connection_completed_)
	{
		time_passed_keep_alive_ += kTaskDelayMs;
		if(time_passed_keep_alive_ >= kTimeToTestConnection)
		{	
			TestConnectionIsUp();
			time_passed_keep_alive_ = 0;
		}
	}
}

void SIM800LModem::ConnectStateMachine(const std::string &apn, const std::string &username, const std::string &password) 
{	
	AtCommandTypes curr_command_type ;
	ATCommands curr_command = ATCommands::Invalid;
	std::list<std::string> curr_parameters;

	if(current_cmd_state_ == modem_cmd_state::kError)
	{
		next_cmd_to_execute_ = ATCommands::Invalid;
		debug_controller_->PrintDebug(this, "Modem connection failed, resetting state machine", true);
	}

	switch (next_cmd_to_execute_)
	{	
		
		case ATCommands::ATE:
		
			if(current_cmd_state_ == modem_cmd_state::kIdle) {
				debug_controller_->PrintDebug(this, "ATCommands:ATE\n", true);

				curr_command_type = AtCommandTypes::Execute;
				curr_command = ATCommands::ATE;
				curr_parameters = {"1"};
				break;
			}

			if(current_cmd_state_ == modem_cmd_state::kLastCommandExecuted) {
				debug_controller_->PrintDebug(this, "Going to ATCommands:CSQ\n", true);
				next_cmd_to_execute_ = ATCommands::CSQ;
				current_cmd_state_ = modem_cmd_state::kIdle;
				break;
			}
			break;
		case ATCommands::CSQ:

			if(current_cmd_state_ == modem_cmd_state::kIdle) {
				debug_controller_->PrintDebug(this, "ATCommands:CSQ\n", true);

				curr_command_type = AtCommandTypes::Execute;
				curr_command = ATCommands::CSQ;
				curr_parameters = {};
				break;
			}

			if(current_cmd_state_ == modem_cmd_state::kLastCommandExecuted) {
				debug_controller_->PrintDebug(this, "Going to ATCommands:CREG\n", true);
				next_cmd_to_execute_ = ATCommands::CREG;
				current_cmd_state_ = modem_cmd_state::kIdle;
				break;
			}
			break;
		case ATCommands::CREG:
			if(current_cmd_state_ == modem_cmd_state::kIdle) {
				debug_controller_->PrintDebug(this, "ATCommands:CREG\n", true);

				curr_command_type = AtCommandTypes::Read;
				curr_command = ATCommands::CREG;
				curr_parameters = {};
				break;
			}
			
			if(current_cmd_state_ == modem_cmd_state::kLastCommandExecuted) {
				debug_controller_->PrintDebug(this, "Going to ATCommands:COPS\n", true);
				next_cmd_to_execute_ = ATCommands::COPS;
				current_cmd_state_ = modem_cmd_state::kIdle;
				break;
			}
			break;
		case ATCommands::COPS:
			if(current_cmd_state_ == modem_cmd_state::kIdle) {
				debug_controller_->PrintDebug(this, "ATCommands:COPS\n", true);

				curr_command_type = AtCommandTypes::Read;
				curr_command = ATCommands::COPS;
				curr_parameters = {};
				break;
			}

			if(current_cmd_state_ == modem_cmd_state::kLastCommandExecuted) {
				debug_controller_->PrintDebug(this, "Going to ATCommands:CGATT\n", true);
				next_cmd_to_execute_ = ATCommands::CGATT;
				current_cmd_state_ = modem_cmd_state::kIdle;
				break;
			}
			break;
		case ATCommands::CGATT:
			if(current_cmd_state_ == modem_cmd_state::kIdle) {
				debug_controller_->PrintDebug(this, "ATCommands:CGATT\n", true);

				curr_command_type = AtCommandTypes::Write;
				curr_command = ATCommands::CGATT;
				curr_parameters = {"1"};
				break;
			}

			if(current_cmd_state_ == modem_cmd_state::kLastCommandExecuted) {
				debug_controller_->PrintDebug(this, "Going to ATCommands:CSTT\n", true);
				next_cmd_to_execute_ = ATCommands::CSTT;
				current_cmd_state_ = modem_cmd_state::kIdle;
				break;
			}
			break;
		case ATCommands::CSTT:
			if(current_cmd_state_ == modem_cmd_state::kIdle) {
				debug_controller_->PrintDebug(this, "ATCommands:CSTT\n", true);

				curr_command_type = AtCommandTypes::Write;
				curr_command = ATCommands::CSTT;
				curr_parameters = {apn, username, password};
				break;
			}

			if(current_cmd_state_ == modem_cmd_state::kLastCommandExecuted) {
				debug_controller_->PrintDebug(this, "Going to ATCommands:CIICR\n", true);
				next_cmd_to_execute_ = ATCommands::CIICR;
				current_cmd_state_ = modem_cmd_state::kIdle;
				break;
			}
			break;
		case ATCommands::CIICR:
			if(current_cmd_state_ == modem_cmd_state::kIdle) {
				debug_controller_->PrintDebug(this, "ATCommands:CIICR\n", true);

				curr_command_type = AtCommandTypes::Execute;
				curr_command = ATCommands::CIICR;
				curr_parameters = {};
				break;
			}

			if(current_cmd_state_ == modem_cmd_state::kLastCommandExecuted) {
				debug_controller_->PrintDebug(this, "Going to ATCommands:CIFSR\n", true);
				next_cmd_to_execute_ = ATCommands::CIFSR;
				current_cmd_state_ = modem_cmd_state::kIdle;
				break;
			}
			break;
		case ATCommands::CIFSR:
			if(current_cmd_state_ == modem_cmd_state::kIdle) {
				debug_controller_->PrintDebug(this, "ATCommands:CIFSR\n", true);

				curr_command_type = AtCommandTypes::Execute;
				curr_command = ATCommands::CIFSR;
				curr_parameters = {};
				break;
			}

			if(current_cmd_state_ == modem_cmd_state::kLastCommandExecuted) {
				
				next_cmd_to_execute_ = ATCommands::Invalid;
				debug_controller_->PrintDebug(this, "Modem connection completed\n", true);
				connection_completed_ = true;
				current_cmd_state_ = modem_cmd_state::kIdle;
			}
			break;
		case ATCommands::Invalid:
			current_cmd_state_ = modem_cmd_state::kIdle;
			next_cmd_to_execute_ = ATCommands::ATE;
			break;
		default:
			break;
	}

	if(connection_completed_ || current_cmd_state_ == modem_cmd_state::kWaitingForResponse ||
	   current_cmd_state_ == modem_cmd_state::kLastCommandExecuted ||curr_command == ATCommands::Invalid) {
		return;
	}

	current_cmd_state_ = modem_cmd_state::kWaitingForResponse;
	SendCommand(curr_command_type, curr_command, curr_parameters);
}

void SIM800LModem::TestConnectionIsUp() 
{
	SendCommand(AtCommandTypes::Execute, ATCommands::CREG, {});
}

bool SIM800LModem::GenericCmdResponse(const std::string &response, const ATCommands &command_to_execute, const AtCommandTypes &command_type)
{	
	debug_controller_->PrintDebug(this, "Generic CB: " + response, true);
	if(response.find("OK") != std::string::npos)
	{	
		current_cmd_state_ = modem_cmd_state::kLastCommandExecuted;
		debug_controller_->PrintDebug(this, "Generic command executed\n", true);
		return true;
	}

	current_cmd_state_ = modem_cmd_state::kError;
	debug_controller_->PrintDebug(this, "Generic command failed\n", true);
	return false;
}

bool SIM800LModem::CREGResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type) 
{	
	debug_controller_->PrintDebug(this, "CREG: " + response, true);
	if(response.find("OK") == std::string::npos)
	{
		if(command_type == AtCommandTypes::Read)
		{
			std::vector<std::string> splitted_response = SplitString(response, ',');

			if(splitted_response.size() == 4)
			{
			}
			else 
			{

			}
		}
		return true;
	}
	return false;
}

bool SIM800LModem::CSQRespoonse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type)
{
	return true;
}

bool SIM800LModem::COPSResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type) 
{
	return true;
}

bool SIM800LModem::CGATTResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type) 
{
	return true;
}

bool SIM800LModem::CSTTResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type) 
{
	return true;
}

bool SIM800LModem::CIICRResponse(const std::string &response, const ATCommands &commmand, const AtCommandTypes &command_type) 
{
	return true;
}

bool SIM800LModem::CIFSRResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type) 
{
	return true;
}

std::vector<std::string> SIM800LModem::SplitString(const std::string &message, char delimiter) 
{
	std::vector<std::string> tokens;
    std::stringstream ss(message);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

}
}
}
}
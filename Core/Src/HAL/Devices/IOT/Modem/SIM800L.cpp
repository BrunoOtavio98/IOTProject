
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
				 kTimeToTestConnection(kTaskDelayMs * 5),
				 connection_completed_(false),
				 last_cmd_status_(true),
				 number_of_expected_responses_(0),
				 number_of_received_responses_(0),
				 time_passed_keep_alive_(0)
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
		Connect("", "", "");
		connection_completed_ = true;
	}

	if(number_of_expected_responses_ == number_of_received_responses_)
	{	
		connection_completed_ = true;
		number_of_received_responses_ = 0;
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

bool SIM800LModem::Connect(const std::string &apn, const std::string &username, const std::string &password) 
{	
	SendCommand(AtCommandTypes::Execute, ATCommands::ATE, {"0"});
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

bool SIM800LModem::GenericCmdResponse(const std::string &response, const ATCommands &command_to_execute, const AtCommandTypes &command_type)
{
	if(response.find("OK") != std::string::npos)
	{
		return true;
	}
	return false;
}

bool SIM800LModem::CREGResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type) 
{
	if(response.find("OK") == std::string::npos)
	{
		return false;
	}

	number_of_received_responses_++;
	return true;
}

bool SIM800LModem::CSQRespoonse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type)
{
	number_of_received_responses_++;
	return true;
}

bool SIM800LModem::COPSResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type) 
{
	number_of_received_responses_++;
	return true;
}

bool SIM800LModem::CGATTResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type) 
{
	number_of_received_responses_++;
	return true;
}

bool SIM800LModem::CSTTResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type) 
{
	number_of_received_responses_++;
	return true;
}

bool SIM800LModem::CIICRResponse(const std::string &response, const ATCommands &commmand, const AtCommandTypes &command_type) 
{
	number_of_received_responses_++;
	return true;
}

bool SIM800LModem::CIFSRResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type) 
{
	number_of_received_responses_++;
	return true;
}

}
}
}
}
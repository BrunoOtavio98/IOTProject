/*
 * ModemInterface.cpp
 *
 *  Created on: 13 de jan de 2024
 *      Author: BrunoOtavio
 */

#include "ModemInterface.h"

#include "Devices/Communication/Interfaces/UartCommunicationInterface.h"
#include "DebugController/DebugController.h"

#include <functional>

using HAL::Devices::Communication::Interfaces::UartCommunicationInterface;
using HAL::DebugController::DebugInterface;
using HAL::DebugController::DebugController;

namespace HAL {
namespace Devices {
namespace IOT {
namespace Interfaces {

static void StaticReceiveCommandCallBackWrapper(void* instance, const std::string& data) {
    static_cast<ModemInterface*>(instance)->ReceiveCommandCallBack(data);
}

ModemInterface::ModemInterface(const std::shared_ptr<UartCommunicationInterface> &uart_communication, 
				   			  const std::shared_ptr<HAL::DebugController::DebugController> debug_controler) :
							   DebugInterface("Modem"),
							   uart_communication_(uart_communication),
							   debug_controller_(debug_controler) {
	debug_controller_->RegisterModuleToDebug(this);
	uart_communication_->ListenRxIT(std::bind(&StaticReceiveCommandCallBackWrapper, this, std::placeholders::_1));
	debug_controller_->RegisterCallBackToReadMessages([this](const std::string &msg){ForwardDebugUartMessage(msg);});
}

ModemInterface::~ModemInterface(){

}

void ModemInterface::RegisterCommand(const ATCommands &at_command, const ATCommandConfiguration &command_configuration) {
	modem_commands_.insert({ at_command, command_configuration });
}

bool ModemInterface::SendCommand(const AtCommandTypes &command_type, const ATCommands &command_to_execute, const std::list<std::string> &parameters) {
	current_command_executed_ = command_to_execute;

	std::string command_as_string = EnumCommandToString(command_to_execute);
	switch (command_type) {
		case AtCommandTypes::Execute:

			SendExecutionCommand(command_as_string, parameters);
			break;
		case AtCommandTypes::Read:

			SendReadCommand(command_as_string);
			break;
		case AtCommandTypes::Test:

			SendTestCommand(command_as_string);
			break;
		case AtCommandTypes::Write:

			SendWriteCommand(command_as_string, parameters);
			break;
		default:
			break;
	}

	return true;
}

void ModemInterface::ReceiveCommandCallBack(const std::string &received_message) {
	debug_controller_->PrintDebug(this, received_message, true);
	auto it = modem_commands_.find(current_command_executed_);

	if(it != modem_commands_.end()) {
		modem_commands_[current_command_executed_].receive_callback(received_message);
	}
}

void ModemInterface::SendTestCommand(const std::string &command) {
	std::string final_command = command + "=?";
	uart_communication_->WriteData(final_command);
}

void ModemInterface::SendReadCommand(const std::string &command) {
	std::string final_command = command + "?";
	uart_communication_->WriteData(final_command);
}

void ModemInterface::SendWriteCommand(const std::string &command, const std::list<std::string> &parameters) {
	std::string final_command = command + "=";
	std::string parameters_as_single_string = "";
	int num_parameters = parameters.size();

	for(const auto &parameter : parameters) {
		num_parameters--;
		if(!num_parameters) {
			parameters_as_single_string += parameter;
		}
		else {
			parameters_as_single_string += parameter + ",";
		}
	}

	final_command += parameters_as_single_string;
	uart_communication_->WriteData(final_command);
}

void ModemInterface::SendExecutionCommand(const std::string &command, const std::list<std::string> &parameters) {
	std::string final_command = command;
	std::string parameters_as_single_string = "";
	int num_parameters = parameters.size();

	for(const auto &parameter : parameters) {

		num_parameters--;
		if(!num_parameters) {
			parameters_as_single_string += parameter;
		}
		else {
			parameters_as_single_string += parameter + ",";
		}
	}

	final_command += parameters_as_single_string;
	uart_communication_->WriteData(final_command);
}

void ModemInterface::ForwardDebugUartMessage(const std::string &msg) {
	uart_communication_->WriteData(msg);
}


std::string ModemInterface::EnumCommandToString(const ATCommands &command) {

	switch (command) {
		case ATE:
			return "ATE";
		case ATI:
			return "ATI";
		case ATL:
			return "ATL";
		case ATM:
			return "ATM";
		case ATN1:
			return "ATN1";
		case ATO:
			return "ATO";
		case ATP:
			return "ATP";
		case ATQ:
			return "ATQ";
		case ATS0:
			return "ATS0";
		case ATS1:
			return "ATS1";
		case ATS2:
			return "ATS2";
		case ATS3:
			return "ATS3";
		case ATS4:
			return "ATS4";
		case ATS5:
			return "ATS5";
		case ATS6:
			return "ATS6";
		case ATS7:
			return "ATS7";
		case ATS8:
			return "ATS8";
		case ATS10:
			return "ATS10";
		case ATS12:
			return "ATS12";
		case ATS25:
			return "ATS25";
		case ATS95:
			return "ATS95";
		case ATT:
			return "ATT";
		case ATV:
			return "ATV";
		case ATX:
			return "ATX";
		case ATZ:
			return "ATZ";
		case ATeC:
			return "AT&C";
		case ATeD:
			return "AT&D";
		case ATeF:
			return "AT&F";
		case ATeK:
			return "AT&K";
		case ATeV:
			return "AT&V";
		case ATeW:
			return "AT&W";
		case AT_DR:
			return "AT+DR";
		case CEER:
			return "AT+CEER";
		case CGMI:
			return "AT+CGMI";
		case CGMM:
			return "AT+CGMM";
		case CGMR:
			return "AT+CGMR";
		case CGOI:
			return "AT+CGOI";
		case CGSN:
			return "AT+CGSN";
		case CIMI:
			return "AT+CIMI";
		case CLCK:
			return "AT+CLCK";
		case CMAR:
			return "AT+CMAR";
		case CMEE:
			return "AT+CMEE";
		case COPS:
			return "AT+COPS";
		case CPIN:
			return "AT+CPIN";
		case CPWD:
			return "AT+CPWD";
		case CREG:
			return "AT+CREG";
		case CPOL:
			return "AT+CPOL";
		case CFUN:
			return "AT+CFUN";
		case CCLK:
			return "AT+CCLK";
		case CSIM:
			return "AT+CSIM";
		case CGREG:
			return "AT+CGREG";
		default:
			break;
	}

	return "";
}

}
}
}
}

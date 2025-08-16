/*
 * ModemInterface.cpp
 *
 *  Created on: 13 de jan de 2024
 *      Author: BrunoOtavio
 */

#include "ModemInterface.h"

#include "Devices/Communication/Interfaces/UartCommunicationInterface.h"
#include "DebugController/DebugController.h"
#include "RTOSWrappers/QueueWrapper.h"

#include <cstring>
#include <functional>
#include <iostream>

using HAL::Devices::Communication::Interfaces::UartCommunicationInterface;
using HAL::DebugController::DebugInterface;
using HAL::DebugController::DebugController;
using HAL::RtosWrappers::TaskWrapper;
using HAL::RtosWrappers::QueueWrapper;

namespace HAL {
namespace Devices {
namespace IOT {
namespace Interfaces {

static void StaticReceiveCommandCallBackWrapper(void* instance, const uint8_t *data, uint16_t data_size) {
    static_cast<ModemInterface*>(instance)->ReceiveCommandCallBack(data, data_size);
}

ModemInterface::ModemInterface(const std::shared_ptr<UartCommunicationInterface> &uart_communication, 
				   			   const std::shared_ptr<HAL::DebugController::DebugController> debug_controler) :
							   DebugInterface("Modem"),
							   TaskWrapper("ModemInterfaceTask", 500, nullptr, 1),
							   task_should_run_(true),
							   debug_controller_(debug_controler),
							   uart_communication_(uart_communication),
							   current_command_executed_(ATCommands::Invalid),
							   rx_buffer_pos_(0),
							   is_isr_executing_(false),
							   queue_manager_(std::make_shared<QueueWrapper>()),
							   time_passed_(0),
							   current_timeout_to_monitor_(0),
							   tx_rx_are_sync_(true) {
	debug_controller_->RegisterModuleToDebug(this);
	uart_communication_->ListenRxIT(std::bind(&StaticReceiveCommandCallBackWrapper, this, std::placeholders::_1, std::placeholders::_2));
	debug_controller_->RegisterCallBackToReadMessages([this](const std::string &msg){ForwardDebugUartMessage(msg);});
	send_cmd_queue_ = queue_manager_->CreateQueue(20, sizeof(struct CurrentCmd));
	
	ChangeVerbosity(DebugInterface::MessageVerbosity::DEBUG_MSG);
}

ModemInterface::~ModemInterface(){
}

void ModemInterface::Task(void *params){
	do
	{
		SendCommandsQueued();
		CommandResponseDispatcher();
		CommandTimeoutMonitor();

		OnLoop();
		TimePassedControl();
		TaskDelay(kTaskDelayMs);

	} while(task_should_run_);
	
}

/**
 * @brief Dispatch the command response received from the modem
 *        to debug controller and to the registered callback
 * 
 */
void ModemInterface::CommandResponseDispatcher() {
	if(CanProcessUartMessage()) {
		std::string received_message(reinterpret_cast<char*>(rx_buffer_), rx_buffer_pos_);
		//debug_controller_->PrintDebug(this, "Rsp: " + received_message + "\n", false);

		auto it = modem_commands_.find(current_command_executed_);
		if(it != modem_commands_.end()) 
		{
			debug_controller_->PrintDebug(this, "Callback has been found\n", false);
			modem_commands_[current_command_executed_].receive_callback(received_message, current_command_executed_, current_command_type_);
			current_timeout_to_monitor_ = 0;
			time_passed_ = 0;
		}
		tx_rx_are_sync_ = true;
		rx_buffer_pos_ = 0;
	}
}

/**
 * @brief After checking if the modem is waiting for a response,
 * 		  this function will send the first queue command to the modem
 * 
 */
void ModemInterface::SendCommandsQueued() {
	struct CurrentCmd current_at_cmd;

	if(IsModemWaitingForResponse() == false)
	{
		if(queue_manager_->QueueReceive(send_cmd_queue_, &current_at_cmd, 0)) 
		{	
			debug_controller_->PrintDebug(this, "Sending command: " + std::string(current_at_cmd.raw_msg) + "\n", false);
			current_command_executed_ = current_at_cmd.current_cmd;
			current_command_type_ = current_at_cmd.current_cmd_type;
			uart_communication_->WriteData( current_at_cmd.raw_msg );
			tx_rx_are_sync_ = false;

			auto it = modem_commands_.find(current_command_executed_);
			if(it != modem_commands_.end()) 
			{
				current_timeout_to_monitor_ = modem_commands_[current_command_executed_].timeout;
			}
		}
	}
}

/**
 * @brief After a command has been sent to the modem, this function will
 *        check if the timeout for the command has been reached. If so,
 *        it will notify the debug controller and the registered callback
 * 
 */
void ModemInterface::CommandTimeoutMonitor() {
	if(IsModemWaitingForResponse()) {
		if(time_passed_ > current_timeout_to_monitor_)
		{	
			debug_controller_->PrintDebug(this, "Timeout has occured\n", false);
			auto it = modem_commands_.find(current_command_executed_);
			if(it != modem_commands_.end()) 
			{
				modem_commands_[current_command_executed_].receive_callback("\r\nTIMEOUT\r\n", current_command_executed_, current_command_type_);
			}
			current_timeout_to_monitor_ = 0;
			time_passed_ = 0;
			tx_rx_are_sync_ = true;
			current_command_executed_  = ATCommands::Invalid;
		}
	}
}

/**
 * @brief Increment the time passed variable control if 
 *        the modem interface is waiting for a response
 * 
 */
void ModemInterface::TimePassedControl() {

	if(IsModemWaitingForResponse())
	{	
		debug_controller_->PrintDebug(this, "Timeout control: " + std::to_string(time_passed_) + "\n", false);
		time_passed_ += kTaskDelayMs;
	}
}

/**
 * @brief Check if the modem interface is waiting for a response
 * 
 * @return true 
 * @return false 
 */
bool ModemInterface::IsModemWaitingForResponse() 
{
	return (tx_rx_are_sync_ == false);
}

/**
 * @brief Check if a complete message has been received from the modem
 * 
 * @return true if a complete message has been received
 * @return false otherwise
 */
bool ModemInterface::CanProcessUartMessage() {
	if( !is_isr_executing_ && (rx_buffer_[rx_buffer_pos_- 1] == '\n' || rx_buffer_[rx_buffer_pos_ - 1] == '\r')) {
		return true;
	}
	return false;
}

/**
 * @brief Register an AT command with its configuration to the modem interface.
 * 
 * @param at_command: The AT command to register.
 * @param command_configuration: The configuration for the AT command, including timeout and a callback function. 
 */
void ModemInterface::RegisterCommand(const ATCommands &at_command, const ATCommandConfiguration &command_configuration) {
	modem_commands_.insert({ at_command, command_configuration });
}

/**
 * @brief Send an AT command to the modem.
 * 
 * @param command_type: The type of command (Test, Read, Write, Execute).
 * @param command_to_execute: The specific AT command to execute.
 * @param parameters: A list of parameters to include with the command.
 * @return true 
 * @return false 
 */
bool ModemInterface::SendCommand(const AtCommandTypes &command_type, const ATCommands &command_to_execute, const std::list<std::string> &parameters) {
	std::string command_as_string = EnumCommandToString(command_to_execute);

	if(command_as_string == "")
	{
		return false;
	}

	switch (command_type) {
		case AtCommandTypes::Execute:

			SendExecutionCommand(command_as_string, command_to_execute, parameters);
			break;
		case AtCommandTypes::Read:

			SendReadCommand(command_as_string, command_to_execute);
			break;
		case AtCommandTypes::Test:

			SendTestCommand(command_as_string, command_to_execute);
			break;
		case AtCommandTypes::Write:

			SendWriteCommand(command_as_string, command_to_execute, parameters);
			break;
		default:
			break;
	}

	return true;
}

/**
 * @brief Callback function to receive data from the modem.
 * 		  Uart communication interface will call this function
 * 
 * @param data: Pointer to the received data.
 * @param data_size: Size of the received data.
 */
void ModemInterface::ReceiveCommandCallBack(const uint8_t *data, uint16_t data_size) {
	if(data == nullptr) {
		return;
	}

	if(data_size >= (kRxBufferSize - rx_buffer_pos_) ) {
		data_size = ((kRxBufferSize - rx_buffer_pos_) - 1);
	}
	debug_controller_->PrintDebug(this, "Received data: " + std::string(reinterpret_cast<const char*>(data), data_size) + "\n", false);

	is_isr_executing_ = true;
	std::memcpy(rx_buffer_ + rx_buffer_pos_, data, data_size);
	rx_buffer_pos_ += data_size;
	is_isr_executing_ = false;
}

void ModemInterface::SendTestCommand(const std::string &command, const ATCommands &command_to_execute) {
	std::string final_command = command + "=?" + "\r\n";
	SendAtMsgToQueue(final_command, command_to_execute, AtCommandTypes::Test);
}

void ModemInterface::SendReadCommand(const std::string &command, const ATCommands &command_to_execute) {
	std::string final_command = command + "?" + "\r\n";
	SendAtMsgToQueue(final_command, command_to_execute, AtCommandTypes::Read);
}

void ModemInterface::SendWriteCommand(const std::string &command, const ATCommands &command_to_execute, const std::list<std::string> &parameters) {
	std::string final_command = command + "=";
	std::string parameters_as_single_string = "";
	int num_parameters = parameters.size();

	if(parameters.empty()) {
		final_command += "\r\n";
	}
	else
	{
		for(const auto &parameter : parameters) {
			num_parameters--;
			if(!num_parameters) {
				parameters_as_single_string += parameter + "\r\n";
			}
			else {
				parameters_as_single_string += parameter + ",";
			}
		}
	}

	final_command += parameters_as_single_string;
	SendAtMsgToQueue(final_command, command_to_execute, AtCommandTypes::Write);
}

void ModemInterface::SendExecutionCommand(const std::string &command, const ATCommands &command_to_execute, const std::list<std::string> &parameters) {
	std::string final_command = command;
	std::string parameters_as_single_string = "";
	int num_parameters = parameters.size();

	if(parameters.empty()) {
		final_command += "\r\n";
	}
	else
	{
		for(const auto &parameter : parameters) {

			num_parameters--;
			if(!num_parameters) {
				parameters_as_single_string += parameter + "\r\n";
			}
			else {
				parameters_as_single_string += parameter + ",";
			}
		}
	}

	final_command += parameters_as_single_string;
	SendAtMsgToQueue(final_command, command_to_execute, AtCommandTypes::Execute);
}

void ModemInterface::SendAtMsgToQueue(const std::string &raw_cmd, const ATCommands &command_to_execute, const AtCommandTypes &current_cmd_type) {

	struct CurrentCmd post_poned_cmd;
	post_poned_cmd.current_cmd = command_to_execute;
	post_poned_cmd.current_cmd_type = current_cmd_type;
	memcpy(post_poned_cmd.raw_msg, raw_cmd.c_str(), sizeof( post_poned_cmd.raw_msg ));
	
	queue_manager_->QueueSend(send_cmd_queue_, &post_poned_cmd, 100);
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
		case CSQ:
			return "AT+CSQ";
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
		case CGATT:
			return "AT+CGATT";
		case CSTT:
			return "AT+CSTT";
		case CIICR:
			return "AT+CIICR";
		case CIFSR:
			return "AT+CIFSR";
		default:
			break;
	}

	return "";
}

}
}
}
}

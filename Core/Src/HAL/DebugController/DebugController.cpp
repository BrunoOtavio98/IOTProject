/*
 * DebugController.cpp
 *
 *  Created on: 17 de fev de 2024
 *      Author: BrunoOtavio
 */

#include "DebugController.h"
#include "Devices/Communication/Interfaces/UartCommunicationInterface.h"
#include "RTOSWrappers/QueueWrapper.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <string.h>

using HAL::RtosWrappers::QueueWrapper;

namespace HAL {
namespace DebugController {

DebugController::DebugController(std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> uart_communication) :  
 TaskWrapper(std::string("DebugTask"), 500, nullptr, 2),
 DebugInterface("DebugController"),
 task_should_run_(true),
 uart_debug_(uart_communication),
 queue_manager_(std::make_shared<QueueWrapper>()),
 rx_buffer_pos_(0) {
 
 uart_debug_->ListenRxIT([this](const uint8_t *data, uint16_t size){CallbackUartMsgReceived(data, size);});
 debug_msgs_queue_ = queue_manager_->CreateQueue(50, sizeof(DataToLog));

 RegisterModuleToDebug(this);
 ChangeVerbosity(MessageVerbosity::INFO_MSG);

 PrintInfo(this, "Starting DebugController\n", false);
}

DebugController::~DebugController() {
}

void DebugController::Task(void *params) {

	DataToLog data_to_log;
	do 
	{

		if(queue_manager_->QueueReceive(debug_msgs_queue_, &data_to_log, 300)) {
			PrintMessage(data_to_log.msg_verbosity, data_to_log.module_name, data_to_log.msg);
		}

		if(CanProcessMessage()) {
			std::string str(reinterpret_cast<char*>(uart_buffer_receive_), rx_buffer_pos_);
			rx_buffer_pos_ = 0;
			DispatchMessage(str);
		}

		TaskDelay(100);
	} while(task_should_run_);
}

void DebugController::CallbackUartMsgReceived(const uint8_t *data, uint16_t size) 
{
	if(data == nullptr) {
		return;
	}

	if(size >= (kBufferSize - rx_buffer_pos_) ) {
		size = ((kBufferSize - rx_buffer_pos_) - 1);
	}

	is_callback_executing_ = true;
	std::memcpy(uart_buffer_receive_ + rx_buffer_pos_, data, size);
	rx_buffer_pos_ += size;
	is_callback_executing_ = false;
}

bool DebugController::CanProcessMessage() {
	return (((uart_buffer_receive_[rx_buffer_pos_ - 1] == '\n' || uart_buffer_receive_[rx_buffer_pos_ - 1] == '\r') ) 
			  && is_callback_executing_ == false);
}

void DebugController::RegisterModuleToDebug(DebugInterface *module) {
	if(module != nullptr) {
		list_of_modules_.push_back(module);
	}
}

void DebugController::PrintDebug(DebugInterface *module, const std::string &msg, bool from_isr) {

	if(!CheckIfModuleCanLog(module, DebugInterface::MessageVerbosity::DEBUG_MSG)) {
		return;
	}
	InsertMsgIntoQueue(DebugInterface::MessageVerbosity::DEBUG_MSG, module->GetModuleName(), msg, from_isr);
}

void DebugController::PrintInfo(DebugInterface *module, const std::string &msg, bool from_isr) {
	
	if(!CheckIfModuleCanLog(module, DebugInterface::MessageVerbosity::INFO_MSG)) {
		return;
	}
	InsertMsgIntoQueue(DebugInterface::MessageVerbosity::INFO_MSG, module->GetModuleName(), msg, from_isr);
}

void DebugController::PrintWarn(DebugInterface *module, const std::string &msg, bool from_isr) {
	
	if(!CheckIfModuleCanLog(module, DebugInterface::MessageVerbosity::WARN_MSG)) {
		return;
	}
	InsertMsgIntoQueue(DebugInterface::MessageVerbosity::WARN_MSG, module->GetModuleName(), msg, from_isr);
}

void DebugController::PrintError(DebugInterface *module, const std::string &msg, bool from_isr) {
	
	if(!CheckIfModuleCanLog(module, DebugInterface::MessageVerbosity::ERROR_MSG)) {
		return;
	}
	InsertMsgIntoQueue(DebugInterface::MessageVerbosity::ERROR_MSG, module->GetModuleName(), msg, from_isr);
}

void DebugController::InsertMsgIntoQueue(const DebugInterface::MessageVerbosity &msg_verbosity, const std::string &module, const std::string &message, bool from_isr) {
	DataToLog data_to_log;
	strncpy(data_to_log.module_name, module.c_str(), sizeof(data_to_log.module_name));
	strncpy(data_to_log.msg, message.c_str(), sizeof(data_to_log.msg));
	data_to_log.msg_verbosity = msg_verbosity;

	if(from_isr) {
	 	if( queue_manager_->QueueSendFromISR(debug_msgs_queue_, &data_to_log, 100) == false) {
			queue_manager_->QueueReset(debug_msgs_queue_);
			//PrintInfo(this, "Debug queue overflow, reseting queue\n", false);
		}
	}
	else {
		if(queue_manager_->QueueSend(debug_msgs_queue_, &data_to_log, 100) == false) {
			queue_manager_->QueueReset(debug_msgs_queue_);
			//PrintInfo(this, "Debug queue overflow, reseting queue\n", false);		
		}
	}
}

bool DebugController::CheckIfModuleCanLog(DebugInterface *module, const DebugInterface::MessageVerbosity &desired_verbosity) {

	if(module == nullptr) {
		return false;
	}

	auto it = std::find(list_of_modules_.begin(), list_of_modules_.end(), module);
	if(it != list_of_modules_.end()) {
		DebugInterface *module_interface = *it;
		if(desired_verbosity < module_interface->GetCurrentVerbosity()) {
			return false;
		}
	} else {
		return false;
	}

	return true;
}

std::string DebugController::MessageTypeToStr(const DebugInterface::MessageVerbosity &verbosity) {

	switch (verbosity) {
		case DebugInterface::MessageVerbosity::DEBUG_MSG:
			return "DBG";
		case DebugInterface::MessageVerbosity::ERROR_MSG:
				return "ERR";
		case DebugInterface::MessageVerbosity::INFO_MSG:
			return "INF";
		case DebugInterface::MessageVerbosity::WARN_MSG:
			return "WRN";
		default:
			return "";
	}
}

void DebugController::PrintMessage(const DebugInterface::MessageVerbosity &msg_verbosity, const std::string &module_name, const std::string &message) {
	std::string str_verbosity = MessageTypeToStr(msg_verbosity);
	uart_debug_->WriteData("[" + str_verbosity + "]" + module_name + ": " + message);
}

void DebugController::RegisterCallBackToReadMessages(std::function<void(const std::string&)> callback) {
	callbacks_.push_back(callback);
}

void DebugController::DispatchMessage(const std::string &message){
	for(const auto cb: callbacks_) {
		cb(message);
	}
}

}
}

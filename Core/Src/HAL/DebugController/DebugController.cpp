/*
 * DebugController.cpp
 *
 *  Created on: 17 de fev de 2024
 *      Author: BrunoOtavio
 */

#include "DebugController.h"
#include "Devices/Communication/Interfaces/UartCommunicationInterface.h"

#include <algorithm>

namespace HAL {
namespace DebugController {

DebugController::DebugController(std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> uart_communication) : uart_debug_(uart_communication) {
	uart_debug_->ListenRxIT([this](const std::string &msg){DispatchMessage(msg);});
}

DebugController::~DebugController() {

}

void DebugController::RegisterModuleToDebug(DebugInterface *module) {
	if(module != nullptr) {
		list_of_modules_.push_back(module);
	}
}


void DebugController::PrintDebug(DebugInterface *module, const std::string &msg) {

	if(!CheckIfModuleCanLog(module, DebugInterface::MessageVerbosity::DEBUG_MSG)) {
		return;
	}

	PrintMessage(DebugInterface::MessageVerbosity::DEBUG_MSG, module->GetModuleName(), msg);
}

void DebugController::PrintInfo(DebugInterface *module, const std::string &msg) {
	if(!CheckIfModuleCanLog(module, DebugInterface::MessageVerbosity::INFO_MSG)) {
		return;
	}

	PrintMessage(DebugInterface::MessageVerbosity::INFO_MSG, module->GetModuleName(), msg);
}

void DebugController::PrintWarn(DebugInterface *module, const std::string &msg) {
	if(!CheckIfModuleCanLog(module, DebugInterface::MessageVerbosity::WARN_MSG)) {
		return;
	}

	PrintMessage(DebugInterface::MessageVerbosity::WARN_MSG, module->GetModuleName(), msg);
}

void DebugController::PrintError(DebugInterface *module, const std::string &msg) {
	if(!CheckIfModuleCanLog(module, DebugInterface::MessageVerbosity::ERROR_MSG)) {
		return;
	}

	PrintMessage(DebugInterface::MessageVerbosity::ERROR_MSG, module->GetModuleName(), msg);
}

bool DebugController::CheckIfModuleCanLog(DebugInterface *module, const DebugInterface::MessageVerbosity &desired_verbosity) {

	if(module == nullptr) {
		return false;
	}

	auto it = std::find(list_of_modules_.begin(), list_of_modules_.end(), module);
	if(it != list_of_modules_.end()) {
		DebugInterface *module_interface = *it;
		if(desired_verbosity > module_interface->GetCurrentVerbosity()) {
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
	uart_debug_->WriteData("(" + str_verbosity + ")" + module_name + ": " + message);
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

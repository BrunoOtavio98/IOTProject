/*
 * DebugInterface.cpp
 *
 *  Created on: 17 de fev de 2024
 *      Author: BrunoOtavio
 */

#include "DebugInterface.h"

namespace HAL {
namespace DebugController {

DebugInterface::DebugInterface(const std::string module_name):current_verbosity_used_(MessageVerbosity::ERROR_MSG), module_name_(module_name) {

}

DebugInterface::~DebugInterface() {

}

void DebugInterface::ChangeVerbosity(const MessageVerbosity &new_verbosity) {
	current_verbosity_used_ = new_verbosity;
}

DebugInterface::MessageVerbosity DebugInterface::GetCurrentVerbosity() {
	return current_verbosity_used_;
}

std::string DebugInterface::GetModuleName() {
	return module_name_;
}

}
}

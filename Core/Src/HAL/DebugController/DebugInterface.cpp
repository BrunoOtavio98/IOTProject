/*
 * DebugInterface.cpp
 *
 *  Created on: 17 de fev de 2024
 *      Author: BrunoOtavio
 */

#include "DebugInterface.h"

namespace HAL {
namespace DebugController {

DebugInterface::DebugInterface(const std::string module_name): module_name_(module_name) {

}

DebugInterface::~DebugInterface() {

}

std::string DebugInterface::GetModuleName() {
	return module_name_;
}

}
}

/*
 * SIM7020E.cpp
 *
 *  Created on: 20 de jan de 2024
 *      Author: BrunoOtavio
 */

#include "SIM7020E.h"

using HAL::Devices::Communication::Interfaces::UartCommunicationInterface;;

namespace HAL {
namespace Devices {
namespace IOT {
namespace Modem {

SIM7020Modem::SIM7020Modem(const std::shared_ptr<UartCommunicationInterface> &uart_communication, 
                           const std::shared_ptr<HAL::DebugController::DebugController> &debug_controller) :
                           ModemInterface(uart_communication, debug_controller) {

}

SIM7020Modem::~SIM7020Modem() {

}

}
}
}
}

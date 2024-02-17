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

SIM7020Modem::SIM7020Modem(const std::shared_ptr<UartCommunicationInterface> &uart_communicatio) : ModemInterface(uart_communicatio) {

}

SIM7020Modem::~SIM7020Modem() {

}

}
}
}
}

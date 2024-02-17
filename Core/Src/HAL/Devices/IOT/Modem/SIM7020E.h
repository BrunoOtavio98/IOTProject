/*
 * SIM7020E.h
 *
 *  Created on: 20 de jan de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEVICES_IOT_MODEM_SIM7020E_H_
#define SRC_HAL_DEVICES_IOT_MODEM_SIM7020E_H_

#include "Devices/IOT/Interfaces/ModemInterface.h"

namespace HAL {
namespace Devices {
namespace IOT {
namespace Modem {

class SIM7020Modem : public HAL::Devices::IOT::Interfaces::ModemInterface {
public:
	SIM7020Modem(const std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> &uart_communicatio);
	~SIM7020Modem();
};

}
}
}
}

#endif /* SRC_HAL_DEVICES_IOT_MODEM_SIM7020E_H_ */

#ifndef SIM800L_H
#define SIM800L_H

#include "Devices/IOT/Interfaces/ModemInterface.h"
#include <string.h>

namespace HAL {
namespace Devices {
namespace IOT {
namespace Modem {

class SIM800LModem : public HAL::Devices::IOT::Interfaces::ModemInterface {
public:
    SIM800LModem(const std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> &uart_communication,
			const std::shared_ptr<HAL::DebugController::DebugController> &debug_controller);
    ~SIM800LModem();

private:
    bool GenericCmdResponse(const std::string &response);
};

}
}
}
}

#endif  // SIM800L_H
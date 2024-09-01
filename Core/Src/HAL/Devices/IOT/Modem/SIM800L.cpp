
#include "SIM800L.h"

using HAL::Devices::Communication::Interfaces::UartCommunicationInterface;;

namespace HAL {
namespace Devices {
namespace IOT {
namespace Modem {

SIM800LModem::SIM800LModem(const std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> &uart_communication,
				 const std::shared_ptr<HAL::DebugController::DebugController> &debug_controller) : ModemInterface(uart_communication, debug_controller) 
{

}

SIM800LModem::~SIM800LModem()
{

}

}
}
}
}
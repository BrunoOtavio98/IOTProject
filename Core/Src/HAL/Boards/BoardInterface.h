/*
 * BoardInterface.h
 *
 *  Created on: Jan 3, 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_BOARDINTERFACE_H_
#define SRC_HAL_BOARDINTERFACE_H_

#include <memory>
#include "Devices/Communication/Interfaces/UartCommunicationInterface.h"
#include "Devices/IOT/Interfaces/ModemInterface.h"

namespace HAL {
namespace DebugController {
class DebugController;
}
}

namespace HAL {
namespace RtosWrappers {
class TaskWrapperManager;
}
}

namespace HAL
{
namespace Devices
{
namespace Communication
{
namespace Interfaces 
{
	class SPIInterface;
}
}
}
}

namespace HAL 
{
namespace Devices
{
namespace Position
{
class GNSSInterface;
}
}
}


namespace HAL {
namespace Storage {
class StorageInterface;
}
}


namespace HAL {
namespace Boards {

class BoardInterface {
public:

	enum AvailableModemInterfaces {
		SIM_7020E
	};

	BoardInterface();
	virtual ~BoardInterface();
	virtual void InitPeripherals(AvailableModemInterfaces selected_modem) = 0;
protected:
	std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> modem_uart_communication_;
	std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> debug_uart_communication_;
	std::shared_ptr<HAL::Devices::Communication::Interfaces::UartCommunicationInterface> gnss_uart_communication_;
	std::shared_ptr<HAL::Devices::Communication::Interfaces::SPIInterface> sd_spi_communication_;

	std::unique_ptr<HAL::Devices::IOT::Interfaces::ModemInterface> modem_interface_;
	std::shared_ptr<HAL::Storage::StorageInterface> storage_interface_;
	std::shared_ptr<HAL::DebugController::DebugController> debug_controller_;
	std::shared_ptr<HAL::RtosWrappers::TaskWrapperManager> rtos_task_manager_;
	std::shared_ptr<HAL::Devices::Position::GNSSInterface> gnss_interface_;
};

}
}


#endif /* SRC_HAL_BOARDINTERFACE_H_ */

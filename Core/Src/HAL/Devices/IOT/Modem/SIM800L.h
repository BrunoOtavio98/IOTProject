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
    bool GenericCmdResponse(const std::string &response, const ATCommands &command_to_execute);
    bool CREGResponse(const std::string &response, const ATCommands &command);
    bool CSQRespoonse(const std::string &response, const ATCommands &command);
    bool COPSResponse(const std::string &response, const ATCommands &command);
    bool CGATTResponse(const std::string &response, const ATCommands &command);
    bool CSTTResponse(const std::string &response, const ATCommands &command);
    bool CIICRResponse(const std::string &response, const ATCommands &commmand);
    bool CIFSRResponse(const std::string &response, const ATCommands &command);    

    bool Connect(const std::string &apn, const std::string &username, const std::string &password) override;
    void OnLoop() override;
    void TestConnectionIsUp();

    bool connection_completed_;
    bool last_cmd_status_;
    uint8_t number_of_expected_responses_;
    uint8_t number_of_received_responses_;
};

}
}
}
}

#endif  // SIM800L_H
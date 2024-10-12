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
    const uint32_t kTimeToTestConnection;

    enum modem_register_state 
    {
        kDisable = 0,
        kEnable,
        kEnableWithLocation
    };

    enum register_status
    {
        kNotRegistered = 0,
        kRegistered,
        kSearching,
        kRegistrationDenied,
        kUnknown,
        kRoaming
    };

    typedef struct 
    {
        modem_register_state modem_state;
        register_status reg_status; 
        std::string location_area;
        std::string cell_id;
    } CRegResponse;

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

    void CheckResponses();

    bool connection_completed_;
    bool last_cmd_status_;
    uint8_t number_of_expected_responses_;
    uint8_t number_of_received_responses_;
    uint32_t time_control_;

    CRegResponse current_creg_rsp;
};

}
}
}
}

#endif  // SIM800L_H
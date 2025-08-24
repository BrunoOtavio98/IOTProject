#ifndef SIM800L_H
#define SIM800L_H

#include "Devices/IOT/Interfaces/ModemInterface.h"
#include <string.h>
#include <vector>
#include <stack>

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

    enum modem_cmd_state
    {
        kIdle = 0,
        kWaitingForResponse,
        kLastCommandExecuted,
        kError
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

    bool GenericCmdResponse(const std::string &response, const ATCommands &command_to_execute, const AtCommandTypes &command_type);
    bool CREGResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type);
    bool CSQRespoonse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type);
    bool COPSResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type);
    bool CGATTResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type);
    bool CSTTResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type);
    bool CIICRResponse(const std::string &response, const ATCommands &commmand, const AtCommandTypes &command_type);
    bool CIFSRResponse(const std::string &response, const ATCommands &command, const AtCommandTypes &command_type);    

    bool ValidateCommandResponse(const std::string& response, const ATCommands& command_to_execute);

    void ConnectStateMachine(const std::string &apn, const std::string &username, const std::string &password) override;
    void OnLoop() override;
    void TestConnectionIsUp();
    void KeepAliveControl();
    std::vector<std::string> SplitString(const std::string &message, char delimiter);

    ATCommands next_cmd_to_execute_;
    bool connection_completed_;
    uint32_t time_passed_keep_alive_;
    modem_cmd_state current_cmd_state_;
};

}
}
}
}

#endif  // SIM800L_H
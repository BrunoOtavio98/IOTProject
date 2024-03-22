#ifndef MOCK_DEBUG_INTERFACE_H
#define MOCK_DEBUG_INTERFACE_H

#include <gmock/gmock.h>
#include "DebugController/DebugInterface.h"

namespace HAL {
namespace DebugController {

class MockDebugInterface : public DebugInterface
{
public:
    MockDebugInterface() : DebugInterface(""){
    };

    MOCK_METHOD1(ChangeVerbosity, void(const MessageVerbosity&));
    MOCK_METHOD0(GetCurrentVerbosity, MessageVerbosity());
    MOCK_METHOD0(GetModuleName, std::string());
};

}
}

#endif // MOCK_DEBUG_INTERFACE_H
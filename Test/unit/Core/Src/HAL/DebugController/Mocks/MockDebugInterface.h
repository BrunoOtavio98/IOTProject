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

    MOCK_METHOD0(GetModuleName, std::string());
};

}
}

#endif // MOCK_DEBUG_INTERFACE_H
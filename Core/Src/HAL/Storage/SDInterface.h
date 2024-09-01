
#ifndef SRC_HAL_STORAGE_SDINTERFACE_H_
#define SRC_HAL_STORAGE_SDINTERFACE_H_

#include "Storage/StorageInterface.h"

namespace HAL {
namespace Storage {

class SDInterface : public HAL::Storage::StorageInterface
{
public:
    SDInterface() {

    }
    virtual ~SDInterface() {

    }
};

}
}

#endif // SRC_HAL_STORAGE_SDINTERFACE_H_

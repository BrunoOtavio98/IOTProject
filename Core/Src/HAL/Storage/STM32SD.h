
#ifndef SRC_HAL_STORAGE_STM32SD_H_
#define SRC_HAL_STORAGE_STM32SD_H_

#include "Storage/SDInterface.h"
#include "stm32f4xx_hal.h"

namespace HAL {
namespace Storage {

class STM32SD : public HAL::Storage::SDInterface
{
public:
    STM32SD();
    virtual ~STM32SD();

    bool InitStorage() override;

private:
 SD_HandleTypeDef hsd;

};
}
}

#endif // SRC_HAL_STORAGE_STM32SD_H_

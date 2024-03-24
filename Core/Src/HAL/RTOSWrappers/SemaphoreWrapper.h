
#ifndef SRC_HAL_RTOSWRAPPERS_QUEUEWRAPPER_H_
#define SRC_HAL_RTOSWRAPPERS_QUEUEWRAPPER_H_

#ifdef FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
typedef SemaphoreHandle_t GenericSemaphoreHandle;
#else
typedef int* GenericSemaphoreHandle;
#endif

namespace HAL {
namespace RtosWrappers {

class SemaphoreWrapper
{
public:
    SemaphoreWrapper();
    ~SemaphoreWrapper();
    GenericSemaphoreHandle CreateBinarySemaphore();
    GenericSemaphoreHandle CreateMutex();
    void DeleteSemaphore(GenericSemaphoreHandle semaphore);
    bool SemaphoreTake(GenericSemaphoreHandle semaphore, int ms_to_wait);
    bool SemaphoreGive(GenericSemaphoreHandle semaphore);

};
}
}

#endif /* SRC_HAL_RTOSWRAPPERS_QUEUEWRAPPER_H_ */

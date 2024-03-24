/*
 * QueueWrapper.h
 *
 *  Created on: Mar 23, 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_RTOSWRAPPERS_QUEUEWRAPPER_H_
#define SRC_HAL_RTOSWRAPPERS_QUEUEWRAPPER_H_

#include <memory>

#ifdef FREERTOS
#include "FreeRTOS.h"
#include "queue.h"
typedef QueueHandle_t GenericQueueHandle;
#else
typedef int* GenericQueueHandle;
#endif

namespace HAL {
namespace RtosWrappers {
class QueueWrapper
{
public:
    QueueWrapper();
    ~QueueWrapper();
    GenericQueueHandle CreateQueue(int size, int item_size);
    int QueueSpacesAvailable(GenericQueueHandle queue);
    void QueueDelete(GenericQueueHandle queue);
    bool QueueSend(GenericQueueHandle queue, const void *item, int ms_to_wait);
    bool QueueReceive(GenericQueueHandle queue, void *buffer, int ms_to_wait);
};

}
}

#endif /* SRC_HAL_RTOSWRAPPERS_QUEUEWRAPPER_H_ */

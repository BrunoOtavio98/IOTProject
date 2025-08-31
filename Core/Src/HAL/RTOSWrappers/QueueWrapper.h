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
    virtual GenericQueueHandle CreateQueue(int size, int item_size);
    virtual int QueueSpacesAvailable(GenericQueueHandle queue);
    virtual void QueueDelete(GenericQueueHandle queue);
    virtual bool QueueSend(GenericQueueHandle queue, const void *item, int ms_to_wait);
    virtual bool QueueSendFromISR(GenericQueueHandle queue, const void *item, int ms_to_wait);
    virtual bool QueueReceive(GenericQueueHandle queue, void *buffer, int ms_to_wait);
    virtual bool QueueReceiveFromISR(GenericQueueHandle queue, void *buffer, int ms_to_wait);
    virtual bool QueueReset(GenericQueueHandle queue);
};

}
}

#endif /* SRC_HAL_RTOSWRAPPERS_QUEUEWRAPPER_H_ */

#ifndef MOCK_QUEUEMANAGER_H_
#define MOCK_QUEUEMANAGER_H_

#include "RTOSWrappers/QueueWrapper.h"
#include <gmock/gmock.h>

namespace HAL {
namespace RtosWrappers {

class MockQueueManager : public HAL::RtosWrappers::QueueWrapper {
public:
    MOCK_METHOD(GenericQueueHandle, CreateQueue, (int size, int item_size), (override));
    MOCK_METHOD(int, QueueSpacesAvailable, (GenericQueueHandle queue), (override));
    MOCK_METHOD(void, QueueDelete, (GenericQueueHandle queue), (override));
    MOCK_METHOD(bool, QueueSend, (GenericQueueHandle queue, const void *item, int ms_to_wait), (override));
    MOCK_METHOD(bool, QueueSendFromISR, (GenericQueueHandle queue, const void *item, int ms_to_wait), (override));
    MOCK_METHOD(bool, QueueReceive, (GenericQueueHandle queue, void *buffer, int ms_to_wait), (override));
    MOCK_METHOD(bool, QueueReceiveFromISR, (GenericQueueHandle queue, void *buffer, int ms_to_wait), (override));
    MOCK_METHOD(bool, QueueReset, (GenericQueueHandle queue), (override));
};

}
}

#endif /* MOCK_QUEUEMANAGER_H_ */
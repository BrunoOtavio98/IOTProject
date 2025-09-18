/*
 * QueueWrapper.cpp
 *
 *  Created on: Mar 23, 2024
 *      Author: BrunoOtavio
 */
#include "QueueWrapper.h"

namespace HAL {
namespace RtosWrappers {

QueueWrapper::QueueWrapper() {

}

QueueWrapper::~QueueWrapper() {

}

GenericQueueHandle QueueWrapper::CreateQueue(int size, int item_size) {
    #ifdef FREERTOS
		return xQueueCreate(size, item_size);
    #endif
		return nullptr;
}

int QueueWrapper::QueueSpacesAvailable(GenericQueueHandle queue) {
#ifdef FREERTOS
	return uxQueueSpacesAvailable(queue);
#endif
	return 0;
}

void QueueWrapper::QueueDelete(GenericQueueHandle queue) {
#ifdef FREERTOS
	vQueueDelete(queue);
#endif
	return;
}

bool QueueWrapper::QueueSend(GenericQueueHandle queue, const void *item, int ms_to_wait) {
#ifdef FREERTOS
	return (xQueueSend(queue, item, pdMS_TO_TICKS(ms_to_wait)) == pdTRUE);
#endif
	return false;
}

bool QueueWrapper::QueueSendFromISR(GenericQueueHandle queue, const void *item, int ms_to_wait) {
#ifdef FREERTOS
	return xQueueSendFromISR(queue, item, pdFALSE);
#endif
	return false;
}	

bool QueueWrapper::QueueReceive(GenericQueueHandle queue, void *buffer, int ms_to_wait) {
#ifdef FREERTOS
	return (xQueueReceive(queue, buffer, pdMS_TO_TICKS(ms_to_wait)) == pdTRUE);
#endif
	return false;
}

bool QueueWrapper::QueueReceiveFromISR(GenericQueueHandle queue, void *buffer, int ms_to_wait) {
#ifdef FREERTOS
	return (xQueueReceiveFromISR(queue, buffer, pdFALSE));
#endif
	return false;
}

bool QueueWrapper::QueueReset(GenericQueueHandle queue) {
#ifdef FREERTOS
	return (xQueueReset(queue) == pdTRUE);	
#endif
	return false;
}

}
}

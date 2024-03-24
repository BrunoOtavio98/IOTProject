/*
 * SemaphoreWrapper.cpp
 *
 *  Created on: Mar 23, 2024
 *      Author: BrunoOtavio
 */

#include "SemaphoreWrapper.h"

namespace HAL {
namespace RtosWrappers {

SemaphoreWrapper::SemaphoreWrapper() {

}

SemaphoreWrapper::~SemaphoreWrapper() {

}

GenericSemaphoreHandle SemaphoreWrapper::CreateBinarySemaphore() {
#ifdef FREERTOS
	return xSemaphoreCreateBinary();
#endif
	return nullptr;
}

GenericSemaphoreHandle SemaphoreWrapper::CreateMutex() {
#ifdef FREERTOS
	return xSemaphoreCreateMutex();
#endif
	return nullptr;
}

void SemaphoreWrapper::DeleteSemaphore(GenericSemaphoreHandle semaphore) {
#ifdef FREERTOS
	vSemaphoreDelete(semaphore);
#endif
	return;
}

bool SemaphoreWrapper::SemaphoreTake(GenericSemaphoreHandle semaphore, int ms_to_wait) {
#ifdef FREERTOS
	return (xSemaphoreTake(semaphore, pdMS_TO_TICKS(ms_to_wait)) == pdTRUE);
#endif
	return false;
}

bool SemaphoreWrapper::SemaphoreGive(GenericSemaphoreHandle semaphore) {
#ifdef FREERTOS
	return (xSemaphoreGive(semaphore) == pdTRUE);
#endif
	return false;
}

}
}

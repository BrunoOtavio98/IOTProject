
#include "TaskWrapper.h"

namespace HAL {
namespace RtosWrappers {

TaskWrapper::TaskWrapper(const std::string &task_name, uint16_t stack_size, void *const parameters, int priority)
                        : parameters_(parameters), name_(task_name), stack_size_(stack_size), priority_(priority) {

}

TaskWrapper::~TaskWrapper() {

}

std::string TaskWrapper::GetTaskname() {
    return name_;
}

uint16_t TaskWrapper::GetStackSize() {
    return stack_size_;
}

int TaskWrapper::GetPriority() {
    return priority_;
}

void TaskWrapper::TaskDelay(int delay_ms) {
#ifdef FREERTOS
    const TickType_t xDelay = delay_ms / portTICK_PERIOD_MS;
    vTaskDelay( xDelay );
#endif
}

void TaskWrapper::ToStaticTask( void *this_task_wrapper) {

	auto task_wrapper = static_cast<TaskWrapper *>(this_task_wrapper);

	if(task_wrapper != nullptr) {
		task_wrapper->Task(task_wrapper->parameters_);
	}
}

}
}

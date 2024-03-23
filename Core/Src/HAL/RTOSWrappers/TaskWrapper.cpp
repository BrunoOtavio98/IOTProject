
#include "TaskWrapper.h"

namespace HAL {
namespace RtosWrappers {

TaskWrapper::TaskWrapper(CallBackType task, const std::string &task_name, uint16_t stack_size, void *const parameters, int priority) 
                        : task_cb_(task), name_(task_name), stack_size_(stack_size), parameters_(parameters) {

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

}
}
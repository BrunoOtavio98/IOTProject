#ifndef SRC_HAL_RTOSWRAPPERS_TASKWRAPPER_H_
#define SRC_HAL_RTOSWRAPPERS_TASKWRAPPER_H_

#ifdef FREERTOS
#include "FreeRTOS.h"
#include "task.h"
typedef TaskHandle_t GenericTaskHandle;
#else
typedef int GenericTaskHandle;
#endif

#include <string>

typedef void (*CallBackType)(void *);

namespace HAL {
namespace RtosWrappers {
class TaskWrapper
{
public:
    TaskWrapper(CallBackType task, const std::string &task_name, uint16_t stack_size, void *const parameters, int priority);
    ~TaskWrapper();
    GenericTaskHandle task_handle_;
    CallBackType task_cb_;
    void *const parameters_;

    std::string GetTaskname();
    uint16_t GetStackSize();
    int GetPriority();

private:
    std::string name_;
    uint16_t stack_size_;
    int priority_;
};

}
}

#endif

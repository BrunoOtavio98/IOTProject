#ifndef SRC_HAL_RTOSWRAPPERS_TASKWRAPPER_H_
#define SRC_HAL_RTOSWRAPPERS_TASKWRAPPER_H_

#ifdef FREERTOS
#include "FreeRTOS.h"
#include "task.h"
typedef TaskHandle_t GenericTaskHandle;
#else
typedef int GenericTaskHandle;
#endif

typedef void (*CallBackType)(void*);

#include <string>

namespace HAL {
namespace RtosWrappers {
class TaskWrapper
{
public:
    TaskWrapper(const std::string &task_name, uint16_t stack_size, void *const parameters, int priority);
    ~TaskWrapper();
    GenericTaskHandle task_handle_;
    virtual void Task(void *params) {

    }

    void *const parameters_;

    std::string GetTaskname();
    uint16_t GetStackSize();
    int GetPriority();
    void RegisterCallback(CallBackType task);
    static void ToStaticTask( void *this_task_wrapper);

private:
    std::string name_;
    uint16_t stack_size_;
    int priority_;
};

}
}

#endif

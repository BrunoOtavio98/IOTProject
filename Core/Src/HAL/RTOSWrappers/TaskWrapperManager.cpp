
#include <RTOSWrappers/TaskWrapperManager.h>

namespace HAL {
namespace RtosWrappers {

TaskWrapperManager::TaskWrapperManager() {
    
}

TaskWrapperManager::~TaskWrapperManager() {

}

bool TaskWrapperManager::CreateTask(TaskWrapper &task)
{

#ifdef FREERTOS
	TaskHandle_t task_handle_freertos = reinterpret_cast<TaskHandle_t>(task.task_handle_);

	if(task_handle_freertos != nullptr) {
		if(xTaskCreate(task.ToStaticTask, task.GetTaskname().c_str(), task.GetStackSize(), &task, task.GetPriority(), &task_handle_freertos) == pdPASS) {
			return true;
		} else {
			return false;
		}

	} else {
		return false;
	}
#endif

    return true;
}

bool TaskWrapperManager::DeleteTask(TaskWrapper &task) {

#ifdef FREERTOS
	vTaskDelete(task.task_handle_);
#endif

	return true;
}

void TaskWrapperManager::TaskDelay(TaskWrapper &task, int delay_ms) {
#ifdef FREERTOS
    const TickType_t xDelay = delay_ms / portTICK_PERIOD_MS;
    vTaskDelay( xDelay );
#endif
}

}
}


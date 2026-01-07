
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
	TaskHandle_t created_task_handle = nullptr;

	if(xTaskCreate(task.ToStaticTask, task.GetTaskname().c_str(), task.GetStackSize(), &task, task.GetPriority(), &created_task_handle) == pdPASS)
	{	
		task.task_handle_ = reinterpret_cast<GenericTaskHandle>(created_task_handle);
		return true;
	} 
	else 
	{
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


}
}


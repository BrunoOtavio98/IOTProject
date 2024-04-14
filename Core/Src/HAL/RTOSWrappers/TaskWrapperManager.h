/*
 * TaskWrapper.h
 *
 *  Created on: Mar 23, 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_RTOSWRAPPERS_TASKWRAPPERMANAGER_H_
#define SRC_HAL_RTOSWRAPPERS_TASKWRAPPERMANAGER_H_

#include <functional>
#include <string>
#include <map>
#include <memory>

#include "TaskWrapper.h"

namespace HAL {
namespace RtosWrappers {
class TaskWrapperManager
{
public:
	TaskWrapperManager();
    ~TaskWrapperManager();

   bool CreateTask(TaskWrapper &task);
   bool DeleteTask(TaskWrapper &task);
};

}
}
#endif /* SRC_HAL_RTOSWRAPPERS_TASKWRAPPERMANAGER_H_ */

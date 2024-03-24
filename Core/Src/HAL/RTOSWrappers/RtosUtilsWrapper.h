/*
 * RtosUtilsWrapper.h
 *
 *  Created on: 24 de mar de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_RTOSWRAPPERS_RTOSUTILSWRAPPER_H_
#define SRC_HAL_RTOSWRAPPERS_RTOSUTILSWRAPPER_H_

#include <memory>

#include "QueueWrapper.h"
#include "SemaphoreWrapper.h"

namespace HAL {
namespace RtosWrappers {

class RtosUtilsWrapper
{
public:
    RtosUtilsWrapper() {

    }

    ~RtosUtilsWrapper() {

    }

    std::shared_ptr<QueueWrapper> queue_manager_;
};
}
}

#endif /* SRC_HAL_RTOSWRAPPERS_RTOSUTILSWRAPPER_H_ */

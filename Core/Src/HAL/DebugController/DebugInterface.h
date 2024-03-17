/*
 * DebugInterface.h
 *
 *  Created on: 17 de fev de 2024
 *      Author: BrunoOtavio
 */

#ifndef SRC_HAL_DEBUGCONTROLLER_DEBUGINTERFACE_H_
#define SRC_HAL_DEBUGCONTROLLER_DEBUGINTERFACE_H_

#include <string>

namespace HAL {
namespace DebugController {

class DebugInterface {
public:
  enum MessageVerbosity {
	DEBUG_MSG = 0,
	ERROR_MSG = 1,
	WARN_MSG,
	INFO_MSG,
  };

 DebugInterface(const std::string module_name);
 virtual ~DebugInterface();

virtual void ChangeVerbosity(const MessageVerbosity &new_verbosity);
virtual MessageVerbosity GetCurrentVerbosity();
virtual std::string GetModuleName();

private:
 MessageVerbosity current_verbosity_used_;
 std::string module_name_;

};

}
}

#endif /* SRC_HAL_DEBUGCONTROLLER_DEBUGINTERFACE_H_ */

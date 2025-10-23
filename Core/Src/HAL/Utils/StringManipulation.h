
#ifndef SRC_HAL_UTILS_STRINGMANIPULATION_H_
#define SRC_HAL_UTILS_STRINGMANIPULATION_H_

#include <string>
#include <vector>

namespace HAL
{
namespace Utils 
{

class StringManipulation 
{

public:
    StringManipulation() = default;
    ~StringManipulation() = default;

    std::vector<std::string> SplitString(const std::string &message, char delimiter);
    std::string RemoveSubstring(const std::string &message, const std::string &substring);
};

}
}

#endif /* SRC_HAL_UTILS_STRINGMANIPULATION_H_ */
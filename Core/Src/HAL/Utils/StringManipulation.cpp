#include "StringManipulation.h" 
#include <sstream>

namespace HAL
{
namespace Utils 
{

std::vector<std::string> StringManipulation::SplitString(const std::string &message, char delimiter) 
{
	std::vector<std::string> tokens;
    std::stringstream ss(message);
    std::string token;

    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

std::string StringManipulation::RemoveSubstring(const std::string &message, const std::string &substring) 
{
	std::string result = message;
	size_t pos = result.find(substring);
	if (pos != std::string::npos)
    {
		result.erase(pos, substring.length());
	}
	return result;
}

}
}
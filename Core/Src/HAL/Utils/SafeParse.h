#ifndef SAFE_PARSE_H
#define SAFE_PARSE_H

#include <string>

namespace HAL
{
namespace Utils 
{

class SafeParse
{
    public: 
        SafeParse();
        ~SafeParse();

        static bool ToFloat(const std::string& str, float& out)
        {
            if (str.empty())
                return false;

            char* endptr;
            out = std::strtof(str.c_str(), &endptr);

            return (endptr != str.c_str() && *endptr == '\0');
        }

        static bool ToInt(const std::string& str, int& out)
        {
            if (str.empty())
                return false;

            char* endptr;
            long value = std::strtol(str.c_str(), &endptr, 10);

            if (endptr == str.c_str() || *endptr != '\0')
                return false;

            out = static_cast<int>(value);
            return true;
        }

        static bool ToUint8t(const std::string& str, uint8_t& out)
        {
            if (str.empty())
                return false;

            char* endptr;
            long value = std::strtol(str.c_str(), &endptr, 10);

            if (endptr == str.c_str() || *endptr != '\0')
                return false;

            if (value < 0 || value > 255)
                return false;

            out = static_cast<uint8_t>(value);
            return true;
        }

        static bool ToUint8tHex(const std::string& str, uint8_t& out)
        {
            if (str.empty())
                return false;

            char* endptr;
            long value = std::strtol(str.c_str(), &endptr, 16);

            if (endptr == str.c_str() || *endptr != '\0')
                return false;

            if (value < 0 || value > 255)
                return false;

            out = static_cast<uint8_t>(value);
            return true;
        }

};


}
}

#endif // SAFE_PARSE_H

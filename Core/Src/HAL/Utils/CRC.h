#ifndef CRC_H
#define CRC_H

#include <cstdint>

namespace HAL
{
namespace Utils 
{

class CRC
{
public:
    CRC();
    ~CRC();

    static uint8_t CRC7(const uint8_t *data, uint16_t data_length)
    {
        uint8_t crc = 0; 
        for (size_t i = 0; i < data_length; i++) 
        {
            uint8_t current = data[i];
            for (int bit = 0; bit < 8; bit++) 
            {
                crc <<= 1;
                if (((current & 0x80) ^ (crc & 0x80)) != 0) 
                { 
                    crc ^= 0x09;
                } 
                current <<= 1; 
            } 
        }

        return (crc & 0x7F);
    }
};

}
}

#endif // CRC_H
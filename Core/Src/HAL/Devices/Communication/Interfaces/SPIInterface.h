#ifndef SPI_INTERFACE_H
#define SPI_INTERFACE_H

#include <cstdint>
#include <functional>

namespace HAL 
{

namespace Devices 
{

namespace Communication 
{
    
namespace Interfaces 
{

class SPIInterface
{
public:

    enum SPINumber
    {
        SPI_1,
        SPI_2,
        SPI_3,
        SPI_4
    };

    enum SPIMode
    {
        Master,
        Slave
    };

    enum SPIDataSize
    {
        SPI_8Bits,
        SPI_16Bits
    };

    enum SPITimmingMode
    {
        CPOL0_CPHA0,
        CPOL0_CPHA1,
        CPOL1_CPHA0,
        CPOL1_CPHA1
    };

    enum SPIBaudRatePrescaler
    {
        BaudRatePrescaler_2,
        BaudratePrescaler_4,
        BaudratePrescaler_8,
        BaudratePrescaler_16,
        BaudratePrescaler_32,
        BaudratePrescaler_64
    };

    typedef struct
    {   
        SPINumber spi_number;
        SPIMode spi_mode;
        SPIDataSize spi_data_size;
        SPITimmingMode spi_timming_mode;
        SPIBaudRatePrescaler spi_baud_selector;
    } SPIConfiguration;

    SPIInterface( const SPIConfiguration &spi_config )
    {

    }

    ~SPIInterface()
    {

    }

    virtual bool WriteData( const uint8_t *data, uint16_t data_size ) = 0;
    virtual bool WriteDataIT( const uint8_t *data, uint16_t data_size, std::function<void(void)> callback_write_finish ) = 0;
    virtual bool ReadData( uint8_t *read_buffer, uint16_t data_size ) = 0;
    virtual bool ReadDataIT(  uint8_t *read_buffer, uint16_t data_size, std::function<void(void)> callback_read_finish ) = 0;
    virtual bool WriteReadData( const uint8_t *data_write, uint8_t *data_read, uint16_t data_size ) = 0;
    virtual bool SetCSPin( uint8_t pin_value ) = 0;

};

}

}

}

}

#endif // SPI_INTERFACE_H
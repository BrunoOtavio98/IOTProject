

#ifndef SRC_HAL_STORAGE_STORAGEINTERFACE_H_
#define SRC_HAL_STORAGE_STORAGEINTERFACE_H_

#include <memory>

namespace HAL 
{
namespace Devices 
{
namespace Communication 
{    
namespace Interfaces 
{
class SPIInterface;
}
}
}
}

namespace HAL 
{
namespace Storage 
{

class StorageInterface
{
public:
    StorageInterface(std::shared_ptr<HAL::Devices::Communication::Interfaces::SPIInterface> spi_communication) :
        spi_communication_(spi_communication)
    {

    }

    virtual ~StorageInterface()
    {

    }

    virtual bool InitStorage() 
    {
        return false;
    }

    virtual uint16_t ReadData( uint32_t address, uint8_t *buffer_read, uint16_t buffer_size )
    {
        return 0;
    }

    virtual uint16_t WriteData( uint32_t address, uint8_t *buffer_write, uint16_t buffer_size )
    {
        return 0;
    }

protected:
    std::shared_ptr<HAL::Devices::Communication::Interfaces::SPIInterface> spi_communication_;

};

}
}

#endif // SRC_HAL_STORAGE_STORAGEINTERFACE_H_



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

protected:
    std::shared_ptr<HAL::Devices::Communication::Interfaces::SPIInterface> spi_communication_;

};

}
}

#endif // SRC_HAL_STORAGE_STORAGEINTERFACE_H_

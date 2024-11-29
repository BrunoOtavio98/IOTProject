

#ifndef SRC_HAL_STORAGE_STORAGEINTERFACE_H_
#define SRC_HAL_STORAGE_STORAGEINTERFACE_H_

namespace HAL {
namespace Storage {

class StorageInterface
{
public:
    StorageInterface() {

    }
    virtual ~StorageInterface() {

    }

    virtual bool InitStorage() {
        return false;
    }
};

}
}

#endif // SRC_HAL_STORAGE_STORAGEINTERFACE_H_

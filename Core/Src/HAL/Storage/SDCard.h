
#ifndef SRC_HAL_STORAGE_SDCARD_H_
#define SRC_HAL_STORAGE_SDCARD_H_

#include "Storage/StorageInterface.h"
#include "RTOSWrappers/TaskWrapper.h"

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

class SDCard : public HAL::Storage::StorageInterface,
               public HAL::RtosWrappers::TaskWrapper
{
public:
    static constexpr int kMaxReponseSizeBytes = 5;
    static constexpr uint8_t kR1BResponse = 0xFE;

    SDCard(std::shared_ptr<HAL::Devices::Communication::Interfaces::SPIInterface> spi_communication);

    virtual ~SDCard();

protected:

    const uint32_t CCS_MASK = 0x01 << 30;

    enum SDCommand
    {
        CMD0,
        CMD1,
        ACMD41 = 41,
        CMD8 = 8,
        CMD9 = 9,
        CMD10 = 10,
        CMD12 = 12,
        CMD16 = 16,
        CMD17 = 17,
        CMD18 = 18,
        CMD23 = 23,
        ACMD23 = 23,
        CMD24 = 24,
        CMD25 = 25,
        CMD55 = 55,
        CMD58 = 58
    };

    enum SDResponseMask
    {
        IdleState       = 1 << 0,
        EraseReset      = 1 << 1,
        IlligalCommand  = 1 << 2,
        CmdCrcError     = 1 << 3,
        EraseSeqError   = 1 << 4,
        AddrError       = 1 << 5,
        ParamError      = 1 << 6
    };

    enum SDCardVersion
    {
        SDVer2_BlockAddr,
        SDVer2_ByteAddr,
        SDVer1,
        SDInvalid
    };

    bool InitStorage() override;
    void Task(void *params) override;
    bool SendCommand( SDCommand cmd, uint32_t argument, uint8_t *response, uint16_t response_buffer_size, bool crc_enabled );
    uint8_t GetCmdResponseSizeBytes( SDCommand cmd );
    bool GetStartValidByteFromBuffer( uint8_t *index_out, uint8_t *buffer, uint16_t buffer_size );
    bool isAnApplicationCommand( SDCommand cmd );

    constexpr bool ResposeFlagSet( uint8_t response, SDResponseMask mask )
    {
        return ( response & mask );
    }

    bool SdCardInitialized;
    SDCardVersion SdCurrentVersion;
};

}
}

#endif // SRC_HAL_STORAGE_SDCARD_H_

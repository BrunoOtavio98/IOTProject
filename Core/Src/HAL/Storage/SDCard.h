
#ifndef SRC_HAL_STORAGE_SDCARD_H_
#define SRC_HAL_STORAGE_SDCARD_H_

#include <memory>

#include "Storage/StorageInterface.h"
#include "RTOSWrappers/TaskWrapper.h"
#include "DebugController/DebugInterface.h"

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
namespace DebugController 
{
class DebugController;
}
}

namespace HAL 
{
namespace Storage 
{

class SDCard : public HAL::Storage::StorageInterface,
               public HAL::RtosWrappers::TaskWrapper,
               public HAL::DebugController::DebugInterface
{
public:

    SDCard( const std::shared_ptr<HAL::Devices::Communication::Interfaces::SPIInterface> &spi_communication, 
            const std::shared_ptr<HAL::DebugController::DebugController> &debug_controler );

    virtual ~SDCard();

protected:

    const uint32_t CCS_MASK = 0x01 << 30;
    static constexpr int kMaxReponseSizeBytes = 5;
    static constexpr uint16_t kMaxBlockLen = 512;
    static constexpr uint8_t kNumberClocksTimeout = 60;

    static constexpr uint8_t kStartBlockToken = 0xFE;
    static constexpr uint8_t kStartMultiBlockWriteToken = 0xFC;
    static constexpr uint8_t kStopMultiBlockWriteToken = 0xFD;
    static constexpr uint8_t kDataResponseTokenAccepted = 0x2;

    enum SDCommand
    {
        CMD0,
        CMD1,
        ACMD41 = 41,
        CMD8 = 8,
        CMD9 = 9,
        CMD10 = 10,
        CMD12 = 12,
        CMD13 = 13,
        CMD16 = 16,
        CMD17 = 17,
        CMD18 = 18,
        CMD23 = 23,
        ACMD23 = 23,
        CMD24 = 24,
        CMD25 = 25,
        CMD32 = 32,
        CMD33 = 33,
        CMD38 = 38,
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
    uint16_t ReadData( uint32_t address, uint8_t *buffer_read, uint16_t buffer_size ) override;
    uint16_t WriteData( uint32_t address, uint8_t *buffer_write, uint16_t buffer_size ) override;
    bool EraseRange( uint32_t start_address, uint32_t end_address ) override;

    uint16_t ReadSingleBlock( uint32_t address, uint8_t *buffer_read, uint16_t buffer_size );
    uint16_t ReadMultipleBlocks( uint32_t address, uint8_t *buffer_read, uint16_t buffer_size );

    uint16_t WriteSingleBlock( uint32_t address, uint8_t *buffer_write, uint16_t buffer_size );
    uint16_t WriteMultipleBlocks( uint32_t address, uint8_t *buffer_write, uint16_t buffer_size );

    bool SendCommand( SDCommand cmd, uint32_t argument, uint8_t *response, uint16_t response_buffer_size, bool crc_enabled );
    uint8_t GetCmdResponseSizeBytes( SDCommand cmd );
    bool GetStartValidByteFromBuffer( uint8_t *index_out, uint8_t *buffer, uint16_t buffer_size );
    bool isAnApplicationCommand( SDCommand cmd );
    bool ErrorTokenReturned( uint8_t token );
    bool BuildSDCommand( SDCommand cmd, uint32_t argument, uint8_t *buffer, uint16_t buffer_size, bool crc_enabled );
    bool WaitForBusyLine( uint8_t *r1response, uint16_t attempts );
    bool IsR1BResponse( SDCommand cmd );

    constexpr bool ResposeFlagSet( uint8_t response, SDResponseMask mask )
    {
        return ( response & mask );
    }

    bool SdCardInitialized;
    SDCardVersion SdCurrentVersion;
    std::shared_ptr<HAL::DebugController::DebugController> debug_controler_;
    uint16_t block_len_;
};

}
}

#endif // SRC_HAL_STORAGE_SDCARD_H_

#include "Storage/SDCard.h"
#include "Devices/Communication/Interfaces/SPIInterface.h"
#include "Utils/CRC.h"
#include "DebugController/DebugController.h"
#include <string.h>

using HAL::Utils::CRC;
using HAL::Devices::Communication::Interfaces::SPIInterface;
using HAL::RtosWrappers::TaskWrapper;
using HAL::DebugController::DebugInterface;

namespace HAL 
{
namespace Storage 
{

SDCard::SDCard( const std::shared_ptr<HAL::Devices::Communication::Interfaces::SPIInterface> &spi_communication,
                const std::shared_ptr<HAL::DebugController::DebugController> &debug_controler  ) : 
                StorageInterface(spi_communication),
                TaskWrapper("SDCard", 2000, nullptr, 2),
                DebugInterface("SDCard"),
                SdCurrentVersion(SDCardVersion::SDInvalid),
                debug_controler_(debug_controler),
                block_len_(kMaxBlockLen)
{
    debug_controler_->RegisterModuleToDebug(this);
}

SDCard::~SDCard()
{
}

uint8_t buffer_write[512];
uint8_t attempts = 1;
uint8_t current_cycle = 0;
void SDCard::Task(void *params)
{   
    memset(buffer_write, 0xFF, sizeof(buffer_write));
    for(int i = 0; i<50; i++)
    {
        buffer_write[i] = i;
    }

    while(1)
    {
        if( SdCurrentVersion == SDCardVersion::SDInvalid )
        {   
            debug_controler_->PrintDebug(this, "Running InitStorage\n", true);
            InitStorage();
        }
        else
        {   
            uint8_t buffer_read[kMaxBlockLen] = {0};

            debug_controler_->PrintDebug(this, "Testing multiple blocks cycles\n", true);

            if(current_cycle < attempts)
            {
                debug_controler_->PrintDebug(this, "New cycle attempt\n", true);
                {
                    WriteData( 0, buffer_write, sizeof(buffer_write) );
                    ReadData( 0, buffer_read, sizeof(buffer_read) );
                }
                current_cycle++;
            }
            else
            {
                debug_controler_->PrintDebug(this, "Number of cycles finished\n", true);
            }
        }

        TaskDelay(100);
    }
}

uint8_t SDCard::GetCmdResponseSizeBytes( SDCommand cmd )
{
    uint8_t response_size_bytes = 1;

    if( cmd == SDCommand::CMD8 || cmd == SDCommand::CMD58 )
    {
        response_size_bytes = 5;
    }

    else if( cmd == SDCommand::CMD13 )
    {
        response_size_bytes = 2;
    }

    else if( cmd == SDCommand::CMD12 )
    {
        response_size_bytes = kR1BResponse;
    }

    return response_size_bytes;
}

bool SDCard::GetStartValidByteFromBuffer( uint8_t *index_out, uint8_t *buffer, uint16_t buffer_size )
{
    for( int i = 0; i<buffer_size; i++ )
    {
        if( buffer[i] != 0xFF )
        {
            *index_out = i;
            return true;
        }
    }

    return false;
}

bool SDCard::ErrorTokenReturned( uint8_t token )
{    
    if( token == kStartBlockToken || 
        token == kStartMultiBlockWriteToken || 
        token == kStopMultiBlockWriteToken )
    {
        return false;
    }

    return true;
}

bool SDCard::BuildSDCommand( SDCommand cmd, uint32_t argument, uint8_t *buffer, uint16_t buffer_size, bool crc_enabled )
{
    uint8_t crc = 0;
    if( buffer == nullptr || buffer_size < 6 )
    {
        return false;
    }

    memset(buffer, 0, 6);

    buffer[0] = 0x40 | cmd;
    buffer[1] = (argument >> 24) & 0xFF;
    buffer[2] = (argument >> 16) & 0xFF;
    buffer[3] = (argument >> 8) & 0xFF;
    buffer[4] =  argument & 0xFF;

    if( crc_enabled )
    {
        crc = CRC::CRC7(&buffer[0], 5);
        crc = crc << 1 | 1;
    }
    memcpy( &buffer[5], &crc, sizeof(crc));
    return true;
}

bool SDCard::InitStorage() 
{
    bool status = true;
    uint8_t response_buffer[5] = {0};
    uint8_t write_buffer[100] = {0};
    uint32_t argument = 0x0;

    do
    {
        TaskDelay(2);

        memset(write_buffer, 0xFF, sizeof(write_buffer));
        spi_communication_->SetCSPin(1);
        spi_communication_->WriteData( write_buffer, 75 );

        status = SendCommand( SDCommand::CMD0, argument, response_buffer, sizeof(response_buffer), true );
        if( status == false )
        {   
            debug_controler_->PrintError(this, "Failed to send first CMD0\n", true);
            break;
        }

        if( response_buffer[0] != SDResponseMask::IdleState )
        {
            debug_controler_->PrintError(this, "SD card not in IDLE\n", true);
            status = false;
            break;
        }

        argument = 0x1AA;
        status = SendCommand( SDCommand::CMD8, argument, response_buffer, sizeof(response_buffer), true );

        uint16_t rsp_echo = response_buffer[4];
        rsp_echo = rsp_echo | (uint16_t)( (response_buffer[3] & 0xF) << 8);

        // If error or no response
        if( status == false || response_buffer[0] > SDResponseMask::IdleState )
        {
            bool is_sd_busy = true;
            do
            {
                status = SendCommand( SDCommand::CMD55, 0x00, response_buffer, sizeof(response_buffer), false );
                if( status == false )
                {   
                    debug_controler_->PrintError(this, "Failed on CMD55\n", true);
                    break;
                }

                argument = 0x0;
                status = SendCommand( SDCommand::ACMD41, argument, response_buffer, sizeof(response_buffer), false );
                if( status == false )
                {   
                    debug_controler_->PrintError(this, "Failed to send ACMD41\n", true);
                    break;
                }

                is_sd_busy = (response_buffer[0] & SDResponseMask::IdleState);
                TaskDelay(1);
            } while( is_sd_busy );

            if( status == false) 
            {   
                debug_controler_->PrintError(this, "Failed on loop for ACMD41\n", true);
                break;
            }

            SdCurrentVersion = SDCardVersion::SDVer1;
        }
        else if( rsp_echo == 0x1AA )
        {            
            status = SendCommand( SDCommand::CMD55, 0x00, response_buffer, sizeof(response_buffer), false );
            if( status == false )
            {   
                debug_controler_->PrintError(this, "Failed on CMD55\n", true);
                break;
            }

            argument = (0x01 << 30);
            status = SendCommand( SDCommand::ACMD41, argument, response_buffer, sizeof(response_buffer), false );
            if( status == false || response_buffer[0] > 0x01 )
            {   
                debug_controler_->PrintError(this, "Failed to send ACMD41\n", true);
                status = false;
                break;
            }

            while( response_buffer[0] == 0x01 )
            {
                status = SendCommand( SDCommand::CMD55, 0x00, response_buffer, sizeof(response_buffer), false );
                if( status == false )
                {   
                    debug_controler_->PrintError(this, "Failed on CMD55\n", true);
                    break;
                }

                status = SendCommand( SDCommand::ACMD41, argument, response_buffer, sizeof(response_buffer), false );
                if( status == false || response_buffer[0] > 0x01 )
                {   
                    debug_controler_->PrintError(this, "Failed to send ACMD41 on loop\n", true);
                    status = false;
                    break;
                }
                TaskDelay(1);
            }

            if( status == false )
            {   
                debug_controler_->PrintError(this, "Failed on loop\n", true);
                break;
            }

            argument = 0x00;
            status = SendCommand( SDCommand::CMD58, argument, response_buffer, sizeof(response_buffer), false );
            if( status == false || response_buffer[4] != 0x00 )
            {   
                debug_controler_->PrintError(this, "Failed to send CMD58\n", true);
                status = false;
                break;
            }

            uint32_t OCR =
                (static_cast<uint32_t>(response_buffer[1]) << 24) |
                (static_cast<uint32_t>(response_buffer[2]) << 16) |
                (static_cast<uint32_t>(response_buffer[3]) << 8)  |
                 static_cast<uint32_t>(response_buffer[4]);

            uint8_t ccs_set = (OCR & CCS_MASK) >> 30;

            if(ccs_set)
            {
                debug_controler_->PrintDebug(this, "Success on Init\n", true);
                debug_controler_->PrintDebug(this, "SDVER BlockAddr\n", true);
                SdCurrentVersion = SDCardVersion::SDVer2_BlockAddr;
            }
            else
            {
                argument = 0x200;
                status = SendCommand( SDCommand::CMD16, argument, response_buffer, sizeof(response_buffer), false );
                if( status == false || response_buffer[0] != 0x00 )
                {   
                    debug_controler_->PrintDebug(this, "Failed to sendCMD16\n", true);
                    status = false;
                    break;
                }

                debug_controler_->PrintDebug(this, "Success on Init\n", true);
                debug_controler_->PrintDebug(this, "SDVer2 ByteAddr\n", true);
                SdCurrentVersion = SDCardVersion::SDVer2_ByteAddr;
            }

        }
        else if(  rsp_echo != 0x1AA )
        {   
            debug_controler_->PrintError(this, "RspPtr != 0x1AA\n", true);
            status = false;
            break;   
        }

    } while( 0 );

    spi_communication_->SetCSPin(1);
    return status;
}

uint16_t SDCard::ReadSingleBlock( uint32_t address, uint8_t *buffer_read, uint16_t buffer_size )
{
    enum BlockReadState
    {
        CmdResp,
        BlockToken,
        DataBlock,
        ErrorState
    };
    
    // 1 CMD17 resp, 1 Data/Error token, kMaxBlocklen for block data,
    // 2 bytes for CRC and kNumberClocksTimout to account for SD processing time
    uint8_t bulk_read[ 1 + 1 + kMaxBlockLen + 2 + kNumberClocksTimeout] = {0};
    uint8_t sd_command[sizeof(bulk_read)] = {0};
    BlockReadState parse_state = CmdResp;
    uint16_t index = 0;
    uint8_t empty_io = 0xFF;
    uint16_t size_read = 0;
    uint16_t crc_received = 0;
    uint16_t crc_calculated = 0;

    do
    {   
        if( buffer_read == nullptr || buffer_size < block_len_ )
        {
            break;
        }

        memset(sd_command, 0xFF, sizeof(sd_command));

        if( !BuildSDCommand( SDCommand::CMD17, address, sd_command, 6, false ) )
        {   
            debug_controler_->PrintError(this, "Failed to build CMD17\n", true);
            break;
        }

        spi_communication_->SetCSPin(0);
        if( !spi_communication_->WriteReadData( sd_command, bulk_read, sizeof(bulk_read) ) )
        {   
            debug_controler_->PrintError(this, "Failed read single block data\n", true);
            break;
        }
        spi_communication_->SetCSPin(1);

        do
        {
            if( index >= sizeof(bulk_read) )
            {
                parse_state = ErrorState;
                break;
            }

            switch (parse_state)
            {
                case CmdResp:
                    if( bulk_read[index] == empty_io )
                    {
                        index++;
                        continue;
                    }
                    else if( bulk_read[index] == 0x00 )
                    {
                        index++;
                        parse_state = BlockToken;
                        continue;
                    }
                    else
                    {
                        parse_state = ErrorState;
                        debug_controler_->PrintError(this, "Failed when searching for CMD17 resp\n", true);
                        break;
                    }

                    break;
                case BlockToken:

                    if( bulk_read[index] == empty_io )
                    {
                        index++;
                        continue;
                    }
                    else if( bulk_read[index] == kStartBlockToken )
                    {
                        index++;
                        parse_state = DataBlock;
                        continue;
                    }

                    parse_state = ErrorState;
                    debug_controler_->PrintError(this, "Failed when searching for CMD17 Data/Error token\n", true);
                    break;
                default:
                break;
            }

        } while( parse_state != DataBlock && 
                 parse_state != ErrorState );

        if( parse_state == DataBlock )
        {
            if( index + block_len_ + 2 <= sizeof(bulk_read) )
            {
                uint16_t crc_pos = index + block_len_;
                crc_received = ( bulk_read[crc_pos] << 8 ) | bulk_read[crc_pos + 1];
                crc_calculated = CRC::CRC16( &bulk_read[index], block_len_ );

                if( crc_received != crc_calculated )
                {
                    debug_controler_->PrintError(this, "Failed on block crc calculation\n", true);
                    break;
                }

                debug_controler_->PrintDebug(this, "Data read successfully\n", true);
                memcpy( buffer_read, &bulk_read[index], block_len_ );
                size_read = block_len_;
            }
        }

    } while(0);

    return size_read;
}

uint16_t SDCard::ReadMultipleBlocks( uint32_t address, uint8_t *buffer_read, uint16_t buffer_size )
{
    return 0;
}

uint16_t SDCard::WriteSingleBlock( uint32_t address, uint8_t *buffer_write, uint16_t buffer_size )
{
    uint16_t size_written = 0;
    uint16_t cmd_response = 0;
    uint8_t data_block[ 1 + kMaxBlockLen + 2 ] = {0x0};
    uint8_t sd_response[ kNumberClocksTimeout ] = {0};
    uint16_t index = 0;

    data_block[0] = kStartBlockToken;
    memcpy( &data_block[1], buffer_write, block_len_ );
    data_block[block_len_ + 1] = 0x01;
    data_block[block_len_ + 2] = 0x02;

    do
    {
        if( buffer_write == nullptr || buffer_size > kMaxBlockLen )
        {
            break;
        }

        if( !SendCommand( SDCommand::CMD24, address, (uint8_t*)&cmd_response, 1, false) )
        {   
            debug_controler_->PrintError(this, "Failed to send CMD24\n", true);
            break;
        }

        if( cmd_response != 0x0 )
        {   
            debug_controler_->PrintError(this, "CMD response != 0x00\n", true);
            break;
        }

        spi_communication_->SetCSPin(0);
        if( !spi_communication_->WriteData( data_block, sizeof(data_block) ) )
        {   
            debug_controler_->PrintError(this, "Failed to write data_block\n", true);
            break;
        }

        memset(data_block, 0xFF, sizeof(data_block));
        if( !spi_communication_->WriteReadData( data_block, sd_response, sizeof(sd_response) ) )
        {
            debug_controler_->PrintError(this, "Failed to read data_response\n", true);
            break;
        }

        while( index < sizeof(sd_response) &&
               sd_response[index] == 0xFF)
        {
            index++;
        }

        if( index == sizeof(sd_response) )
        {   
            debug_controler_->PrintError(this, "Failed to find data resp token\n", true);
            break;
        }

        uint8_t data_response_token = (sd_response[index] & 0xE) >> 1;
        if( data_response_token != kDataResponseTokenAccepted )
        {   
            debug_controler_->PrintError(this, "Data resp token is not accepted\n", true);
            break;
        }

        index++;
        while( index < sizeof(sd_response) &&
               sd_response[index] == 0x00 )
        {
            index++;
        }

        if( index == sizeof(sd_response) )
        {
            debug_controler_->PrintDebug(this, "Failed to find non-busy byte\n", true);
            break;
        }

        if( !SendCommand( SDCommand::CMD13, 0x00, (uint8_t*)&cmd_response, 2, false ) )
        {   
            debug_controler_->PrintError(this, "Failed to send CMD13\n", true);
            break;
        }

        if( cmd_response != 0x00 )
        {   
            debug_controler_->PrintError(this, "Data was not written\n", true);
            break;
        }

        debug_controler_->PrintDebug(this, "Data written successfully\n", true);
        size_written = block_len_;
    } while( 0 );

    spi_communication_->SetCSPin(1);
    return size_written;
}

uint16_t SDCard::WriteMultipleBlocks( uint32_t address, uint8_t *buffer_write, uint16_t buffer_size )
{
    return 0;
}

uint16_t SDCard::ReadData( uint32_t address, uint8_t *buffer_read, uint16_t buffer_size )
{
    uint16_t num_bytes_read = 0;
    do
    {
        if( buffer_read == nullptr || buffer_size == 0 )
        {
            break;
        }

        if( buffer_size == block_len_ )
        {
           num_bytes_read = ReadSingleBlock( address, buffer_read, buffer_size );
        }
        else
        {
            num_bytes_read = ReadMultipleBlocks( address, buffer_read, buffer_size );
        }

    } while( 0 );

    return num_bytes_read;
}

uint16_t SDCard::WriteData( uint32_t address, uint8_t *buffer_write, uint16_t buffer_size )
{
    uint16_t num_bytes_written = 0;
    do
    {
        if( buffer_write == nullptr || buffer_size == 0 )
        {
            break;
        }

        if( buffer_size == block_len_ )
        {
           num_bytes_written = WriteSingleBlock( address, buffer_write, buffer_size );
        }
        else
        {
            num_bytes_written = WriteMultipleBlocks( address, buffer_write, buffer_size );
        }

    } while( 0 );

    return num_bytes_written;
}

bool SDCard::SendCommand( SDCommand cmd, uint32_t argument, uint8_t *response, uint16_t response_buffer_size, bool crc_enabled )
{
    uint8_t sd_command[6] = {0};
    uint8_t data_read_back[16] = {0};
    uint8_t empty_write[sizeof(data_read_back)];
    uint8_t expected_cmd_rsp_size = GetCmdResponseSizeBytes(cmd);
    bool status;
    uint8_t index = 0;

    if( response == nullptr || 
        ( response_buffer_size < expected_cmd_rsp_size ) || 
        expected_cmd_rsp_size > sizeof(data_read_back) )
    {   
        debug_controler_->PrintError(this, "Error on input data\n", true);
        return false;
    }

    memset(empty_write, 0xFF, sizeof(empty_write));

    do
    {   
        if( !BuildSDCommand( cmd, argument, sd_command, sizeof(sd_command), crc_enabled ) )
        {
            break;
        }

        status = spi_communication_->SetCSPin( 0 );
        if( status == false )
        {   
            debug_controler_->PrintError(this, "Failed to set CS to low\n", true);
            break;
        }

        // Write the command
        status = spi_communication_->WriteData( sd_command, sizeof(sd_command) );
        if(status == false)
        {   
            debug_controler_->PrintError(this, "Failed to write command\n", true);
            break;
        }

        if( expected_cmd_rsp_size == kR1BResponse )
        {
            // TODO: implement R1b response
            break;
        }

        // Try to read the data back, it could need until 8 bytes to SD card to respond
        status = spi_communication_->WriteReadData( empty_write, data_read_back, sizeof(data_read_back) );
        if( status == false )
        {
            debug_controler_->PrintError(this, "Failed to read data back\n", true);
            break;
        }

        // If theres no valid byte in the response, it means the sd card for some reason does not responded. driver will interpret as fail
        if( GetStartValidByteFromBuffer( &index, data_read_back, sizeof(data_read_back) ) == false )
        {
            debug_controler_->PrintError(this, "SD card did not respond\n", true);
            status = false;
            break;
        }

        if( index + expected_cmd_rsp_size > sizeof(data_read_back) )
        {
            debug_controler_->PrintError(this, "Out of bound data rsp\n", true);
            status = false;
            break;
        }

        memcpy( response, &data_read_back[index], expected_cmd_rsp_size );
        status = true;
        break;

    } while(0);

    spi_communication_->SetCSPin( 1 );
    return status;
}

}
}

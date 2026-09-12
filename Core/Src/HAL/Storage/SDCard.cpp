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
    memset(blocks_buffer_, 0, sizeof(blocks_buffer_));
}

SDCard::~SDCard()
{
}

uint8_t buffer_write[512];
uint8_t attempts = 1;
uint8_t current_cycle = 0;
uint8_t buffer_read[512 * 6] = {0};
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

            debug_controler_->PrintDebug(this, "Testing multiple blocks cycles\n", true);

            if(current_cycle < attempts)
            {
                debug_controler_->PrintDebug(this, "New cycle attempt\n", true);
                //for(int i = 0; i<25; i++)
                {
                    //WriteData( i, buffer_write, sizeof(buffer_write) );
                    ReadData( 0, buffer_read, sizeof(buffer_read) );
                    //EraseRange(0, 24);
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

bool SDCard::IsR1BResponse( SDCommand cmd )
{
    if( cmd == SDCommand::CMD12 || cmd == SDCommand::CMD38 )
    {
        return true;
    }

    return false;
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

bool SDCard::WaitForBusyLine( uint8_t *r1response, uint16_t attempts )
{
    bool status = false; 
    uint8_t buffer_read[ kNumberClocksTimeout ] = {0};

    if( r1response == nullptr )
    {
        return false;
    }

    for(int i = 0; i < attempts; i++)
    {
        status = false;
        if( !spi_communication_->ReadData( buffer_read, sizeof(buffer_read) ) )
        {
            continue;
        }

        uint8_t index = 0;
        while( index < sizeof(buffer_read) &&
               buffer_read[index] > 0 )
        {
            index++;
        }

        if( index == sizeof(buffer_read) )
        {   
            debug_controler_->PrintError(this, "Failed to find cmd resp\n", true);
            continue;
        }

        if( buffer_read[index] != 0x00 )
        {   
            debug_controler_->PrintError(this, "Cmd resp error abort\n", true);
            i = attempts;
            continue;
        }

        *r1response = buffer_read[index];
        index++;

        while( index < sizeof(buffer_read) && 
               buffer_read[index] == 0x00 )
        {
            index++;
        }

        if( index == sizeof(buffer_read) )
        {   
            debug_controler_->PrintError(this, "failed to find non-busy flag\n", true);
            continue;
        }

        status = true;
        i = attempts;
    }

    return status;
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

bool SDCard::ParseSingleBlock(  SDCommand cmd_sent, uint8_t *block_buffer, uint16_t size_block_buffer, uint8_t *data_read, uint16_t *last_block_byte )
{
    uint16_t index = 0;
    uint8_t empty_io = 0xFF;
    uint16_t crc_received = 0;
    uint16_t crc_calculated = 0;

    if( block_buffer == nullptr )
    {
        return false;
    }

    if( last_block_byte != nullptr )
    {
        index = *last_block_byte;
    }

    if( cmd_sent == SDCommand::CMD17)
    {

        while( index < size_block_buffer &&
                block_buffer[index] == empty_io )
        {
            index++;
        }

        if( index >= size_block_buffer || block_buffer[index] != 0x00 )
        {
            return false;
        }

        index++;
    }

    while( index < size_block_buffer &&
            block_buffer[index] == empty_io )
    {
        index++;
    }

    if( index >= size_block_buffer || block_buffer[index] != kStartBlockToken )
    {   
        return false;
    }

    index++;
    if( index + block_len_ + 2 <= size_block_buffer )
    {
        uint16_t crc_pos = index + block_len_;
        crc_received = ( block_buffer[crc_pos] << 8 ) | block_buffer[crc_pos + 1];
        crc_calculated = CRC::CRC16( &block_buffer[index], block_len_ );

        if( last_block_byte != nullptr )
        {
            *last_block_byte = crc_pos + 1;
        }

        if( crc_received != crc_calculated )
        {   
            debug_controler_->PrintError(this, "Failed on block crc calculation\n", true);
            return false;
        }
        else
        {
            debug_controler_->PrintDebug(this, "Data read successfully\n", true);
            memcpy( data_read, &block_buffer[index], block_len_ );
            return true;
        }
    }

    return false;
}

bool SDCard::ParseBlockWriteResponse( uint8_t *block_buffer, uint16_t block_size )
{   
    uint16_t index = 0;

    if( block_buffer == nullptr || block_size == 0 )
    {
        return false;
    }

    while( index < block_size &&
            block_buffer[index] == 0xFF)
    {
        index++;
    }

    if( index == block_size )
    {   
        debug_controler_->PrintError(this, "Failed to find data resp token\n", true);
        return false;
    }

    uint8_t data_response_token = (block_buffer[index] & 0xE) >> 1;
    if( data_response_token != kDataResponseTokenAccepted )
    {   
        debug_controler_->PrintError(this, "Data resp token is not accepted\n", true);
        return false;
    }

    index++;
    while( index < block_size &&
            block_buffer[index] == 0x00 )
    {
        index++;
    }

    if( index == block_size )
    {   
        debug_controler_->PrintError(this, "Failed to find non-busy flag\n", true);
        return false;
    }

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
    // 1 CMD17 resp, 1 Data/Error token, kMaxBlocklen for block data,
    // 2 bytes for CRC and kNumberClocksTimout to account for SD processing time
    uint8_t bulk_read[ 1 + 1 + kMaxBlockLen + 2 + kNumberClocksTimeout] = {0};
    uint8_t sd_command[sizeof(bulk_read)] = {0};
    uint16_t size_read = 0;

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
            spi_communication_->SetCSPin(1);
            debug_controler_->PrintError(this, "Failed read single block data\n", true);
            break;
        }
        spi_communication_->SetCSPin(1);

        if( !ParseSingleBlock( SDCommand::CMD17, bulk_read, sizeof(bulk_read), buffer_read, nullptr ) )
        {
            break;
        }

        size_read = block_len_;
    } while(0);

    return size_read;
}

uint16_t SDCard::ReadMultipleBlocks( uint32_t address, uint8_t *buffer_read, uint16_t buffer_size )
{
    uint8_t cmd_response = 0;
    uint16_t num_bytes_to_read = (buffer_size / block_len_) * block_len_ + kNumberClocksTimeout;
    uint16_t last_pckg_byte = 0;

    if( buffer_read == nullptr || buffer_size == 0 || num_bytes_to_read > sizeof(blocks_buffer_) )
    {
        return 0;
    }

    if( !SendCommand( SDCommand::CMD18, address, &cmd_response, sizeof(cmd_response), false ) )
    {   
        debug_controler_->PrintError(this, "Failed to send CMD18\n", true);
        return 0;
    }

    if( cmd_response != 0 )
    {   
        debug_controler_->PrintError(this, "CMD response != 0\n", true);
        return 0;
    }

    spi_communication_->SetCSPin(0);

    if( !spi_communication_->ReadData( blocks_buffer_, num_bytes_to_read ) )
    {
        spi_communication_->SetCSPin(1);
        return 0;
    }
    spi_communication_->SetCSPin(1);

    if( !SendCommand( SDCommand::CMD12, 0, &cmd_response, sizeof(cmd_response), false) )
    {
        return 0;
    }

    if( cmd_response != 0 )
    {
        return 0;
    }

    uint16_t index = 0;
    uint16_t requested_blocks = buffer_size / block_len_;
    while( index < requested_blocks )
    {
        if( !ParseSingleBlock( SDCommand::CMD18, blocks_buffer_, sizeof(blocks_buffer_) - last_pckg_byte, &buffer_read[index * block_len_], &last_pckg_byte ) )
        {
            break;
        }

        last_pckg_byte++;
        index++;
    }

    memset(blocks_buffer_, 0, sizeof(blocks_buffer_));
    return index * block_len_;
}

uint16_t SDCard::WriteSingleBlock( uint32_t address, uint8_t *buffer_write, uint16_t buffer_size )
{
    uint16_t size_written = 0;
    uint16_t cmd_response = 0;
    uint8_t data_block[ 1 + kMaxBlockLen + 2 ] = {0x0};
    uint8_t sd_response[ kNumberClocksTimeout ] = {0};

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

        if( !spi_communication_->ReadData( sd_response, sizeof(sd_response) ) )
        {
            spi_communication_->SetCSPin(1);
            debug_controler_->PrintError(this, "Failed to read data_response\n", true);
            break;
        }
        spi_communication_->SetCSPin(1);

        if( !ParseBlockWriteResponse( sd_response, sizeof(sd_response) ) )
        {
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

    return size_written;
}

uint16_t SDCard::WriteMultipleBlocks( uint32_t address, uint8_t *buffer_write, uint16_t buffer_size )
{
    uint8_t data_write[1 + kMaxBlockLen + 2] = {0};
    uint8_t data_read[kNumberClocksTimeout] = {0};
    uint8_t cmd_response = 0;
    uint16_t current_size = 0;
    uint16_t size_missing = buffer_size;

    uint16_t index = 0;
    uint16_t num_blocks = buffer_size / block_len_;

    if( buffer_write == nullptr || buffer_size == 0 )
    {
        return false;
    }

    if( buffer_size % block_len_ )
    {
        num_blocks++;
    }

    if( !SendCommand( SDCommand::CMD25, address, &cmd_response, sizeof(cmd_response), false) )
    {   
        debug_controler_->PrintError(this, "Failed to send CMD25\n", true);
        return 0;
    }

    if( cmd_response != 0 )
    {   
        debug_controler_->PrintError(this, "CMD 25 resp != 0\n", true);
        return 0;
    }

    data_write[0] = kStartMultiBlockWriteToken;
    data_write[block_len_ + 1] = 0x1;
    data_write[block_len_ + 2] = 0x2;

    spi_communication_->SetCSPin(0);
    while(index < num_blocks)
    {   
        if( size_missing > block_len_ )
        {
            current_size = block_len_;
        }
        else
        {
            current_size = size_missing;
        }
        size_missing -=current_size;

        memcpy( &data_write[1], &buffer_write[index * block_len_], current_size );
        if( !spi_communication_->WriteData( data_write, sizeof(data_write) ) )
        {   
            debug_controler_->PrintError(this, "Failed to write block\n", true);
            break;
        }

        if( !spi_communication_->ReadData( data_read, sizeof(data_read) ) )
        {   
            debug_controler_->PrintError(this, "Failed to read SD response\n", true);
            break;
        }

        if( !ParseBlockWriteResponse( data_read, sizeof(data_read) ) )
        {   
            debug_controler_->PrintError(this, "Failed to parse sd response\n", true);
            break;
        }

        memset( &data_write[1], 0, block_len_ );
        index++;
    }

    uint8_t stop_token = kStopMultiBlockWriteToken;
    if( !spi_communication_->WriteData( &stop_token, sizeof(stop_token)) )
    {
        spi_communication_->SetCSPin(1);
        return 0;
    }

    if( !spi_communication_->ReadData( data_read, sizeof(data_read) ) )
    {   
        debug_controler_->PrintError(this, "Failed to read StopTrans resp\n", true);
        spi_communication_->SetCSPin(1);
        return 0;
    }
    spi_communication_->SetCSPin(1);


    uint16_t i = 0;
    while(i < sizeof(data_read))
    {
        if(data_read[i] > 0)
        {
            break;
        }

        i++;
    }

    if( i == sizeof(data_read) )
    {       
        debug_controler_->PrintError(this, "Failed to find non-busy flag\n", true);
        return 0;
    }

    return index * block_len_;
}

bool SDCard::EraseRange( uint32_t start_address, uint32_t end_address ) 
{
    uint8_t cmd_response = 0x00;

    if( end_address < start_address )
    {
        return false;
    }

    if( !SendCommand( SDCommand::CMD32, start_address, &cmd_response, sizeof(cmd_response), false ) )
    {
        return false;
    }

    if( cmd_response != 0x00 )
    {
        return false;
    }

    if( !SendCommand( SDCommand::CMD33, end_address, &cmd_response, sizeof(cmd_response), false ) )
    {
        return false;
    }

    if( cmd_response != 0x00 )
    {
        return false;
    }

    uint8_t dummy = 0xFF;
    if( !SendCommand( SDCommand::CMD38, dummy, &cmd_response, sizeof(cmd_response), false ) )
    {
        return false;
    }

    if( cmd_response != 0x00 )
    {
        return false;
    }

    return true;
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
        else if( buffer_size > block_len_ )
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
        else if( buffer_size > block_len_ )
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

        if( IsR1BResponse( cmd ) )
        {
            status = WaitForBusyLine( response, 5 );
            if( status == false)
            {
                debug_controler_->PrintError(this, "Failed to retrieve R1B response\n", true);
            }

            break;
        }

        // Try to read the data back, it could need until 8 bytes to SD card to respond
        status = spi_communication_->ReadData( data_read_back, sizeof(data_read_back) );
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

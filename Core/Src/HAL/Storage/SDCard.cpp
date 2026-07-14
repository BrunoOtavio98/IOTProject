#include "Storage/SDCard.h"
#include "Devices/Communication/Interfaces/SPIInterface.h"
#include "Utils/CRC.h"
#include <string.h>

using HAL::Utils::CRC;
using HAL::Devices::Communication::Interfaces::SPIInterface;
using HAL::RtosWrappers::TaskWrapper;

namespace HAL 
{
namespace Storage 
{

SDCard::SDCard( std::shared_ptr<HAL::Devices::Communication::Interfaces::SPIInterface> spi_communication ) : 
                StorageInterface(spi_communication),
                TaskWrapper("SDCard", 500, nullptr, 2),
                SdCurrentVersion(SDCardVersion::SDInvalid)
{
}

SDCard::~SDCard()
{
}

void SDCard::Task(void *params)
{
    while(1)
    {
        if( SdCurrentVersion == SDCardVersion::SDInvalid )
        {
            InitStorage();
        }

        TaskDelay(100);
    }
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

        spi_communication_->SetCSPin(0);
        status = SendCommand( SDCommand::CMD0, argument, response_buffer, sizeof(response_buffer), true );
        if( status == false )
        {
            break;
        }

        if( response_buffer[0] != SDResponseMask::IdleState )
        {
            status = false;
            break;
        }

        argument = 0x1AA;
        status = SendCommand( SDCommand::CMD8, argument, response_buffer, sizeof(response_buffer), true );

        uint16_t *rsp_ptr = (uint16_t*)&response_buffer[0];
        *rsp_ptr = *rsp_ptr & 0xFFF;

        // If error or no response
        if( status == false || response_buffer[4] != 0x00 )
        {
            bool is_sd_busy = true;
            do
            {   

                // status = SendCommand( SDCommand::CMD55, 0x00, response_buffer, sizeof(response_buffer), false );
                // if( status == false )
                // {
                //     break;
                // }

                argument = 0x0;
                status = SendCommand( SDCommand::ACMD41, argument, response_buffer, sizeof(response_buffer), false );
                if( status == false )
                {
                    break;
                }

                is_sd_busy = (response_buffer[0] & SDResponseMask::IdleState);
            } while( is_sd_busy );

            if( status == false) 
            {
                break;
            }

            SdCurrentVersion = SDCardVersion::SDVer1;
        }
        else if( *rsp_ptr == 0x1AA )
        {   
            
            // status = SendCommand( SDCommand::CMD55, 0x00, response_buffer, sizeof(response_buffer), false );
            // if( status == false )
            // {
            //     break;
            // }

            argument = (0x01 << 30);
            status = SendCommand( SDCommand::ACMD41, argument, response_buffer, sizeof(response_buffer), false );
            if( status == false || response_buffer[0] > 0x01 )
            {
                status = false;
                break;
            }

            while( response_buffer[0] == 0x01 )
            {
                // status = SendCommand( SDCommand::CMD55, 0x00, response_buffer, sizeof(response_buffer), false );
                // if( status == false )
                // {
                //     break;
                // }

                status = SendCommand( SDCommand::ACMD41, argument, response_buffer, sizeof(response_buffer), false );
                if( status == false || response_buffer[0] > 0x01 )
                {   
                    status = false;
                    break;
                }
            }

            if( status == false )
            {
                break;
            }

            argument = 0x00;
            status = SendCommand( SDCommand::CMD58, argument, response_buffer, sizeof(response_buffer), false );
            if( status == false || response_buffer[4] != 0x00 )
            {
                status = false;
                break;
            }

            uint32_t *OCR = (uint32_t *)&response_buffer[0];
            uint8_t ccs_set = (*OCR) & CCS_MASK;

            if(ccs_set)
            {
                SdCurrentVersion = SDCardVersion::SDVer2_BlockAddr;
            }
            else
            {
                argument = 0x200;
                status = SendCommand( SDCommand::CMD16, argument, response_buffer, sizeof(response_buffer), false );
                if( status == false || response_buffer[0] != 0x00 )
                {
                    status = false;
                    break;
                }

                SdCurrentVersion = SDCardVersion::SDVer2_ByteAddr;
            }

        }
        else if(  *rsp_ptr != 0x1AA )
        {
            status = false;
            break;   
        }

    } while( 0 );

    spi_communication_->SetCSPin(0);
    return status;
}

uint8_t SDCard::GetCmdResponseSizeBytes( SDCommand cmd )
{
    uint8_t response_size_bytes = 1;

    if( cmd == SDCommand::CMD8 || cmd == SDCommand::CMD58 )
    {
        response_size_bytes = 5;
    }

    if( cmd == SDCommand::CMD12 )
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

bool SDCard::SendCommand( SDCommand cmd, uint32_t argument, uint8_t *response, uint16_t response_buffer_size, bool crc_enabled )
{
    uint32_t local_argument = argument;
    uint8_t sd_command[6] = {0};
    uint8_t empty_write[16];
    uint8_t data_read_back[16] = {0};
    uint8_t expected_cmd_rsp_size = GetCmdResponseSizeBytes(cmd);
    uint8_t crc = 0;
    uint8_t index = 0;
    bool status;

    if( response == nullptr || ( response_buffer_size < expected_cmd_rsp_size ) )
    {
        return false;
    }

    memset(empty_write, 0xFF, sizeof(empty_write));

    sd_command[0] = 0x40 | cmd;
    sd_command[1] = (local_argument >> 24) && 0xFF;
    sd_command[2] = (local_argument >> 16) && 0xFF;
    sd_command[3] = (local_argument >> 8) && 0xFF;
    sd_command[4] =  local_argument && 0xFF;

    if( crc_enabled )
    {
        crc = CRC::CRC7(&sd_command[0], 5);
        crc = crc << 1 | 1;
    }
    memcpy( &sd_command[5], &crc, sizeof(crc));

    do
    {
        status = spi_communication_->SetCSPin( 0 );
        if( status == false )
        {
            break;
        }

        // Write the command
        status = spi_communication_->WriteData( sd_command, sizeof(sd_command) );
        if(status == false)
        {
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
            break;
        }

        // If theres no valid byte in the response, it means the sd card for some reason does not responded. driver will interpret as fail
        if( GetStartValidByteFromBuffer( &index, data_read_back, sizeof(data_read_back) ) == false )
        {
            status = false;
            break;
        }

        if( index + expected_cmd_rsp_size > sizeof(data_read_back) )
        {
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

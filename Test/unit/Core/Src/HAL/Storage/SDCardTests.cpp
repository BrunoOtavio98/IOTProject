#include "Storage/SDCard.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <array>
#include <cstring>

#include "Utils/CRC.h"
#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockSPICommunicationInterface.h"
#include "Core/Src/HAL/DebugController/Mocks/MockDebugController.h"
#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockUartCommunicationInterface.h"

using HAL::Devices::Communication::Interfaces::MockSPICommunicationInterface;
using HAL::Devices::Communication::Interfaces::MockUartCommunicationInterface;
using HAL::DebugController::MockDebugController;

namespace HAL
{
namespace Storage
{

class SDCardHelper : public SDCard
{
public:
    using SDCard::GetCmdResponseSizeBytes;
    using SDCard::GetStartValidByteFromBuffer;
    using SDCard::EraseRange;
    using SDCard::ReadData;
    using SDCard::ReadMultipleBlocks;
    using SDCard::SendCommand;
    using SDCard::WriteData;
    using SDCard::WriteSingleBlock;
    using SDCard::kNumberClocksTimeout;
    using SDCard::kMaxBlockLen;

    uint8_t Cmd0Value() const
    { 
        return static_cast<uint8_t>(SDCommand::CMD0); 
    }

    uint8_t Cmd8Value() const
    {
        return static_cast<uint8_t>(SDCommand::CMD8);
    }

    uint8_t Cmd58Value() const
    {
        return static_cast<uint8_t>(SDCommand::CMD58);
    }

    uint8_t ResponseSizeForCmd0() const
    {
        return const_cast<SDCardHelper*>(this)->GetCmdResponseSizeBytes(static_cast<SDCommand>(Cmd0Value()));
    }

    uint8_t ResponseSizeForCmd8() const
    {
        return const_cast<SDCardHelper*>(this)->GetCmdResponseSizeBytes(static_cast<SDCommand>(Cmd8Value()));
    }

    uint8_t ResponseSizeForCmd58() const
    {
        return const_cast<SDCardHelper*>(this)->GetCmdResponseSizeBytes(static_cast<SDCommand>(Cmd58Value()));
    }

    bool SendCommandCmd0(uint32_t argument, uint8_t *response, uint16_t response_buffer_size, bool crc_enabled)
    {
        return SendCommand(static_cast<SDCommand>(Cmd0Value()), argument, response, response_buffer_size, crc_enabled);
    }

    bool SendCommandCmd8(uint32_t argument, uint8_t *response, uint16_t response_buffer_size, bool crc_enabled)
    {
        return SendCommand(static_cast<SDCommand>(Cmd8Value()), argument, response, response_buffer_size, crc_enabled);
    }

    bool InitStorageWrapper()
    {
        return InitStorage();
    }

    uint8_t CurrentVersion() const
    {
        return static_cast<uint8_t>(SdCurrentVersion);
    }

    SDCardHelper( std::shared_ptr<MockSPICommunicationInterface> spi_comm,
                  std::shared_ptr<MockDebugController> debug_controller )
                  : SDCard( spi_comm, debug_controller )
    {
    }
};

class SDCardTests : public ::testing::Test
{
public:
    SDCardTests() :
        spi_comm_(std::make_shared<MockSPICommunicationInterface>()),
        uart_comm_(std::make_shared<MockUartCommunicationInterface>()),
        debug_controller_(std::make_shared<MockDebugController>(uart_comm_)),
        sd_card_(spi_comm_, debug_controller_)
    {
    }

    std::shared_ptr<MockSPICommunicationInterface> spi_comm_;
    std::shared_ptr<MockUartCommunicationInterface> uart_comm_;
    std::shared_ptr<MockDebugController> debug_controller_;
    SDCardHelper sd_card_;
};

TEST_F(SDCardTests, GetCmdResponseSizeBytesUsesExpectedSizes)
{
    EXPECT_EQ(sd_card_.ResponseSizeForCmd8(), 5u);
    EXPECT_EQ(sd_card_.ResponseSizeForCmd58(), 5u);
    EXPECT_EQ(sd_card_.ResponseSizeForCmd0(), 1u);
}

TEST_F(SDCardTests, GetStartValidByteFromBufferFindsFirstNonFFByte)
{
    uint8_t buffer[] = {0xFF, 0xFF, 0x01, 0x7F};
    uint8_t index = 0;

    EXPECT_TRUE(sd_card_.GetStartValidByteFromBuffer(&index, buffer, sizeof(buffer)));
    EXPECT_EQ(index, 2u);
}

TEST_F(SDCardTests, GetStartValidByteFromBufferReturnsFalseWhenAllBytesAreFF)
{
    uint8_t buffer[] = {0xFF, 0xFF, 0xFF};
    uint8_t index = 0;

    EXPECT_FALSE(sd_card_.GetStartValidByteFromBuffer(&index, buffer, sizeof(buffer)));
}

TEST_F(SDCardTests, SendCommandBuildsCommandFrameAndReturnsResponseForCmd0)
{
    uint8_t response[5] = {0};
    const uint8_t expected_read_back[] = {0xFF, 0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    EXPECT_CALL(*spi_comm_, SetCSPin(0)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
            .WillOnce([&](const uint8_t *data, uint16_t data_size) {
            EXPECT_EQ(data_size, 6u);
            EXPECT_EQ(data[0], 0x40u | sd_card_.Cmd0Value());
            EXPECT_EQ(data[1], 0x00u);
            EXPECT_EQ(data[2], 0x00u);
            EXPECT_EQ(data[3], 0x00u);
            EXPECT_EQ(data[4], 0x00u);
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](uint8_t *data_read, uint16_t data_size) {
            EXPECT_EQ(data_size, 16u);
            memcpy(data_read, expected_read_back, sizeof(expected_read_back));
            return true;
        });
    EXPECT_CALL(*spi_comm_, SetCSPin(1)).WillOnce(::testing::Return(true));

    ASSERT_TRUE(sd_card_.SendCommandCmd0(0x0, response, sizeof(response), true));
    EXPECT_EQ(response[0], 0x01u);
    EXPECT_EQ(response[1], 0x00u);
    EXPECT_EQ(response[2], 0x00u);
    EXPECT_EQ(response[3], 0x00u);
    EXPECT_EQ(response[4], 0x00u);
}

TEST_F(SDCardTests, SendCommandBuildsCommandFrameAndReturnsResponseForCmd8)
{
    uint8_t response[5] = {0};
    uint32_t argument = 0x1234;
    const uint8_t expected_read_back[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xCA, 0xFE, 0xBA, 0xBE, 0xBE, 0xFF, 0xFF, 0xFF, 0xFF};

    EXPECT_CALL(*spi_comm_, SetCSPin(0)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
            .WillOnce([&](const uint8_t *data, uint16_t data_size) {
            EXPECT_EQ(data_size, 6u);
            EXPECT_EQ(data[0], 0x40u | sd_card_.Cmd8Value());
            EXPECT_EQ(data[1], 0x00u);
            EXPECT_EQ(data[2], 0x00u);
            EXPECT_EQ(data[3], 0x12u);
            EXPECT_EQ(data[4], 0x34u);
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](uint8_t *data_read, uint16_t data_size) {
            EXPECT_EQ(data_size, 16u);
            memcpy(data_read, expected_read_back, sizeof(expected_read_back));
            return true;
        });
    EXPECT_CALL(*spi_comm_, SetCSPin(1)).WillOnce(::testing::Return(true));

    ASSERT_TRUE(sd_card_.SendCommandCmd8(argument, response, sizeof(response), true));
    EXPECT_EQ(response[0], 0xCAu);
    EXPECT_EQ(response[1], 0xFEu);
    EXPECT_EQ(response[2], 0xBAu);
    EXPECT_EQ(response[3], 0xBEu);
    EXPECT_EQ(response[4], 0xBEu);
}

TEST_F(SDCardTests, SendCommandBuildsCommandFrameAndSDCardDoesNotResponds)
{
    uint8_t response[5] = {0};
    const uint8_t expected_read_back[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    EXPECT_CALL(*spi_comm_, SetCSPin(0)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
            .WillOnce([&](const uint8_t *data, uint16_t data_size) {
            EXPECT_EQ(data_size, 6u);
            EXPECT_EQ(data[0], 0x40u | sd_card_.Cmd0Value());
            EXPECT_EQ(data[1], 0x00u);
            EXPECT_EQ(data[2], 0x00u);
            EXPECT_EQ(data[3], 0x00u);
            EXPECT_EQ(data[4], 0x00u);
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](uint8_t *data_read, uint16_t data_size) {
            EXPECT_EQ(data_size, 16u);
            memcpy(data_read, expected_read_back, sizeof(expected_read_back));
            return true;
        });
    EXPECT_CALL(*spi_comm_, SetCSPin(1)).WillOnce(::testing::Return(true));

    ASSERT_FALSE(sd_card_.SendCommandCmd0(0x0, response, sizeof(response), true));
    EXPECT_EQ(response[0], 0x00u);
    EXPECT_EQ(response[1], 0x00u);
    EXPECT_EQ(response[2], 0x00u);
    EXPECT_EQ(response[3], 0x00u);
    EXPECT_EQ(response[4], 0x00u);
}

TEST_F(SDCardTests, SendCommandBuildsCommandFrameOutOfBoundsIndex)
{
    uint8_t response[5] = {0};
    uint32_t argument = 0xABCDEF;
    const uint8_t expected_read_back[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0xFF};

    EXPECT_CALL(*spi_comm_, SetCSPin(0)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
            .WillOnce([&](const uint8_t *data, uint16_t data_size) {
            EXPECT_EQ(data_size, 6u);
            EXPECT_EQ(data[0], 0x40u | sd_card_.Cmd8Value());
            EXPECT_EQ(data[1], 0x00u);
            EXPECT_EQ(data[2], 0xABu);
            EXPECT_EQ(data[3], 0xCDu);
            EXPECT_EQ(data[4], 0xEFu);
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](uint8_t *data_read, uint16_t data_size) {
            EXPECT_EQ(data_size, 16u);
            memcpy(data_read, expected_read_back, sizeof(expected_read_back));
            return true;
        });
    EXPECT_CALL(*spi_comm_, SetCSPin(1)).WillOnce(::testing::Return(true));

    ASSERT_FALSE(sd_card_.SendCommandCmd8(argument, response, sizeof(response), true));
    EXPECT_EQ(response[0], 0x00u);
    EXPECT_EQ(response[1], 0x00u);
    EXPECT_EQ(response[2], 0x00u);
    EXPECT_EQ(response[3], 0x00u);
    EXPECT_EQ(response[4], 0x00u);
}

TEST_F(SDCardTests, InitStorageSucceedsWithExpectedSdCardSequence)
{
    size_t write_read_call_count = 0;

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 75u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).Times(6).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
            .Times(6)
        .WillRepeatedly([&](uint8_t *data_read, uint16_t) {
            std::array<uint8_t, 16> read_back = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

            switch (write_read_call_count++)
            {
                case 0:
                    read_back = {0xFF, 0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    break;
                case 1:
                    read_back = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    break;
                case 2:
                    read_back = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    break;
                case 3:
                    read_back = {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    break;
                case 4:
                    read_back = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    break;
                case 5:
                    read_back = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    break;
                default:
                    break;
            }

            memcpy(data_read, read_back.data(), read_back.size());
            return true;
        });

    ASSERT_TRUE(sd_card_.InitStorageWrapper());
    EXPECT_EQ(sd_card_.CurrentVersion(), 2u);
}

TEST_F(SDCardTests, InitStorageSucceedsForSdCardVersion2Path)
{
    size_t write_read_call_count = 0;

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 75u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).Times(5).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .Times(5)
        .WillRepeatedly([&](uint8_t *data_read, uint16_t) {
            std::array<uint8_t, 16> read_back = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

            switch (write_read_call_count++)
            {
                case 0:
                    read_back = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                    break;
                case 1:
                    read_back = {0x01, 0x00, 0x00, 0x01, 0xAA, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                    break;
                case 2:
                    read_back = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                    break;
                case 3:
                    read_back = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                    break;
                case 4:
                    read_back = {0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                    break;
                default:
                    break;
            }

            memcpy(data_read, read_back.data(), read_back.size());
            return true;
        });

    ASSERT_TRUE(sd_card_.InitStorageWrapper());
    EXPECT_EQ(sd_card_.CurrentVersion(), 0u);
}

TEST_F(SDCardTests, InitStorageFailsWhenCmd8EchoDoesNotMatchSd2Pattern)
{
    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 75u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).Times(2).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            const std::array<uint8_t, 16> read_back = {0x01, 0x00, 0x00, 0x00, 0x11, 0xFF, 0xFF, 0xFF,
                                                      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            memcpy(data_read, read_back.data(), read_back.size());
            return true;
        })
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            const std::array<uint8_t, 16> read_back = {0x01, 0x00, 0x00, 0x00, 0x11, 0xFF, 0xFF, 0xFF,
                                                      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            memcpy(data_read, read_back.data(), read_back.size());
            return true;
        });

    ASSERT_FALSE(sd_card_.InitStorageWrapper());
    EXPECT_EQ(sd_card_.CurrentVersion(), 3u);
}

TEST_F(SDCardTests, InitStorageFailsWhenCardDoesNotLeaveIdleState)
{
    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 75u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).Times(1).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            const std::array<uint8_t, 16> read_back = {0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            memcpy(data_read, read_back.data(), read_back.size());
            return true;
        });

    ASSERT_FALSE(sd_card_.InitStorageWrapper());
    EXPECT_EQ(sd_card_.CurrentVersion(), 3u);
}

TEST_F(SDCardTests, EraseRangeSucceedsWithExpectedCommandSequence)
{
    const uint32_t start_address = 0x00000010;
    const uint32_t end_address = 0x00000020;
    size_t command_call_count = 0;

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
        .Times(3)
        .WillRepeatedly([&](const uint8_t *data, uint16_t data_size) {
            EXPECT_EQ(data_size, 6u);

            if (command_call_count == 0)
            {
                EXPECT_EQ(data[0], 0x60u);
                EXPECT_EQ(data[1], 0x00u);
                EXPECT_EQ(data[2], 0x00u);
                EXPECT_EQ(data[3], 0x00u);
                EXPECT_EQ(data[4], 0x10u);
            }
            else if (command_call_count == 1)
            {
                EXPECT_EQ(data[0], 0x61u);
                EXPECT_EQ(data[1], 0x00u);
                EXPECT_EQ(data[2], 0x00u);
                EXPECT_EQ(data[3], 0x00u);
                EXPECT_EQ(data[4], 0x20u);
            }
            else
            {
                EXPECT_EQ(data[0], 0x66u);
                EXPECT_EQ(data[1], 0x00u);
                EXPECT_EQ(data[2], 0x00u);
                EXPECT_EQ(data[3], 0x00u);
                EXPECT_EQ(data[4], 0xFFu);
            }

            command_call_count++;
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .Times(2)
        .WillRepeatedly([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 16, 0xFF);
            data_read[0] = 0x00;
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u))
        .WillOnce([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 60, 0xFF);
            data_read[1] = 0x00;
            data_read[2] = 0xFF;
            return true;
        });

    EXPECT_TRUE(sd_card_.EraseRange(start_address, end_address));
    EXPECT_EQ(command_call_count, 3u);
}

TEST_F(SDCardTests, EraseRangeFailsWhenEndAddressPrecedesStartAddress)
{
    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).Times(0);
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), ::testing::_)).Times(0);
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 1084u)).Times(0);
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u)).Times(0);

    EXPECT_FALSE(sd_card_.EraseRange(0x20, 0x10));
}

TEST_F(SDCardTests, EraseRangeFailsWhenStartCommandIsRejected)
{
    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 16, 0xFF);
            data_read[0] = 0x04;
            return true;
        });

    EXPECT_FALSE(sd_card_.EraseRange(0x10, 0x20));
}

TEST_F(SDCardTests, EraseRangeFailsWhenEndCommandIsRejected)
{
    size_t response_call_count = 0;

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
        .Times(2)
        .WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .Times(2)
        .WillRepeatedly([&](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 16, 0xFF);
            data_read[0] = response_call_count++ == 0 ? 0x00 : 0x04;
            return true;
        });

    EXPECT_FALSE(sd_card_.EraseRange(0x10, 0x20));
}

TEST_F(SDCardTests, EraseRangeSucceedsWhenBusyPollingSucceedsOnSecondAttempt)
{
    size_t busy_call_count = 0;

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
        .Times(3)
        .WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .Times(2)
        .WillRepeatedly([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 16, 0xFF);
            data_read[0] = 0x00;
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u))
        .Times(2)
        .WillRepeatedly([&](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 60, 0x00);
            if (busy_call_count++ == 1)
            {
                data_read[59] = 0xFF;
            }
            return true;
        });

    EXPECT_TRUE(sd_card_.EraseRange(0x10, 0x20));
}

TEST_F(SDCardTests, EraseRangeFailsWhenCardRemainsBusy)
{
    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
        .Times(3)
        .WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .Times(2)
        .WillRepeatedly([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 16, 0xFF);
            data_read[0] = 0x00;
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u))
        .Times(5)
        .WillRepeatedly([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 60, 0x00);
            return true;
        });

    EXPECT_FALSE(sd_card_.EraseRange(0x10, 0x20));
}

TEST_F(SDCardTests, WriteDataSucceedsWhenSdCardAcceptsSingleBlockWrite)
{
    const uint32_t address = 0x0000;
    std::array<uint8_t, 512> payload = {};
    std::array<uint8_t, 512 + 3> block = {};
    std::array<uint8_t, 60> busy_response = {};
    std::array<uint8_t, 16> cmd_response = {0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    size_t write_call_count = 0;
    size_t read_call_count = 0;

    for (uint16_t i = 0; i < payload.size(); ++i)
    {
        payload[i] = static_cast<uint8_t>((i + 7) & 0xFF);
    }

    block[0] = 0xFE;
    std::copy(payload.begin(), payload.end(), block.begin() + 1);
    block[513] = 0x01;
    block[514] = 0x02;

    busy_response[0] = 0x05;
    for (size_t i = 1; i < busy_response.size(); ++i)
    {
        busy_response[i] = 0xFF;
    }

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
        .Times(2)
        .WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 515u))
        .WillOnce([&](const uint8_t *data, uint16_t data_size) {
            EXPECT_EQ(data_size, 515u);
            EXPECT_EQ(data[0], 0xFEu);
            EXPECT_EQ(std::memcmp(data + 1, payload.data(), payload.size()), 0);
            EXPECT_EQ(data[513], 0x01u);
            EXPECT_EQ(data[514], 0x02u);
            write_call_count++;
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, cmd_response.data(), cmd_response.size());
            read_call_count++;
            return true;
        })
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            const std::array<uint8_t, 16> final_status = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            std::memcpy(data_read, final_status.data(), final_status.size());
            read_call_count++;
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u))
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, busy_response.data(), busy_response.size());
            read_call_count++;
            return true;
        });

    EXPECT_EQ(sd_card_.WriteData(address, payload.data(), payload.size()), 512u);
}

TEST_F(SDCardTests, ReadMultipleBlocksReturnsCompleteBlocksForNonAlignedBufferSize)
{
    const uint32_t address = 0x00000010;
    const uint16_t complete_bytes = 3 * 512;
    const uint16_t requested_bytes = complete_bytes + 100;
    const uint16_t stream_size = complete_bytes + 3 * 3 + 51;
    std::array<uint8_t, requested_bytes> actual;
    std::array<uint8_t, stream_size> stream;
    std::array<std::array<uint8_t, 512>, 3> expected = {};

    actual.fill(0xA5);
    stream.fill(0xFF);

    for (size_t block = 0; block < expected.size(); ++block)
    {
        for (size_t byte = 0; byte < expected[block].size(); ++byte)
        {
            expected[block][byte] = static_cast<uint8_t>((block * 17 + byte) & 0xFF);
        }

        const size_t block_offset = block * (1 + 512 + 2);
        stream[block_offset] = 0xFE;
        std::copy(expected[block].begin(), expected[block].end(), stream.begin() + block_offset + 1);

        const uint16_t crc = HAL::Utils::CRC::CRC16(expected[block].data(), expected[block].size());
        stream[block_offset + 513] = static_cast<uint8_t>((crc >> 8) & 0xFF);
        stream[block_offset + 514] = static_cast<uint8_t>(crc & 0xFF);
    }

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
        .Times(2)
        .WillRepeatedly([&](const uint8_t *data, uint16_t data_size) {
            EXPECT_EQ(data_size, 6u);
            if (data[0] == 0x52u)
            {
                EXPECT_EQ(data[4], 0x10u);
            }
            else
            {
                EXPECT_EQ(data[0], 0x4Cu);
                EXPECT_EQ(data[1], 0x00u);
                EXPECT_EQ(data[2], 0x00u);
                EXPECT_EQ(data[3], 0x00u);
                EXPECT_EQ(data[4], 0x00u);
            }
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 16, 0xFF);
            data_read[0] = 0x00;
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), stream_size))
        .WillOnce([&](uint8_t *data_read, uint16_t data_size) {
            EXPECT_EQ(data_size, stream_size);
            std::memcpy(data_read, stream.data(), stream.size());
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u))
        .WillOnce([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 60, 0xFF);
            data_read[0] = 0x00;
            data_read[1] = 0xFF;
            return true;
        });

    EXPECT_EQ(sd_card_.ReadData(address, actual.data(), actual.size()), complete_bytes);

    for (size_t block = 0; block < expected.size(); ++block)
    {
        EXPECT_TRUE(std::equal(expected[block].begin(), expected[block].end(), actual.begin() + block * 512));
    }
    EXPECT_TRUE(std::all_of(actual.begin() + complete_bytes, actual.end(), [](uint8_t value) {
        return value == 0xA5;
    }));
}

TEST_F(SDCardTests, ReadMultipleBlocksReturnsZeroWhenCmd18IsRejected)
{
    std::array<uint8_t, 1024> actual = {};

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 16, 0xFF);
            data_read[0] = 0x04;
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 1084u)).Times(0);
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u)).Times(0);

    EXPECT_EQ(sd_card_.ReadData(0x10, actual.data(), actual.size()), 0u);
}

TEST_F(SDCardTests, ReadMultipleBlocksReturnsZeroWhenBlockStreamReadFails)
{
    std::array<uint8_t, 1024> actual = {};

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 16, 0xFF);
            data_read[0] = 0x00;
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 1084u))
        .WillOnce(::testing::Return(false));
    EXPECT_CALL(*spi_comm_, SetCSPin(1)).Times(2);

    EXPECT_EQ(sd_card_.ReadData(0x10, actual.data(), actual.size()), 0u);
}

TEST_F(SDCardTests, ReadMultipleBlocksReturnsZeroWhenADataBlockIsInvalid)
{
    std::array<uint8_t, 1024> actual = {};
    std::array<uint8_t, 1084> stream = {};
    std::array<uint8_t, 512> first_payload = {};

    stream.fill(0xFF);
    stream[0] = 0xFE;
    std::copy(first_payload.begin(), first_payload.end(), stream.begin() + 1);
    const uint16_t first_crc = HAL::Utils::CRC::CRC16(first_payload.data(), first_payload.size());
    stream[513] = static_cast<uint8_t>((first_crc >> 8) & 0xFF);
    stream[514] = static_cast<uint8_t>(first_crc & 0xFF);
    stream[515] = 0x00;

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
        .Times(2)
        .WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 16, 0xFF);
            data_read[0] = 0x00;
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 1084u))
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, stream.data(), stream.size());
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u))
        .WillOnce([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 60, 0xFF);
            data_read[0] = 0x00;
            data_read[1] = 0xFF;
            return true;
        });

    EXPECT_EQ(sd_card_.ReadData(0x10, actual.data(), actual.size()), 512u);
}

TEST_F(SDCardTests, ReadMultipleBlocksReturnsZeroWhenStopCommandFails)
{
    std::array<uint8_t, 1024> actual = {};
    std::array<uint8_t, 1084> stream = {};

    stream.fill(0xFF);
    stream[0] = 0xFE;

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u))
        .Times(2)
        .WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 16, 0xFF);
            data_read[0] = 0x00;
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 1084u))
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, stream.data(), stream.size());
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u))
        .Times(5)
        .WillRepeatedly([](uint8_t *data_read, uint16_t) {
            std::fill(data_read, data_read + 60, 0x00);
            return true;
        });

    EXPECT_EQ(sd_card_.ReadData(0x10, actual.data(), actual.size()), 0u);
}

TEST_F(SDCardTests, WriteDataFailsWhenCmd24ResponseIsNotIdle)
{
    const uint32_t address = 0x0001;
    std::array<uint8_t, 512> payload = {};
    std::array<uint8_t, 16> cmd_response = {0xFF, 0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, cmd_response.data(), cmd_response.size());
            return true;
        });

    EXPECT_EQ(sd_card_.WriteData(address, payload.data(), payload.size()), 0u);
}

TEST_F(SDCardTests, WriteDataFailsWhenDataResponseTokenIsNotAccepted)
{
    const uint32_t address = 0x0002;
    std::array<uint8_t, 512> payload = {};
    std::array<uint8_t, 16> cmd_response = {0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    std::array<uint8_t, 60> busy_response = {};

    busy_response[0] = 0x00;
    for (size_t i = 1; i < busy_response.size(); ++i)
    {
        busy_response[i] = 0xFF;
    }

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).Times(1).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 515u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, cmd_response.data(), cmd_response.size());
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u))
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, busy_response.data(), busy_response.size());
            return true;
        });

    EXPECT_EQ(sd_card_.WriteData(address, payload.data(), payload.size()), 0u);
}

TEST_F(SDCardTests, WriteDataFailsWhenCardRemainsBusy)
{
    const uint32_t address = 0x0003;
    std::array<uint8_t, 512> payload = {};
    std::array<uint8_t, 16> cmd_response = {0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    std::array<uint8_t, 60> busy_response = {};

    busy_response[0] = 0x05;
    std::fill(busy_response.begin() + 1, busy_response.end(), 0x00);

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).Times(1).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 515u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .Times(1)
        .WillRepeatedly([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, cmd_response.data(), cmd_response.size());
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u))
        .Times(1)
        .WillRepeatedly([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, busy_response.data(), busy_response.size());
            return true;
        });

    EXPECT_EQ(sd_card_.WriteData(address, payload.data(), payload.size()), 0u);
}

TEST_F(SDCardTests, WriteDataFailsWhenCmd13StatusIsNotZero)
{
    const uint32_t address = 0x0004;
    std::array<uint8_t, 512> payload = {};
    std::array<uint8_t, 16> cmd_response = {0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                           0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    std::array<uint8_t, 60> busy_response = {};
    std::array<uint8_t, 16> final_status = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    busy_response[0] = 0x05;
    for (size_t i = 1; i < busy_response.size(); ++i)
    {
        busy_response[i] = 0xFF;
    }

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).Times(2).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 515u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, cmd_response.data(), cmd_response.size());
            return true;
        })
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, final_status.data(), final_status.size());
            return true;
        });
    EXPECT_CALL(*spi_comm_, ReadData(::testing::An<uint8_t*>(), 60u))
        .WillOnce([&](uint8_t *data_read, uint16_t) {
            std::memcpy(data_read, busy_response.data(), busy_response.size());
            return true;
        });

    EXPECT_EQ(sd_card_.WriteData(address, payload.data(), payload.size()), 0u);
}

TEST_F(SDCardTests, ReadDataReturns512BytesWhenSdCardRespondsWithValidBlock)
{
    const uint32_t address = 0x0000;
    std::array<uint8_t, 512> payload = {};
    std::array<uint8_t, 1 + 1 + 512 + 2 + 60> stream = {};
    std::array<uint8_t, 512> expected = {};

    for (uint16_t i = 0; i < expected.size(); ++i)
    {
        expected[i] = static_cast<uint8_t>(i & 0xFF);
    }

    stream[0] = 0xFF;
    stream[1] = 0xFF;
    stream[2] = 0x00;
    stream[3] = 0xFE;

    std::copy(expected.begin(), expected.end(), stream.begin() + 4);

    uint16_t crc = HAL::Utils::CRC::CRC16(expected.data(), expected.size());
    stream[4 + expected.size()] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    stream[5 + expected.size()] = static_cast<uint8_t>(crc & 0xFF);

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteReadData(::testing::An<const uint8_t*>(), ::testing::An<uint8_t*>(), ::testing::_))
        .WillOnce([&](const uint8_t *data_write, uint8_t *data_read, uint16_t data_size) {
            EXPECT_EQ(data_write[0], 0x51u);
            EXPECT_EQ(data_write[1], 0x00u);
            EXPECT_EQ(data_write[2], 0x00u);
            EXPECT_EQ(data_write[3], 0x00u);
            EXPECT_EQ(data_write[4], 0x00u);
            EXPECT_EQ(data_size, stream.size());
            std::memcpy(data_read, stream.data(), stream.size());
            return true;
        });

    std::array<uint8_t, 512> actual = {};
    EXPECT_EQ(sd_card_.ReadData(address, actual.data(), actual.size()), 512u);
    EXPECT_EQ(actual, expected);
}

TEST_F(SDCardTests, ReadDataSkipsIdleClocksBeforeCommandResponse)
{
    const uint32_t address = 0x0001;
    std::array<uint8_t, 512> payload = {};
    std::array<uint8_t, 1 + 1 + 512 + 2 + 60> stream = {};
    std::array<uint8_t, 512> expected = {};

    for (uint16_t i = 0; i < expected.size(); ++i)
    {
        expected[i] = static_cast<uint8_t>((i + 5) & 0xFF);
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        stream[i] = 0xFF;
    }

    stream[5] = 0x00;
    stream[6] = 0xFE;
    std::copy(expected.begin(), expected.end(), stream.begin() + 7);

    uint16_t crc = HAL::Utils::CRC::CRC16(expected.data(), expected.size());
    stream[7 + expected.size()] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    stream[8 + expected.size()] = static_cast<uint8_t>(crc & 0xFF);

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteReadData(::testing::An<const uint8_t*>(), ::testing::An<uint8_t*>(), ::testing::_))
        .WillOnce([&](const uint8_t *data_write, uint8_t *data_read, uint16_t data_size) {
            EXPECT_EQ(data_write[0], 0x51u);
            EXPECT_EQ(data_write[1], 0x00u);
            EXPECT_EQ(data_write[2], 0x00u);
            EXPECT_EQ(data_write[3], 0x00u);
            EXPECT_EQ(data_write[4], 0x01u);
            EXPECT_EQ(data_size, stream.size());
            std::memcpy(data_read, stream.data(), stream.size());
            return true;
        });

    std::array<uint8_t, 512> actual = {};
    EXPECT_EQ(sd_card_.ReadData(address, actual.data(), actual.size()), 512u);
    EXPECT_EQ(actual, expected);
}

TEST_F(SDCardTests, ReadDataFailsWhenCommandResponseIsNotIdle)
{
    const uint32_t address = 0x0002;
    std::array<uint8_t, 1 + 1 + 512 + 2 + 60> stream = {};
    std::array<uint8_t, 512> actual = {};

    stream[0] = 0xFF;
    stream[1] = 0x01;

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteReadData(::testing::An<const uint8_t*>(), ::testing::An<uint8_t*>(), ::testing::_))
        .WillOnce([&](const uint8_t *data_write, uint8_t *data_read, uint16_t data_size) {
            EXPECT_EQ(data_write[0], 0x51u);
            EXPECT_EQ(data_size, stream.size());
            std::memcpy(data_read, stream.data(), stream.size());
            return true;
        });

    EXPECT_EQ(sd_card_.ReadData(address, actual.data(), actual.size()), 0u);
}

TEST_F(SDCardTests, ReadDataFailsWhenDataTokenIsNotStartBlock)
{
    const uint32_t address = 0x0003;
    std::array<uint8_t, 1 + 1 + 512 + 2 + 60> stream = {};
    std::array<uint8_t, 512> actual = {};
    std::array<uint8_t, 512> payload = {};

    stream[0] = 0xFF;
    stream[1] = 0xFF;
    stream[2] = 0x00;
    stream[3] = 0x00;
    std::copy(payload.begin(), payload.end(), stream.begin() + 4);

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteReadData(::testing::An<const uint8_t*>(), ::testing::An<uint8_t*>(), ::testing::_))
        .WillOnce([&](const uint8_t *data_write, uint8_t *data_read, uint16_t data_size) {
            EXPECT_EQ(data_write[0], 0x51u);
            EXPECT_EQ(data_size, stream.size());
            std::memcpy(data_read, stream.data(), stream.size());
            return true;
        });

    EXPECT_EQ(sd_card_.ReadData(address, actual.data(), actual.size()), 0u);
}

TEST_F(SDCardTests, ReadDataFailsWhenCrcDoesNotMatch)
{
    const uint32_t address = 0x0004;
    std::array<uint8_t, 512> payload = {};
    std::array<uint8_t, 1 + 1 + 512 + 2 + 60> stream = {};
    std::array<uint8_t, 512> actual = {};

    for (uint16_t i = 0; i < payload.size(); ++i)
    {
        payload[i] = static_cast<uint8_t>(i & 0xFF);
    }

    stream[0] = 0xFF;
    stream[1] = 0xFF;
    stream[2] = 0x00;
    stream[3] = 0xFE;
    std::copy(payload.begin(), payload.end(), stream.begin() + 4);

    uint16_t crc = HAL::Utils::CRC::CRC16(payload.data(), payload.size());
    stream[4 + payload.size()] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    stream[5 + payload.size()] = static_cast<uint8_t>((crc & 0xFF) ^ 0xFF);

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteReadData(::testing::An<const uint8_t*>(), ::testing::An<uint8_t*>(), ::testing::_))
        .WillOnce([&](const uint8_t *data_write, uint8_t *data_read, uint16_t data_size) {
            EXPECT_EQ(data_write[0], 0x51u);
            EXPECT_EQ(data_size, stream.size());
            std::memcpy(data_read, stream.data(), stream.size());
            return true;
        });

    EXPECT_EQ(sd_card_.ReadData(address, actual.data(), actual.size()), 0u);
}

TEST_F(SDCardTests, ReadDataReturnsZeroWhenBufferIsTooSmall)
{
    const uint32_t address = 0x0005;
    std::array<uint8_t, 32> actual = {};

    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).Times(0);

    EXPECT_EQ(sd_card_.ReadData(address, actual.data(), actual.size()), 0u);
}

}
}

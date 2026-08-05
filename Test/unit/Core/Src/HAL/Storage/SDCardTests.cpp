#include "Storage/SDCard.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <array>
#include <cstring>

#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockSPICommunicationInterface.h"

using HAL::Devices::Communication::Interfaces::MockSPICommunicationInterface;

namespace HAL
{
namespace Storage
{

class SDCardHelper : public SDCard
{
public:
    using SDCard::GetCmdResponseSizeBytes;
    using SDCard::GetStartValidByteFromBuffer;
    using SDCard::SendCommand;

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

    SDCardHelper(std::shared_ptr<MockSPICommunicationInterface> spi_comm) : SDCard(spi_comm)
    {
    }
};

class SDCardTests : public ::testing::Test
{
public:
    SDCardTests() :
        spi_comm_(std::make_shared<MockSPICommunicationInterface>()),
        sd_card_(spi_comm_)
    {
    }

    std::shared_ptr<MockSPICommunicationInterface> spi_comm_;
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
    EXPECT_CALL(*spi_comm_, WriteReadData(::testing::An<const uint8_t*>(), ::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](const uint8_t *data_write, uint8_t *data_read, uint16_t data_size) {
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
    EXPECT_CALL(*spi_comm_, WriteReadData(::testing::An<const uint8_t*>(), ::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](const uint8_t *data_write, uint8_t *data_read, uint16_t data_size) {
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
    EXPECT_CALL(*spi_comm_, WriteReadData(::testing::An<const uint8_t*>(), ::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](const uint8_t *data_write, uint8_t *data_read, uint16_t data_size) {
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
    EXPECT_CALL(*spi_comm_, WriteReadData(::testing::An<const uint8_t*>(), ::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](const uint8_t *data_write, uint8_t *data_read, uint16_t data_size) {
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
    EXPECT_CALL(*spi_comm_, WriteReadData(::testing::An<const uint8_t*>(), ::testing::An<uint8_t*>(), 16u))
        .Times(6)
        .WillRepeatedly([&](const uint8_t *, uint8_t *data_read, uint16_t) {
            std::array<uint8_t, 16> read_back = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

            switch (write_read_call_count++)
            {
                case 0:
                    read_back = {0xFF, 0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    break;
                case 1:
                    read_back = {0xAA, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                    break;
                case 2:
                    read_back = {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    break;
                case 3:
                    read_back = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    break;
                case 4:
                    read_back = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
    EXPECT_EQ(sd_card_.CurrentVersion(), 1u);
}

TEST_F(SDCardTests, InitStorageFailsWhenCardDoesNotLeaveIdleState)
{
    EXPECT_CALL(*spi_comm_, SetCSPin(::testing::_)).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 75u)).WillOnce(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteData(::testing::An<const uint8_t*>(), 6u)).Times(1).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*spi_comm_, WriteReadData(::testing::An<const uint8_t*>(), ::testing::An<uint8_t*>(), 16u))
        .WillOnce([&](const uint8_t *, uint8_t *data_read, uint16_t) {
            const std::array<uint8_t, 16> read_back = {0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            memcpy(data_read, read_back.data(), read_back.size());
            return true;
        });

    ASSERT_FALSE(sd_card_.InitStorageWrapper());
    EXPECT_EQ(sd_card_.CurrentVersion(), 3u);
}

}
}

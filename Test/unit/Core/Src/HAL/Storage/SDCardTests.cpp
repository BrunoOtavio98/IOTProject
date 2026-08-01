#include "Storage/SDCard.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
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

TEST_F(SDCardTests, SendCommandBuildsCommandFrameAndReturnsResponse)
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

}
}
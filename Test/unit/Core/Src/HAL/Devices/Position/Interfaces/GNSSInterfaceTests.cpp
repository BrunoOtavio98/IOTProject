#include "Devices/Position/GNSSInterface.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockUartCommunicationInterface.h"
#include "Core/Src/HAL/DebugController/Mocks/MockDebugController.h"

using HAL::Devices::Communication::Interfaces::MockUartCommunicationInterface;
using HAL::DebugController::MockDebugController;

namespace HAL
{
namespace Devices
{
namespace Position
{

class GNSSInterfaceHelper: public GNSSInterface 
{
    public:
        GNSSInterfaceHelper( std::shared_ptr<MockUartCommunicationInterface> gnss_uart,
                             std::shared_ptr<MockDebugController> debug_controller) :
                             GNSSInterface(gnss_uart, debug_controller)
        {
        }

        uint16_t GetInsertBufferCtrl() { return insert_buffer_ctrl_; }
        uint16_t GetProcessBufferCtrl() { return process_buffer_ctrl_; }
        void ClearRxBuffer() { uart_buffer_receive_[0] = '\0'; }
        int GetRxBufferSize() { return kRxBufferSize; }


    using GNSSInterface::UpdateGNSSData;
    using GNSSInterface::GetNMEAFrame;
    using GNSSInterface::UartCallBack;

};

class GNSSInterfaceTests : public testing::Test
{   
    public:
        GNSSInterfaceTests() :
            gnss_uart_(std::make_shared<MockUartCommunicationInterface>()),
            debug_controller_(std::make_shared<MockDebugController>(gnss_uart_)),
            gnss_interface_(gnss_uart_, debug_controller_)
        {

        }

        void SetUp()
        {   
            gnss_interface_.ClearRxBuffer();
        }
    
    std::shared_ptr<MockUartCommunicationInterface> gnss_uart_;
    std::shared_ptr<MockDebugController> debug_controller_;
    GNSSInterfaceHelper gnss_interface_;
};

TEST_F(GNSSInterfaceTests, TestGSAData)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::GSA;
    nmea_data.payload_received.gsa_data.pdop = 0.75;
    nmea_data.payload_received.gsa_data.hdop = 0.25;
    nmea_data.payload_received.gsa_data.vdop = 1.2412;

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );

    EXPECT_EQ( gnss_data.vdop, nmea_data.payload_received.gsa_data.vdop );
    EXPECT_EQ( gnss_data.hdop, nmea_data.payload_received.gsa_data.hdop );
    EXPECT_EQ( gnss_data.pdop, nmea_data.payload_received.gsa_data.pdop );
}

TEST_F(GNSSInterfaceTests, TestRMCData)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::RMC;
    nmea_data.payload_received.rmc_data.year = (uint8_t)2026;
    nmea_data.payload_received.rmc_data.month = (uint8_t)02;
    nmea_data.payload_received.rmc_data.day = (uint8_t)21;

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );

    EXPECT_EQ( gnss_data.year, nmea_data.payload_received.rmc_data.year );
    EXPECT_EQ( gnss_data.month, nmea_data.payload_received.rmc_data.month );
    EXPECT_EQ( gnss_data.day, nmea_data.payload_received.rmc_data.day );
}

TEST_F(GNSSInterfaceTests, TestVTGData)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::VTG;
    nmea_data.payload_received.vtg_data.speedKmh = 83;
    nmea_data.payload_received.vtg_data.trueTrack = 125;

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );

    EXPECT_EQ( gnss_data.speedKmh, nmea_data.payload_received.vtg_data.speedKmh );
    EXPECT_EQ( gnss_data.courseDeg, nmea_data.payload_received.vtg_data.trueTrack );
}

TEST_F(GNSSInterfaceTests, TestGGADataWithoutPosition)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::GGA;
    nmea_data.payload_received.gga_data.mslAltitude = 83;
    nmea_data.payload_received.gga_data.hour = 06;
    nmea_data.payload_received.gga_data.minutes = 59;
    nmea_data.payload_received.gga_data.seconds = 37;

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );


    EXPECT_EQ( gnss_data.alt, nmea_data.payload_received.gga_data.mslAltitude );
    EXPECT_EQ( gnss_data.hour, nmea_data.payload_received.gga_data.hour );
    EXPECT_EQ( gnss_data.minutes, nmea_data.payload_received.gga_data.minutes );
    EXPECT_EQ( gnss_data.seconds, nmea_data.payload_received.gga_data.seconds );
}

TEST_F(GNSSInterfaceTests, TestGGAPositionNE)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::GGA;
    nmea_data.payload_received.gga_data.latitude = 4807.038;
    nmea_data.payload_received.gga_data.longitude = 01131.000;
    nmea_data.payload_received.gga_data.NSIndicator = 'N';
    nmea_data.payload_received.gga_data.EWIndicator = 'E';

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );

    EXPECT_NEAR( gnss_data.lat, 48.117300f, 0.0001f );
    EXPECT_NEAR( gnss_data.lon, 11.516667f, 0.0001f);
}

TEST_F(GNSSInterfaceTests, TestGGAPositionNW)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::GGA;
    nmea_data.payload_received.gga_data.latitude = 3751.650;
    nmea_data.payload_received.gga_data.longitude = 14507.360;
    nmea_data.payload_received.gga_data.NSIndicator = 'N';
    nmea_data.payload_received.gga_data.EWIndicator = 'W';

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );

    EXPECT_NEAR( gnss_data.lat, 37.860833, 0.0001f );
    EXPECT_NEAR( gnss_data.lon, -145.122667, 0.0001f);
}

TEST_F(GNSSInterfaceTests, TestGGAPositionSE)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::GGA;
    nmea_data.payload_received.gga_data.latitude = 3751.650;
    nmea_data.payload_received.gga_data.longitude = 14507.360;
    nmea_data.payload_received.gga_data.NSIndicator = 'S';
    nmea_data.payload_received.gga_data.EWIndicator = 'E';

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );

    EXPECT_NEAR( gnss_data.lat, -37.860833, 0.0001f );
    EXPECT_NEAR( gnss_data.lon, 145.122667, 0.0001f);
}

TEST_F(GNSSInterfaceTests, TestGGAPositionSW)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::GGA;
    nmea_data.payload_received.gga_data.latitude = 2233.000;
    nmea_data.payload_received.gga_data.longitude = 04312.000;
    nmea_data.payload_received.gga_data.NSIndicator = 'S';
    nmea_data.payload_received.gga_data.EWIndicator = 'W';

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );

    EXPECT_NEAR( gnss_data.lat, -22.550000, 0.0001f );
    EXPECT_NEAR( gnss_data.lon, -43.200000, 0.0001f);
}

TEST_F(GNSSInterfaceTests, TestGGAPositionEquatorPrimeMeridian)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::GGA;
    nmea_data.payload_received.gga_data.latitude = 0000.000;
    nmea_data.payload_received.gga_data.longitude = 00000.000;
    nmea_data.payload_received.gga_data.NSIndicator = 'N';
    nmea_data.payload_received.gga_data.EWIndicator = 'E';

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );

    EXPECT_NEAR( gnss_data.lat, 0.000000, 0.0001f );
    EXPECT_NEAR( gnss_data.lon, 0.000000, 0.0001f);
}

TEST_F(GNSSInterfaceTests, TestGGAPositionEquatorWest)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::GGA;
    nmea_data.payload_received.gga_data.latitude = 0000.000;
    nmea_data.payload_received.gga_data.longitude = 07730.000;
    nmea_data.payload_received.gga_data.NSIndicator = 'N';
    nmea_data.payload_received.gga_data.EWIndicator = 'W';

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );

    EXPECT_NEAR( gnss_data.lat, 0.000000, 0.0001f );
    EXPECT_NEAR( gnss_data.lon, -77.500000, 0.0001f);
}

TEST_F(GNSSInterfaceTests, TestGGAPositionMaxValidLatitude)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::GGA;
    nmea_data.payload_received.gga_data.latitude = 8959.999;
    nmea_data.payload_received.gga_data.longitude = 17959.999;
    nmea_data.payload_received.gga_data.NSIndicator = 'N';
    nmea_data.payload_received.gga_data.EWIndicator = 'E';

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );

    EXPECT_NEAR( gnss_data.lat, 89.999983, 0.0001f );
    EXPECT_NEAR( gnss_data.lon, 179.999983, 0.0001f);
}

TEST_F(GNSSInterfaceTests, TestGGAPositionMinValidLatitude)
{
    NMEAParser::NMEA_Data nmea_data;
    GNSSInterface::BasicGNSSData gnss_data;

    nmea_data.nmea_type = NMEAParser::GGA;
    nmea_data.payload_received.gga_data.latitude = 8959.999;
    nmea_data.payload_received.gga_data.longitude = 17959.999;
    nmea_data.payload_received.gga_data.NSIndicator = 'S';
    nmea_data.payload_received.gga_data.EWIndicator = 'W';

    gnss_interface_.UpdateGNSSData( nmea_data );
    gnss_interface_.GetUpdatedGnssData( gnss_data );

    EXPECT_NEAR( gnss_data.lat, -89.999983, 0.0001f );
    EXPECT_NEAR( gnss_data.lon, -179.999983, 0.0001f);
}

TEST_F(GNSSInterfaceTests, TestWriteOnceReadOnce)
{
    std::string test_string = "ABC\r\n";
    std::string final_string;
    const uint8_t* raw_string = reinterpret_cast<const uint8_t*>(test_string.data());

    gnss_interface_.UartCallBack( raw_string, test_string.size() );

    EXPECT_EQ( gnss_interface_.GetInsertBufferCtrl(), test_string.size() );
    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string) );
    EXPECT_EQ( test_string.size() - 2, final_string.size() );
    EXPECT_EQ( "ABC", final_string );
}

TEST_F(GNSSInterfaceTests, TestConcatenatedStrings)
{
    std::string test_string = "ABC\r\nDEFG\r\nBruno\r\n";
    std::string final_string;
    const uint8_t* raw_string = reinterpret_cast<const uint8_t*>(test_string.data());

    gnss_interface_.UartCallBack( raw_string, test_string.size() );

    EXPECT_EQ( gnss_interface_.GetInsertBufferCtrl(), test_string.size() );
    
    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string) );
    EXPECT_EQ( 3, final_string.size() );
    EXPECT_EQ( "ABC", final_string );
    EXPECT_EQ( 5, gnss_interface_.GetProcessBufferCtrl() );

    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string) );
    EXPECT_EQ( 4, final_string.size() );
    EXPECT_EQ( "DEFG", final_string );
    EXPECT_EQ( 11, gnss_interface_.GetProcessBufferCtrl() );

    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string) );
    EXPECT_EQ( 5, final_string.size() );
    EXPECT_EQ( "Bruno", final_string );
    EXPECT_EQ( 18, gnss_interface_.GetProcessBufferCtrl() );
}

TEST_F(GNSSInterfaceTests, TestNoCRLFOneMessage)
{
    std::string test_string = "HELLO";
    std::string final_string;
    const uint8_t* raw_string = reinterpret_cast<const uint8_t*>(test_string.data());

    gnss_interface_.UartCallBack( raw_string, test_string.size() );

    EXPECT_EQ( gnss_interface_.GetInsertBufferCtrl(), test_string.size() );
    
    EXPECT_FALSE( gnss_interface_.GetNMEAFrame(final_string) );
    EXPECT_EQ( gnss_interface_.GetProcessBufferCtrl(), 0 );
}

TEST_F(GNSSInterfaceTests, TestMultipleUartEvents)
{
    std::string test_string1 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean sit amet suscipit nunc, at molestie.\r\n";
    std::string final_string1;
    const uint8_t* raw_string1 = reinterpret_cast<const uint8_t*>(test_string1.data());

    std::string test_string2 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut vitae erat tellus. Nunc nibh nulla.\r\n";
    std::string final_string2;
    const uint8_t* raw_string2 = reinterpret_cast<const uint8_t*>(test_string2.data());

    std::string test_string3 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\r\n";
    std::string final_string3;
    const uint8_t* raw_string3 = reinterpret_cast<const uint8_t*>(test_string3.data());

    uint16_t expect_str_size = test_string1.size();

    gnss_interface_.UartCallBack( raw_string1, test_string1.size() );
    EXPECT_EQ( gnss_interface_.GetInsertBufferCtrl(), expect_str_size );

    expect_str_size += test_string2.size();
    gnss_interface_.UartCallBack( raw_string2, test_string2.size() );
    EXPECT_EQ( gnss_interface_.GetInsertBufferCtrl(), expect_str_size );

    expect_str_size += test_string3.size();
    gnss_interface_.UartCallBack( raw_string3, test_string3.size() );
    EXPECT_EQ( gnss_interface_.GetInsertBufferCtrl(), expect_str_size );

    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string1) );
    EXPECT_EQ( 100, final_string1.size() );
    EXPECT_EQ( "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean sit amet suscipit nunc, at molestie.", final_string1 );
    EXPECT_EQ( 102, gnss_interface_.GetProcessBufferCtrl() );

    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string2) );
    EXPECT_EQ( 95, final_string2.size() );
    EXPECT_EQ( "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut vitae erat tellus. Nunc nibh nulla.", final_string2 );
    EXPECT_EQ( 199, gnss_interface_.GetProcessBufferCtrl() );

    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string3) );
    EXPECT_EQ( 56, final_string3.size() );
    EXPECT_EQ( "Lorem ipsum dolor sit amet, consectetur adipiscing elit.", final_string3 );
    EXPECT_EQ( 257, gnss_interface_.GetProcessBufferCtrl() );
}

TEST_F(GNSSInterfaceTests, TestMultipleUartEventsWritWrap)
{
    std::string test_string1 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean sit amet suscipit nunc, at molestie.\r\n";
    std::string final_string1;
    const uint8_t* raw_string1 = reinterpret_cast<const uint8_t*>(test_string1.data());

    std::string test_string2 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut vitae erat tellus. Nunc nibh nulla.\r\n";
    std::string final_string2;
    const uint8_t* raw_string2 = reinterpret_cast<const uint8_t*>(test_string2.data());

    std::string test_string3 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\r\n";
    std::string final_string3;
    const uint8_t* raw_string3 = reinterpret_cast<const uint8_t*>(test_string3.data());

    std::string test_string4 = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum aliquet sagittis arcu, id condimentum lectus suscipit quis.\r\n";
    std::string final_string4;
    const uint8_t* raw_string4 = reinterpret_cast<const uint8_t*>(test_string4.data());

    std::string test_string5 = "Morbi placerat pellentesque tristique. Pellentesque est nisi, varius a scelerisque quis, hendrerit in enim. Maecenas lacus sapien, aliquet vitae orci eget, mollis interdum enim efficitur.\r\n";
    std::string final_string5;
    const uint8_t* raw_string5 = reinterpret_cast<const uint8_t*>(test_string5.data());

    uint16_t expect_str_size = test_string1.size();

    gnss_interface_.UartCallBack( raw_string1, test_string1.size() );
    EXPECT_EQ( gnss_interface_.GetInsertBufferCtrl(), expect_str_size );

    expect_str_size += test_string2.size();
    gnss_interface_.UartCallBack( raw_string2, test_string2.size() );
    EXPECT_EQ( gnss_interface_.GetInsertBufferCtrl(), expect_str_size );

    expect_str_size += test_string3.size();
    gnss_interface_.UartCallBack( raw_string3, test_string3.size() );
    EXPECT_EQ( gnss_interface_.GetInsertBufferCtrl(), expect_str_size );

    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string1) );
    EXPECT_EQ( 100, final_string1.size() );
    EXPECT_EQ( "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean sit amet suscipit nunc, at molestie.", final_string1 );
    EXPECT_EQ( 102, gnss_interface_.GetProcessBufferCtrl() );

    expect_str_size += test_string4.size();
    gnss_interface_.UartCallBack( raw_string4, test_string4.size() );
    EXPECT_EQ( gnss_interface_.GetInsertBufferCtrl(), expect_str_size );

    // Wrap happens here
    expect_str_size += test_string5.size();
    expect_str_size = expect_str_size % gnss_interface_.GetRxBufferSize();

    gnss_interface_.UartCallBack( raw_string5, test_string5.size() );
    EXPECT_EQ( gnss_interface_.GetInsertBufferCtrl(), expect_str_size );

    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string2) );
    EXPECT_EQ( 95, final_string2.size() );
    EXPECT_EQ( "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut vitae erat tellus. Nunc nibh nulla.", final_string2 );
    EXPECT_EQ( 199, gnss_interface_.GetProcessBufferCtrl() );

    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string3) );
    EXPECT_EQ( 56, final_string3.size() );
    EXPECT_EQ( "Lorem ipsum dolor sit amet, consectetur adipiscing elit.", final_string3 );
    EXPECT_EQ( 257, gnss_interface_.GetProcessBufferCtrl() );

    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string4) );
    EXPECT_EQ( 127, final_string4.size() );
    EXPECT_EQ( "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum aliquet sagittis arcu, id condimentum lectus suscipit quis.", final_string4 );
    EXPECT_EQ( 386, gnss_interface_.GetProcessBufferCtrl() );

    EXPECT_TRUE( gnss_interface_.GetNMEAFrame(final_string5) );
    EXPECT_EQ( 187, final_string5.size() );
    EXPECT_EQ( "Morbi placerat pellentesque tristique. Pellentesque est nisi, varius a scelerisque quis, hendrerit in enim. Maecenas lacus sapien, aliquet vitae orci eget, mollis interdum enim efficitur.", final_string5 );
    EXPECT_EQ( expect_str_size, gnss_interface_.GetProcessBufferCtrl() );
}

TEST_F(GNSSInterfaceTests, TestFragmentedFrame)
{
    std::string part1 = "ABC";
    std::string part2 = "\r";
    std::string part3 = "\n";

    gnss_interface_.UartCallBack(
        reinterpret_cast<const uint8_t*>(part1.data()),
        part1.size());

    gnss_interface_.UartCallBack(
        reinterpret_cast<const uint8_t*>(part2.data()),
        part2.size());

    gnss_interface_.UartCallBack(
        reinterpret_cast<const uint8_t*>(part3.data()),
        part3.size());

    std::string frame;

    EXPECT_TRUE(gnss_interface_.GetNMEAFrame(frame));
    EXPECT_EQ("ABC", frame);
}

}
}
}
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

    using GNSSInterface::UpdateGNSSData;
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

}
}
}
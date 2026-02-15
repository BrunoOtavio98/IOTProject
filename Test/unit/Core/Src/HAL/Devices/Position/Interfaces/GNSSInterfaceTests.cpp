#include "Devices/Position/GNSSInterface.h"

#include <gtest/gtest.h>

#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockUartCommunicationInterface.h"

using HAL::Devices::Communication::Interfaces::MockUartCommunicationInterface;

namespace HAL 
{
namespace Devices
{
namespace Position
{

class GNSSInterfaceHelper : public GNSSInterface 
{
    public:
        GNSSInterfaceHelper( std::shared_ptr<MockUartCommunicationInterface> uart_gnss ) : 
                            GNSSInterface(uart_gnss)
        {

        }

    using GNSSInterface::GetNMEAMessageType;
    using GNSSInterface::ProcessNMEAMessage;

    using GNSSInterface::GGACallback;
    using GNSSInterface::GLLCallback;
    using GNSSInterface::GSACallback;
    using GNSSInterface::GSVCallback;
    using GNSSInterface::MSSCallback;
    using GNSSInterface::RMCCallback;
    using GNSSInterface::VTGCallback;

    MOCK_METHOD1(GGACallback, bool(const std::string &msg));
    MOCK_METHOD1(GLLCallback, bool(const std::string &msg));
    MOCK_METHOD1(GSACallback, bool(const std::string &msg));
    MOCK_METHOD1(GSVCallback, bool(const std::string &msg));
    MOCK_METHOD1(MSSCallback, bool(const std::string &msg));
    MOCK_METHOD1(RMCCallback, bool(const std::string &msg));
    MOCK_METHOD1(VTGCallback, bool(const std::string &msg));
};

class GNSSInterfaceRealHelper: public GNSSInterface 
{
    public:
        GNSSInterfaceRealHelper( std::shared_ptr<MockUartCommunicationInterface> uart_gnss ) : 
                                 GNSSInterface(uart_gnss)
        {

        }

    using GNSSInterface::GetNMEAMessageType;
    using GNSSInterface::ProcessNMEAMessage;

    using GNSSInterface::GGACallback;
    using GNSSInterface::GLLCallback;
    using GNSSInterface::GSACallback;
    using GNSSInterface::GSVCallback;
    using GNSSInterface::MSSCallback;
    using GNSSInterface::RMCCallback;
    using GNSSInterface::VTGCallback;
};


class GNSSInterfaceTests: public testing::Test
{
    public:
        GNSSInterfaceTests() :
        uart_gnss_(std::make_shared<MockUartCommunicationInterface>()),
        gnss_interface_(uart_gnss_)
        {

        }

    std::shared_ptr<MockUartCommunicationInterface> uart_gnss_;
    GNSSInterfaceHelper gnss_interface_;
};

TEST_F(GNSSInterfaceTests, TestMsgTypeStrGGA)
{
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    GNSSInterface::NMEAMessageType msg_type;

    msg_type = gnss_interface_.GetNMEAMessageType(strGGA);

    EXPECT_EQ(msg_type, GNSSInterface::NMEAMessageType::GGA);
}

TEST_F(GNSSInterfaceTests, TestMsgTypeStrGLL)
{
    std::string strGGA = "$GPGLL,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    GNSSInterface::NMEAMessageType msg_type;

    msg_type = gnss_interface_.GetNMEAMessageType(strGGA);

    EXPECT_EQ(msg_type, GNSSInterface::NMEAMessageType::GLL);
}

TEST_F(GNSSInterfaceTests, TestMsgTypeStrGSA)
{
    std::string strGSA = "$GPGSA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    GNSSInterface::NMEAMessageType msg_type;

    msg_type = gnss_interface_.GetNMEAMessageType(strGSA);

    EXPECT_EQ(msg_type, GNSSInterface::NMEAMessageType::GSA);
}

TEST_F(GNSSInterfaceTests, TestMsgTypeStrGSV)
{
    std::string strGSV = "$GPGSV,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    GNSSInterface::NMEAMessageType msg_type;

    msg_type = gnss_interface_.GetNMEAMessageType(strGSV);

    EXPECT_EQ(msg_type, GNSSInterface::NMEAMessageType::GSV);
}

TEST_F(GNSSInterfaceTests, TestMsgTypeStrMSS)
{
    std::string strMSS = "$GPMSS,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    GNSSInterface::NMEAMessageType msg_type;

    msg_type = gnss_interface_.GetNMEAMessageType(strMSS);

    EXPECT_EQ(msg_type, GNSSInterface::NMEAMessageType::MSS);
}

TEST_F(GNSSInterfaceTests, TestMsgTypeStrRMC)
{
    std::string strRMC = "$GPRMC,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    GNSSInterface::NMEAMessageType msg_type;

    msg_type = gnss_interface_.GetNMEAMessageType(strRMC);

    EXPECT_EQ(msg_type, GNSSInterface::NMEAMessageType::RMC);
}

TEST_F(GNSSInterfaceTests, TestMsgTypeStrVTG)
{
    std::string strVTG = "$GPVTG,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    GNSSInterface::NMEAMessageType msg_type;

    msg_type = gnss_interface_.GetNMEAMessageType(strVTG);

    EXPECT_EQ(msg_type, GNSSInterface::NMEAMessageType::VTG);
}

TEST_F(GNSSInterfaceTests, TestMsgTypeStrInvalid)
{
    std::string strABC = "$GPABC,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    GNSSInterface::NMEAMessageType msg_type;

    msg_type = gnss_interface_.GetNMEAMessageType(strABC);

    EXPECT_EQ(msg_type, GNSSInterface::NMEAMessageType::MAXMessagesTypes);
}

TEST_F(GNSSInterfaceTests, TestMsgTypeStrEmpty)
{
    std::string strABC = "";
    GNSSInterface::NMEAMessageType msg_type;

    msg_type = gnss_interface_.GetNMEAMessageType(strABC);

    EXPECT_EQ(msg_type, GNSSInterface::NMEAMessageType::MAXMessagesTypes);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackInvoked)
{
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    
    EXPECT_CALL(gnss_interface_, GGACallback(strGGA))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = gnss_interface_.ProcessNMEAMessage(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackInvoked)
{
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,123519,A,A*5C";
    
    EXPECT_CALL(gnss_interface_, GLLCallback(strGLL))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = gnss_interface_.ProcessNMEAMessage(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackInvoked)
{
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*30";
    
    EXPECT_CALL(gnss_interface_, GSACallback(strGSA))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = gnss_interface_.ProcessNMEAMessage(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSVCallbackInvoked)
{
    std::string strGSV = "$GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75";
    
    EXPECT_CALL(gnss_interface_, GSVCallback(strGSV))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = gnss_interface_.ProcessNMEAMessage(strGSV);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestMSSCallbackInvoked)
{
    std::string strMSS = "$GPMSS,Y,,,,,*1D";
    
    EXPECT_CALL(gnss_interface_, MSSCallback(strMSS))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = gnss_interface_.ProcessNMEAMessage(strMSS);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestRMCCallbackInvoked)
{
    std::string strRMC = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";
    
    EXPECT_CALL(gnss_interface_, RMCCallback(strRMC))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = gnss_interface_.ProcessNMEAMessage(strRMC);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestVTGCallbackInvoked)
{
    std::string strVTG = "$GPVTG,054.7,T,034.4,M,5.5,N,10.2,K*48";
    
    EXPECT_CALL(gnss_interface_, VTGCallback(strVTG))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = gnss_interface_.ProcessNMEAMessage(strVTG);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestInvalidMessageNoCallbackInvoked)
{
    std::string strInvalid = "$GPABC,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    
    EXPECT_CALL(gnss_interface_, GGACallback)
        .Times(0);
    EXPECT_CALL(gnss_interface_, GLLCallback)
        .Times(0);
    
    bool result = gnss_interface_.ProcessNMEAMessage(strInvalid);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackRealParsingValidMessage)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackInvalidChecksum)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*FF";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackMissingChecksum)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackTooFewFields)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1*47";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackAllFieldsEmpty)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,,,,,,,,,,,,,,*56";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackEmptyUTCTime)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*4A";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackEmptyLatitude)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*59";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackEmptyNSIndicator)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*09";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackNSIndicatorSouth)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,S,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*5A";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackEmptyLongitude)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,,E,1,08,0.9,545.4,M,46.9,M,,*6B";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackEmptyEWIndicator)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,,1,08,0.9,545.4,M,46.9,M,,*02";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackEWIndicatorWest)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,W,1,08,0.9,545.4,M,46.9,M,,*55";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackEmptyPositionFix)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,,08,0.9,545.4,M,46.9,M,,*76";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackEmptyHDOP)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,,545.4,M,46.9,M,,*60";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackEmptyAltitude)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,,M,46.9,M,,*69";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackEmptyGeoidSeparation)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,,M,,*52";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackNegativeLatitude)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,-4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*6A";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackNegativeLongitude)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,-01131.000,E,1,08,0.9,545.4,M,46.9,M,,*6A";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGGACallbackNegativeAltitude)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,-545.4,M,46.9,M,,*6A";
    
    bool result = gnss_real.GGACallback(strGGA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackRealParsingValidMessage)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,123519,A*25";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackInvalidChecksum)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,123519,A*20";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackMissingChecksum)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,123519,A";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackTooFewFields)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000*00";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackAllFieldsEmpty)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,,,,,,,*7C";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackEmptyLatitude)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,,N,01131.000,E,123519,A*3B";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackEmptyNSIndicator)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,,01131.000,E,123519,A*6B";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackNSIndicatorSouth)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,S,01131.000,E,123519,A*38";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackEmptyLongitude)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,,E,123519,A*09";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackEmptyEWIndicator)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,,123519,A*60";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackEWIndicatorWest)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,W,123519,A*37";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackEmptyUTCTime)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,,A*28";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackNegativeLatitude)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,-4807.038,N,01131.000,E,123519,A*08";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackNegativeLongitude)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,-01131.000,E,123519,A*08";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackInvalidNSIndicator)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,X,01131.000,E,123519,A*33";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackInvalidEWIndicator)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,X,123519,A*38";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackInvalidUTCTimeLength)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,12351999,A*25";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGLLCallbackInvalidUTCTimeFormat)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,ABC123,A*58";
    
    bool result = gnss_real.GLLCallback(strGLL);
    
    EXPECT_FALSE(result);
}


TEST_F(GNSSInterfaceTests, TestGSACallbackRealParsingValidMessage)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackInvalidChecksum)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*FF";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackMissingChecksum)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackTooFewFields)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12*30";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackAllFieldsEmpty)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,,,,,,,,,,,,,,,,,,*42";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackEmptyModeSelection)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*78";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackModeSelectionManual)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,M,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*35";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackInvalidModeSelection)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,X,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*20";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackEmptyMode)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*0A";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackMode2D)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,2,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*38";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackMode1Fix)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,1,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*3B";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackInvalidMode)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,4,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*3E";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_FALSE(result);
}

// TEST_F(GNSSInterfaceTests, TestGSACallbackEmptySatelliteIDs)
// {
//     auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
//     GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
//     std::string strGSA = "$GPGSA,A,3,,,,,,,,,,,,,2.5,1.3,2.1*34";
    
//     bool result = gnss_real.GSACallback(strGSA);
    
//     EXPECT_TRUE(result);
// }

// TEST_F(GNSSInterfaceTests, TestGSACallbackSingleSatelliteID)
// {
//     auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
//     GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
//     std::string strGSA = "$GPGSA,A,3,01,,,,,,,,,,,2.5,1.3,2.1*19";
    
//     bool result = gnss_real.GSACallback(strGSA);
    
//     EXPECT_TRUE(result);
// }

// TEST_F(GNSSInterfaceTests, TestGSACallbackAllSatelliteIDs)
// {
//     auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
//     GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
//     std::string strGSA = "$GPGSA,A,3,01,02,03,04,05,06,07,08,09,10,11,12,2.5,1.3,2.1*37";
    
//     bool result = gnss_real.GSACallback(strGSA);
    
//     EXPECT_TRUE(result);
// }

// TEST_F(GNSSInterfaceTests, TestGSACallbackInvalidSatelliteID)
// {
//     auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
//     GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
//     std::string strGSA = "$GPGSA,A,3,AB,05,,09,12,,,24,,,,,2.5,1.3,2.1*3E";
    
//     bool result = gnss_real.GSACallback(strGSA);
    
//     EXPECT_FALSE(result);
// }

TEST_F(GNSSInterfaceTests, TestGSACallbackEmptyPDOP)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,0,1.3,2.1*20";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackEmptyHDOP)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,,2.1*15";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackEmptyVDOP)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,,*38";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackNegativePDOP)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,-2.5,1.3,2.1*14";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackNegativeHDOP)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,-1.3,2.1*14";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackNegativeVDOP)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,-2.1*14";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackInvalidPDOP)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,ABC,1.3,2.1*50";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackInvalidHDOP)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,XYZ,2.1*01";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackInvalidVDOP)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,DEF*0A";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_FALSE(result);
}

TEST_F(GNSSInterfaceTests, TestGSACallbackHighDilutionValues)
{
    auto uart_gnss_real = std::make_shared<MockUartCommunicationInterface>();
    GNSSInterfaceRealHelper gnss_real(uart_gnss_real);
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,99.9,99.9,99.9*06";
    
    bool result = gnss_real.GSACallback(strGSA);
    
    EXPECT_TRUE(result);
}

}
}
}
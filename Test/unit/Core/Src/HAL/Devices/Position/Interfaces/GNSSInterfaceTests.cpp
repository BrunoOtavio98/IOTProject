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

}
}
}
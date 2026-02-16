#include "Devices/Position/NMEAParser.h"

#include <gtest/gtest.h>

#include "Core/Src/HAL/Devices/Communication/Interfaces/Mocks/MockUartCommunicationInterface.h"


namespace HAL 
{
namespace Devices
{
namespace Position
{

class NMEAParserHelper : public NMEAParser 
{
    public:
        NMEAParserHelper()
        {}

    using NMEAParser::GetNMEAMessageType;
    using NMEAParser::ProcessNMEAMessage;

    using NMEAParser::GGACallback;
    using NMEAParser::GLLCallback;
    using NMEAParser::GSACallback;
    using NMEAParser::GSVCallback;
    using NMEAParser::MSSCallback;
    using NMEAParser::RMCCallback;
    using NMEAParser::VTGCallback;

    MOCK_METHOD2(GGACallback, bool(const std::string &msg, NMEA_Data &parsed_data));
    MOCK_METHOD2(GLLCallback, bool(const std::string &msg, NMEA_Data &parsed_data));
    MOCK_METHOD2(GSACallback, bool(const std::string &msg, NMEA_Data &parsed_data));
    MOCK_METHOD2(GSVCallback, bool(const std::string &msg, NMEA_Data &parsed_data));
    MOCK_METHOD2(MSSCallback, bool(const std::string &msg, NMEA_Data &parsed_data));
    MOCK_METHOD2(RMCCallback, bool(const std::string &msg, NMEA_Data &parsed_data));
    MOCK_METHOD2(VTGCallback, bool(const std::string &msg, NMEA_Data &parsed_data));
};

class NMEAParserRealHelper: public NMEAParser 
{
    public:
        NMEAParserRealHelper() 
        {}

    using NMEAParser::GetNMEAMessageType;
    using NMEAParser::ProcessNMEAMessage;

    using NMEAParser::GGACallback;
    using NMEAParser::GLLCallback;
    using NMEAParser::GSACallback;
    using NMEAParser::GSVCallback;
    using NMEAParser::MSSCallback;
    using NMEAParser::RMCCallback;
    using NMEAParser::VTGCallback;
};

class NMEAParserTests: public testing::Test
{
    public:
        NMEAParserTests()
        {}

    NMEAParserHelper nmea_parser_;
    NMEAParser::NMEA_Data nmea_data;
};

TEST_F(NMEAParserTests, TestMsgTypeStrGGA)
{
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    NMEAParser::NMEAMessageType msg_type;

    msg_type = nmea_parser_.GetNMEAMessageType(strGGA);

    EXPECT_EQ(msg_type, NMEAParser::NMEAMessageType::GGA);
}

TEST_F(NMEAParserTests, TestMsgTypeStrGLL)
{
    std::string strGGA = "$GPGLL,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    NMEAParser::NMEAMessageType msg_type;

    msg_type = nmea_parser_.GetNMEAMessageType(strGGA);

    EXPECT_EQ(msg_type, NMEAParser::NMEAMessageType::GLL);
}

TEST_F(NMEAParserTests, TestMsgTypeStrGSA)
{
    std::string strGSA = "$GPGSA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    NMEAParser::NMEAMessageType msg_type;

    msg_type = nmea_parser_.GetNMEAMessageType(strGSA);

    EXPECT_EQ(msg_type, NMEAParser::NMEAMessageType::GSA);
}

TEST_F(NMEAParserTests, TestMsgTypeStrGSV)
{
    std::string strGSV = "$GPGSV,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    NMEAParser::NMEAMessageType msg_type;

    msg_type = nmea_parser_.GetNMEAMessageType(strGSV);

    EXPECT_EQ(msg_type, NMEAParser::NMEAMessageType::GSV);
}

TEST_F(NMEAParserTests, TestMsgTypeStrMSS)
{
    std::string strMSS = "$GPMSS,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    NMEAParser::NMEAMessageType msg_type;

    msg_type = nmea_parser_.GetNMEAMessageType(strMSS);

    EXPECT_EQ(msg_type, NMEAParser::NMEAMessageType::MSS);
}

TEST_F(NMEAParserTests, TestMsgTypeStrRMC)
{
    std::string strRMC = "$GPRMC,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    NMEAParser::NMEAMessageType msg_type;

    msg_type = nmea_parser_.GetNMEAMessageType(strRMC);

    EXPECT_EQ(msg_type, NMEAParser::NMEAMessageType::RMC);
}

TEST_F(NMEAParserTests, TestMsgTypeStrVTG)
{
    std::string strVTG = "$GPVTG,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    NMEAParser::NMEAMessageType msg_type;

    msg_type = nmea_parser_.GetNMEAMessageType(strVTG);

    EXPECT_EQ(msg_type, NMEAParser::NMEAMessageType::VTG);
}

TEST_F(NMEAParserTests, TestMsgTypeStrInvalid)
{
    std::string strABC = "$GPABC,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    NMEAParser::NMEAMessageType msg_type;

    msg_type = nmea_parser_.GetNMEAMessageType(strABC);

    EXPECT_EQ(msg_type, NMEAParser::NMEAMessageType::MAXMessagesTypes);
}

TEST_F(NMEAParserTests, TestMsgTypeStrEmpty)
{
    std::string strABC = "";
    NMEAParser::NMEAMessageType msg_type;

    msg_type = nmea_parser_.GetNMEAMessageType(strABC);

    EXPECT_EQ(msg_type, NMEAParser::NMEAMessageType::MAXMessagesTypes);
}

TEST_F(NMEAParserTests, TestGGACallbackInvoked)
{
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";

    EXPECT_CALL(nmea_parser_, GGACallback(strGGA, testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = nmea_parser_.ProcessNMEAMessage(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackInvoked)
{
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,123519,A,A*5C";

    EXPECT_CALL(nmea_parser_, GLLCallback(strGLL, testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = nmea_parser_.ProcessNMEAMessage(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackInvoked)
{
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*30";

    EXPECT_CALL(nmea_parser_, GSACallback(strGSA, testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = nmea_parser_.ProcessNMEAMessage(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSVCallbackInvoked)
{
    std::string strGSV = "$GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75";

    EXPECT_CALL(nmea_parser_, GSVCallback(strGSV, testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = nmea_parser_.ProcessNMEAMessage(strGSV, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestMSSCallbackInvoked)
{
    std::string strMSS = "$GPMSS,Y,,,,,*1D";

    EXPECT_CALL(nmea_parser_, MSSCallback(strMSS, testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = nmea_parser_.ProcessNMEAMessage(strMSS, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestRMCCallbackInvoked)
{
    std::string strRMC = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";

    EXPECT_CALL(nmea_parser_, RMCCallback(strRMC, testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = nmea_parser_.ProcessNMEAMessage(strRMC, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestVTGCallbackInvoked)
{
    std::string strVTG = "$GPVTG,054.7,T,034.4,M,5.5,N,10.2,K*48";

    EXPECT_CALL(nmea_parser_, VTGCallback(strVTG, testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));
    
    bool result = nmea_parser_.ProcessNMEAMessage(strVTG, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestInvalidMessageNoCallbackInvoked)
{
    std::string strInvalid = "$GPABC,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";

    EXPECT_CALL(nmea_parser_, GGACallback)
        .Times(0);
    EXPECT_CALL(nmea_parser_, GLLCallback)
        .Times(0);
    
    bool result = nmea_parser_.ProcessNMEAMessage(strInvalid, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackRealParsingValidMessage)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackInvalidChecksum)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*FF";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackMissingChecksum)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackTooFewFields)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1*47";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackAllFieldsEmpty)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,,,,,,,,,,,,,,*56";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackEmptyUTCTime)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*4A";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackEmptyLatitude)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,123519,,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*59";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackEmptyNSIndicator)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,123519,4807.038,,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*09";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackNSIndicatorSouth)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,123519,4807.038,S,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*5A";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackEmptyLongitude)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,,E,1,08,0.9,545.4,M,46.9,M,,*6B";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackEmptyEWIndicator)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,,1,08,0.9,545.4,M,46.9,M,,*02";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackEWIndicatorWest)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,W,1,08,0.9,545.4,M,46.9,M,,*55";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackEmptyPositionFix)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,,08,0.9,545.4,M,46.9,M,,*76";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackEmptyHDOP)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,,545.4,M,46.9,M,,*60";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackEmptyAltitude)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,,M,46.9,M,,*69";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackEmptyGeoidSeparation)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,,M,,*52";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackNegativeLatitude)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,123519,-4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*6A";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackNegativeLongitude)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,-01131.000,E,1,08,0.9,545.4,M,46.9,M,,*6A";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGGACallbackNegativeAltitude)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string strGGA = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,-545.4,M,46.9,M,,*6A";
    
    bool result = nmea_real.GGACallback(strGGA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackRealParsingValidMessage)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,123519,A*25";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackInvalidChecksum)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,123519,A*20";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackMissingChecksum)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,123519,A";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackTooFewFields)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000*00";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackAllFieldsEmpty)
{
    NMEAParserRealHelper nmea_real;

    std::string strGLL = "$GPGLL,,,,,,,*7C";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackEmptyLatitude)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,,N,01131.000,E,123519,A*3B";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackEmptyNSIndicator)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,,01131.000,E,123519,A*6B";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackNSIndicatorSouth)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,S,01131.000,E,123519,A*38";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackEmptyLongitude)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,N,,E,123519,A*09";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackEmptyEWIndicator)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,,123519,A*60";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackEWIndicatorWest)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,W,123519,A*37";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackEmptyUTCTime)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,,A*28";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackNegativeLatitude)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,-4807.038,N,01131.000,E,123519,A*08";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackNegativeLongitude)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,N,-01131.000,E,123519,A*08";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackInvalidNSIndicator)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,X,01131.000,E,123519,A*33";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackInvalidEWIndicator)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,X,123519,A*38";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackInvalidUTCTimeLength)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,12351999,A*25";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGLLCallbackInvalidUTCTimeFormat)
{
    NMEAParserRealHelper nmea_real;

    std::string strGLL = "$GPGLL,4807.038,N,01131.000,E,ABC123,A*58";
    
    bool result = nmea_real.GLLCallback(strGLL, nmea_data);
    
    EXPECT_FALSE(result);
}


TEST_F(NMEAParserTests, TestGSACallbackRealParsingValidMessage)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackInvalidChecksum)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*FF";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackMissingChecksum)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackTooFewFields)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGSA = "$GPGSA,A,3,04,05,,09,12*30";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackAllFieldsEmpty)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGSA = "$GPGSA,,,,,,,,,,,,,,,,,,*42";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackEmptyModeSelection)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*78";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackModeSelectionManual)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGSA = "$GPGSA,M,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*35";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackInvalidModeSelection)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,X,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*20";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackEmptyMode)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGSA = "$GPGSA,A,,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*0A";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackMode2D)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGSA = "$GPGSA,A,2,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*38";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackMode1Fix)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGSA = "$GPGSA,A,1,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*3B";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackInvalidMode)
{
    NMEAParserRealHelper nmea_real;
    
    std::string strGSA = "$GPGSA,A,4,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*3E";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_FALSE(result);
}

// TEST_F(NMEAParserTests, TestGSACallbackEmptySatelliteIDs)
// {
//     NMEAParserRealHelper nmea_real;
    
//     std::string strGSA = "$GPGSA,A,3,,,,,,,,,,,,,2.5,1.3,2.1*34";
    
//     bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
//     EXPECT_TRUE(result);
// }

// TEST_F(NMEAParserTests, TestGSACallbackSingleSatelliteID)
// {
//     NMEAParserRealHelper nmea_real;
//     std::string strGSA = "$GPGSA,A,3,01,,,,,,,,,,,2.5,1.3,2.1*19";
    
//     bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
//     EXPECT_TRUE(result);
// }

// TEST_F(NMEAParserTests, TestGSACallbackAllSatelliteIDs)
// {
//     NMEAParserRealHelper nmea_real;
//     std::string strGSA = "$GPGSA,A,3,01,02,03,04,05,06,07,08,09,10,11,12,2.5,1.3,2.1*37";
    
//     bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
//     EXPECT_TRUE(result);
// }

// TEST_F(NMEAParserTests, TestGSACallbackInvalidSatelliteID)
// {
//     NMEAParserRealHelper nmea_real;
//     std::string strGSA = "$GPGSA,A,3,AB,05,,09,12,,,24,,,,,2.5,1.3,2.1*3E";
    
//     bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
//     EXPECT_FALSE(result);
// }

TEST_F(NMEAParserTests, TestGSACallbackEmptyPDOP)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,0,1.3,2.1*20";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackEmptyHDOP)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,,2.1*15";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackEmptyVDOP)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,,*38";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackNegativePDOP)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,-2.5,1.3,2.1*14";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackNegativeHDOP)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,-1.3,2.1*14";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackNegativeVDOP)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,-2.1*14";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackInvalidPDOP)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,ABC,1.3,2.1*50";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackInvalidHDOP)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,XYZ,2.1*01";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackInvalidVDOP)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,DEF*0A";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_FALSE(result);
}

TEST_F(NMEAParserTests, TestGSACallbackHighDilutionValues)
{
    NMEAParserRealHelper nmea_real;

    std::string strGSA = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,99.9,99.9,99.9*06";
    
    bool result = nmea_real.GSACallback(strGSA, nmea_data);
    
    EXPECT_TRUE(result);
}

TEST_F(NMEAParserTests, TestMSSCallbackRealParsingValidMessage)
{
    NMEAParserRealHelper nmea_real;

    std::string msg = "$GPMSS,55,27,318.0,100*4A";

    EXPECT_TRUE(nmea_real.MSSCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestMSSCallbackInvalidChecksum)
{
    NMEAParserRealHelper nmea_real;

    std::string msg = "$GPMSS,55,27,318.0,100*FF";

    EXPECT_FALSE(nmea_real.MSSCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestMSSCallbackMissingChecksum)
{
    NMEAParserRealHelper nmea_real;

    std::string msg = "$GPMSS,55,27,318.0,100";

    EXPECT_FALSE(nmea_real.MSSCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestMSSCallbackAllFieldsEmpty)
{
    NMEAParserRealHelper nmea_real;

    std::string msg = "$GPMSS,,,,*5A";

    EXPECT_TRUE(nmea_real.MSSCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestMSSCallbackSSMissing)
{
    NMEAParserRealHelper nmea_real;

    std::string msg = "$GPMSS,,27,318.0,100*4A";

    EXPECT_TRUE(nmea_real.MSSCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestMSSCallbackBeaconMissing)
{
    NMEAParserRealHelper nmea_real;

    std::string msg = "$GPMSS,55,27,,100*6E";

    EXPECT_TRUE(nmea_real.MSSCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackRealParsingValidMessage)
{
    NMEAParserRealHelper nmea_real;

    std::string msg =
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";

    EXPECT_TRUE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackInvalidChecksum)
{
    NMEAParserRealHelper nmea_real;

    std::string msg =
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*FF";

    EXPECT_FALSE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackMissingChecksum)
{
    NMEAParserRealHelper nmea_real;

    std::string msg =
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394";

    EXPECT_FALSE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackTooFewFields)
{
    NMEAParserRealHelper nmea_real;

    std::string msg = "$GPRMC,123519,A,4807.038,N*68";

    EXPECT_FALSE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackStatusInvalid)
{
    NMEAParserRealHelper nmea_real;

    std::string msg =
        "$GPRMC,123519,V,4807.038,N,01131.000,E,0.0,0.0,230394,003.1,W*71";

    EXPECT_TRUE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackAllFieldsEmpty)
{
    NMEAParserRealHelper nmea_real;

    std::string msg = "$GPRMC,,,,,,,,,,,*67";

    EXPECT_TRUE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackEmptyUTCTime)
{
    NMEAParserRealHelper nmea_real;

    std::string msg =
        "$GPRMC,,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*67";

    EXPECT_TRUE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackInvalidUTCTimeLength)
{
    NMEAParserRealHelper nmea_real;

    std::string msg =
        "$GPRMC,12351999,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";

    EXPECT_FALSE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackInvalidUTCTimeFormat)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPRMC,ABCDEF,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*60";

    EXPECT_FALSE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackEmptyLatitude)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPRMC,123519,A,,N,01131.000,E,022.4,084.4,230394,003.1,W*74";

    EXPECT_TRUE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackInvalidNSIndicator)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPRMC,123519,A,4807.038,X,01131.000,E,022.4,084.4,230394,003.1,W*7C";

    EXPECT_FALSE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackInvalidEWIndicator)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPRMC,123519,A,4807.038,N,01131.000,X,022.4,084.4,230394,003.1,W*77";

    EXPECT_FALSE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackEmptySpeedAndCourse)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPRMC,123519,A,4807.038,N,01131.000,E,,,230394,003.1,W*66";

    EXPECT_TRUE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackInvalidSpeed)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPRMC,123519,A,4807.038,N,01131.000,E,ABC,084.4,230394,003.1,W*00";

    EXPECT_FALSE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestRMCCallbackInvalidDateFormat)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,23AB94,003.1,W*6A";

    EXPECT_FALSE(nmea_real.RMCCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestVTGCallbackRealParsingValidMessage)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPVTG,054.7,T,034.4,M,5.5,N,10.2,K*78";

    EXPECT_TRUE(nmea_real.VTGCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestVTGCallbackInvalidChecksum)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPVTG,054.7,T,034.4,M,5.5,N,10.2,K*FF";

    EXPECT_FALSE(nmea_real.VTGCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestVTGCallbackMissingChecksum)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPVTG,054.7,T,034.4,M,5.5,N,10.2,K";

    EXPECT_FALSE(nmea_real.VTGCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestVTGCallbackTooFewFields)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg = "$GPVTG,054.7,T*32";

    EXPECT_FALSE(nmea_real.VTGCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestVTGCallbackAllFieldsEmpty)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg = "$GPVTG,,,,,,,,*52";

    EXPECT_TRUE(nmea_real.VTGCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestVTGCallbackEmptySpeedFields)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPVTG,054.7,T,034.4,M,,,,K*05";

    EXPECT_TRUE(nmea_real.VTGCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestVTGCallbackInvalidTrueCourse)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPVTG,ABC,T,034.4,M,5.5,N,10.2,K*10";

    EXPECT_FALSE(nmea_real.VTGCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestVTGCallbackInvalidMagneticCourse)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPVTG,054.7,T,XYZ,M,5.5,N,10.2,K*0E";

    EXPECT_FALSE(nmea_real.VTGCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestVTGCallbackInvalidSpeedKnots)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;
    
    std::string msg =
        "$GPVTG,054.7,T,034.4,M,ABC,N,10.2,K*16";

    EXPECT_FALSE(nmea_real.VTGCallback(msg, nmea_data));
}

TEST_F(NMEAParserTests, TestVTGCallbackInvalidSpeedKmh)
{
    NMEAParserRealHelper nmea_real;
    NMEAParser::NMEA_Data nmea_data;

    std::string msg =
        "$GPVTG,054.7,T,034.4,M,5.5,N,XYZ,K*3E";

    EXPECT_FALSE(nmea_real.VTGCallback(msg, nmea_data));
}

// TEST_F(NMEAParserTests, TestVTGCallbackInvalidTrueCourseIndicator)
// {
//     NMEAParserRealHelper nmea_real;

//     std::string msg =
//         "$GPVTG,054.7,X,034.4,M,5.5,N,10.2,K*74";

//     EXPECT_FALSE(nmea_real.VTGCallback(msg, nmea_data));
// }

// TEST_F(NMEAParserTests, TestVTGCallbackInvalidMagneticCourseIndicator)
// {
//     NMEAParserRealHelper nmea_real;

//     std::string msg =
//         "$GPVTG,054.7,T,034.4,X,5.5,N,10.2,K*6D";

//     EXPECT_FALSE(nmea_real.VTGCallback(msg, nmea_data));
// }

}
}
}

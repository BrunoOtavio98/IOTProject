#include "Utils/StringManipulation.h"

#include <gtest/gtest.h>

namespace HAL
{
namespace Utils 
{

TEST(StringManipulationTests, TestSplitString) 
{
    StringManipulation str_manip;
    std::string test_string = "Hello,World,This,is,a,test";
    char delimiter = ',';
    std::vector<std::string> expected_tokens = {"Hello", "World", "This", "is", "a", "test"};

    std::vector<std::string> result_tokens = str_manip.SplitString(test_string, delimiter);
    EXPECT_EQ(result_tokens, expected_tokens);
}

TEST(StringManipulationTests, TestSplitStringNoDelimiter) 
{
    StringManipulation str_manip;
    std::string test_string = "NoDelimiterHere";
    char delimiter = ',';
    std::vector<std::string> expected_tokens = {"NoDelimiterHere"};

    std::vector<std::string> result_tokens = str_manip.SplitString(test_string, delimiter);
    EXPECT_EQ(result_tokens, expected_tokens);
}

TEST(StringManipulationTests, TestSplitStringEmptyString) 
{
    StringManipulation str_manip;
    std::string test_string = "";
    char delimiter = ',';
    std::vector<std::string> expected_tokens = {};

    std::vector<std::string> result_tokens = str_manip.SplitString(test_string, delimiter);
    EXPECT_EQ(result_tokens, expected_tokens);
}

TEST(StringManipulationTests, TestRemoveSubstring) 
{
    StringManipulation str_manip;
    std::string test_string = "Hello, World! This is a test string.";
    std::string substring_to_remove = "test ";
    std::string expected_result = "Hello, World! This is a string.";

    std::string result_string = str_manip.RemoveSubstring(test_string, substring_to_remove);
    EXPECT_EQ(result_string, expected_result);
}

TEST(StringManipulationTests, TestRemoveSubstringNotFound) 
{
    StringManipulation str_manip;
    std::string test_string = "Hello, World! This is a test string.";
    std::string substring_to_remove = "absent";
    std::string expected_result = test_string;

    std::string result_string = str_manip.RemoveSubstring(test_string, substring_to_remove);
    EXPECT_EQ(result_string, expected_result);
}

TEST(StringManipulationTests, TestRemoveSubstringEmptyString) 
{
    StringManipulation str_manip;
    std::string test_string = "Hello, World! This is a test string.";
    std::string substring_to_remove = "";
    std::string expected_result = test_string;

    std::string result_string = str_manip.RemoveSubstring(test_string, substring_to_remove);
    EXPECT_EQ(result_string, expected_result);
}

}
}


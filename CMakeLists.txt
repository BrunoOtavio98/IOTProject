cmake_minimum_required(VERSION 3.7)

project(Testing)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(TOP_LEVEL_DIR ${CMAKE_SOURCE_DIR}/Core/Src/HAL)
SET(TEST_DIR ${CMAKE_SOURCE_DIR}/Test)
SET(LIB_IOT "IOT_LIB")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(${TOP_LEVEL_DIR})
add_subdirectory(${TEST_DIR})
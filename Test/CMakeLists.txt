
file(GLOB_RECURSE ALL_TEST_FILES "${TEST_DIR}/*.cpp")

enable_testing()

add_executable(${PROJECT_NAME} ${ALL_TEST_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${TEST_DIR}/unit)

target_link_libraries(${PROJECT_NAME} 
        ${LIB_IOT}
        GTest::gtest_main
        GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER executables)
install (TARGETS ${PROJECT_NAME}
         RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/_install)
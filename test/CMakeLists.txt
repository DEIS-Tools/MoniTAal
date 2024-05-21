find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories (${TEST_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})

add_definitions (-DBOOST_TEST_DYN_LINK)

add_executable(Monitor_test          Monitor_test.cpp)
add_executable(Presentation_examples Presentation_examples.cpp)
add_executable(EventParserTest       EventParserTest.cpp)
add_executable(delay_tests           DelayTest.cpp)

target_link_libraries(Monitor_test         ${Boost_LIBRARIES} MoniTAal)
target_link_libraries(Presentation_examples ${Boost_LIBRARIES} MoniTAal)
target_link_libraries(EventParserTest ${Boost_LIBRARIES} MoniTAal)
target_link_libraries(delay_tests ${Boost_LIBRARIES} MoniTAal)

add_test(NAME Monitor_test COMMAND Monitor_test)
add_test(NAME Presentation_examples COMMAND Presentation_examples)
add_test(NAME EventParserTest COMMAND EventParserTest)
add_test(NAME delay_tests COMMAND delay_tests)

add_subdirectory(models)
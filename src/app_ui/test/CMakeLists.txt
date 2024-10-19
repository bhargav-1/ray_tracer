# Testing

# Creating Test targets
add_executable(${PROJECT_NAME}_tests test.cpp)

target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME}_lib Catch2::Catch2WithMain)

# Now we can include testing stuff to discover tests
include(Catch)
#set(CMAKE_CATCH_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
catch_discover_tests(${PROJECT_NAME}_tests)

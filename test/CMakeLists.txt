cmake_minimum_required(VERSION 3.8)

message("CMake version: " ${CMAKE_VERSION})

if(CMAKE_HOST_WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(../include)

file(GLOB TEST_FILES *.cpp)
set(TEST_SRC ${TEST_FILES})

add_executable(easy_json_test ${TEST_SRC})
target_link_libraries(easy_json_test
                      easy_json)

# install 
install(TARGETS easy_json_test DESTINATION bin)

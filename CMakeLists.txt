cmake_minimum_required(VERSION 3.8)

message("CMake version: " ${CMAKE_VERSION})
project("easy_json")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags")

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

add_subdirectory(src obj/src)

# test
option(EASY_JSON_BUILD_WITH_TEST "build with test programs" NO)
if(EASY_JSON_BUILD_WITH_TEST) 
    message("build test")
    add_subdirectory(test obj/test)
endif()



cmake_minimum_required(VERSION 3.8)
message("CMake version: " ${CMAKE_VERSION})

if (CMAKE_HOST_WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

include_directories(../include)

# include
set(EASY_JSON_INCLUED_FILE
    ../include/easy_json.h)

# source
file(GLOB EASY_JSON_SRC
	./*.cpp)

# easy_json
add_library(easy_json SHARED
            ${EASY_JSON_SRC})

# install 
install(FILES ${EASY_JSON_INCLUED_FILE} DESTINATION include)
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
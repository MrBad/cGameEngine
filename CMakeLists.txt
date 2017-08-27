cmake_minimum_required(VERSION 3.3)
project(cGameEngine)
#
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#
#set(SOURCE_FILES main.c)
#add_executable(cGameEngine ${SOURCE_FILES})

add_custom_command(OUTPUT cgame
        COMMAND make run
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        )
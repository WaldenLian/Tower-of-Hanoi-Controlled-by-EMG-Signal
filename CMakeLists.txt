cmake_minimum_required(VERSION 3.8)
project(Tower_of_Hanoi_Controlled_by_EMG_Signal)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/Displayer.c
        Doc.md
        README.md)

add_executable(Tower_of_Hanoi_Controlled_by_EMG_Signal ${SOURCE_FILES})
cmake_minimum_required(VERSION 3.23)
project(SimpleOS C)

set(CMAKE_C_STANDARD 99)

add_executable(SimpleOS main.c tools/floppy.c tools/fileinfo.c)

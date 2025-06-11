cmake_minimum_required(VERSION 3.30)
project(PipeMail C)

set(CMAKE_C_STANDARD 11)

add_executable(PipeMail
        src/main.c
        src/ipc_utils.c)

target_link_libraries(PipeMail pthread rt)

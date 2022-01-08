cmake_minimum_required(VERSION 3.7)
project(SemestrakaChatPOS C)

set(CMAKE_C_STANDARD 99)

add_executable(SemestrakaChatPOS main.c client.h client.c server.h server.c sharedCode.h arrayList.h arrayList.c)

target_link_libraries(SemestrakaChatPOS pthread)
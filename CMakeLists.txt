cmake_minimum_required(VERSION 3.16)
project(Malachite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(Malachite main.cpp)

set(scr/text/CustomTextEdit.cpp)

target_link_libraries(Malachite Qt6::Widgets)
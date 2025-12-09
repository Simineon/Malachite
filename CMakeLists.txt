cmake_minimum_required(VERSION 3.16)
project(Malachite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

if(Qt6_FOUND)
    message(STATUS "Found Qt6 version: ${Qt6_VERSION}")
    message(STATUS "Qt6 include directories: ${Qt6Core_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Qt6 not found. Please install Qt6 development packages.")
endif()

add_executable(Malachite 
    main.cpp
    scr/app/app.cpp
    scr/parser/parser.cpp
    scr/text/CustomTextEdit.h
    scr/app/execute/executer.h
    scr/app/execute/executer.cpp
    scr/app/tab/tab.h
    scr/app/tab/tab.cpp
    scr/app/engine_search/engine.h
)

target_link_libraries(Malachite 
    Qt6::Core
    Qt6::Widgets
)

target_include_directories(Malachite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(Malachite PRIVATE
    QT_CORE_LIB
    QT_WIDGETS_LIB
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(Malachite PRIVATE QT_NO_DEBUG_OUTPUT)
endif()
cmake_minimum_required(VERSION 3.16)
project(Malachite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Добавляем исходные файлы
add_executable(Malachite 
    main.cpp
    scr/app/app.cpp
    scr/parser/parser.cpp
    scr/text/CustomTextEdit.h
    scr/app/execute/executer.h
    scr/app/execute/executer.cpp
    scr/app/tab/tab.h
    scr/app/tab/tab.cpp
)

target_link_libraries(Malachite Qt6::Widgets)

# Включаем директории для заголовочных файлов
target_include_directories(Malachite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
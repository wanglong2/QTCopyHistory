cmake_minimum_required(VERSION 3.16)
project(QTCopyHistory)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_PREFIX_PATH "/home/wanglong/Qt/6.11.0/gcc_64")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        REQUIRED)

add_executable(QTCopyHistory
        main.cpp
        clipboardmanager.h
        clipboardmanager.cpp
        historywindow.h
        historywindow.cpp
)

target_link_libraries(QTCopyHistory
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
)

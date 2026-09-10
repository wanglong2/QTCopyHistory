cmake_minimum_required(VERSION 3.16)
project(QTCopyHistory)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)



find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        DBus
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
        Qt::DBus
        Qt::Network
)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(interaction_test tests/interaction_test.cpp
        historywindow.cpp clipboardmanager.cpp)
    target_include_directories(interaction_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(interaction_test PRIVATE Qt::Widgets Qt::Test)
    add_test(NAME interaction COMMAND interaction_test)
    set_tests_properties(interaction PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

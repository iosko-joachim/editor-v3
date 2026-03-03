cmake_minimum_required(VERSION 3.16)
project(editor_v3 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt finden
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport Charts)

# Executable
add_executable(editor_v3
    main.cpp
    mainwindow.cpp
    mainwindow.h
    richtexteditor.cpp
    richtexteditor.h
    specialchardialog.cpp
    specialchardialog.h
    tabledialog.cpp
    tabledialog.h
    ngramanalyzer.cpp
    ngramanalyzer.h
    ngramtablemodel.cpp
    ngramtablemodel.h
)

# Qt-Module linken
target_link_libraries(editor_v3
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
    Qt6::Charts
)

# Windows: kein separates Konsolenfenster
if(WIN32)
    set_target_properties(editor_v3 PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

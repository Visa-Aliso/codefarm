cmake_minimum_required(VERSION 3.20)

project(CodeFarmQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(codefarm
    src/main.cpp
    src/MainWindow.cpp
    src/GameEngine.cpp
    src/CodeHighlighter.cpp
    src/CustomButton.cpp
    src/FarmCanvas.cpp
    src/CodeEditor.cpp
    src/FarmBackground.cpp
    resources.qrc
)

target_include_directories(codefarm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(codefarm PRIVATE Qt6::Widgets)

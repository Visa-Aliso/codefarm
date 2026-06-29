cmake_minimum_required(VERSION 3.22)
project(CodeFarm VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

# Optional pybind11 for Python scripting
find_package(pybind11 QUIET)
find_package(Python3 QUIET COMPONENTS Development Interpreter)

if(pybind11_FOUND AND Python3_FOUND)
    message(STATUS "pybind11 found — Python scripting enabled")
    set(HAS_PYBIND11 TRUE)
else()
    message(STATUS "pybind11 not found — Python scripting disabled (simulated)")
    set(HAS_PYBIND11 FALSE)
endif()

set(SOURCES
    src/main.cpp
    src/core/GameState.cpp
    src/core/CropData.cpp
    src/core/Weather.cpp
    src/core/Drone.cpp
    src/core/TechTree.cpp
    src/core/PathFinder.cpp
    src/script/ScriptEngine.cpp
    src/script/ScriptAPI.cpp
    src/script/ScriptManager.cpp
    src/serialization/SaveManager.cpp
    src/ui/MainWindow.cpp
    src/ui/MenuScreen.cpp
    src/ui/MapView.cpp
    src/ui/MapTileItem.cpp
    src/ui/DroneItem.cpp
    src/ui/InfoPanel.cpp
    src/ui/TechPanel.cpp
    src/ui/ScriptEditor.cpp
    src/ui/ScriptConsole.cpp
    src/ui/CropDetailPopup.cpp
    src/ui/ErrorIndicator.cpp
    src/ui/FloatingPanel.cpp
    src/animation/AnimationManager.cpp
    src/animation/CloudAnimation.cpp
    src/animation/ParticleEffect.cpp
)

set(HEADERS
    src/core/GameState.h
    src/core/LandPlot.h
    src/core/CropData.h
    src/core/Weather.h
    src/core/Drone.h
    src/core/TechTree.h
    src/core/PathFinder.h
    src/script/ScriptEngine.h
    src/script/ScriptAPI.h
    src/script/ScriptManager.h
    src/serialization/SaveManager.h
    src/ui/MainWindow.h
    src/ui/MenuScreen.h
    src/ui/MapView.h
    src/ui/MapTileItem.h
    src/ui/DroneItem.h
    src/ui/InfoPanel.h
    src/ui/TechPanel.h
    src/ui/ScriptEditor.h
    src/ui/ScriptConsole.h
    src/ui/CropDetailPopup.h
    src/ui/ErrorIndicator.h
    src/ui/FloatingPanel.h
    src/animation/AnimationManager.h
    src/animation/CloudAnimation.h
    src/animation/ParticleEffect.h
)

qt_add_resources(RESOURCES resources/game.qrc)

add_executable(CodeFarm ${SOURCES} ${HEADERS} ${RESOURCES})

target_include_directories(CodeFarm PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(HAS_PYBIND11)
    target_include_directories(CodeFarm PRIVATE
        ${pybind11_INCLUDE_DIRS}
        ${Python3_INCLUDE_DIRS}
    )
    target_link_libraries(CodeFarm PRIVATE
        pybind11::embed
        Python3::Python
    )
    target_compile_definitions(CodeFarm PRIVATE HAS_PYBIND11=1)
else()
    target_compile_definitions(CodeFarm PRIVATE HAS_PYBIND11=0)
endif()

target_link_libraries(CodeFarm PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)

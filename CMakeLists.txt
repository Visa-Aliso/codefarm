cmake_minimum_required(VERSION 3.20)
project(CodeFarm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS
    Quick
    QuickControls2
    Svg
    Multimedia
)

find_package(Python3 QUIET COMPONENTS Interpreter Development)
find_package(pybind11 QUIET)

qt_standard_project_setup()

qt_policy(SET QTP0001 OLD)
qt_policy(SET QTP0004 OLD)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_executable(codefarm
    src/main.cpp
    src/core/gameengine.cpp
    src/core/farmmap.cpp
    src/core/drone.cpp
    src/core/cell.cpp
    src/core/goalsystem.cpp
    src/levels/levelmanager.cpp
    src/python/pythonexecutor.cpp
    src/python/gameapi.cpp
    src/editor/syntaxhighlighter.cpp
    src/save/savemanager.cpp
    src/ui/appviewmodel.cpp
)

qt_add_qml_module(codefarm
    URI CodeFarm
    VERSION 1.0
    QML_FILES
        qml/main.qml
        qml/Theme.qml
        qml/pages/MainMenu.qml
        qml/pages/LevelSelect.qml
        qml/pages/GameView.qml
        qml/pages/ResultScreen.qml
        qml/pages/FailScreen.qml
        qml/pages/HelpPage.qml
        qml/components/MenuButton.qml
        qml/components/ResourceBar.qml
        qml/components/LevelTile.qml
        qml/components/IsometricMap.qml
        qml/components/DroneSprite.qml
        qml/components/FloatingEditor.qml
        qml/components/GoalOverlay.qml
        qml/components/ConfirmDialog.qml
    RESOURCES
        resources/fonts/Nunito-Regular.ttf
        resources/fonts/Nunito-SemiBold.ttf
        resources/fonts/Nunito-Bold.ttf
        resources/fonts/JetBrainsMono-Regular.ttf
        resources/fonts/fredoka-one.one-regular.ttf
        resources/pictures/jackstraw.png
        resources/sprites/crops/wheat_small.png
        resources/sprites/crops/wheat_growing.png
        resources/sprites/crops/wheat_mature.png
        resources/sprites/crops/carrot_small.png
        resources/sprites/crops/carrot_growing.png
        resources/sprites/crops/carrot_mature.png
        resources/sprites/crops/tomato_small.png
        resources/sprites/crops/tomato_growing.png
        resources/sprites/crops/tomato_mature.png
        resources/sprites/crops/corn_small.png
        resources/sprites/crops/corn_growing.png
        resources/sprites/crops/corn_mature.png
        resources/sprites/crops/sunflower_small.png
        resources/sprites/crops/sunflower_growing.png
        resources/sprites/crops/sunflower_mature.png
)

target_include_directories(codefarm PRIVATE src)

target_link_libraries(codefarm PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Svg
    Qt6::Multimedia
)

# Delete save.json after each build so game starts fresh during development
add_custom_command(TARGET codefarm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove "$ENV{HOME}/.codefarm/save.json"
    COMMENT "Clearing save data for fresh start"
)

if(Python3_FOUND)
    target_link_libraries(codefarm PRIVATE Python3::Python)
    target_compile_definitions(codefarm PRIVATE HAS_PYTHON)
endif()

if(pybind11_FOUND)
    target_link_libraries(codefarm PRIVATE pybind11::embed)
    target_compile_definitions(codefarm PRIVATE HAS_PYBIND11)
endif()

install(TARGETS codefarm
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

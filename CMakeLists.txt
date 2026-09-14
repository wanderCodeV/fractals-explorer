cmake_minimum_required(VERSION 3.10)

project(Fractals)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Config SFML
set(SFML_DIR "D:/Soft/SFML-3.0.2/lib/cmake/SFML")
# Find SFML package
find_package(SFML 3.0.2 COMPONENTS Graphics Window System REQUIRED)
# Add source files and headers to the executable
add_executable(FractalApp 
    main.cpp 
    Tab.cpp 
    Tab.h
    Fractals.cpp 
    Fractals.h
    NewtonExplorer.cpp
    NewtonExplorer.h
    Settings.h
    SettingsPanel.cpp
    SettingsPanel.h
    RenderUtils.cpp
    RenderUtils.h
    FractalController.cpp
    FractalController.h
)
# Link SFML libraries to the executable
target_link_libraries(FractalApp SFML::Graphics SFML::Window SFML::System)


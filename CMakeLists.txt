cmake_minimum_required(VERSION 3.10)
project(Fractals)

# 1. Standard C++ (wymagany dla Twojego kodu)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Ścieżka do konfiguracji SFML
# Używamy zwykłych ukośników '/', CMake je preferuje
set(SFML_DIR "D:/Soft/SFML-3.0.2/lib/cmake/SFML")

# 3. Szukanie pakietu - TUTAJ BYŁ BŁĄD
# W COMPONENTS podajemy tylko: Graphics, Window, System
find_package(SFML 3.0.2 COMPONENTS Graphics Window System REQUIRED)

# 4. Dodanie plików źródłowych
add_executable(FractalApp 
    main.cpp 
    Tab.cpp 
    Tab.h
    Fractals.cpp 
    Fractals.h
    Settings.h
    SettingsPanel.cpp
    SettingsPanel.h
)

# 5. Linkowanie - tutaj używamy nazw z prefiksem SFML::
target_link_libraries(FractalApp SFML::Graphics SFML::Window SFML::System)
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

struct Settings {
    unsigned int windowWidth = 1200;
    unsigned int windowHeight = 800;
    unsigned int framerateLimitFPS = 60;
    
    std::string fontPath = "../fonts/arial.ttf";
    sf::Font font;
    
    // Fractal settings
    float drawSpeed = 0.5f;  
    float cameraZoom = 1.3f;
    
    // Colors
    sf::Color backgroundColor = sf::Color::White;
    sf::Color fractalColor = sf::Color(194, 123, 160);

    // UI Colors
    sf::Color tabInactiveColor = sf::Color(50, 50, 50);
    sf::Color tabActiveColor = sf::Color(100, 100, 250);
    sf::Color tabOutlineColor = sf::Color::White;
    sf::Color buttonColor = sf::Color(70, 70, 70);
    sf::Color buttonHoverColor = sf::Color(100, 100, 100);
    sf::Color settingsPanelColor = sf::Color(30, 30, 30);
    
    // Koch curve
    sf::Vector2f koch_p1 = { 300, 250 };
    sf::Vector2f koch_p2 = { 900, 250 };
    
    // Sierpinski triangle 
    sf::Vector2f sierpinski_p1 = { 600, 100 };
    sf::Vector2f sierpinski_p2 = { 150, 750 };
    sf::Vector2f sierpinski_p3 = { 1050, 750 };
    
    // Fractal tree 
    sf::Vector2f tree_start = { 600, 550 };
    float tree_initial_length = 100.0f;
    float tree_angle_left = 30.0f;  // degrees
    float tree_angle_right = -30.0f;  // degrees
    float tree_length_factor = 0.7f;  // branch length reduction
    float tree_min_angle = 5.0f;
    float tree_max_angle = 75.0f;

    // Pythagoras tree 
    float pythagoras_size = 150.0f;
    float pythagoras_angle = 60.0f; 
    float pythagoras_min_angle = 15.0f;
    float pythagoras_max_angle = 75.0f;
    int pythagoras_min_depth = 0;
    int pythagoras_max_depth = 14;
    
    // Settings panel UI
    float settingsPanelX = 1050;
    float settingsPanelY = 50;
    float settingsPanelWidth = 140;
    float settingsPanelHeight = 260;
    float settingsButtonSize = 30;

    Settings() {
        if (!font.openFromFile(fontPath)) 
        {
            std::cerr << "Error: Could not load font from " << fontPath << std::endl;
            exit(-1);
        }
    }
};

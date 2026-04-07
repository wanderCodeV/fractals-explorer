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
    int fractalIterations = 5;
    float drawSpeed = 0.5f;  
    float cameraZoom = 1.3f;
    
    // Colors
    sf::Color backgroundColor = sf::Color(5, 5, 10);
    sf::Color fractalColor = sf::Color::Red;

    // UI Colors
    sf::Color tabInactiveColor = sf::Color(50, 50, 50);
    sf::Color tabActiveColor = sf::Color(100, 100, 250);
    sf::Color tabOutlineColor = sf::Color::White;
    sf::Color buttonColor = sf::Color(70, 70, 70);
    sf::Color buttonHoverColor = sf::Color(100, 100, 100);
    sf::Color settingsPanelColor = sf::Color(30, 30, 30);
    
    // Fractal positions
    sf::Vector2f koch_p1 = { 300, 250 };
    sf::Vector2f koch_p2 = { 900, 250 };
    
    // Sierpinski triangle points
    sf::Vector2f sierpinski_p1 = { 600, 100 };
    sf::Vector2f sierpinski_p2 = { 300, 500 };
    sf::Vector2f sierpinski_p3 = { 900, 500 };
    
    // Fractal tree start
    sf::Vector2f tree_start = { 600, 550 };
    float tree_initial_length = 100.0f;
    float tree_angle_left = 30.0f;  // degrees
    float tree_angle_right = -30.0f;  // degrees
    float tree_length_factor = 0.7f;  // branch length reduction
    
    // Settings panel UI
    float settingsPanelX = 1050;
    float settingsPanelY = 50;
    float settingsPanelWidth = 140;
    float settingsPanelHeight = 280;  // Increased for color sliders
    float settingsButtonSize = 30;

    Settings() {
        if (!font.openFromFile(fontPath)) 
        {
            std::cerr << "Error: Could not load font from " << fontPath << std::endl;
            exit(-1);
        }
    }
};

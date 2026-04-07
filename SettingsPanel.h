#pragma once

#include <SFML/Graphics.hpp>
#include "Settings.h"

class SettingsPanel {
public:
    sf::CircleShape settingsButton;  // Gear/settings button
    sf::RectangleShape panel;        // Settings panel background
    bool isOpen = false;
    
    // Slider elements for speed
    sf::RectangleShape speedSliderTrack;
    sf::CircleShape speedSliderHandle;
    float speedMin = 0.1f;
    float speedMax = 2.0f;
    bool isDraggingSpeed = false;
    
    // Slider elements for background color
    sf::RectangleShape backgroundColorSliderTrackR, backgroundColorSliderTrackG, backgroundColorSliderTrackB;
    sf::CircleShape backgroundColorSliderHandleR, backgroundColorSliderHandleG, backgroundColorSliderHandleB;
    bool isDraggingBackgroundColorR = false, isDraggingBackgroundColorG = false, isDraggingBackgroundColorB = false;
    
    // Slider elements for fractal color
    sf::RectangleShape fractalColorSliderTrackR, fractalColorSliderTrackG, fractalColorSliderTrackB;
    sf::CircleShape fractalColorSliderHandleR, fractalColorSliderHandleG, fractalColorSliderHandleB;
    bool isDraggingFractalColorR = false, isDraggingFractalColorG = false, isDraggingFractalColorB = false;
    
    // Text elements
    sf::Text speedLabel;
    sf::Text backgroundColorLabel;
    sf::Text fractalColorLabel;
    
    SettingsPanel(const Settings& settings);
    void update(const Settings& settings);
    void handleMousePress(const sf::Event::MouseButtonPressed& mouseEvent, Settings& settings, sf::RenderWindow& window);
    void handleMouseRelease(const sf::Event::MouseButtonReleased& mouseEvent, Settings& settings);
    void handleMouseMove(const sf::Vector2i& mousePos, Settings& settings);
    void drawSettingsButton(sf::RenderWindow& window, const Settings& settings);
    void drawSettingsPanel(sf::RenderWindow& window, const Settings& settings);
    void draw(sf::RenderWindow& window, const Settings& settings);

private:
    void setupSlider(sf::RectangleShape& track, sf::CircleShape& handle, const sf::Vector2f& position, const sf::Color& trackColor, const sf::Color& handleColor);
    void updateSliderHandle(sf::RectangleShape& track, sf::CircleShape& handle, float ratio);
};

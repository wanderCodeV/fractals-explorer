#pragma once

#include <SFML/Graphics.hpp>
#include "Settings.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

struct Slider
{
    sf::RectangleShape track;
    sf::CircleShape handle;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    bool isDragging = false;

    void setup(const sf::Vector2f& position, const sf::Color& trackColor, const sf::Color& handleColor);
    void setRatio(float ratio);
    float ratioFromMouseX(float mouseX) const;
    bool contains(const sf::Vector2f& point) const;
};

struct SliderBlock
{
    sf::Text label;
    std::vector<Slider> sliders;

    SliderBlock(const sf::Font& font, const std::string& text, const sf::Vector2f& labelPosition);
    Slider& addSlider(const sf::Vector2f& position, const sf::Color& trackColor, const sf::Color& handleColor,
        float minValue = 0.0f, float maxValue = 1.0f);
    void draw(sf::RenderWindow& window) const;
};

class SettingsPanel
{
public:
    sf::CircleShape settingsButton;
    sf::RectangleShape panel;
    bool isOpen = false;

    SliderBlock speedBlock;
    SliderBlock backgroundColorBlock;
    SliderBlock fractalColorBlock;

    SettingsPanel(const Settings& settings);
    void update(const Settings& settings);
    void handleMousePress(const sf::Event::MouseButtonPressed& mouseEvent, Settings& settings, sf::RenderWindow& window);
    void handleMouseRelease(const sf::Event::MouseButtonReleased& mouseEvent, Settings& settings);
    void handleMouseMove(const sf::Vector2i& mousePos, Settings& settings);
    void drawSettingsButton(sf::RenderWindow& window, const Settings& settings);
    void drawSettingsPanel(sf::RenderWindow& window, const Settings& settings);
    void draw(sf::RenderWindow& window, const Settings& settings);

private:
    void forEachSlider(const std::function<void(Slider&)>& action);
    void updateColorSliders(SliderBlock& block, const sf::Color& color);
    static void setColorChannel(sf::Color& color, std::size_t channel, std::uint8_t value);
};

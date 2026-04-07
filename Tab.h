#pragma once

#include <SFML/Graphics.hpp>
#include <string>

enum class FractalType { Koch, Sierpinski, Tree };

struct Tab {
    sf::RectangleShape shape;
    sf::Text text;
    FractalType type;
    sf::Color inactiveColor;

    Tab(std::string label, float x, sf::Font& font, FractalType t, sf::Color inactive);
    void draw(sf::RenderWindow& window, FractalType current, sf::Color activeColor);
};
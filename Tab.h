#pragma once
#include <SFML/Graphics.hpp>
#include <string>

#define TAB_HEIGHT 40

enum class FractalType
{
    Koch,
    Sierpinski,
    SierpinskiCarpet,
    SierpinskiPentagon,
    Tree,
    Pythagoras,
    Dragon,
    Vicsek,
    Hilbert,
    Newton
};

inline bool isExplorerMode(FractalType type) { return type == FractalType::Newton; }

struct Tab
{
    sf::RectangleShape shape;
    sf::Text text;
    FractalType type;
    sf::Color inactiveColor;
    float width = 0.0f;
    Tab(std::string label, float x, float tabWidth, sf::Font &font, FractalType t,
        sf::Color inactive);
    void draw(sf::RenderWindow &window, FractalType current, sf::Color activeColor);
};

#pragma once

#include <SFML/Graphics.hpp>

#include <array>

#include "Fractals.h"

class NewtonExplorer
{
public:
    NewtonExplorer(unsigned int width, unsigned int height, sf::Font& font, sf::Color baseColor);
    void setPalette(sf::Color baseColor);
    void handleMouseClick(const sf::Vector2i& mousePosition);
    void draw(sf::RenderWindow& window);
    void requestRedraw();

private:
    unsigned int displayWidth;
    unsigned int displayHeight;
    unsigned int renderWidth;
    unsigned int renderHeight;
    sf::Image image;
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Font& font;
    sf::Text degreeLabel;
    sf::RectangleShape toggleButton;
    std::array<sf::Color, kPaletteSize> palette;
    int degree = 3;
    bool needsRedraw = true;
    void render();
    sf::Color colorForRoot(int rootIndex, int iterations) const;
};

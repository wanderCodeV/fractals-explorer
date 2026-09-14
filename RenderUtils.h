#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>

#include "Fractals.h"

struct VertexBounds
{
    sf::Vector2f minPoint;
    sf::Vector2f maxPoint;
};

VertexBounds computeVertexBounds(const std::vector<sf::Vertex>& vertices);
void fitViewToBounds(sf::View& view, const VertexBounds& bounds,
    unsigned int windowWidth, unsigned int windowHeight, float zoom);
void drawThickLines(sf::RenderWindow& window, const std::vector<sf::Vertex>& vertices, float lineThickness);
void drawPythagorasSquares(sf::RenderWindow& window, const std::vector<PythagorasSquare>& squares,
    const std::array<sf::Color, kPaletteSize>& palette);

#include "RenderUtils.h"

#include <algorithm>
#include <cmath>

VertexBounds computeVertexBounds(const std::vector<sf::Vertex>& vertices)
{
    VertexBounds bounds{ vertices.front().position, vertices.front().position };
    for (size_t i = 1; i < vertices.size(); ++i)
    {
        const sf::Vector2f point = vertices[i].position;
        bounds.minPoint.x = std::min(bounds.minPoint.x, point.x);
        bounds.minPoint.y = std::min(bounds.minPoint.y, point.y);
        bounds.maxPoint.x = std::max(bounds.maxPoint.x, point.x);
        bounds.maxPoint.y = std::max(bounds.maxPoint.y, point.y);
    }
    return bounds;
}

void fitViewToBounds(sf::View& view, const VertexBounds& bounds,
    unsigned int windowWidth, unsigned int windowHeight, float zoom)
{
    const float drawWidth = std::max(50.0f, bounds.maxPoint.x - bounds.minPoint.x);
    const float drawHeight = std::max(50.0f, bounds.maxPoint.y - bounds.minPoint.y);
    const float windowAspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    const float drawAspect = drawWidth / drawHeight;

    float viewWidth;
    float viewHeight;
    if (drawAspect > windowAspect)
    {
        viewWidth = drawWidth;
        viewHeight = drawWidth / windowAspect;
    }
    else
    {
        viewHeight = drawHeight;
        viewWidth = drawHeight * windowAspect;
    }

    view.setSize(sf::Vector2f(viewWidth * zoom, viewHeight * zoom));
    view.setCenter((bounds.minPoint + bounds.maxPoint) / 2.0f);
}

void drawThickLines(sf::RenderWindow& window, const std::vector<sf::Vertex>& vertices, float lineThickness)
{
    constexpr float radiansToDegrees = 180.0f / static_cast<float>(M_PI);

    for (size_t i = 0; i + 1 < vertices.size(); i += 2)
    {
        const sf::Vector2f start = vertices[i].position;
        const sf::Vector2f end = vertices[i + 1].position;
        const sf::Vector2f delta = end - start;
        const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (length <= 0.0f)
        {
            continue;
        }

        sf::RectangleShape line(sf::Vector2f(length, lineThickness));
        line.setPosition(start);
        line.setOrigin(sf::Vector2f(0.0f, lineThickness * 0.3f));
        line.setRotation(sf::degrees(std::atan2(delta.y, delta.x) * radiansToDegrees));
        line.setFillColor(vertices[i].color);
        window.draw(line);
    }
}

void drawPythagorasSquares(sf::RenderWindow& window, const std::vector<PythagorasSquare>& squares,
    const std::array<sf::Color, kPaletteSize>& palette)
{
    for (const PythagorasSquare& square : squares)
    {
        sf::ConvexShape rectangle;
        rectangle.setPointCount(4);
        rectangle.setPoint(0, square.bottomLeft);
        rectangle.setPoint(1, square.bottomRight);
        rectangle.setPoint(2, square.topRight);
        rectangle.setPoint(3, square.topLeft);
        rectangle.setFillColor(palette[square.depth % kPaletteSize]);
        rectangle.setOutlineThickness(0.0f);
        window.draw(rectangle);
    }
}

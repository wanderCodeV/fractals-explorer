#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>

class MandelbrotExplorer {
public:
    MandelbrotExplorer(unsigned int width, unsigned int height);

    void reset(int iterationLevel = 5);
    void handleMouseClick(sf::Mouse::Button button, const sf::Vector2i& mousePosition);
    void handleMouseMove(const sf::Vector2i& mousePosition);
    void handleMouseRelease(sf::Mouse::Button button);
    void handleKeyPressed(sf::Keyboard::Key key);
    void update();
    void draw(sf::RenderWindow& window);

private:
    unsigned int displayWidth;
    unsigned int displayHeight;
    unsigned int width;
    unsigned int height;
    sf::Image image;
    sf::Texture texture;
    sf::Sprite sprite;

    double centerX = -0.5;
    double centerY = 0.0;
    double viewWidth = 3.5;
    double rotation = 0.0;
    double targetRotation = 0.0;
    int maxIterations = 180;
    int colorOffset = 0;
    sf::Vector2i lastMousePosition;
    bool hasMousePosition = false;
    bool isDragging = false;
    sf::Clock rotationClock;
    bool needsRedraw = true;

    void render();
    sf::Color colorFor(double smoothIterations) const;
};
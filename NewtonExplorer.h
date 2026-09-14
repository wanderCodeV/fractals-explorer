#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <complex>

class NewtonExplorer
{
public:
    NewtonExplorer(unsigned int width, unsigned int height, sf::Font& font);

    void handleMouseClick(const sf::Vector2i& mousePosition);
    void draw(sf::RenderWindow& window);
    void requestRedraw() { needsRedraw = true; }

private:
    void render();
    sf::Color colorForRoot(int rootIndex, int iterations) const;
    std::vector<std::complex<double>> generateRoots() const;

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

    int degree = 3;
    bool needsRedraw = true;
};
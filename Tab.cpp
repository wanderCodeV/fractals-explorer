#include "Tab.h"

Tab::Tab(std::string label, float x, sf::Font& font, FractalType t, sf::Color inactive) : text(font, label, 15) {
    type = t;
    inactiveColor = inactive;
    shape.setSize({ 150, 40 });
    shape.setPosition(sf::Vector2f(x, 0));
    shape.setFillColor(inactiveColor);
    shape.setOutlineThickness(1);
    shape.setOutlineColor(sf::Color::White);

    text.setFillColor(sf::Color::White);
    text.setPosition(sf::Vector2f(x + 10, 10));
}

void Tab::draw(sf::RenderWindow& window, FractalType current, sf::Color activeColor) {
    shape.setFillColor(type == current ? activeColor : inactiveColor);
    window.draw(shape);
    window.draw(text);
}
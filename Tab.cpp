#include "Tab.h"
Tab::Tab(std::string label, float x, float tabWidth, sf::Font& font, FractalType t, sf::Color inactive)
    : text(font, label, 13), type(t), inactiveColor(inactive), width(tabWidth)
{
    shape.setSize({ tabWidth, TAB_HEIGHT });
    shape.setPosition(sf::Vector2f(x, 0.0f));
    shape.setFillColor(inactiveColor);
    shape.setOutlineThickness(1.0f);
    shape.setOutlineColor(sf::Color::White);
    text.setFillColor(sf::Color::White);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(sf::Vector2f(bounds.position.x + bounds.size.x / 2.0f,
        bounds.position.y + bounds.size.y / 2.0f));
    text.setPosition(sf::Vector2f(x + tabWidth / 2.0f, TAB_HEIGHT / 2.0f));
}

void Tab::draw(sf::RenderWindow& window, FractalType current, sf::Color activeColor)
{
    shape.setFillColor(type == current ? activeColor : inactiveColor);
    window.draw(shape);
    window.draw(text);
}

#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

void generujKocha(int stopien, sf::Vector2f p1, sf::Vector2f p2, std::vector<sf::Vertex>& punkty, sf::Color color);
void generujSierpinski(int stopien, sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3, std::vector<sf::Vertex>& punkty, sf::Color color);
void generujTree(int stopien, sf::Vector2f start, float length, float angle, std::vector<sf::Vertex>& punkty, sf::Color color, float length_factor, float angle_left, float angle_right);

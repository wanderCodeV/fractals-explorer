#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

struct KochSegment {
	sf::Vector2f start;
	sf::Vector2f end;
};

struct Triangle {
	sf::Vector2f first;
	sf::Vector2f second;
	sf::Vector2f third;
};

struct PythagorasSquare {
	sf::Vector2f bottomLeft;
	sf::Vector2f bottomRight;
	sf::Vector2f topRight;
	sf::Vector2f topLeft;
	int depth;
};

void generujTree(int stopien, sf::Vector2f start, float length, float angle, std::vector<sf::Vertex>& punkty, sf::Color color, float length_factor, float angle_left, float angle_right);
std::vector<KochSegment> expandKocha(const std::vector<KochSegment>& segments);
void segmentsToVertices(const std::vector<KochSegment>& segments, std::vector<sf::Vertex>& punkty, sf::Color color);
std::vector<Triangle> expandSierpinski(const std::vector<Triangle>& triangles);
void trianglesToVertices(const std::vector<Triangle>& triangles, std::vector<sf::Vertex>& punkty, sf::Color color);
std::vector<PythagorasSquare> expandPythagoras(const std::vector<PythagorasSquare>& squares, float angleDegrees);
void generateBarnsleyFern(int level, std::vector<sf::Vertex>& points, sf::Color color, unsigned int width, unsigned int height);
void generateSierpinskiCarpet(int level, std::vector<sf::Vertex>& triangles, sf::Color color, unsigned int width, unsigned int height);
void generateSierpinskiPentagon(int level, std::vector<sf::Vertex>& triangles, sf::Color color, unsigned int width, unsigned int height);
void generateVicsek(int level, std::vector<sf::Vertex>& triangles, sf::Color color, unsigned int width, unsigned int height);
void generateHilbertCurve(int level, std::vector<sf::Vertex>& segments, sf::Color color, unsigned int width, unsigned int height);

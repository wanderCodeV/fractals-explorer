#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <cstdint>
#include <vector>
#include <cmath>

inline constexpr std::size_t kPaletteSize = 8;

using sf::Vector2f;
using sf::Vertex;
using std::vector;

inline std::array<sf::Color, kPaletteSize> makePalette(sf::Color base)
{
	std::array<sf::Color, kPaletteSize> palette{};

	// Calculate oposite color components for the palette
	const float targetR = 255.0f - base.r;
	const float targetG = 255.0f - base.g;
	const float targetB = 255.0f - base.b;

	constexpr float step = 1.0f / static_cast<float>(kPaletteSize - 1);

	for (std::size_t i = 0; i < kPaletteSize; ++i)
	{
		const float t = static_cast<float>(i) * step;

		// Linear Interpolation: base + t * (target - base)
		palette[i] = sf::Color(
			static_cast<std::uint8_t>(base.r + t * (targetR - base.r)),
			static_cast<std::uint8_t>(base.g + t * (targetG - base.g)),
			static_cast<std::uint8_t>(base.b + t * (targetB - base.b)),
			base.a
		);
	}
	return palette;
}

struct KochSegment
{
	Vector2f start;
	Vector2f end;
	int generation = 0;
};

struct Triangle
{
	Vector2f first;
	Vector2f second;
	Vector2f third;
	int generation = 0;
};

struct PythagorasSquare
{
	Vector2f bottomLeft;
	Vector2f bottomRight;
	Vector2f topRight;
	Vector2f topLeft;
	int depth;
};

void generujTree(int stopien, Vector2f start, float length, float angle, vector<Vertex>& punkty, const std::array<sf::Color, kPaletteSize>& palette, int maxStages, float length_factor, float angle_left, float angle_right);
vector<KochSegment> expandKocha(const vector<KochSegment>& segments, int expansionLevel);
void segmentsToVertices(const vector<KochSegment>& segments, vector<Vertex>& punkty, const std::array<sf::Color, kPaletteSize>& palette, bool append = false);
vector<Triangle> expandSierpinski(const vector<Triangle>& triangles, int expansionLevel);
void trianglesToVertices(const vector<Triangle>& triangles, vector<Vertex>& punkty, const std::array<sf::Color, kPaletteSize>& palette, bool append = false);
vector<PythagorasSquare> expandPythagoras(const vector<PythagorasSquare>& squares, float angleDegrees);
void generateDragon(int level, vector<Vertex>& segments, const std::array<sf::Color, kPaletteSize>& palette, unsigned int width, unsigned int height);
void generateSierpinskiTriangle(int level, vector<Vertex>& triangles, const std::array<sf::Color, kPaletteSize>& palette, Vector2f p1, Vector2f p2, Vector2f p3);
void generateSierpinskiCarpet(int level, vector<Vertex>& triangles, const std::array<sf::Color, kPaletteSize>& palette, unsigned int width, unsigned int height);
void generateSierpinskiPentagon(int level, vector<Vertex>& triangles, const std::array<sf::Color, kPaletteSize>& palette, unsigned int width, unsigned int height);
void generateVicsek(int level, vector<Vertex>& triangles, const std::array<sf::Color, kPaletteSize>& palette, unsigned int width, unsigned int height);
void generateHilbertCurve(int level, vector<Vertex>& segments, const std::array<sf::Color, kPaletteSize>& palette, unsigned int width, unsigned int height);

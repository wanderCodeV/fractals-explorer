#include "Fractals.h"

#include <array>
#include <cstdint>
#include <functional>

void generujTree(int stopien, sf::Vector2f start, float length, float angle, std::vector<sf::Vertex>& punkty, sf::Color color, float length_factor, float angle_left, float angle_right) {
    if (stopien == 0 || length < 1.0f) return;

    float rad = angle * M_PI / 180.0f;
    sf::Vector2f end = start + sf::Vector2f(length * cos(rad), -length * sin(rad));  

    sf::Vertex v1;
    v1.position = start;
    v1.color = color;
    sf::Vertex v2;
    v2.position = end;
    v2.color = color;
    punkty.push_back(v1);
    punkty.push_back(v2);

    float new_length = length * length_factor;
    generujTree(stopien - 1, end, new_length, angle + angle_left, punkty, color, length_factor, angle_left, angle_right);
    generujTree(stopien - 1, end, new_length, angle + angle_right, punkty, color, length_factor, angle_left, angle_right);
}

std::vector<KochSegment> expandKocha(const std::vector<KochSegment>& segments) {
    std::vector<KochSegment> expanded;
    expanded.reserve(segments.size() * 4);
    const float sine60 = std::sqrt(3.0f) * 0.5f;

    for (const KochSegment& segment : segments) {
        const sf::Vector2f third = (segment.end - segment.start) / 3.0f;
        const sf::Vector2f first = segment.start + third;
        const sf::Vector2f second = segment.start + third * 2.0f;
        const sf::Vector2f side = second - first;
        const sf::Vector2f peak(
            first.x + side.x * 0.5f + side.y * sine60,
            first.y - side.x * sine60 + side.y * 0.5f
        );

        expanded.push_back({segment.start, first});
        expanded.push_back({first, peak});
        expanded.push_back({peak, second});
        expanded.push_back({second, segment.end});
    }

    return expanded;
}

void segmentsToVertices(const std::vector<KochSegment>& segments, std::vector<sf::Vertex>& punkty, sf::Color color) {
    punkty.clear();
    punkty.reserve(segments.size() * 2);

    for (const KochSegment& segment : segments) {
        sf::Vertex start;
        start.position = segment.start;
        start.color = color;
        sf::Vertex end;
        end.position = segment.end;
        end.color = color;
        punkty.push_back(start);
        punkty.push_back(end);
    }
}

std::vector<Triangle> expandSierpinski(const std::vector<Triangle>& triangles) {
    std::vector<Triangle> expanded;
    expanded.reserve(triangles.size() * 3);

    for (const Triangle& triangle : triangles) {
        const sf::Vector2f midpoint12 = (triangle.first + triangle.second) / 2.0f;
        const sf::Vector2f midpoint23 = (triangle.second + triangle.third) / 2.0f;
        const sf::Vector2f midpoint31 = (triangle.third + triangle.first) / 2.0f;

        expanded.push_back({triangle.first, midpoint12, midpoint31});
        expanded.push_back({midpoint12, triangle.second, midpoint23});
        expanded.push_back({midpoint31, midpoint23, triangle.third});
    }

    return expanded;
}

void trianglesToVertices(const std::vector<Triangle>& triangles, std::vector<sf::Vertex>& punkty, sf::Color color) {
    punkty.clear();
    punkty.reserve(triangles.size() * 3);

    for (const Triangle& triangle : triangles) {
        const sf::Vector2f points[] = {triangle.first, triangle.second, triangle.third};

        for (const sf::Vector2f& point : points) {
            sf::Vertex vertex;
            vertex.position = point;
            vertex.color = color;
            punkty.push_back(vertex);
        }
    }
}

std::vector<PythagorasSquare> expandPythagoras(const std::vector<PythagorasSquare>& squares, float angleDegrees) {
    std::vector<PythagorasSquare> expanded;
    expanded.reserve(squares.size() * 2);
    const float angle = angleDegrees * M_PI / 180.0f;

    for (const PythagorasSquare& square : squares) {
        sf::Vector2f leftBase = square.topLeft;
        sf::Vector2f rightBase = square.topRight;

        float dx = rightBase.x - leftBase.x;
        float dy = rightBase.y - leftBase.y;

        float cos_a = std::cos(angle);
        float sin_a = std::sin(angle);

        sf::Vector2f middleTop = {
            leftBase.x + cos_a * (dx * cos_a + dy * sin_a),
            leftBase.y + cos_a * (dy * cos_a - dx * sin_a)
        };

        auto addSquare = [&](sf::Vector2f p1, sf::Vector2f p2) {
            float vx = p2.x - p1.x;
            float vy = p2.y - p1.y;
            
            sf::Vector2f v_perp = {vy, -vx};

            sf::Vector2f topLeft = p1 + v_perp;
            sf::Vector2f topRight = p2 + v_perp;

            expanded.push_back({p1, p2, topRight, topLeft, square.depth + 1});
        };

        addSquare(leftBase, middleTop);

        addSquare(middleTop, rightBase);
    }

    return expanded;
}

namespace {
void appendSquare(std::vector<sf::Vertex>& triangles, float left, float top, float size, sf::Color color) {
    const sf::Vector2f a(left, top);
    const sf::Vector2f b(left + size, top);
    const sf::Vector2f c(left + size, top + size);
    const sf::Vector2f d(left, top + size);
    for (const sf::Vector2f& point : {a, b, c, a, c, d}) {
        triangles.push_back({point, color});
    }
}

void appendPentagon(std::vector<sf::Vertex>& triangles, sf::Vector2f center, float radius, sf::Color color) {
    std::array<sf::Vector2f, 5> points{};
    for (int index = 0; index < 5; ++index) {
        const float angle = -3.14159265359f * 0.5f + index * 2.0f * 3.14159265359f / 5.0f;
        points[index] = center + sf::Vector2f(std::cos(angle), std::sin(angle)) * radius;
    }
    for (int index = 0; index < 5; ++index) {
        triangles.push_back({center, color});
        triangles.push_back({points[index], color});
        triangles.push_back({points[(index + 1) % 5], color});
    }
}

void rotateHilbert(int size, int& x, int& y, int rotateX, int rotateY) {
    if (rotateY == 0) {
        if (rotateX == 1) {
            x = size - 1 - x;
            y = size - 1 - y;
        }
        std::swap(x, y);
    }
}

sf::Vector2i hilbertPoint(int order, int distance) {
    int x = 0;
    int y = 0;
    for (int size = 1, value = distance; size < order; size *= 2) {
        const int rotateX = 1 & (value / 2);
        const int rotateY = 1 & (value ^ rotateX);
        rotateHilbert(size, x, y, rotateX, rotateY);
        x += size * rotateX;
        y += size * rotateY;
        value /= 4;
    }
    return {x, y};
}
}

void generateBarnsleyFern(int level, std::vector<sf::Vertex>& points, sf::Color color, unsigned int width, unsigned int height) {
    points.clear();
    constexpr int stemPoints = 80;
    for (int index = 0; index < stemPoints; ++index) {
        const float progress = static_cast<float>(index) / (stemPoints - 1);
        points.push_back({
            {width * 0.5f, height - progress * height * 0.15f},
            color
        });
    }

    std::uint32_t state = 0x12345678u;
    sf::Vector2f point(0.0f, 0.0f);
    const int count = 350 * level;
    if (level == 0) {
        return;
    }

    for (int index = 0; index < count + 20; ++index) {
        state = state * 1664525u + 1013904223u;
        const float random = static_cast<float>(state % 10000u) / 10000.0f;
        if (random < 0.01f) {
            point = {0.0f, 0.16f * point.y};
        } else if (random < 0.86f) {
            point = {0.85f * point.x + 0.04f * point.y, -0.04f * point.x + 0.85f * point.y + 1.6f};
        } else if (random < 0.93f) {
            point = {0.2f * point.x - 0.26f * point.y, 0.23f * point.x + 0.22f * point.y + 1.6f};
        } else {
            point = {-0.15f * point.x + 0.28f * point.y, 0.26f * point.x + 0.24f * point.y + 0.44f};
        }
        if (index >= 20) {
            points.push_back({
                {width * 0.5f + point.x * width * 0.1f, height - point.y * height * 0.09f},
                color
            });
        }
    }
}

void generateSierpinskiCarpet(int level, std::vector<sf::Vertex>& triangles, sf::Color color, unsigned int width, unsigned int height) {
    triangles.clear();
    const float size = std::min(width, height) * 0.72f;
    const float left = (width - size) * 0.5f;
    const float top = (height - size) * 0.5f;
    std::function<void(float, float, float, int)> add = [&](float x, float y, float currentSize, int currentLevel) {
        if (currentLevel == 0) {
            appendSquare(triangles, x, y, currentSize, color);
            return;
        }
        const float part = currentSize / 3.0f;
        for (int row = 0; row < 3; ++row) {
            for (int column = 0; column < 3; ++column) {
                if (row != 1 || column != 1) {
                    add(x + column * part, y + row * part, part, currentLevel - 1);
                }
            }
        }
    };
    add(left, top, size, level);
}

void generateSierpinskiPentagon(int level, std::vector<sf::Vertex>& triangles, sf::Color color, unsigned int width, unsigned int height) {
    triangles.clear();
    const sf::Vector2f center(width * 0.5f, height * 0.5f);
    const float radius = std::min(width, height) * 0.36f;
    std::function<void(sf::Vector2f, float, int)> add = [&](sf::Vector2f currentCenter, float currentRadius, int currentLevel) {
        if (currentLevel == 0) {
            appendPentagon(triangles, currentCenter, currentRadius, color);
            return;
        }
        for (int index = 0; index < 5; ++index) {
            const float angle = -3.14159265359f * 0.5f + index * 2.0f * 3.14159265359f / 5.0f;
            add(currentCenter + sf::Vector2f(std::cos(angle), std::sin(angle)) * currentRadius * 0.62f, currentRadius * 0.38f, currentLevel - 1);
        }
    };
    add(center, radius, level);
}

void generateVicsek(int level, std::vector<sf::Vertex>& triangles, sf::Color color, unsigned int width, unsigned int height) {
    triangles.clear();
    const float size = std::min(width, height) * 0.72f;
    const float left = (width - size) * 0.5f;
    const float top = (height - size) * 0.5f;
    std::function<void(float, float, float, int)> add = [&](float x, float y, float currentSize, int currentLevel) {
        if (currentLevel == 0) {
            appendSquare(triangles, x, y, currentSize, color);
            return;
        }
        const float part = currentSize / 3.0f;
        const int positions[][2] = {{1, 0}, {0, 1}, {1, 1}, {2, 1}, {1, 2}};
        for (const auto& position : positions) {
            add(x + position[0] * part, y + position[1] * part, part, currentLevel - 1);
        }
    };
    add(left, top, size, level);
}

void generateHilbertCurve(int level, std::vector<sf::Vertex>& segments, sf::Color color, unsigned int width, unsigned int height) {
    segments.clear();
    const int clampedLevel = std::max(0, std::min(level, 7));
    const int order = 1 << clampedLevel;
    const float size = std::min(width, height) * 0.72f;
    const float cellSize = size / static_cast<float>(order);
    const float left = (width - size) * 0.5f;
    const float top = (height - size) * 0.5f;
    std::vector<sf::Vector2f> points;
    points.reserve(order * order);
    for (int distance = 0; distance < order * order; ++distance) {
        const sf::Vector2i gridPoint = hilbertPoint(order, distance);
        points.emplace_back(
            left + (gridPoint.x + 0.5f) * cellSize,
            top + (gridPoint.y + 0.5f) * cellSize
        );
    }
    for (std::size_t point = 1; point < points.size(); ++point) {
        segments.push_back({points[point - 1], color});
        segments.push_back({points[point], color});
    }
}
#include "Fractals.h"

void generujTree(int stopien, sf::Vector2f start, float length, float angle, std::vector<sf::Vertex>& punkty, sf::Color color, float length_factor, float angle_left, float angle_right) {
    if (stopien == 0 || length < 1.0f) return;

    // Calculate end point
    float rad = angle * 3.14159f / 180.0f;
    sf::Vector2f end = start + sf::Vector2f(length * cos(rad), -length * sin(rad));  // Negative sin for SFML y-down

    // Add line
    sf::Vertex v1;
    v1.position = start;
    v1.color = color;
    sf::Vertex v2;
    v2.position = end;
    v2.color = color;
    punkty.push_back(v1);
    punkty.push_back(v2);

    // Recurse for branches
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
    punkty.reserve(triangles.size() * 6);

    for (const Triangle& triangle : triangles) {
        const sf::Vector2f points[] = {
            triangle.first, triangle.second,
            triangle.second, triangle.third,
            triangle.third, triangle.first
        };

        for (const sf::Vector2f& point : points) {
            sf::Vertex vertex;
            vertex.position = point;
            vertex.color = color;
            punkty.push_back(vertex);
        }
    }
}
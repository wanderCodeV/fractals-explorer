#include "Fractals.h"

void generujKocha(int stopien, sf::Vector2f p1, sf::Vector2f p2, std::vector<sf::Vertex>& punkty, sf::Color color) {
    if (stopien == 0) {
        sf::Vertex v1;
        v1.position = p1;
        v1.color = color;
        punkty.push_back(v1);
        sf::Vertex v2;
        v2.position = p2;
        v2.color = color;
        punkty.push_back(v2);
    }
    else {
        sf::Vector2f v = (p2 - p1) / 3.0f;
        sf::Vector2f a = p1 + v;
        sf::Vector2f b = p2 - v;

        float kat = -60.0f * 3.14159f / 180.0f;
        sf::Vector2f c;
        c.x = a.x + (v.x * cos(kat) - v.y * sin(kat));
        c.y = a.y + (v.x * sin(kat) + v.y * cos(kat));

        generujKocha(stopien - 1, p1, a, punkty, color);
        generujKocha(stopien - 1, a, c, punkty, color);
        generujKocha(stopien - 1, c, b, punkty, color);
        generujKocha(stopien - 1, b, p2, punkty, color);
    }
}

void generujSierpinski(int stopien, sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3, std::vector<sf::Vertex>& punkty, sf::Color color) {
    if (stopien == 0) {
        // Draw the triangle outline
        sf::Vertex v1;
        v1.position = p1;
        v1.color = color;
        sf::Vertex v2;
        v2.position = p2;
        v2.color = color;
        sf::Vertex v3;
        v3.position = p3;
        v3.color = color;
        sf::Vertex v4;
        v4.position = p1;
        v4.color = color;  // Close the triangle
        punkty.push_back(v1);
        punkty.push_back(v2);
        punkty.push_back(v2);
        punkty.push_back(v3);
        punkty.push_back(v3);
        punkty.push_back(v4);
    } else {
        // Compute midpoints
        sf::Vector2f m12 = (p1 + p2) / 2.0f;
        sf::Vector2f m23 = (p2 + p3) / 2.0f;
        sf::Vector2f m31 = (p3 + p1) / 2.0f;

        // Recurse on three smaller triangles
        generujSierpinski(stopien - 1, p1, m12, m31, punkty, color);
        generujSierpinski(stopien - 1, m12, p2, m23, punkty, color);
        generujSierpinski(stopien - 1, m31, m23, p3, punkty, color);
    }
}

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
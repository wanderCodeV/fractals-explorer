#include "Fractals.h"

#include <array>
#include <functional>

using namespace std;
using sf::Color;
using sf::Vector2f;
using sf::Vertex;

void generujTree(int stopien, Vector2f start, float length, float angle, vector<Vertex> &punkty,
                 const array<Color, kPaletteSize> &palette, int maxStages,
                 float length_factor, float angle_left, float angle_right)
{

    if (stopien == 0 || length < 1.0f)
        return;

    const Color branchColor = palette[(maxStages - stopien) % kPaletteSize];
    const float rad = angle * static_cast<float>(M_PI) / 180.0f;
    const Vector2f end = start + Vector2f(length * cos(rad), -length * sin(rad));

    punkty.push_back({start, branchColor}); // start point of the branch
    punkty.push_back({end, branchColor});   // end point of the branch

    generujTree(stopien - 1, end, length * length_factor, angle + angle_left, punkty,
                palette, maxStages, length_factor, angle_left, angle_right);
    generujTree(stopien - 1, end, length * length_factor, angle + angle_right, punkty,
                palette, maxStages, length_factor, angle_left, angle_right);
}

// Replace each segment with four shorter ones and a triangular bump.
vector<KochSegment> expandKocha(const vector<KochSegment> &segments, int expansionLevel)
{
    vector<KochSegment> expanded;
    expanded.reserve(segments.size() * 4);
    const float sine60 = sqrt(3.0f) * 0.5f;

    for (const KochSegment &segment : segments)
    {
        const Vector2f third = (segment.end - segment.start) / 3.0f;
        const Vector2f first = segment.start + third;
        const Vector2f second = segment.start + third * 2.0f;
        const Vector2f side = second - first;
        const Vector2f peak(
            first.x + side.x * 0.5f + side.y * sine60,
            first.y - side.x * sine60 + side.y * 0.5f);

        expanded.push_back({segment.start, first, segment.generation});
        expanded.push_back({first, peak, expansionLevel});
        expanded.push_back({peak, second, expansionLevel});
        expanded.push_back({second, segment.end, segment.generation});
    }

    return expanded;
}

void segmentsToVertices(const vector<KochSegment> &segments, vector<Vertex> &punkty,
                        const array<Color, kPaletteSize> &palette, bool append)
{
    if (!append)
    {
        punkty.clear();
    }
    punkty.reserve(punkty.size() + segments.size() * 2);

    for (const KochSegment &segment : segments)
    {
        const Color segmentColor = palette[segment.generation % kPaletteSize];
        punkty.push_back({segment.start, segmentColor});
        punkty.push_back({segment.end, segmentColor});
    }
}

vector<Triangle> expandSierpinski(const vector<Triangle> &triangles, int expansionLevel)
{
    vector<Triangle> expanded;
    expanded.reserve(triangles.size() * 3);

    for (const Triangle &triangle : triangles)
    {
        const Vector2f midpoint12 = (triangle.first + triangle.second) / 2.0f;
        const Vector2f midpoint23 = (triangle.second + triangle.third) / 2.0f;
        const Vector2f midpoint31 = (triangle.third + triangle.first) / 2.0f;

        expanded.push_back({triangle.first, midpoint12, midpoint31, triangle.generation});
        expanded.push_back({midpoint12, triangle.second, midpoint23, expansionLevel});
        expanded.push_back({midpoint31, midpoint23, triangle.third, triangle.generation});
    }

    return expanded;
}

void trianglesToVertices(const vector<Triangle> &triangles, vector<Vertex> &punkty,
                         const array<Color, kPaletteSize> &palette, bool append)
{
    if (!append)
    {
        punkty.clear();
    }
    punkty.reserve(punkty.size() + triangles.size() * 3);

    for (const Triangle &triangle : triangles)
    {
        const Color triangleColor = palette[triangle.generation % kPaletteSize];
        for (const Vector2f &point : {triangle.first, triangle.second, triangle.third})
        {
            punkty.push_back({point, triangleColor});
        }
    }
}

vector<PythagorasSquare> expandPythagoras(const vector<PythagorasSquare> &squares, float angleDegrees)
{
    vector<PythagorasSquare> expanded;
    expanded.reserve(squares.size() * 2);
    const float angle = angleDegrees * static_cast<float>(M_PI) / 180.0f;

    for (const PythagorasSquare &square : squares)
    {
        const Vector2f leftBase = square.topLeft;
        const Vector2f rightBase = square.topRight;
        const float dx = rightBase.x - leftBase.x;
        const float dy = rightBase.y - leftBase.y;
        const float cosA = cos(angle);
        const float sinA = sin(angle);
        const Vector2f middleTop = {
            leftBase.x + cosA * (dx * cosA + dy * sinA),
            leftBase.y + cosA * (dy * cosA - dx * sinA)};

        auto addSquare = [&](Vector2f p1, Vector2f p2)
        {
            const float vx = p2.x - p1.x;
            const float vy = p2.y - p1.y;
            const Vector2f vPerp = {vy, -vx};
            expanded.push_back({p1, p2, p2 + vPerp, p1 + vPerp, square.depth + 1});
        };

        addSquare(leftBase, middleTop);
        addSquare(middleTop, rightBase);
    }

    return expanded;
}

namespace
{
    struct SquareLayout
    {
        float size;
        float left;
        float top;
    };

    SquareLayout centeredSquareLayout(unsigned int width, unsigned int height, float scale = 0.72f)
    {
        const float size = min(width, height) * scale;
        return {size, (width - size) * 0.5f, (height - size) * 0.5f};
    }

    void appendSquare(vector<Vertex> &triangles, float left, float top, float size, Color color)
    {
        const Vector2f a(left, top);
        const Vector2f b(left + size, top);
        const Vector2f c(left + size, top + size);
        const Vector2f d(left, top + size);
        for (const Vector2f &point : {a, b, c, a, c, d})
        {
            triangles.push_back({point, color});
        }
    }

    void appendPentagon(vector<Vertex> &triangles, Vector2f center, float radius, Color color)
    {
        array<Vector2f, 5> points{};
        for (int index = 0; index < 5; ++index)
        {
            const float angle = -3.14159265359f * 0.5f + index * 2.0f * 3.14159265359f / 5.0f;
            points[index] = center + Vector2f(cos(angle), sin(angle)) * radius;
        }
        for (int index = 0; index < 5; ++index)
        {
            triangles.push_back({center, color});
            triangles.push_back({points[index], color});
            triangles.push_back({points[(index + 1) % 5], color});
        }
    }

    void rotateHilbert(int size, int &x, int &y, int rotateX, int rotateY)
    {
        if (rotateY == 0)
        {
            if (rotateX == 1)
            {
                x = size - 1 - x;
                y = size - 1 - y;
            }
            swap(x, y);
        }
    }

    sf::Vector2i hilbertPoint(int order, int distance)
    {
        int x = 0;
        int y = 0;
        for (int size = 1, value = distance; size < order; size *= 2)
        {
            const int rotateX = 1 & (value / 2);
            const int rotateY = 1 & (value ^ rotateX);
            rotateHilbert(size, x, y, rotateX, rotateY);
            x += size * rotateX;
            y += size * rotateY;
            value /= 4;
        }
        return {x, y};
    }

    void fitPolylineToWindow(vector<Vector2f> &points, unsigned int width, unsigned int height)
    {
        Vector2f minimum = points.front();
        Vector2f maximum = points.front();
        for (const Vector2f &point : points)
        {
            minimum.x = min(minimum.x, point.x);
            minimum.y = min(minimum.y, point.y);
            maximum.x = max(maximum.x, point.x);
            maximum.y = max(maximum.y, point.y);
        }
        const float sourceWidth = max(1.0f, maximum.x - minimum.x);
        const float sourceHeight = max(1.0f, maximum.y - minimum.y);
        const float scale = min(width * 0.7f / sourceWidth, height * 0.7f / sourceHeight);
        const Vector2f offset(
            (width - sourceWidth * scale) * 0.5f,
            (height - sourceHeight * scale) * 0.5f);
        for (Vector2f &point : points)
        {
            point = offset + Vector2f(
                                 (point.x - minimum.x) * scale,
                                 (point.y - minimum.y) * scale);
        }
    }
}

// Build the dragon curve turn sequence, then trace it into line segments.
void generateDragon(int level, vector<Vertex> &segments,
                    const array<Color, kPaletteSize> &palette, unsigned int width, unsigned int height)
{
    vector<int> turns;
    for (int step = 0; step < max(0, level); ++step)
    {
        vector<int> nextTurns = turns;
        nextTurns.push_back(1);
        for (auto turn = turns.rbegin(); turn != turns.rend(); ++turn)
        {
            nextTurns.push_back(-*turn);
        }
        turns = move(nextTurns);
    }

    vector<Vector2f> points = {{0.0f, 0.0f}};
    Vector2f position(0.0f, 0.0f);
    Vector2f direction(1.0f, 0.0f);
    const float segmentLength = 1.0f;
    for (int turn : turns)
    {
        position += direction * segmentLength;
        points.push_back(position);
        direction = turn > 0
                        ? Vector2f(-direction.y, direction.x)
                        : Vector2f(direction.y, -direction.x);
    }

    fitPolylineToWindow(points, width, height);

    segments.clear();
    segments.reserve((points.size() - 1) * 2);
    const Color segmentColor = palette[level % kPaletteSize];
    for (size_t index = 1; index < points.size(); ++index)
    {
        segments.push_back({points[index - 1], segmentColor});
        segments.push_back({points[index], segmentColor});
    }
}

void generateSierpinskiTriangle(int level, vector<Vertex> &triangles,
                                const array<Color, kPaletteSize> &palette, Vector2f p1, Vector2f p2, Vector2f p3)
{
    triangles.clear();
    function<void(Vector2f, Vector2f, Vector2f, int)> add = [&](Vector2f a, Vector2f b, Vector2f c, int currentLevel)
    {
        if (currentLevel == 0)
        {
            const Color color = palette[(level - currentLevel) % kPaletteSize];
            for (const Vector2f &point : {a, b, c})
            {
                triangles.push_back({point, color});
            }
            return;
        }

        const Vector2f ab = (a + b) * 0.5f;
        const Vector2f bc = (b + c) * 0.5f;
        const Vector2f ca = (c + a) * 0.5f;
        add(a, ab, ca, currentLevel - 1);
        add(ab, b, bc, currentLevel - 1);
        add(ca, bc, c, currentLevel - 1);
    };
    add(p1, p2, p3, level);
}

void generateSierpinskiCarpet(int level, vector<Vertex> &triangles,
                              const array<Color, kPaletteSize> &palette, unsigned int width, unsigned int height)
{
    triangles.clear();
    const SquareLayout layout = centeredSquareLayout(width, height);
    function<void(float, float, float, int)> add = [&](float x, float y, float currentSize, int currentLevel)
    {
        if (currentLevel == 0)
        {
            appendSquare(triangles, x, y, currentSize, palette[(level - currentLevel) % kPaletteSize]);
            return;
        }
        const float part = currentSize / 3.0f;
        for (int row = 0; row < 3; ++row)
        {
            for (int column = 0; column < 3; ++column)
            {
                if (row != 1 || column != 1)
                {
                    add(x + column * part, y + row * part, part, currentLevel - 1);
                }
            }
        }
    };
    add(layout.left, layout.top, layout.size, level);
}

void generateSierpinskiPentagon(int level, vector<Vertex> &triangles,
                                const array<Color, kPaletteSize> &palette, unsigned int width, unsigned int height)
{
    triangles.clear();
    const Vector2f center(width * 0.5f, height * 0.5f);
    const float radius = min(width, height) * 0.36f;
    function<void(Vector2f, float, int)> add = [&](Vector2f currentCenter, float currentRadius, int currentLevel)
    {
        if (currentLevel == 0)
        {
            appendPentagon(triangles, currentCenter, currentRadius,
                           palette[(level - currentLevel) % kPaletteSize]);
            return;
        }
        for (int index = 0; index < 5; ++index)
        {
            const float angle = -3.14159265359f * 0.5f + index * 2.0f * 3.14159265359f / 5.0f;
            add(currentCenter + Vector2f(cos(angle), sin(angle)) * currentRadius * 0.62f,
                currentRadius * 0.38f, currentLevel - 1);
        }
    };
    add(center, radius, level);
}

void generateVicsek(int level, vector<Vertex> &triangles,
                    const array<Color, kPaletteSize> &palette, unsigned int width, unsigned int height)
{
    triangles.clear();
    const SquareLayout layout = centeredSquareLayout(width, height);
    function<void(float, float, float, int)> add = [&](float x, float y, float currentSize, int currentLevel)
    {
        if (currentLevel == 0)
        {
            appendSquare(triangles, x, y, currentSize, palette[(level - currentLevel) % kPaletteSize]);
            return;
        }
        const float part = currentSize / 3.0f;
        const int positions[][2] = {{1, 0}, {0, 1}, {1, 1}, {2, 1}, {1, 2}};
        for (const auto &position : positions)
        {
            add(x + position[0] * part, y + position[1] * part, part, currentLevel - 1);
        }
    };
    add(layout.left, layout.top, layout.size, level);
}

// Walk the Hilbert space-filling curve for the given recursion level.
void generateHilbertCurve(int level, vector<Vertex> &segments,
                          const array<Color, kPaletteSize> &palette, unsigned int width, unsigned int height)
{
    segments.clear();
    const int clampedLevel = max(0, min(level, 7));
    const int order = 1 << clampedLevel;
    const SquareLayout layout = centeredSquareLayout(width, height);
    const float cellSize = layout.size / static_cast<float>(order);
    vector<Vector2f> points;
    points.reserve(order * order);
    for (int distance = 0; distance < order * order; ++distance)
    {
        const sf::Vector2i gridPoint = hilbertPoint(order, distance);
        points.emplace_back(
            layout.left + (gridPoint.x + 0.5f) * cellSize,
            layout.top + (gridPoint.y + 0.5f) * cellSize);
    }
    const Color segmentColor = palette[level % kPaletteSize];
    for (size_t point = 1; point < points.size(); ++point)
    {
        segments.push_back({points[point - 1], segmentColor});
        segments.push_back({points[point], segmentColor});
    }
}

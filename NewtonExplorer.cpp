#include "NewtonExplorer.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr int maxIterations = 50;
    constexpr double convergenceRadius = 1e-4;
    constexpr double minDenominator = 1e-10;

    // Obliczanie potęgi liczby zespolonej za pomocą wzoru De Moivre'a
    std::complex<double> raiseComplex(std::complex<double> z, int power)
    {
        if (power == 0) return { 1.0, 0.0 };
        if (power == 1) return z;

        const double mag = std::pow(std::abs(z), power);
        const double ang = std::arg(z) * power;
        return { mag * std::cos(ang), mag * std::sin(ang) };
    }

    // Krok metody Newtona dla z^n - 1 = 0
    std::complex<double> newtonStep(std::complex<double> z, int n)
    {
        const auto denom = static_cast<double>(n) * raiseComplex(z, n - 1);
        if (std::abs(denom) < minDenominator) return z;
        return z - (raiseComplex(z, n) - 1.0) / denom;
    }

    // Konwersja kolorów z przestrzeni HSL do sf::Color (H: 0-360, S: 0-1, L: 0-1)
    sf::Color hslToRgb(float h, float s, float l)
    {
        auto f = [h, s, l](float n) {
            float k = std::fmod(n + h / 30.0f, 12.0f);
            float a = s * std::min(l, 1.0f - l);
            return l - a * std::max(-1.0f, std::min({ k - 3.0f, 9.0f - k, 1.0f }));
            };
        return sf::Color(
            static_cast<std::uint8_t>(f(0) * 255),
            static_cast<std::uint8_t>(f(8) * 255),
            static_cast<std::uint8_t>(f(4) * 255)
        );
    }
}

NewtonExplorer::NewtonExplorer(unsigned int width, unsigned int height, sf::Font& fontRef)
    : displayWidth(width),
    displayHeight(height),
    renderWidth(std::min(width, 800u)),
    renderHeight(std::max(1u, renderWidth* height / width)),
    image(sf::Vector2u(renderWidth, renderHeight), sf::Color::Black),
    texture(sf::Vector2u(renderWidth, renderHeight)),
    sprite(texture),
    font(fontRef),
    degreeLabel(fontRef, "n = 3", 16)
{
    texture.setSmooth(true);
    sprite.setScale({ static_cast<float>(displayWidth) / renderWidth, static_cast<float>(displayHeight) / renderHeight });

    toggleButton.setSize({ 90.0f, 30.0f });
    toggleButton.setPosition({ 10.0f, static_cast<float>(displayHeight) - 40.0f });
    toggleButton.setFillColor(sf::Color(50, 50, 50));
    toggleButton.setOutlineThickness(1.0f);
    toggleButton.setOutlineColor(sf::Color::White);

    degreeLabel.setFillColor(sf::Color::White);
    degreeLabel.setPosition({ 18.0f, static_cast<float>(displayHeight) - 37.0f });
}

void NewtonExplorer::handleMouseClick(const sf::Vector2i& mousePosition)
{
    if (toggleButton.getGlobalBounds().contains(sf::Vector2f(mousePosition)))
    {
        degree = (degree >= 6) ? 3 : degree + 1;
        degreeLabel.setString("n = " + std::to_string(degree));
        needsRedraw = true;
    }
}

sf::Color NewtonExplorer::colorForRoot(int rootIndex, int iterations) const
{
    const float hue = (360.0f / degree) * rootIndex;
    const float lightness = std::max(0.2f, 0.7f - (iterations * 0.01f));
    return hslToRgb(hue, 0.85f, lightness);
}

std::vector<std::complex<double>> NewtonExplorer::generateRoots() const
{
    std::vector<std::complex<double>> roots(degree);
    const double pi = std::acos(-1.0);

    for (int i = 0; i < degree; ++i)
    {
        const double angle = 2.0 * pi * i / degree;
        roots[i] = { std::cos(angle), std::sin(angle) };
    }
    return roots;
}

void NewtonExplorer::render()
{
    const double viewWidth = 3.2;
    const double viewHeight = viewWidth * (static_cast<double>(renderHeight) / renderWidth);
    const auto roots = generateRoots();

    for (unsigned int y = 0; y < renderHeight; ++y)
    {
        for (unsigned int x = 0; x < renderWidth; ++x)
        {
            std::complex<double> z(
                (static_cast<double>(x) / renderWidth - 0.5) * viewWidth,
                (0.5 - static_cast<double>(y) / renderHeight) * viewHeight
            );

            int iterations = 0;
            int rootIndex = -1;

            for (; iterations < maxIterations; ++iterations)
            {
                z = newtonStep(z, degree);
                for (int candidate = 0; candidate < degree; ++candidate)
                {
                    if (std::abs(z - roots[candidate]) < convergenceRadius)
                    {
                        rootIndex = candidate;
                        break;
                    }
                }
                if (rootIndex != -1) break;
            }

            const sf::Color color = (rootIndex == -1)
                ? sf::Color(15, 15, 25)
                : colorForRoot(rootIndex, iterations);

            image.setPixel(sf::Vector2u(x, y), color);
        }
    }

    if (!texture.loadFromImage(image))
    {
        needsRedraw = true;
        return;
    }

    needsRedraw = false;
}

void NewtonExplorer::draw(sf::RenderWindow& window)
{
    if (needsRedraw) render();

    window.draw(sprite);
    window.draw(toggleButton);
    window.draw(degreeLabel);
}
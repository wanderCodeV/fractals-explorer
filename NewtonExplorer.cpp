#include "NewtonExplorer.h"
#include <algorithm>
#include <cmath>
#include <complex>

namespace
{
    constexpr int maxIterations = 50;
    constexpr double convergenceRadius = 1e-4;
    constexpr double minDenominator = 1e-10;
    std::complex<double> raiseComplex(std::complex<double> z, int power)
    {
        if (power == 0)
            return { 1.0, 0.0 };
        if (power == 1)
            return z;

        const double magnitude = std::abs(z);
        const double angle = std::arg(z);
        const double newMagnitude = std::pow(magnitude, power);
        const double newAngle = angle * static_cast<double>(power);
        return { newMagnitude * std::cos(newAngle), newMagnitude * std::sin(newAngle) };
    }

    // One Newton step for f(z) = z^n - 1.
    std::complex<double> newtonStep(std::complex<double> z, int n)
    {
        const std::complex<double> numerator = raiseComplex(z, n) - std::complex<double>(1.0, 0.0);
        const std::complex<double> denominator = static_cast<double>(n) * raiseComplex(z, n - 1);
        if (std::abs(denominator) < minDenominator)
        {
            return z;
        }
        return z - numerator / denominator;
    }

}

NewtonExplorer::NewtonExplorer(unsigned int width, unsigned int height, sf::Font& fontRef, sf::Color baseColor) : displayWidth(width),
displayHeight(height),
renderWidth(std::min(width, 800u)),
renderHeight(std::max(1u, static_cast<unsigned int>(static_cast<double>(std::min(width, 800u))* height / width))),
image(sf::Vector2u(renderWidth, renderHeight), sf::Color::Black),
texture(sf::Vector2u(renderWidth, renderHeight)),
sprite(texture),
font(fontRef),
degreeLabel(fontRef, "n = 3", 16),
palette(makePalette(baseColor))
{
    texture.setSmooth(true);
    sprite.setScale(sf::Vector2f(static_cast<float>(displayWidth) / static_cast<float>(renderWidth), static_cast<float>(displayHeight) / static_cast<float>(renderHeight)));
    toggleButton.setSize(sf::Vector2f(90.0f, 30.0f));
    toggleButton.setPosition(sf::Vector2f(10.0f, static_cast<float>(displayHeight) - 40.0f));
    toggleButton.setFillColor(sf::Color(50, 50, 50));
    toggleButton.setOutlineThickness(1.0f);
    toggleButton.setOutlineColor(sf::Color::White);
    degreeLabel.setFillColor(sf::Color::White);
    degreeLabel.setPosition(sf::Vector2f(18.0f, static_cast<float>(displayHeight) - 37.0f));
}

void NewtonExplorer::setPalette(sf::Color baseColor)
{
    palette = makePalette(baseColor);
    needsRedraw = true;
}

void NewtonExplorer::requestRedraw()
{
    needsRedraw = true;
}

void NewtonExplorer::handleMouseClick(const sf::Vector2i& mousePosition)
{
    const sf::Vector2f point(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y));
    if (toggleButton.getGlobalBounds().contains(point))
    {
        degree = (degree >= 6) ? 3 : degree + 1;
        degreeLabel.setString("n = " + std::to_string(degree));
        needsRedraw = true;
    }
}

sf::Color NewtonExplorer::colorForRoot(int rootIndex, int iterations) const
{
    const sf::Color base = palette[rootIndex % kPaletteSize];
    const float shade = std::max(0.35f, 1.0f - static_cast<float>(iterations) * 0.02f);
    return sf::Color(
        static_cast<std::uint8_t>(base.r * shade),
        static_cast<std::uint8_t>(base.g * shade),
        static_cast<std::uint8_t>(base.b * shade));
}

// Color each pixel by which root Newton's method converges to.
void NewtonExplorer::render()
{
    const double aspectRatio = static_cast<double>(renderHeight) / static_cast<double>(renderWidth);
    const double viewWidth = 3.2;
    const double viewHeight = viewWidth * aspectRatio;
    std::vector<std::complex<double>> roots(static_cast<std::size_t>(degree));
    for (int rootIndex = 0; rootIndex < degree; ++rootIndex)
    {
        const double angle = 2.0 * M_PI * static_cast<double>(rootIndex) / static_cast<double>(degree);
        roots[static_cast<std::size_t>(rootIndex)] = { std::cos(angle), std::sin(angle) };
    }

    for (unsigned int y = 0; y < renderHeight; ++y)
    {
        for (unsigned int x = 0; x < renderWidth; ++x)
        {
            const double real = (static_cast<double>(x) / renderWidth - 0.5) * viewWidth;
            const double imag = (0.5 - static_cast<double>(y) / renderHeight) * viewHeight;
            std::complex<double> z(real, imag);
            int iterations = 0;
            int rootIndex = 0;
            bool converged = false;
            for (; iterations < maxIterations; ++iterations)
            {
                z = newtonStep(z, degree);
                for (int candidate = 0; candidate < degree; ++candidate)
                {
                    if (std::abs(z - roots[static_cast<std::size_t>(candidate)]) < convergenceRadius)
                    {
                        rootIndex = candidate;
                        converged = true;
                        break;
                    }
                }

                if (converged)
                    break;
            }

            if (!converged)
                image.setPixel(sf::Vector2u(x, y), sf::Color(20, 20, 30));
            else
                image.setPixel(sf::Vector2u(x, y), colorForRoot(rootIndex, iterations));
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
    if (needsRedraw)
        render();

    window.draw(sprite);
    window.draw(toggleButton);
    window.draw(degreeLabel);
}

#include "Mandelbrot.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr double pi = 3.14159265358979323846;
constexpr double focusPoints[][2] = {
    {-0.743643887037151, 0.13182590420533},
    {-0.101096, 0.956286},
    {-0.088, 0.654}
};
constexpr int focusPointCount = sizeof(focusPoints) / sizeof(focusPoints[0]);
}

MandelbrotExplorer::MandelbrotExplorer(unsigned int width, unsigned int height)
    : displayWidth(width)
    , displayHeight(height)
    , width(std::min(width, 640u))
    , height(std::max(1u, static_cast<unsigned int>(
        static_cast<double>(std::min(width, 800u)) * height / width)))
    , image(sf::Vector2u(this->width, this->height), sf::Color::Black)
    , texture(sf::Vector2u(this->width, this->height))
    , sprite(texture) {
    texture.setSmooth(true);
    sprite.setScale(sf::Vector2f(
        static_cast<float>(displayWidth) / this->width,
        static_cast<float>(displayHeight) / this->height
    ));
    reset();
}

void MandelbrotExplorer::reset(int iterationLevel) {
    centerX = -0.5;
    centerY = 0.0;
    viewWidth = 3.5;
    manualViewWidth = 3.5;
    rotation = 0.0;
    targetRotation = 0.0;
    animationTime = 0.0;
    focusCycle = -1;
    focusStartX = centerX;
    focusStartY = centerY;
    maxIterations = std::clamp(iterationLevel * 16, 64, 160);
    colorOffset = 0;
    hasMousePosition = false;
    isDragging = false;
    rotationClock.restart();
    needsRedraw = true;
}

void MandelbrotExplorer::handleMouseClick(sf::Mouse::Button button, const sf::Vector2i& mousePosition) {
    if (button == sf::Mouse::Button::Left) {
        lastMousePosition = mousePosition;
        hasMousePosition = true;
        isDragging = true;
        return;
    }

    if (button != sf::Mouse::Button::Right) {
        return;
    }

    const double oldViewWidth = viewWidth;
    const double zoomFactor = button == sf::Mouse::Button::Left ? 0.35 : 1.0 / 0.35;
    const double offsetX = (static_cast<double>(mousePosition.x) / displayWidth - 0.5) * oldViewWidth;
    const double offsetY = (0.5 - static_cast<double>(mousePosition.y) / displayHeight) * oldViewWidth * displayHeight / displayWidth;
    const double cosine = std::cos(rotation);
    const double sine = std::sin(rotation);
    const double clickedReal = centerX + offsetX * cosine - offsetY * sine;
    const double clickedImaginary = centerY + offsetX * sine + offsetY * cosine;

    manualViewWidth *= zoomFactor;
    viewWidth = manualViewWidth;
    targetRotation += button == sf::Mouse::Button::Left ? pi / 8.0 : -pi / 8.0;
    const double newOffsetX = offsetX * viewWidth / oldViewWidth;
    const double newOffsetY = offsetY * viewWidth / oldViewWidth;
    centerX = clickedReal - (newOffsetX * cosine - newOffsetY * sine);
    centerY = clickedImaginary - (newOffsetX * sine + newOffsetY * cosine);
    needsRedraw = true;
}

void MandelbrotExplorer::handleMouseMove(const sf::Vector2i& mousePosition) {
    if (isDragging && hasMousePosition) {
        const double viewHeight = viewWidth * static_cast<double>(height) / width;
        const double screenDeltaX = static_cast<double>(mousePosition.x - lastMousePosition.x);
        const double screenDeltaY = static_cast<double>(mousePosition.y - lastMousePosition.y);
        const double localDeltaX = screenDeltaX * viewWidth / displayWidth;
        const double localDeltaY = -screenDeltaY * viewHeight / displayHeight;
        const double cosine = std::cos(rotation);
        const double sine = std::sin(rotation);
        const double worldDeltaX = localDeltaX * cosine - localDeltaY * sine;
        const double worldDeltaY = localDeltaX * sine + localDeltaY * cosine;

        // Move the complex-plane center opposite to the cursor delta so the
        // fractal follows the cursor on screen.
        centerX -= worldDeltaX;
        centerY -= worldDeltaY;
        needsRedraw = true;
    }

    lastMousePosition = mousePosition;
    hasMousePosition = true;
}

void MandelbrotExplorer::handleMouseRelease(sf::Mouse::Button button) {
    if (button == sf::Mouse::Button::Left) {
        isDragging = false;
    }
}

void MandelbrotExplorer::handleKeyPressed(sf::Keyboard::Key key) {
    const double zoomFactor = 0.35;
    const double rotationStep = pi / 5.0;

    if (key == sf::Keyboard::Key::Up) {
        manualViewWidth *= zoomFactor;
        viewWidth = manualViewWidth;
        targetRotation += rotationStep;
        needsRedraw = true;
    } else if (key == sf::Keyboard::Key::Down) {
        manualViewWidth /= zoomFactor;
        viewWidth = manualViewWidth;
        targetRotation -= rotationStep;
        needsRedraw = true;
    } else if (key == sf::Keyboard::Key::Left) {
        targetRotation -= rotationStep;
        needsRedraw = true;
    } else if (key == sf::Keyboard::Key::Right) {
        targetRotation += rotationStep;
        needsRedraw = true;
    }
}

void MandelbrotExplorer::update(bool automaticAnimation) {
    const double elapsedSeconds = rotationClock.restart().asSeconds();
    if (automaticAnimation) {
        animationTime += elapsedSeconds;
        constexpr double zoomCycleSeconds = 18.0;
        const double cycleProgress = std::fmod(animationTime, zoomCycleSeconds) / zoomCycleSeconds;
        const double zoomProgress = cycleProgress <= 0.5
            ? cycleProgress * 2.0
            : (1.0 - cycleProgress) * 2.0;
        const double deepZoomFactor = std::pow(10.0, -10.0 * zoomProgress);
        viewWidth = manualViewWidth * deepZoomFactor;

        const int currentCycle = static_cast<int>(animationTime / zoomCycleSeconds);
        if (currentCycle != focusCycle) {
            focusCycle = currentCycle;
            focusStartX = centerX;
            focusStartY = centerY;
        }

        const int focusIndex = currentCycle % focusPointCount;
        const double transition = std::clamp(cycleProgress / 0.18, 0.0, 1.0);
        const double smoothTransition = transition * transition * (3.0 - 2.0 * transition);
        centerX = focusStartX + (focusPoints[focusIndex][0] - focusStartX) * smoothTransition;
        centerY = focusStartY + (focusPoints[focusIndex][1] - focusStartY) * smoothTransition;
        const double animatedTargetRotation = targetRotation + animationTime * 0.35;
        const double difference = animatedTargetRotation - rotation;
        const double maximumStep = elapsedSeconds * 3.5;

        if (std::abs(difference) > 0.0001) {
            rotation += std::clamp(difference, -maximumStep, maximumStep);
            needsRedraw = true;
        } else {
            rotation = animatedTargetRotation;
        }
    }
}

sf::Color MandelbrotExplorer::colorFor(double smoothIterations) const {
    if (smoothIterations >= maxIterations) {
        return sf::Color::Black;
    }

    const double hue = std::fmod(
        smoothIterations * 18.0 + static_cast<double>(colorOffset),
        360.0
    );
    const double saturation = 0.85;
    const double value = 0.95;
    const double sector = hue / 60.0;
    const int sectorIndex = static_cast<int>(sector) % 6;
    const double fraction = sector - std::floor(sector);
    const double p = value * (1.0 - saturation);
    const double q = value * (1.0 - saturation * fraction);
    const double t = value * (1.0 - saturation * (1.0 - fraction));

    double red = 0.0;
    double green = 0.0;
    double blue = 0.0;
    switch (sectorIndex) {
    case 0: red = value; green = t; blue = p; break;
    case 1: red = q; green = value; blue = p; break;
    case 2: red = p; green = value; blue = t; break;
    case 3: red = p; green = q; blue = value; break;
    case 4: red = t; green = p; blue = value; break;
    default: red = value; green = p; blue = q; break;
    }

    return sf::Color(
        static_cast<std::uint8_t>(red * 255.0),
        static_cast<std::uint8_t>(green * 255.0),
        static_cast<std::uint8_t>(blue * 255.0)
    );
}

void MandelbrotExplorer::render() {
    const double aspectRatio = static_cast<double>(height) / width;
    const double viewHeight = viewWidth * aspectRatio;
    const double cosine = std::cos(rotation);
    const double sine = std::sin(rotation);

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            // Map the pixel to the complex plane. The vertical coordinate is
            // inverted because screen Y grows down while complex Y grows up.
            const double offsetX =
                (static_cast<double>(x) / width - 0.5) * viewWidth;
            const double offsetY =
                (0.5 - static_cast<double>(y) / height) * viewHeight;
            const double real = centerX + offsetX * cosine - offsetY * sine;
            const double imaginary = centerY + offsetX * sine + offsetY * cosine;

            double zReal = 0.0;
            double zImaginary = 0.0;
            int iteration = 0;
            double magnitudeSquared = 0.0;

            while (magnitudeSquared <= 4.0 && iteration < maxIterations) {
                const double nextReal = zReal * zReal - zImaginary * zImaginary + real;
                zImaginary = 2.0 * zReal * zImaginary + imaginary;
                zReal = nextReal;
                magnitudeSquared = zReal * zReal + zImaginary * zImaginary;
                ++iteration;
            }

            double smoothIterations = static_cast<double>(iteration);
            if (iteration < maxIterations) {
                smoothIterations -= std::log2(std::log2(std::sqrt(magnitudeSquared)));
            }

            image.setPixel(sf::Vector2u(x, y), colorFor(smoothIterations));
        }
    }

    if (!texture.loadFromImage(image)) {
        needsRedraw = true;
        return;
    }
    needsRedraw = false;
}

void MandelbrotExplorer::draw(sf::RenderWindow& window) {
    if (needsRedraw) {
        render();
    }

    window.draw(sprite);
}
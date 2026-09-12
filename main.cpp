#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <vector>
#include "Tab.h"
#include "Fractals.h"
#include "Mandelbrot.h"
#include "Settings.h"
#include "SettingsPanel.h"
#include <iostream>
#include <variant>

int main() {
    // Load settings
    Settings settings;
    
    // 1. Inicjalizacja okna i zasobów
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(settings.windowWidth, settings.windowHeight)), "Fractal Explorer");
    window.setFramerateLimit(settings.framerateLimitFPS);

    // Create settings panel
    SettingsPanel settingsPanel(settings);

    std::vector<Tab> tabs;
    tabs.emplace_back("Koch Snowflake", 0, settings.font, FractalType::Koch, settings.tabInactiveColor);
    tabs.emplace_back("Sierpinski", 151, settings.font, FractalType::Sierpinski, settings.tabInactiveColor);
    tabs.emplace_back("Fractal Tree", 302, settings.font, FractalType::Tree, settings.tabInactiveColor);
    tabs.emplace_back("Mandelbrot", 453, settings.font, FractalType::Mandelbrot, settings.tabInactiveColor);

    FractalType currentFractal = FractalType::Koch;
    std::vector<sf::Vertex> punkty;
    sf::View kamera = window.getDefaultView();
    sf::Color previousFractalColor = settings.fractalColor;
    MandelbrotExplorer mandelbrot(settings.windowWidth, settings.windowHeight);
    std::vector<KochSegment> kochSegments;
    int kochStage = 0;
    std::vector<Triangle> sierpinskiTriangles;
    int sierpinskiStage = 0;
    int treeStage = 0;
    sf::Clock kochAnimationClock;
    bool isAnimationPaused = false;
    constexpr int constructionSteps = 10;

    auto rebuildConstruction = [&](int targetStage) {
        punkty.clear();

        if (currentFractal == FractalType::Koch) {
            kochSegments = {{settings.koch_p1, settings.koch_p2}};
            kochStage = 0;
            targetStage = std::clamp(targetStage, 0, constructionSteps);
            while (kochStage < targetStage) {
                kochSegments = expandKocha(kochSegments);
                ++kochStage;
            }
            segmentsToVertices(kochSegments, punkty, settings.fractalColor);
        } else if (currentFractal == FractalType::Sierpinski) {
            sierpinskiTriangles = {{settings.sierpinski_p1, settings.sierpinski_p2, settings.sierpinski_p3}};
            sierpinskiStage = 0;
            targetStage = std::clamp(targetStage, 0, constructionSteps);
            while (sierpinskiStage < targetStage) {
                sierpinskiTriangles = expandSierpinski(sierpinskiTriangles);
                ++sierpinskiStage;
            }
            trianglesToVertices(sierpinskiTriangles, punkty, settings.fractalColor);
        } else if (currentFractal == FractalType::Tree) {
            treeStage = std::clamp(targetStage, 1, constructionSteps);
            generujTree(treeStage, settings.tree_start, settings.tree_initial_length, 90.0f, punkty, settings.fractalColor, settings.tree_length_factor, settings.tree_angle_left, settings.tree_angle_right);
        } else if (currentFractal == FractalType::Mandelbrot) {
            mandelbrot.reset(10);
        }
    };

    auto generateFractal = [&]() {
        kochAnimationClock.restart();
        rebuildConstruction(0);
    };

    auto advanceConstruction = [&]() {
        if (currentFractal == FractalType::Koch && kochStage < constructionSteps) {
            rebuildConstruction(kochStage + 1);
            return true;
        }

        if (currentFractal == FractalType::Sierpinski && sierpinskiStage < constructionSteps) {
            rebuildConstruction(sierpinskiStage + 1);
            return true;
        }

        if (currentFractal == FractalType::Tree && treeStage < constructionSteps) {
            rebuildConstruction(treeStage + 1);
            return true;
        }

        return false;
    };

    auto retreatConstruction = [&]() {
        if (currentFractal == FractalType::Koch && kochStage > 0) {
            rebuildConstruction(kochStage - 1);
            return true;
        }
        if (currentFractal == FractalType::Sierpinski && sierpinskiStage > 0) {
            rebuildConstruction(sierpinskiStage - 1);
            return true;
        }
        if (currentFractal == FractalType::Tree && treeStage > 1) {
            rebuildConstruction(treeStage - 1);
            return true;
        }
        return false;
    };

    auto drawThickLines = [&](const std::vector<sf::Vertex>& vertices, float lineThickness) {
        constexpr float radiansToDegrees = 180.0f / M_PI;

        for (std::size_t i = 0; i + 1 < vertices.size(); i += 2) {
            const sf::Vector2f start = vertices[i].position;
            const sf::Vector2f end = vertices[i + 1].position;
            const sf::Vector2f delta = end - start;
            const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            if (length <= 0.0f) {
                continue;
            }

            sf::RectangleShape line(sf::Vector2f(length, lineThickness));
            line.setPosition(start);
            line.setOrigin(sf::Vector2f(0.0f, lineThickness * 0.3));
            line.setRotation(sf::degrees(std::atan2(delta.y, delta.x) * radiansToDegrees));
            line.setFillColor(vertices[i].color);
            window.draw(line);
        }
    };

    generateFractal();

    while (window.isOpen()) {
        while (auto eventOpt = window.pollEvent()) {
            auto event = *eventOpt;
            
            if (event.getIf<sf::Event::Closed>()) {
                window.close();
            }

            if (auto mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
                const sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                if (currentFractal == FractalType::Mandelbrot) {
                    mandelbrot.handleMouseClick(mouseEvent->button, mousePos);
                }

                if (mouseEvent->button == sf::Mouse::Button::Left) {

                    // Check settings panel button
                    settingsPanel.handleMousePress(*mouseEvent, settings, window);

                    // Check tab clicks
                    for (auto& tab : tabs) {
                        if (tab.shape.getGlobalBounds().contains(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))) {
                            if (currentFractal != tab.type) {
                                currentFractal = tab.type;
                                generateFractal();
                            }
                        }
                    }
                }
            }

            if (auto keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Space) {
                    isAnimationPaused = !isAnimationPaused;
                    kochAnimationClock.restart();
                } else if (keyEvent->code == sf::Keyboard::Key::Right && isAnimationPaused && currentFractal != FractalType::Mandelbrot) {
                    advanceConstruction();
                } else if (keyEvent->code == sf::Keyboard::Key::Left && isAnimationPaused && currentFractal != FractalType::Mandelbrot) {
                    retreatConstruction();
                } else if (currentFractal == FractalType::Mandelbrot) {
                    mandelbrot.handleKeyPressed(keyEvent->code);
                }
            }

            if (auto mouseMove = event.getIf<sf::Event::MouseMoved>()) {
                const sf::Vector2i mousePosition(mouseMove->position.x, mouseMove->position.y);
                settingsPanel.handleMouseMove(mousePosition, settings);
                if (currentFractal == FractalType::Mandelbrot) {
                    mandelbrot.handleMouseMove(mousePosition);
                }
            }

            if (auto mouseRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
                settingsPanel.handleMouseRelease(*mouseRelease, settings);
                if (currentFractal == FractalType::Mandelbrot) {
                    mandelbrot.handleMouseRelease(mouseRelease->button);
                }
            }

            if (auto mouseEvent = event.getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    settingsPanel.handleMouseRelease(*mouseEvent, settings);
                }
            }

            if (auto mouseEvent = event.getIf<sf::Event::MouseMoved>()) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                settingsPanel.handleMouseMove(mousePos, settings);
            }
        }

        if (currentFractal == FractalType::Mandelbrot && !isAnimationPaused) {
            mandelbrot.update();
        }

        const float stageDelay = std::max(0.1f, 0.8f / settings.drawSpeed);
        if (!isAnimationPaused &&
            currentFractal != FractalType::Mandelbrot &&
            kochAnimationClock.getElapsedTime().asSeconds() >= stageDelay) {
            const bool advanced = advanceConstruction();
            if (advanced) {
                kochAnimationClock.restart();
            }
        }

        // Update settings panel
        settingsPanel.update(settings);

        // Update fractal color if changed
        if (settings.fractalColor != previousFractalColor) {
            for (auto& v : punkty) {
                v.color = settings.fractalColor;
            }
            previousFractalColor = settings.fractalColor;
        }

        window.clear(settings.backgroundColor);

        // 6. RYSOWANIE FRAKTALA (z Twoją kamerą)
        if (currentFractal == FractalType::Mandelbrot) {
            window.setView(window.getDefaultView());
            mandelbrot.draw(window);
        } else if (punkty.size() >= 2) {
            std::vector<sf::Vertex> pointsToDraw;
            const std::vector<sf::Vertex>* renderPoints = &punkty;
            // Obliczanie granic aktualnie narysowanej części
            sf::Vector2f minP = (*renderPoints)[0].position;
            sf::Vector2f maxP = (*renderPoints)[0].position;

                for (std::size_t i = 0; i < punkty.size(); ++i) {
                    sf::Vector2f p = (*renderPoints)[i].position;
                    if (p.x < minP.x) minP.x = p.x;
                    if (p.y < minP.y) minP.y = p.y;
                    if (p.x > maxP.x) maxP.x = p.x;
                    if (p.y > maxP.y) maxP.y = p.y;
                }

                float rysSzer = std::max(50.0f, maxP.x - minP.x);
                float rysWys = std::max(50.0f, maxP.y - minP.y);
                float proporcjeOkna = static_cast<float>(settings.windowWidth) / settings.windowHeight;
                float proporcjeRysunku = rysSzer / rysWys;

                float viewW, viewH;
                if (proporcjeRysunku > proporcjeOkna) {
                    viewW = rysSzer;
                    viewH = rysSzer / proporcjeOkna;
                }
                else {
                    viewH = rysWys;
                    viewW = rysWys * proporcjeOkna;
                }

                kamera.setSize(sf::Vector2f(viewW * settings.cameraZoom, viewH * settings.cameraZoom));
                kamera.setCenter((minP + maxP) / 2.0f);

            window.setView(kamera);

            const int currentStage = currentFractal == FractalType::Koch
                ? kochStage
                : currentFractal == FractalType::Sierpinski
                    ? sierpinskiStage
                    : treeStage;
                const float lineThickness = std::max(
                    0.8f,
                    5.0f - static_cast<float>(currentStage) * 0.43f
            );
            drawThickLines(*renderPoints, lineThickness);
        }

        // Update settings panel sliders
        settingsPanel.update(settings);

        // 7. RYSOWANIE INTERFEJSU (na widoku domyślnym)
        window.setView(window.getDefaultView());
        for (auto& tab : tabs) {
            tab.draw(window, currentFractal, settings.fractalColor);
        }

        // Draw settings panel and button
        settingsPanel.draw(window, settings);

        window.display();
    }

    return 0;
}
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>
#include <variant>

#include "Tab.h"
#include "Fractals.h"
#include "Mandelbrot.h"
#include "Settings.h"
#include "SettingsPanel.h"

int main() {
    Settings settings;
    
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(settings.windowWidth, settings.windowHeight)), "Fractal Explorer");
    window.setFramerateLimit(settings.framerateLimitFPS);

    SettingsPanel settingsPanel(settings);

    std::vector<Tab> tabs;
    tabs.emplace_back("Koch Curve", 0, settings.font, FractalType::Koch, settings.tabInactiveColor);
    tabs.emplace_back("Sierpinski", 100, settings.font, FractalType::Sierpinski, settings.tabInactiveColor);
    tabs.emplace_back("Carpet", 200, settings.font, FractalType::SierpinskiCarpet, settings.tabInactiveColor);
    tabs.emplace_back("Pentagon", 300, settings.font, FractalType::SierpinskiPentagon, settings.tabInactiveColor);
    tabs.emplace_back("Tree", 400, settings.font, FractalType::Tree, settings.tabInactiveColor);
    tabs.emplace_back("Fern", 500, settings.font, FractalType::BarnsleyFern, settings.tabInactiveColor);
    tabs.emplace_back("Pythagoras", 600, settings.font, FractalType::Pythagoras, settings.tabInactiveColor);
    tabs.emplace_back("Vicsek", 700, settings.font, FractalType::Vicsek, settings.tabInactiveColor);
    tabs.emplace_back("Hilbert", 800, settings.font, FractalType::Hilbert, settings.tabInactiveColor);
    tabs.emplace_back("Mandelbrot", 900, settings.font, FractalType::Mandelbrot, settings.tabInactiveColor);

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
    float treeBranchAngle = settings.tree_angle_left;
    float treeAngleDirection = 1.0f;
    int additionalStage = 0;
    
    std::vector<PythagorasSquare> pythagorasSquares;
    std::vector<PythagorasSquare> pythagorasFrontier;
    int pythagorasStage = 0;
    int pythagorasDepth = 4;
    float pythagorasAngle = settings.pythagoras_angle;
    float pythagorasAngleDirection = 1.0f;
    
    sf::Clock kochAnimationClock;
    sf::Clock pythagorasAngleClock;
    sf::Clock treeAngleClock;
    bool isAnimationPaused = false;
    constexpr int constructionSteps = 10;

    auto gradientColor = [](sf::Color baseColor, float amount) {
        constexpr float hueShift = 180.0f;
        amount = std::clamp(amount, 0.0f, 1.0f);
        const float red = baseColor.r / 255.0f;
        const float green = baseColor.g / 255.0f;
        const float blue = baseColor.b / 255.0f;
        const float maximum = std::max({red, green, blue});
        const float minimum = std::min({red, green, blue});
        const float difference = maximum - minimum;
        float hue = 0.0f;

        if (difference > 0.0f) {
            if (maximum == red) {
                hue = 60.0f * std::fmod((green - blue) / difference, 6.0f);
            } else if (maximum == green) {
                hue = 60.0f * ((blue - red) / difference + 2.0f);
            } else {
                hue = 60.0f * ((red - green) / difference + 4.0f);
            }
        }
        if (hue < 0.0f) {
            hue += 360.0f;
        }

        hue = std::fmod(hue + hueShift, 360.0f);
        const float saturation = maximum == 0.0f ? 0.0f : difference / maximum;
        const float value = maximum;
        const float hueSector = hue / 60.0f;
        const int sector = static_cast<int>(hueSector);
        const float chroma = value * saturation;
        const float second = chroma * (1.0f - std::abs(std::fmod(hueSector, 2.0f) - 1.0f));
        const float match = value - chroma;
        const float shiftedRed = (sector == 0 ? chroma : sector == 1 ? second : sector == 4 ? second : sector == 5 ? chroma : 0.0f) + match;
        const float shiftedGreen = (sector == 0 ? second : sector == 1 || sector == 2 ? chroma : sector == 3 ? second : 0.0f) + match;
        const float shiftedBlue = (sector == 2 ? second : sector == 3 || sector == 4 ? chroma : sector == 5 ? second : match * 0.0f) + match;
        const auto blendChannel = [amount](float original, float shifted) {
            return static_cast<std::uint8_t>((original + (shifted - original) * amount) * 255.0f);
        };

        return sf::Color(
            blendChannel(red, shiftedRed),
            blendChannel(green, shiftedGreen),
            blendChannel(blue, shiftedBlue),
            baseColor.a
        );
    };

    auto maxConstructionStage = [&]() {
        if (currentFractal == FractalType::SierpinskiPentagon) return 5;
        if (currentFractal == FractalType::SierpinskiCarpet || currentFractal == FractalType::Vicsek) return 6;
        if (currentFractal == FractalType::Hilbert) return 7;
        if (currentFractal == FractalType::BarnsleyFern) return constructionSteps;
        if (currentFractal == FractalType::Pythagoras) return pythagorasDepth;
        return constructionSteps;
    };

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
            segmentsToVertices(kochSegments, punkty, gradientColor(settings.fractalColor, static_cast<float>(kochStage) / constructionSteps));
        } else if (currentFractal == FractalType::Sierpinski) {
            sierpinskiTriangles = {{settings.sierpinski_p1, settings.sierpinski_p2, settings.sierpinski_p3}};
            sierpinskiStage = 0;
            targetStage = std::clamp(targetStage, 0, constructionSteps);
            while (sierpinskiStage < targetStage) {
                sierpinskiTriangles = expandSierpinski(sierpinskiTriangles);
                ++sierpinskiStage;
            }
            trianglesToVertices(sierpinskiTriangles, punkty, gradientColor(settings.fractalColor, static_cast<float>(sierpinskiStage) / constructionSteps));
        } else if (currentFractal == FractalType::Tree) {
            treeStage = std::clamp(targetStage, 1, constructionSteps);
            generujTree(treeStage, settings.tree_start, settings.tree_initial_length, 90.0f, punkty, gradientColor(settings.fractalColor, static_cast<float>(treeStage) / constructionSteps), settings.tree_length_factor, treeBranchAngle, -treeBranchAngle);
        } else if (currentFractal == FractalType::BarnsleyFern) {
            additionalStage = std::clamp(targetStage, 0, maxConstructionStage());
            generateBarnsleyFern(additionalStage, punkty, gradientColor(settings.fractalColor, static_cast<float>(additionalStage) / constructionSteps), settings.windowWidth, settings.windowHeight);
        } else if (currentFractal == FractalType::SierpinskiCarpet) {
            additionalStage = std::clamp(targetStage, 0, maxConstructionStage());
            generateSierpinskiCarpet(additionalStage, punkty, gradientColor(settings.fractalColor, static_cast<float>(additionalStage) / constructionSteps), settings.windowWidth, settings.windowHeight);
        } else if (currentFractal == FractalType::SierpinskiPentagon) {
            additionalStage = std::clamp(targetStage, 0, maxConstructionStage());
            generateSierpinskiPentagon(additionalStage, punkty, gradientColor(settings.fractalColor, static_cast<float>(additionalStage) / constructionSteps), settings.windowWidth, settings.windowHeight);
        } else if (currentFractal == FractalType::Vicsek) {
            additionalStage = std::clamp(targetStage, 0, maxConstructionStage());
            generateVicsek(additionalStage, punkty, gradientColor(settings.fractalColor, static_cast<float>(additionalStage) / constructionSteps), settings.windowWidth, settings.windowHeight);
        } else if (currentFractal == FractalType::Hilbert) {
            additionalStage = std::clamp(targetStage, 0, maxConstructionStage());
            generateHilbertCurve(additionalStage, punkty, gradientColor(settings.fractalColor, static_cast<float>(additionalStage) / constructionSteps), settings.windowWidth, settings.windowHeight);
        } else if (currentFractal == FractalType::Pythagoras) {
            const float halfSize = settings.pythagoras_size * 0.5f;
            const float baseY = settings.windowHeight - 80.0f;
            const PythagorasSquare root = {
                {settings.windowWidth * 0.5f - halfSize, baseY},
                {settings.windowWidth * 0.5f + halfSize, baseY},
                {settings.windowWidth * 0.5f + halfSize, baseY - settings.pythagoras_size},
                {settings.windowWidth * 0.5f - halfSize, baseY - settings.pythagoras_size},
                0
            };
            pythagorasSquares = {root};
            pythagorasFrontier = {root};
            pythagorasStage = 0;
            targetStage = std::clamp(targetStage, settings.pythagoras_min_depth, pythagorasDepth);
            
            while (pythagorasStage < targetStage) {
                pythagorasFrontier = expandPythagoras(pythagorasFrontier, pythagorasAngle);
                pythagorasSquares.insert(
                    pythagorasSquares.end(),
                    pythagorasFrontier.begin(),
                    pythagorasFrontier.end()
                );
                ++pythagorasStage;
            }
        } else if (currentFractal == FractalType::Mandelbrot) {
            mandelbrot.reset(10);
        }
    };

    auto generateFractal = [&]() {
        kochAnimationClock.restart();
        treeAngleClock.restart();
        rebuildConstruction(currentFractal == FractalType::Pythagoras
            ? pythagorasDepth
            : currentFractal == FractalType::Tree ? constructionSteps : 0);
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
           if ((currentFractal == FractalType::BarnsleyFern ||
             currentFractal == FractalType::SierpinskiCarpet ||
             currentFractal == FractalType::SierpinskiPentagon ||
             currentFractal == FractalType::Vicsek ||
             currentFractal == FractalType::Hilbert) && additionalStage < maxConstructionStage()) {
            rebuildConstruction(additionalStage + 1);
            return true;
        }
        if (currentFractal == FractalType::Pythagoras && pythagorasStage < pythagorasDepth) {
            rebuildConstruction(pythagorasStage + 1);
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
        if ((currentFractal == FractalType::BarnsleyFern ||
             currentFractal == FractalType::SierpinskiCarpet ||
             currentFractal == FractalType::SierpinskiPentagon ||
             currentFractal == FractalType::Vicsek ||
             currentFractal == FractalType::Hilbert) && additionalStage > 0) {
            rebuildConstruction(additionalStage - 1);
            return true;
        }
        if (currentFractal == FractalType::Pythagoras && pythagorasStage > 0) {
            rebuildConstruction(pythagorasStage - 1);
            return true;
        }
        return false;
    };

    auto currentStage = [&]() {
        if (currentFractal == FractalType::Koch) return kochStage;
        if (currentFractal == FractalType::Sierpinski) return sierpinskiStage;
        if (currentFractal == FractalType::Tree) return treeStage;
        if (currentFractal == FractalType::Pythagoras) return pythagorasStage;
        return additionalStage;
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
            line.setOrigin(sf::Vector2f(0.0f, lineThickness * 0.3f));
            line.setRotation(sf::degrees(std::atan2(delta.y, delta.x) * radiansToDegrees));
            line.setFillColor(vertices[i].color);
            window.draw(line);
        }
    };

    auto drawPythagoras = [&]() {
        for (const PythagorasSquare& square : pythagorasSquares) {
            sf::ConvexShape rectangle;
            rectangle.setPointCount(4);
            rectangle.setPoint(0, square.bottomLeft);
            rectangle.setPoint(1, square.bottomRight);
            rectangle.setPoint(2, square.topRight);
            rectangle.setPoint(3, square.topLeft);
            
            const float gradientProgress = static_cast<float>(square.depth) /
                static_cast<float>(settings.pythagoras_max_depth);
            rectangle.setFillColor(gradientColor(settings.fractalColor, gradientProgress));
            
            rectangle.setOutlineThickness(0.0f);

            window.draw(rectangle);
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
                    if (currentFractal != FractalType::Mandelbrot) {
                        settingsPanel.handleMousePress(*mouseEvent, settings, window);
                    }

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
                    pythagorasAngleClock.restart();
                    treeAngleClock.restart();
                } 
                else if (currentFractal == FractalType::Pythagoras) {
                    if (keyEvent->code == sf::Keyboard::Key::Up) {
                        pythagorasAngle = std::min(settings.pythagoras_max_angle, pythagorasAngle + 2.5f);
                        rebuildConstruction(pythagorasDepth);
                    } else if (keyEvent->code == sf::Keyboard::Key::Down) {
                        pythagorasAngle = std::max(settings.pythagoras_min_angle, pythagorasAngle - 2.5f);
                        rebuildConstruction(pythagorasDepth);
                    } else if (keyEvent->code == sf::Keyboard::Key::Right) {
                        pythagorasDepth = std::min(settings.pythagoras_max_depth, pythagorasDepth + 1);
                        rebuildConstruction(pythagorasDepth);
                    } else if (keyEvent->code == sf::Keyboard::Key::Left) {
                        pythagorasDepth = std::max(settings.pythagoras_min_depth, pythagorasDepth - 1);
                        rebuildConstruction(pythagorasDepth);
                    }
                } else if (currentFractal == FractalType::Tree) {
                    if (keyEvent->code == sf::Keyboard::Key::Up) {
                        treeBranchAngle = std::min(settings.tree_max_angle, treeBranchAngle + 2.5f);
                        rebuildConstruction(treeStage);
                    } else if (keyEvent->code == sf::Keyboard::Key::Down) {
                        treeBranchAngle = std::max(settings.tree_min_angle, treeBranchAngle - 2.5f);
                        rebuildConstruction(treeStage);
                    } else if (keyEvent->code == sf::Keyboard::Key::Right) {
                        treeStage = std::min(constructionSteps, treeStage + 1);
                        rebuildConstruction(treeStage);
                    } else if (keyEvent->code == sf::Keyboard::Key::Left) {
                        treeStage = std::max(1, treeStage - 1);
                        rebuildConstruction(treeStage);
                    }
                } 
                else if (keyEvent->code == sf::Keyboard::Key::Right && isAnimationPaused && currentFractal != FractalType::Mandelbrot) {
                    advanceConstruction();
                } else if (keyEvent->code == sf::Keyboard::Key::Left && isAnimationPaused && currentFractal != FractalType::Mandelbrot) {
                    retreatConstruction();
                } else if (currentFractal == FractalType::Mandelbrot) {
                    mandelbrot.handleKeyPressed(keyEvent->code);
                }
            }

            if (auto mouseMove = event.getIf<sf::Event::MouseMoved>()) {
                const sf::Vector2i mousePosition(mouseMove->position.x, mouseMove->position.y);
                if (currentFractal != FractalType::Mandelbrot) {
                    settingsPanel.handleMouseMove(mousePosition, settings);
                } else {
                    mandelbrot.handleMouseMove(mousePosition);
                }
            }

            if (auto mouseRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
                if (currentFractal != FractalType::Mandelbrot) {
                    settingsPanel.handleMouseRelease(*mouseRelease, settings);
                } else {
                    mandelbrot.handleMouseRelease(mouseRelease->button);
                }
            }
        }

        if (currentFractal == FractalType::Mandelbrot) {
            mandelbrot.update(!isAnimationPaused);
        }

        if (currentFractal == FractalType::Pythagoras && !isAnimationPaused) {
            const float angleAnimationSpeed = settings.drawSpeed * 10.0f;
            const float elapsedSeconds = pythagorasAngleClock.restart().asSeconds();
            pythagorasAngle += pythagorasAngleDirection * angleAnimationSpeed * elapsedSeconds;

            if (pythagorasAngle >= settings.pythagoras_max_angle ||
                pythagorasAngle <= settings.pythagoras_min_angle) {
                pythagorasAngle = std::clamp(
                    pythagorasAngle,
                    settings.pythagoras_min_angle,
                    settings.pythagoras_max_angle
                );
                pythagorasAngleDirection *= -1.0f;
            }

            rebuildConstruction(pythagorasDepth);
        } else {
            pythagorasAngleClock.restart();
        }

        if (currentFractal == FractalType::Tree && !isAnimationPaused) {
            const float angleAnimationSpeed = settings.drawSpeed * 20.0f;
            const float elapsedSeconds = treeAngleClock.restart().asSeconds();
            treeBranchAngle += treeAngleDirection * angleAnimationSpeed * elapsedSeconds;
            if (treeBranchAngle >= settings.tree_max_angle ||
                treeBranchAngle <= settings.tree_min_angle) {
                treeBranchAngle = std::clamp(
                    treeBranchAngle,
                    settings.tree_min_angle,
                    settings.tree_max_angle
                );
                treeAngleDirection *= -1.0f;
            }
            rebuildConstruction(treeStage);
        } else {
            treeAngleClock.restart();
        }

        const float stageDelay = std::max(0.1f, 0.8f / settings.drawSpeed);
        if (!isAnimationPaused &&
            currentFractal != FractalType::Mandelbrot &&
            currentFractal != FractalType::Pythagoras &&
            currentFractal != FractalType::Tree &&
            kochAnimationClock.getElapsedTime().asSeconds() >= stageDelay) {
            const bool advanced = advanceConstruction();
            if (advanced) {
                kochAnimationClock.restart();
            }
        }

        if (currentFractal != FractalType::Mandelbrot) {
            settingsPanel.update(settings);
        }

        if (settings.fractalColor != previousFractalColor) {
            if (currentFractal == FractalType::Pythagoras) {
                rebuildConstruction(pythagorasStage);
            } else if (currentFractal != FractalType::Mandelbrot) {
                rebuildConstruction(currentStage());
            }
            previousFractalColor = settings.fractalColor;
        }

        window.clear(settings.backgroundColor);

        if (currentFractal == FractalType::Mandelbrot) {
            window.setView(window.getDefaultView());
            mandelbrot.draw(window);
        } else if ((currentFractal == FractalType::Sierpinski ||
                    currentFractal == FractalType::SierpinskiCarpet ||
                    currentFractal == FractalType::SierpinskiPentagon ||
                    currentFractal == FractalType::Vicsek) && punkty.size() >= 3) {
            window.setView(window.getDefaultView());
            window.draw(&punkty[0], static_cast<std::size_t>(punkty.size()), sf::PrimitiveType::Triangles);
        } else if (currentFractal == FractalType::BarnsleyFern && !punkty.empty()) {
            window.setView(window.getDefaultView());
            window.draw(&punkty[0], static_cast<std::size_t>(punkty.size()), sf::PrimitiveType::Points);
        } else if (currentFractal == FractalType::Pythagoras && !pythagorasSquares.empty()) {
            window.setView(window.getDefaultView());
            drawPythagoras();
        } else if (punkty.size() >= 2) {
            const std::vector<sf::Vertex>* renderPoints = &punkty;
            
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
            } else {
                viewH = rysWys;
                viewW = rysWys * proporcjeOkna;
            }

            kamera.setSize(sf::Vector2f(viewW * settings.cameraZoom, viewH * settings.cameraZoom));
            kamera.setCenter((minP + maxP) / 2.0f);

            window.setView(kamera);

            const int stage = currentStage();
            const float lineThickness = std::max(
                0.7f,
                5.0f - static_cast<float>(stage) * 0.58f
            );
            drawThickLines(*renderPoints, lineThickness);
        }

        window.setView(window.getDefaultView());
        for (auto& tab : tabs) {
            tab.draw(window, currentFractal, settings.fractalColor);
        }

        if (currentFractal != FractalType::Mandelbrot) {
            settingsPanel.draw(window, settings);
        }

        window.display();
    }

    return 0;
}
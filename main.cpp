#include <SFML/Graphics.hpp>
#include <vector>
#include "Tab.h"
#include "Fractals.h"
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

    FractalType currentFractal = FractalType::Koch;
    std::vector<sf::Vertex> punkty;
    float narysowane = 0;
    sf::View kamera = window.getDefaultView();
    sf::Color previousFractalColor = settings.fractalColor;

    generujKocha(settings.fractalIterations, settings.koch_p1, settings.koch_p2, punkty, settings.fractalColor);

    while (window.isOpen()) {
        while (auto eventOpt = window.pollEvent()) {
            auto event = *eventOpt;
            
            if (event.getIf<sf::Event::Closed>()) {
                window.close();
            }

            if (auto mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                    // Check settings panel button
                    settingsPanel.handleMousePress(*mouseEvent, settings, window);

                    // Check tab clicks
                    for (auto& tab : tabs) {
                        if (tab.shape.getGlobalBounds().contains(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))) {
                            if (currentFractal != tab.type) {
                                currentFractal = tab.type;
                                narysowane = 0;
                                punkty.clear();

                                if (currentFractal == FractalType::Koch) {
                                    generujKocha(settings.fractalIterations, settings.koch_p1, settings.koch_p2, punkty, settings.fractalColor);
                                } else if (currentFractal == FractalType::Sierpinski) {
                                    generujSierpinski(settings.fractalIterations, settings.sierpinski_p1, settings.sierpinski_p2, settings.sierpinski_p3, punkty, settings.fractalColor);
                                } else if (currentFractal == FractalType::Tree) {
                                    generujTree(settings.fractalIterations*2, settings.tree_start, settings.tree_initial_length, 90.0f, punkty, settings.fractalColor, settings.tree_length_factor, settings.tree_angle_left, settings.tree_angle_right);
                                }
                            }
                        }
                    }
                }
            }

            if (auto mouseMove = event.getIf<sf::Event::MouseMoved>()) {
                settingsPanel.handleMouseMove(sf::Vector2i(mouseMove->position.x, mouseMove->position.y), settings);
            }

            if (auto mouseRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
                settingsPanel.handleMouseRelease(*mouseRelease, settings);
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

        // 5. Aktualizacja animacji rysowania
        if (narysowane < punkty.size()) {
            narysowane += settings.drawSpeed;
            if (narysowane > punkty.size()) narysowane = static_cast<float>(punkty.size());
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
        if (narysowane > 2) {
            // Obliczanie granic aktualnie narysowanej części
            sf::Vector2f minP = punkty[0].position;
            sf::Vector2f maxP = punkty[0].position;

            for (int i = 0; i < (int)narysowane; ++i) {
                sf::Vector2f p = punkty[i].position;
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
            window.draw(&punkty[0], static_cast<int>(narysowane), sf::PrimitiveType::Lines);
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
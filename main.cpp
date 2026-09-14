#include <SFML/Graphics.hpp>

#include <array>

#include <vector>

#include "Tab.h"

#include "Settings.h"

#include "SettingsPanel.h"

#include "NewtonExplorer.h"

#include "FractalController.h"

namespace
{
    struct TabDefinition
    {
        const char* label;
        FractalType type;
    };
    constexpr std::array<TabDefinition, 10> kTabs = { {
        { "Koch Snowflake", FractalType::Koch },
        { "Sierpinski Triangle", FractalType::Sierpinski },
        { "Sierpinski Carpet", FractalType::SierpinskiCarpet },
        { "Sierpinski Pentagon", FractalType::SierpinskiPentagon },
        { "Fractal Tree", FractalType::Tree },
        { "Dragon Curve", FractalType::Dragon },
        { "Pythagoras Tree", FractalType::Pythagoras },
        { "Vicsek Fractal", FractalType::Vicsek },
        { "Hilbert Curve", FractalType::Hilbert },
        { "Newton Fractal", FractalType::Newton },
    } };
}

int main()
{

    Settings settings;
    sf::RenderWindow window(
        sf::VideoMode(sf::Vector2u(settings.windowWidth, settings.windowHeight)),
        "Fractal Explorer");

    SettingsPanel settingsPanel(settings);
    NewtonExplorer newton(settings.windowWidth, settings.windowHeight, settings.font);
    FractalController controller(newton, settings);
    sf::View camera = window.getDefaultView();

    const float tabWidth = settings.windowWidth / static_cast<float>(kTabs.size());
    std::vector<Tab> tabs;
    tabs.reserve(kTabs.size());

    for (size_t i = 0; i < kTabs.size(); ++i)
    {
        tabs.emplace_back(
            kTabs[i].label,
            tabWidth * static_cast<float>(i),
            tabWidth,
            settings.font,
            kTabs[i].type,
            settings.tabInactiveColor);
    }
    controller.generateFractal(settings);

    while (window.isOpen())
    {

        // --- Input ---
        while (auto eventOpt = window.pollEvent())
        {
            const sf::Event& event = *eventOpt;
            if (event.is<sf::Event::Closed>())
                window.close();

            if (auto mouseEvent = event.getIf<sf::Event::MouseButtonPressed>())
            {
                const sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (controller.currentType == FractalType::Newton)
                    newton.handleMouseClick(mousePos);

                // check tab click
                if (mouseEvent->button == sf::Mouse::Button::Left)
                {
                    if (!isExplorerMode(controller.currentType))
                        settingsPanel.handleMousePress(*mouseEvent, settings, window);

                    const sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
                    for (Tab& tab : tabs)
                    {
                        if (tab.shape.getGlobalBounds().contains(mousePosF) && controller.currentType != tab.type)
                        {
                            controller.currentType = tab.type;
                            controller.generateFractal(settings);
                        }
                    }
                }
            }

            if (auto keyEvent = event.getIf<sf::Event::KeyPressed>())
                controller.handleKeyPressed(*keyEvent, settings);

            if (auto mouseMove = event.getIf<sf::Event::MouseMoved>())
            {
                const sf::Vector2i mousePosition(mouseMove->position.x, mouseMove->position.y);
                if (!isExplorerMode(controller.currentType))
                    settingsPanel.handleMouseMove(mousePosition, settings);
            }

            if (auto mouseRelease = event.getIf<sf::Event::MouseButtonReleased>())
            {
                if (!isExplorerMode(controller.currentType))
                    settingsPanel.handleMouseRelease(*mouseRelease, settings);
            }
        }

        // --- Update ---
        controller.updateAnimations(settings);
        if (!isExplorerMode(controller.currentType))
        {
            settingsPanel.update(settings);
            controller.applyBaseColor(settings);
        }

        // --- Draw ---
        window.clear(settings.backgroundColor);
        controller.draw(window, settings, camera);
        window.setView(window.getDefaultView());

        for (Tab& tab : tabs)
            tab.draw(window, controller.currentType, settings.fractalColor);

        if (!isExplorerMode(controller.currentType))
            settingsPanel.draw(window, settings);

        window.display();
    }
    return 0;
}

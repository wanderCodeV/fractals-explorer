#include "SettingsPanel.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstdint>

using namespace sf;
using std::min;
using std::max;

SettingsPanel::SettingsPanel(const Settings& settings)
    : speedLabel(settings.font, "Speed", 12)
    , backgroundColorLabel(settings.font, "Background color", 12)
    , fractalColorLabel(settings.font, "Fractal color", 12)
{
    settingsButton.setRadius(settings.settingsButtonSize / 2.0f);
    settingsButton.setPosition(Vector2f(
        settings.windowWidth - settings.settingsButtonSize - 10,
        10
    ));
    settingsButton.setFillColor(settings.buttonColor);
    
    panel.setSize(Vector2f(settings.settingsPanelWidth, settings.settingsPanelHeight));
    panel.setPosition(Vector2f(settings.settingsPanelX, settings.settingsPanelY));
    panel.setFillColor(settings.settingsPanelColor);
    panel.setOutlineThickness(2);
    panel.setOutlineColor(Color::White);
    
    setupSlider(speedSliderTrack, speedSliderHandle, Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 60), Color(50, 50, 50), Color::Cyan);
    
    speedLabel.setPosition(Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 35));
    speedLabel.setFillColor(Color::White);

    // Background color label
    backgroundColorLabel.setPosition(Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 80));
    backgroundColorLabel.setFillColor(Color::White);
    
    // Background color sliders
    setupSlider(backgroundColorSliderTrackR, backgroundColorSliderHandleR, Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 105), Color(80, 20, 20), Color::Red);
    setupSlider(backgroundColorSliderTrackG, backgroundColorSliderHandleG, Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 125), Color(20, 80, 20), Color::Green);
    setupSlider(backgroundColorSliderTrackB, backgroundColorSliderHandleB, Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 145), Color(20, 20, 80), Color::Blue);

    // Fractal color label
    fractalColorLabel.setPosition(Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 180));
    fractalColorLabel.setFillColor(Color::White);
    
    // Fractal color sliders
    setupSlider(fractalColorSliderTrackR, fractalColorSliderHandleR, Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 205), Color(80, 20, 20), Color::Red);
    setupSlider(fractalColorSliderTrackG, fractalColorSliderHandleG, Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 225), Color(20, 80, 20), Color::Green);
    setupSlider(fractalColorSliderTrackB, fractalColorSliderHandleB, Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 245), Color(20, 20, 80), Color::Blue);

    update(settings);
}

void SettingsPanel::update(const Settings& settings) {
    float speedRatio = (settings.drawSpeed - speedMin) / (speedMax - speedMin);
    updateSliderHandle(speedSliderTrack, speedSliderHandle, speedRatio);

    float backgroundRedRatio = static_cast<float>(settings.backgroundColor.r) / 255.0f;
    float backgroundGreenRatio = static_cast<float>(settings.backgroundColor.g) / 255.0f;
    float backgroundBlueRatio = static_cast<float>(settings.backgroundColor.b) / 255.0f;

    updateSliderHandle(backgroundColorSliderTrackR, backgroundColorSliderHandleR, backgroundRedRatio);
    updateSliderHandle(backgroundColorSliderTrackG, backgroundColorSliderHandleG, backgroundGreenRatio);
    updateSliderHandle(backgroundColorSliderTrackB, backgroundColorSliderHandleB, backgroundBlueRatio);

    float fractalRedRatio = static_cast<float>(settings.fractalColor.r) / 255.0f;
    float fractalGreenRatio = static_cast<float>(settings.fractalColor.g) / 255.0f;
    float fractalBlueRatio = static_cast<float>(settings.fractalColor.b) / 255.0f;  

    updateSliderHandle(fractalColorSliderTrackR, fractalColorSliderHandleR, fractalRedRatio);
    updateSliderHandle(fractalColorSliderTrackG, fractalColorSliderHandleG, fractalGreenRatio);
    updateSliderHandle(fractalColorSliderTrackB, fractalColorSliderHandleB, fractalBlueRatio);
}

void SettingsPanel::updateSliderHandle(RectangleShape& track, CircleShape& handle, float ratio) {
    float clampedRatio = max(0.0f, min(ratio, 1.0f));
    float x = track.getPosition().x + clampedRatio * track.getSize().x - handle.getRadius();
    float y = track.getPosition().y + track.getSize().y * 0.5f - handle.getRadius();
    handle.setPosition(Vector2f(x, y));
}

void SettingsPanel::handleMousePress(const Event::MouseButtonPressed& mouseEvent, Settings& settings, RenderWindow& window) {
    Vector2i mousePosInt = Mouse::getPosition(window);
    Vector2f mousePos(static_cast<float>(mousePosInt.x), 
                         static_cast<float>(mousePosInt.y));
    
    if (settingsButton.getGlobalBounds().contains(mousePos)) {
        isOpen = !isOpen;
        return;
    }
    
    if (isOpen) {
        if (speedSliderHandle.getGlobalBounds().contains(mousePos)) {
            isDraggingSpeed = true;
        } else if (backgroundColorSliderHandleR.getGlobalBounds().contains(mousePos)) {
            isDraggingBackgroundColorR = true;
        } else if (backgroundColorSliderHandleG.getGlobalBounds().contains(mousePos)) {
            isDraggingBackgroundColorG = true;
        } else if (backgroundColorSliderHandleB.getGlobalBounds().contains(mousePos)) {
            isDraggingBackgroundColorB = true;
        } else if (fractalColorSliderHandleR.getGlobalBounds().contains(mousePos)) {
            isDraggingFractalColorR = true;
        } else if (fractalColorSliderHandleG.getGlobalBounds().contains(mousePos)) {
            isDraggingFractalColorG = true;
        } else if (fractalColorSliderHandleB.getGlobalBounds().contains(mousePos)) {
            isDraggingFractalColorB = true;
        }
    }
}

void SettingsPanel::handleMouseRelease(const Event::MouseButtonReleased& mouseEvent, Settings& settings) {
    isDraggingSpeed = false;
    isDraggingBackgroundColorR = false;
    isDraggingBackgroundColorG = false;
    isDraggingBackgroundColorB = false;
    isDraggingFractalColorR = false;
    isDraggingFractalColorG = false;
    isDraggingFractalColorB = false;
}

void SettingsPanel::handleMouseMove(const Vector2i& mousePos, Settings& settings) {
    float mouseX = static_cast<float>(mousePos.x);
    
    if (isDraggingSpeed && isOpen) {
        float trackStart = speedSliderTrack.getPosition().x;
        float trackEnd = trackStart + speedSliderTrack.getSize().x;
        mouseX = max(trackStart, min(mouseX, trackEnd));
        float ratio = (mouseX - trackStart) / speedSliderTrack.getSize().x;
        settings.drawSpeed = speedMin + ratio * (speedMax - speedMin);
    }
    
    if (isDraggingBackgroundColorR && isOpen) {
        float trackStart = backgroundColorSliderTrackR.getPosition().x;
        float trackEnd = trackStart + backgroundColorSliderTrackR.getSize().x;
        mouseX = max(trackStart, min(mouseX, trackEnd));
        float ratio = (mouseX - trackStart) / backgroundColorSliderTrackR.getSize().x;
        settings.backgroundColor.r = static_cast<std::uint8_t>(ratio * 255);
    }
    
    if (isDraggingBackgroundColorG && isOpen) {
        float trackStart = backgroundColorSliderTrackG.getPosition().x;
        float trackEnd = trackStart + backgroundColorSliderTrackG.getSize().x;
        mouseX = max(trackStart, min(mouseX, trackEnd));
        float ratio = (mouseX - trackStart) / backgroundColorSliderTrackG.getSize().x;
        settings.backgroundColor.g = static_cast<std::uint8_t>(ratio * 255);
    }
    
    if (isDraggingBackgroundColorB && isOpen) {
        float trackStart = backgroundColorSliderTrackB.getPosition().x;
        float trackEnd = trackStart + backgroundColorSliderTrackB.getSize().x;
        mouseX = max(trackStart, min(mouseX, trackEnd));
        float ratio = (mouseX - trackStart) / backgroundColorSliderTrackB.getSize().x;
        settings.backgroundColor.b = static_cast<std::uint8_t>(ratio * 255);
    }

    if (isDraggingFractalColorR && isOpen) {
        float trackStart = fractalColorSliderTrackR.getPosition().x;
        float trackEnd = trackStart + fractalColorSliderTrackR.getSize().x;
        mouseX = max(trackStart, min(mouseX, trackEnd));
        float ratio = (mouseX - trackStart) / fractalColorSliderTrackR.getSize().x;
        settings.fractalColor.r = static_cast<std::uint8_t>(ratio * 255);
    }

    if (isDraggingFractalColorG && isOpen) {
        float trackStart = fractalColorSliderTrackG.getPosition().x;
        float trackEnd = trackStart + fractalColorSliderTrackG.getSize().x;
        mouseX = max(trackStart, min(mouseX, trackEnd));
        float ratio = (mouseX - trackStart) / fractalColorSliderTrackG.getSize().x;
        settings.fractalColor.g = static_cast<std::uint8_t>(ratio * 255);
    }

    if (isDraggingFractalColorB && isOpen) {
        float trackStart = fractalColorSliderTrackB.getPosition().x;
        float trackEnd = trackStart + fractalColorSliderTrackB.getSize().x;
        mouseX = max(trackStart, min(mouseX, trackEnd));
        float ratio = (mouseX - trackStart) / fractalColorSliderTrackB.getSize().x;
        settings.fractalColor.b = static_cast<std::uint8_t>(ratio * 255);
    }
}

void SettingsPanel::drawSettingsButton(RenderWindow& window, const Settings& settings) {
    Vector2i mousePos = Mouse::getPosition(window);
    Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    
    if (settingsButton.getGlobalBounds().contains(mousePosF)) {
        settingsButton.setFillColor(settings.buttonHoverColor);
    } else {
        settingsButton.setFillColor(settings.buttonColor);
    }
    
    window.draw(settingsButton);
    
    CircleShape dot(2);
    dot.setFillColor(Color::White);
    dot.setPosition(Vector2f(
        settingsButton.getPosition().x + settings.settingsButtonSize / 2.0f - 2,
        settingsButton.getPosition().y + settings.settingsButtonSize / 2.0f - 2
    ));
    window.draw(dot);
}

void SettingsPanel::drawSettingsPanel(RenderWindow& window, const Settings& settings) {
    if (!isOpen) return;
    
    window.draw(panel);
    window.draw(speedLabel);
    window.draw(speedSliderTrack);
    window.draw(speedSliderHandle);
    
    // Background color section
    window.draw(backgroundColorLabel);
    window.draw(backgroundColorSliderTrackR);
    window.draw(backgroundColorSliderHandleR);
    window.draw(backgroundColorSliderTrackG);
    window.draw(backgroundColorSliderHandleG);
    window.draw(backgroundColorSliderTrackB);
    window.draw(backgroundColorSliderHandleB);

    // Fractal color section
    window.draw(fractalColorLabel);
    window.draw(fractalColorSliderTrackR);
    window.draw(fractalColorSliderHandleR);
    window.draw(fractalColorSliderTrackG);
    window.draw(fractalColorSliderHandleG);
    window.draw(fractalColorSliderTrackB);
    window.draw(fractalColorSliderHandleB);
}

void SettingsPanel::setupSlider(RectangleShape& track, CircleShape& handle, const Vector2f& position, const Color& trackColor, const Color& handleColor) {
    track.setSize(Vector2f(110, 5));
    track.setPosition(position);
    track.setFillColor(trackColor);
    handle.setRadius(7);
    handle.setFillColor(handleColor);
    handle.setPosition(Vector2f(position.x - 4.0f, position.y - 2.0f));
}

void SettingsPanel::draw(RenderWindow& window, const Settings& settings) {
    drawSettingsButton(window, settings);
    drawSettingsPanel(window, settings);
}

#include "SettingsPanel.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <array>

using namespace sf;
using std::max;
using std::min;

void Slider::setup(const Vector2f &position, const Color &trackColor, const Color &handleColor)
{
    track.setSize(Vector2f(110, 5));
    track.setPosition(position);
    track.setFillColor(trackColor);
    handle.setRadius(7);
    handle.setFillColor(handleColor);
    setRatio(0.0f);
}

void Slider::setRatio(float ratio)
{
    float clampedRatio = max(0.0f, min(ratio, 1.0f));
    float x = track.getPosition().x + clampedRatio * track.getSize().x - handle.getRadius();
    float y = track.getPosition().y + track.getSize().y * 0.5f - handle.getRadius();
    handle.setPosition(Vector2f(x, y));
}

float Slider::ratioFromMouseX(float mouseX) const
{
    float trackStart = track.getPosition().x;
    return max(0.0f, min((mouseX - trackStart) / track.getSize().x, 1.0f));
}

bool Slider::contains(const Vector2f &point) const
{
    return handle.getGlobalBounds().contains(point);
}

SliderBlock::SliderBlock(const Font &font, const std::string &text, const Vector2f &labelPosition)
    : label(font, text, 12)
{
    label.setPosition(labelPosition);
    label.setFillColor(Color::White);
}

Slider &SliderBlock::addSlider(const Vector2f &position, const Color &trackColor, const Color &handleColor,
                               float minValue, float maxValue)
{
    sliders.emplace_back();
    Slider &slider = sliders.back();
    slider.minValue = minValue;
    slider.maxValue = maxValue;
    slider.setup(position, trackColor, handleColor);
    return slider;
}

void SliderBlock::draw(RenderWindow &window) const
{
    window.draw(label);
    for (const Slider &slider : sliders)
    {
        window.draw(slider.track);
        window.draw(slider.handle);
    }
}

SettingsPanel::SettingsPanel(const Settings &settings)
    : speedBlock(settings.font, "Speed", Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 5)), backgroundColorBlock(settings.font, "Background color", Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 50)), fractalColorBlock(settings.font, "Fractal color", Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 130))
{
    settingsButton.setRadius(settings.settingsButtonSize / 2.0f);
    settingsButton.setPosition(Vector2f(
        settings.windowWidth - settings.settingsButtonSize - 10,
        settings.settingsPanelY + settings.settingsPanelHeight - settings.settingsButtonSize / 2.0f));
    settingsButton.setFillColor(settings.buttonColor);

    panel.setSize(Vector2f(settings.settingsPanelWidth, settings.settingsPanelHeight));
    panel.setPosition(Vector2f(settings.settingsPanelX, settings.settingsPanelY));
    panel.setFillColor(settings.settingsPanelColor);
    panel.setOutlineThickness(2);
    panel.setOutlineColor(Color::White);

    speedBlock.addSlider(Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + 30), Color(50, 50, 50), Color::Cyan, 0.1f, 2.0f);
    for (SliderBlock *block : {&backgroundColorBlock, &fractalColorBlock})
    {
        float offset = block == &backgroundColorBlock ? 80.0f : 160.0f;
        block->addSlider(Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + offset), Color(80, 20, 20), Color::Red);
        block->addSlider(Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + offset + 20), Color(20, 80, 20), Color::Green);
        block->addSlider(Vector2f(settings.settingsPanelX + 15, settings.settingsPanelY + offset + 40), Color(20, 20, 80), Color::Blue);
    }

    update(settings);
}

void SettingsPanel::update(const Settings &settings)
{
    Slider &speedSlider = speedBlock.sliders[0];
    speedSlider.setRatio((settings.drawSpeed - speedSlider.minValue) / (speedSlider.maxValue - speedSlider.minValue));

    updateColorSliders(backgroundColorBlock, settings.backgroundColor);
    updateColorSliders(fractalColorBlock, settings.fractalColor);
}

void SettingsPanel::updateColorSliders(SliderBlock &block, const Color &color)
{
    const std::array<std::uint8_t, 3> channels = {color.r, color.g, color.b};
    for (std::size_t i = 0; i < block.sliders.size() && i < channels.size(); ++i)
        block.sliders[i].setRatio(static_cast<float>(channels[i]) / 255.0f);
}

void SettingsPanel::forEachSlider(const std::function<void(Slider &)> &action)
{
    for (SliderBlock *block : {&speedBlock, &backgroundColorBlock, &fractalColorBlock})
        for (Slider &slider : block->sliders)
            action(slider);
}

void SettingsPanel::setColorChannel(Color &color, std::size_t channel, std::uint8_t value)
{
    if (channel == 0)
        color.r = value;
    else if (channel == 1)
        color.g = value;
    else if (channel == 2)
        color.b = value;
}

void SettingsPanel::handleMousePress(const Event::MouseButtonPressed &mouseEvent, Settings &settings, RenderWindow &window)
{
    const Vector2f mousePos(
        static_cast<float>(mouseEvent.position.x),
        static_cast<float>(mouseEvent.position.y));

    if (settingsButton.getGlobalBounds().contains(mousePos))
    {
        isOpen = !isOpen;
        return;
    }

    if (!isOpen)
        return;

    forEachSlider([&](Slider &slider)
                  {
        if (slider.contains(mousePos)) 
            slider.isDragging = true; });
}

void SettingsPanel::handleMouseRelease(const Event::MouseButtonReleased &mouseEvent, Settings &settings)
{
    forEachSlider([](Slider &slider)
                  { slider.isDragging = false; });
}

void SettingsPanel::handleMouseMove(const Vector2i &mousePos, Settings &settings)
{
    if (!isOpen)
        return;

    auto updateColor = [&](SliderBlock &block, Color &color)
    {
        for (std::size_t i = 0; i < block.sliders.size() && i < 3; ++i)
        {
            Slider &slider = block.sliders[i];
            if (!slider.isDragging)
                continue;
            const float ratio = slider.ratioFromMouseX(static_cast<float>(mousePos.x));
            slider.setRatio(ratio);
            setColorChannel(color, i, static_cast<std::uint8_t>(ratio * 255.0f));
        }
    };

    for (Slider &slider : speedBlock.sliders)
    {
        if (slider.isDragging)
        {
            float ratio = slider.ratioFromMouseX(static_cast<float>(mousePos.x));
            slider.setRatio(ratio);
            settings.drawSpeed = slider.minValue + ratio * (slider.maxValue - slider.minValue);
        }
    }

    updateColor(backgroundColorBlock, settings.backgroundColor);
    updateColor(fractalColorBlock, settings.fractalColor);
}

void SettingsPanel::drawSettingsButton(RenderWindow &window, const Settings &settings)
{
    Vector2i mousePos = Mouse::getPosition(window);
    Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

    if (settingsButton.getGlobalBounds().contains(mousePosF))
        settingsButton.setFillColor(settings.buttonHoverColor);
    else
        settingsButton.setFillColor(settings.buttonColor);

    window.draw(settingsButton);

    CircleShape dot(2);
    dot.setFillColor(Color::White);
    dot.setPosition(Vector2f(
        settingsButton.getPosition().x + settings.settingsButtonSize / 2.0f - 2,
        settingsButton.getPosition().y + settings.settingsButtonSize / 2.0f - 2));
    window.draw(dot);
}

void SettingsPanel::drawSettingsPanel(RenderWindow &window, const Settings &settings)
{
    if (!isOpen)
        return;

    window.draw(panel);

    speedBlock.draw(window);
    backgroundColorBlock.draw(window);
    fractalColorBlock.draw(window);
}

void SettingsPanel::draw(RenderWindow &window, const Settings &settings)
{
    drawSettingsButton(window, settings);
    drawSettingsPanel(window, settings);
}

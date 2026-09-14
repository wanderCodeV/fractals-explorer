#include "FractalController.h"

#include "RenderUtils.h"

#include <algorithm>

namespace
{
    bool isManualStageFractal(FractalType type)
    {
        return type == FractalType::Pythagoras || type == FractalType::Tree;
    }
}

FractalController::FractalController(NewtonExplorer& newtonExplorer, const Settings& settings)
    : newton(newtonExplorer), palette(makePalette(settings.fractalColor)), treeBranchAngle(settings.tree_angle_left), pythagorasAngle(settings.pythagoras_angle)
{
}

int FractalController::maxConstructionStage() const
{
    switch (currentType)
    {
    case FractalType::SierpinskiPentagon:
        return 5;
    case FractalType::SierpinskiCarpet:
    case FractalType::Vicsek:
        return 6;
    case FractalType::Hilbert:
        return 7;
    case FractalType::Dragon:
        return constructionSteps;
    case FractalType::Pythagoras:
        return pythagorasDepth;
    default:
        return constructionSteps;
    }
}

int FractalController::minConstructionStage() const
{
    return startsAtStageOne(currentType) ? 1 : 0;
}

int FractalController::currentStage() const
{
    switch (currentType)
    {
    case FractalType::Koch:
        return kochStage;
    case FractalType::Sierpinski:
        return sierpinskiStage;
    case FractalType::Tree:
        return treeStage;
    case FractalType::Pythagoras:
        return pythagorasStage;
    default:
        return additionalStage;
    }
}

bool FractalController::usesAdditionalStage(FractalType type)
{
    return type == FractalType::Dragon || type == FractalType::SierpinskiCarpet || type == FractalType::SierpinskiPentagon || type == FractalType::Vicsek || type == FractalType::Hilbert;
}

bool FractalController::isFilledTriangleFractal(FractalType type)
{
    return type == FractalType::Sierpinski || type == FractalType::SierpinskiCarpet || type == FractalType::SierpinskiPentagon || type == FractalType::Vicsek;
}

bool FractalController::startsAtStageOne(FractalType type)
{
    return type == FractalType::Dragon || type == FractalType::Hilbert;
}

void FractalController::animateOscillatingAngle(float& angle, float& direction,
    float minAngle, float maxAngle, float speed, float elapsedSeconds)
{
    angle += direction * speed * elapsedSeconds;
    if (angle >= maxAngle || angle <= minAngle)
    {
        angle = std::clamp(angle, minAngle, maxAngle);
        direction *= -1.0f;
    }
}

void FractalController::syncPalette(Settings& settings)
{
    palette = makePalette(settings.fractalColor);
}

PythagorasSquare FractalController::createPythagorasRoot(const Settings& settings) const
{
    const float halfSize = settings.pythagoras_size * 0.5f;
    const float baseY = settings.windowHeight - 80.0f;
    return {
        { settings.windowWidth * 0.5f - halfSize, baseY },
        { settings.windowWidth * 0.5f + halfSize, baseY },
        { settings.windowWidth * 0.5f + halfSize, baseY - settings.pythagoras_size },
        { settings.windowWidth * 0.5f - halfSize, baseY - settings.pythagoras_size },
        0 };
}

// Rebuild the Koch curve up to the requested construction stage
void FractalController::rebuildKoch(int targetStage, const Settings& settings)
{
    kochSegments = { { settings.koch_p1, settings.koch_p2, 0 } };
    kochStage = 0;
    targetStage = std::clamp(targetStage, 0, constructionSteps);
    while (kochStage < targetStage)
    {
        kochSegments = expandKocha(kochSegments, kochStage + 1);
        ++kochStage;
    }
    segmentsToVertices(kochSegments, vertices, palette);
}

// Rebuild the Sierpinski triangle up to the requested construction stage
void FractalController::rebuildSierpinski(int targetStage, const Settings& settings)
{
    sierpinskiStage = std::clamp(targetStage, 0, constructionSteps);
    generateSierpinskiTriangle(
        sierpinskiStage,
        vertices,
        palette,
        settings.sierpinski_p1,
        settings.sierpinski_p2,
        settings.sierpinski_p3);
}

// Rebuild the Pythagoras tree up to the requested construction stage
void FractalController::rebuildPythagoras(int targetStage, const Settings& settings)
{
    pythagorasSquares = { createPythagorasRoot(settings) };
    pythagorasFrontier = pythagorasSquares;
    pythagorasStage = 0;
    targetStage = std::clamp(targetStage, settings.pythagoras_min_depth, pythagorasDepth);

    while (pythagorasStage < targetStage)
    {
        pythagorasFrontier = expandPythagoras(pythagorasFrontier, pythagorasAngle);
        pythagorasSquares.insert(
            pythagorasSquares.end(),
            pythagorasFrontier.begin(),
            pythagorasFrontier.end());
        ++pythagorasStage;
    }
}

// Rebuild the active fractal geometry up to the requested construction stage.
void FractalController::rebuildConstruction(int targetStage, Settings& settings)
{
    syncPalette(settings);
    vertices.clear();

    switch (currentType)
    {
    case FractalType::Koch:
        rebuildKoch(targetStage, settings);
        break;
    case FractalType::Sierpinski:
        rebuildSierpinski(targetStage, settings);
        break;
    case FractalType::Tree:
        treeStage = std::clamp(targetStage, 1, constructionSteps);
        generujTree(treeStage, settings.tree_start, settings.tree_initial_length, 90.0f, vertices,
            palette, constructionSteps, settings.tree_length_factor, treeBranchAngle, -treeBranchAngle);
        break;
    case FractalType::Dragon:
        additionalStage = std::clamp(targetStage, minConstructionStage(), maxConstructionStage());
        generateDragon(additionalStage, vertices, palette, settings.windowWidth, settings.windowHeight);
        break;
    case FractalType::SierpinskiCarpet:
        additionalStage = std::clamp(targetStage, 0, maxConstructionStage());
        generateSierpinskiCarpet(additionalStage, vertices, palette, settings.windowWidth, settings.windowHeight);
        break;
    case FractalType::SierpinskiPentagon:
        additionalStage = std::clamp(targetStage, 0, maxConstructionStage());
        generateSierpinskiPentagon(additionalStage, vertices, palette, settings.windowWidth, settings.windowHeight);
        break;
    case FractalType::Vicsek:
        additionalStage = std::clamp(targetStage, 0, maxConstructionStage());
        generateVicsek(additionalStage, vertices, palette, settings.windowWidth, settings.windowHeight);
        break;
    case FractalType::Hilbert:
        additionalStage = std::clamp(targetStage, minConstructionStage(), maxConstructionStage());
        generateHilbertCurve(additionalStage, vertices, palette, settings.windowWidth, settings.windowHeight);
        break;
    case FractalType::Pythagoras:
        rebuildPythagoras(targetStage, settings);
        break;
    case FractalType::Newton:
        newton.requestRedraw();
        break;
    default:
        break;
    }
}

void FractalController::generateFractal(Settings& settings)
{
    syncPalette(settings);
    constructionClock.restart();
    treeAngleClock.restart();
    pythagorasAngleClock.restart();

    if (currentType == FractalType::Pythagoras)
        rebuildConstruction(pythagorasDepth, settings);
    else if (currentType == FractalType::Tree)
        rebuildConstruction(constructionSteps, settings);
    else if (currentType == FractalType::Newton)
        newton.requestRedraw();
    else if (startsAtStageOne(currentType))
        rebuildConstruction(1, settings);
    else
        rebuildConstruction(0, settings);
}

bool FractalController::advanceConstruction(Settings& settings)
{
    switch (currentType)
    {
    case FractalType::Koch:
        if (kochStage < constructionSteps)
        {
            rebuildConstruction(kochStage + 1, settings);
            return true;
        }
        break;
    case FractalType::Sierpinski:
        if (sierpinskiStage < constructionSteps)
        {
            rebuildConstruction(sierpinskiStage + 1, settings);
            return true;
        }
        break;
    case FractalType::Tree:
        break;
    case FractalType::Pythagoras:
        if (pythagorasStage < pythagorasDepth)
        {
            rebuildConstruction(pythagorasStage + 1, settings);
            return true;
        }
        break;
    default:
        if (usesAdditionalStage(currentType) && additionalStage < maxConstructionStage())
        {
            rebuildConstruction(additionalStage + 1, settings);
            return true;
        }
        break;
    }
    return false;
}

bool FractalController::retreatConstruction(Settings& settings)
{
    const int minStage = minConstructionStage();

    switch (currentType)
    {
    case FractalType::Koch:
        if (kochStage > 0)
        {
            rebuildConstruction(kochStage - 1, settings);
            return true;
        }
        break;
    case FractalType::Sierpinski:
        if (sierpinskiStage > 0)
        {
            rebuildConstruction(sierpinskiStage - 1, settings);
            return true;
        }
        break;
    case FractalType::Tree:
        if (treeStage > 1)
        {
            rebuildConstruction(treeStage - 1, settings);
            return true;
        }
        break;
    case FractalType::Pythagoras:
        if (pythagorasStage > 0)
        {
            rebuildConstruction(pythagorasStage - 1, settings);
            return true;
        }
        break;
    default:
        if (usesAdditionalStage(currentType) && additionalStage > minStage)
        {
            rebuildConstruction(additionalStage - 1, settings);
            return true;
        }
        break;
    }
    return false;
}

// Rebuild when the user changes the fractal color slider.
void FractalController::applyBaseColor(Settings& settings)
{
    if (palette[0] == settings.fractalColor || isExplorerMode(currentType))
        return;

    rebuildConstruction(currentStage(), settings);
}

// Advance construction stages and run angle animations each frame.
void FractalController::updateAnimations(Settings& settings)
{
    if (currentType == FractalType::Pythagoras && !isAnimationPaused)
    {
        animateOscillatingAngle(
            pythagorasAngle,
            pythagorasAngleDirection,
            settings.pythagoras_min_angle,
            settings.pythagoras_max_angle,
            settings.drawSpeed * 10.0f,
            pythagorasAngleClock.restart().asSeconds());
        rebuildConstruction(pythagorasDepth, settings);
    }
    else
        pythagorasAngleClock.restart();

    if (currentType == FractalType::Tree && !isAnimationPaused)
    {
        animateOscillatingAngle(
            treeBranchAngle,
            treeAngleDirection,
            settings.tree_min_angle,
            settings.tree_max_angle,
            settings.drawSpeed * 20.0f,
            treeAngleClock.restart().asSeconds());
        rebuildConstruction(treeStage, settings);
    }
    else
        treeAngleClock.restart();

    const float stageDelay = std::max(0.1f, 0.8f / settings.drawSpeed);
    if (!isAnimationPaused && !isManualStageFractal(currentType) && !isExplorerMode(currentType) && constructionClock.getElapsedTime().asSeconds() >= stageDelay && advanceConstruction(settings))
        constructionClock.restart();

}

void FractalController::handleKeyPressed(const sf::Event::KeyPressed& keyEvent, Settings& settings)
{
    if (keyEvent.code == sf::Keyboard::Key::Space)
    {
        isAnimationPaused = !isAnimationPaused;
        constructionClock.restart();
        pythagorasAngleClock.restart();
        treeAngleClock.restart();
        return;
    }

    if (isExplorerMode(currentType))
        return;

    if (currentType == FractalType::Pythagoras)
    {
        if (keyEvent.code == sf::Keyboard::Key::Up)
        {
            pythagorasAngle = std::min(settings.pythagoras_max_angle, pythagorasAngle + 2.5f);
            rebuildConstruction(pythagorasDepth, settings);
        }
        else if (keyEvent.code == sf::Keyboard::Key::Down)
        {
            pythagorasAngle = std::max(settings.pythagoras_min_angle, pythagorasAngle - 2.5f);
            rebuildConstruction(pythagorasDepth, settings);
        }
        else if (keyEvent.code == sf::Keyboard::Key::Right)
        {
            pythagorasDepth = std::min(settings.pythagoras_max_depth, pythagorasDepth + 1);
            rebuildConstruction(pythagorasDepth, settings);
        }
        else if (keyEvent.code == sf::Keyboard::Key::Left)
        {
            pythagorasDepth = std::max(settings.pythagoras_min_depth, pythagorasDepth - 1);
            rebuildConstruction(pythagorasDepth, settings);
        }
        return;
    }

    if (currentType == FractalType::Tree)
    {
        if (keyEvent.code == sf::Keyboard::Key::Up)
        {
            treeBranchAngle = std::min(settings.tree_max_angle, treeBranchAngle + 2.5f);
            rebuildConstruction(treeStage, settings);
        }
        else if (keyEvent.code == sf::Keyboard::Key::Down)
        {
            treeBranchAngle = std::max(settings.tree_min_angle, treeBranchAngle - 2.5f);
            rebuildConstruction(treeStage, settings);
        }
        else if (keyEvent.code == sf::Keyboard::Key::Right)
        {
            treeStage = std::min(constructionSteps, treeStage + 1);
            rebuildConstruction(treeStage, settings);
        }
        else if (keyEvent.code == sf::Keyboard::Key::Left)
        {
            treeStage = std::max(1, treeStage - 1);
            rebuildConstruction(treeStage, settings);
        }
        return;
    }

    if (keyEvent.code == sf::Keyboard::Key::Right && isAnimationPaused)
        advanceConstruction(settings);
    else if (keyEvent.code == sf::Keyboard::Key::Left && isAnimationPaused)
        retreatConstruction(settings);
}

void FractalController::draw(sf::RenderWindow& window, Settings& settings, sf::View& camera)
{
    if (currentType == FractalType::Newton)
    {
        window.setView(window.getDefaultView());
        newton.draw(window);
        return;
    }

    if (isFilledTriangleFractal(currentType) && vertices.size() >= 3)
    {
        window.setView(window.getDefaultView());
        window.draw(vertices.data(), vertices.size(), sf::PrimitiveType::Triangles);
        return;
    }

    if (currentType == FractalType::Pythagoras && !pythagorasSquares.empty())
    {
        window.setView(window.getDefaultView());
        drawPythagorasSquares(window, pythagorasSquares, palette);
        return;
    }

    if (vertices.size() >= 2)
    {
        const VertexBounds bounds = computeVertexBounds(vertices);
        fitViewToBounds(camera, bounds, settings.windowWidth, settings.windowHeight, settings.cameraZoom);
        window.setView(camera);

        const float baseLineThickness = std::max(
            0.7f,
            5.0f - static_cast<float>(currentStage()) * 0.58f);
        const float lineThickness = currentType == FractalType::Dragon
            ? std::max(2.5f, baseLineThickness)
            : baseLineThickness;
        drawThickLines(window, vertices, lineThickness);
    }
}

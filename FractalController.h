#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <vector>

#include "Fractals.h"
#include "NewtonExplorer.h"
#include "Settings.h"
#include "Tab.h"

class FractalController
{
public:
    static constexpr int constructionSteps = 10;

    FractalType currentType = FractalType::Koch;
    std::vector<sf::Vertex> vertices;
    bool isAnimationPaused = false;

    FractalController(NewtonExplorer& newtonExplorer, const Settings& settings);

    int maxConstructionStage() const;
    int minConstructionStage() const;
    int currentStage() const;
    void rebuildConstruction(int targetStage, Settings& settings);
    void generateFractal(Settings& settings);
    bool advanceConstruction(Settings& settings);
    bool retreatConstruction(Settings& settings);
    void applyBaseColor(Settings& settings);
    void updateAnimations(Settings& settings);
    void handleKeyPressed(const sf::Event::KeyPressed& keyEvent, Settings& settings);
    void draw(sf::RenderWindow& window, Settings& settings, sf::View& camera);

private:
    NewtonExplorer& newton;
    std::array<sf::Color, kPaletteSize> palette{};

    std::vector<KochSegment> kochSegments;
    int kochStage = 0;

    std::vector<Triangle> sierpinskiTriangles;
    int sierpinskiStage = 0;

    int treeStage = 0;
    float treeBranchAngle = 0.0f;
    float treeAngleDirection = 1.0f;

    int additionalStage = 0;

    std::vector<PythagorasSquare> pythagorasSquares;
    std::vector<PythagorasSquare> pythagorasFrontier;
    int pythagorasStage = 0;
    int pythagorasDepth = 4;
    float pythagorasAngle = 0.0f;
    float pythagorasAngleDirection = 1.0f;

    sf::Clock constructionClock;
    sf::Clock pythagorasAngleClock;
    sf::Clock treeAngleClock;

    static bool usesAdditionalStage(FractalType type);
    static bool isFilledTriangleFractal(FractalType type);
    static bool startsAtStageOne(FractalType type);
    static void animateOscillatingAngle(float& angle, float& direction,
        float minAngle, float maxAngle, float speed, float elapsedSeconds);
    void syncPalette(Settings& settings);
    void rebuildKoch(int targetStage, const Settings& settings);
    void rebuildSierpinski(int targetStage, const Settings& settings);
    void rebuildPythagoras(int targetStage, const Settings& settings);
    PythagorasSquare createPythagorasRoot(const Settings& settings) const;
};

//
// Created by tdragon on 9/17/26.
//

#ifndef MARYSFLIGHTS_RENDERER_H
#define MARYSFLIGHTS_RENDERER_H

#pragma once

#include <vector>

#include "../simulation/Simulation.h"
#include "raylib.h"

class Renderer
{
public:
    Renderer();

    void draw(
        const Simulation& simulation
    );

    void updateCamera(
        const Simulation& simulation
    );

    void updateSelection(
        const SolarSystem& solarSystem
    );

    void focusBody(
        const Body* body
    );

    void focusNextBody(
        const SolarSystem& solarSystem
    );

    [[nodiscard]] const Body* getHoveredBody(
        const SolarSystem& solarSystem
    ) const;

    [[nodiscard]] const Body* getFocusedBody() const
    {
        return focusedBody;
    }

    static Color toRaylibColor(
        const BodyColor& color
    );

private:
    /*
     * ------------------------------------------------------------------
     * Scale configuration
     * ------------------------------------------------------------------
     */

    static constexpr double auInKm = 149'597'870.7;

    /*
     * 1 AU = 10 world units.
     */
    static constexpr double unitsPerAu = 10.0;

    static constexpr double kmToUnits =
        unitsPerAu / auInKm;

    /*
     * ------------------------------------------------------------------
     * Apparent-size configuration (all values are in screen pixels)
     * ------------------------------------------------------------------
     *
     * A body is drawn at its TRUE physical radius whenever that radius
     * covers at least "markerPixels" on screen. Once it would shrink
     * below that, the radius is clamped so the body keeps a constant
     * on-screen size. That is the "switch to visual scale" behaviour.
     */

    static constexpr double planetMarkerPixels = 5.5;
    static constexpr double moonMarkerPixels = 3.0;

    /*
     * A moon is only worth drawing once it is far enough from its
     * parent on screen to be distinguishable from it. Below the
     * "appear" threshold it is fully hidden, between the two it fades.
     */
    static constexpr double moonAppearPixels = 6.0;
    static constexpr double moonFullyVisiblePixels = 18.0;

    /*
     * Minimum clickable size, so marker-sized bodies stay pickable.
     */
    static constexpr double pickPixels = 7.0;

    /*
     * ------------------------------------------------------------------
     * Depth configuration
     * ------------------------------------------------------------------
     *
     * The scene is rendered in units of the current camera distance, so
     * the camera always sits exactly 1.0 unit from the origin. Anything
     * further away than maxRenderDepth is projected onto a shell at that
     * depth with its radius scaled to match, which preserves apparent
     * size while keeping everything inside the default far plane.
     */
    static constexpr double cameraUnitDistance = 1.0;
    static constexpr double maxRenderDepth = 400.0;

    /*
     * ------------------------------------------------------------------
     */

    struct BodyVisual
    {
        const Body* body = nullptr;

        Vector3 position = {0.0f, 0.0f, 0.0f};

        double depth = 0.0;

        double realRadius = 0.0;
        double drawRadius = 0.0;
        double pickRadius = 0.0;
        double screenRadius = 0.0;

        float alpha = 1.0f;

        bool realScale = false;
    };

    Camera3D camera;

    const Body* focusedBody;
    const Body* hoveredBody;

    /*
     * Camera distance expressed in WORLD units (1 AU = 10 units),
     * not in the scaled scene space that is actually rendered.
     */
    double cameraDistance;

    float orbitYaw;
    float orbitPitch;

    std::vector<BodyVisual> visuals;

    /*
     * ------------------------------------------------------------------
     * Space conversion
     * ------------------------------------------------------------------
     */

    [[nodiscard]] double getSceneScale() const;

    /*
     * Simulation position (km, z-up) -> world units (raylib, y-up).
     */
    [[nodiscard]] Vector3 toRaylib(
        const Vec3& position
    ) const;

    /*
     * Simulation position -> render space:
     * relative to the focused body, divided by the camera distance.
     */
    [[nodiscard]] Vector3 toScene(
        const Vec3& position
    ) const;

    [[nodiscard]] double worldPerPixelAt(
        double depth
    ) const;

    /*
     * ------------------------------------------------------------------
     * Body evaluation
     * ------------------------------------------------------------------
     */

    void buildVisuals(
        const SolarSystem& solarSystem
    );

    [[nodiscard]] BodyVisual computeVisual(
        const Body& body
    ) const;

    [[nodiscard]] float computeMoonAlpha(
        const Body& body,
        const Vector3& scenePosition,
        double depth
    ) const;

    [[nodiscard]] bool isMoon(
        const Body& body
    ) const;

    [[nodiscard]] double getRealRadius(
        const Body& body
    ) const;

    [[nodiscard]] Color getBodyColor(
        const Body& body,
        float alpha
    ) const;

    /*
     * ------------------------------------------------------------------
     * Rendering
     * ------------------------------------------------------------------
     */

    void drawGrid() const;

    void drawHud() const;

    void updateOrbitCamera();

    [[nodiscard]] double getMinimumCameraDistance() const;
};

#endif //MARYSFLIGHTS_RENDERER_H

#pragma once

#include <unordered_map>
#include <vector>

#include "../simulation/Simulation.h"
#include "raylib.h"

class Renderer
{
public:
    Renderer();
    ~Renderer();

    /*
     * Textures and GPU resources are owned by this object and are not
     * safe to copy or move without extra bookkeeping.
     */
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

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

    static constexpr double planetMarkerPixels = 3.5;
    static constexpr double moonMarkerPixels = 2.0;

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
     * Furthest the camera may zoom out, in world units. 2e10 AU is about
     * 97 kpc - enough to frame the whole Milky Way - while the double
     * precision subtraction in toScene() keeps close-up views exact.
     */
    static constexpr double maxCameraDistance = unitsPerAu * 2.0e10;

    /*
     * Anything with at least this gravitational parameter (0.07 solar
     * masses, roughly the hydrogen-burning limit) is treated as a star:
     * self-lit, and the light source for whatever orbits it.
     */
    static constexpr double minimumStellarMu = 0.07 * 1.32712440018e11;

    /*
     * ------------------------------------------------------------------
     * Surface shading configuration
     * ------------------------------------------------------------------
     *
     * Below this on-screen radius a body is drawn as a flat-shaded dot,
     * same as before - not worth the cost of a lit, textured sphere when
     * it is only a handful of pixels wide. Above it, and only once the
     * body has crossed into real-scale rendering, it gets a lit,
     * procedurally-textured sphere instead.
     */
    static constexpr double textureLodPixels = 22.0;

    /*
     * Procedural texture resolution. Equirectangular (longitude x
     * latitude); 256x128 is plenty for anything short of a close, full
     * screen surface flyby.
     */
    static constexpr int textureWidth = 256;
    static constexpr int textureHeight = 128;

    enum class SurfaceStyle
    {
        Rocky,
        GasGiant,
        Cratered,
        Star
    };

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
     * Shading resources
     * ------------------------------------------------------------------
     *
     * Loaded lazily on first use (ensureShaderLoaded) rather than in the
     * constructor, since raylib's GPU calls require an open window/GL
     * context, which may not exist yet when Renderer is constructed.
     */

    bool shaderReady = false;

    Shader planetShader{};
    Mesh unitSphereMesh{};
    Model unitSphereModel{};

    int locLightPos = -1;
    int locViewPos = -1;
    int locTint = -1;
    int locAmbientStrength = -1;
    int locAtmosphereStrength = -1;
    int locAtmosphereColor = -1;

    /*
     * One procedural texture per body, generated once and cached for
     * the process lifetime. Keyed by pointer since Body identities are
     * stable for the run of the program.
     */
    std::unordered_map<const Body*, Texture2D> planetTextures;

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
     * Surface shading
     * ------------------------------------------------------------------
     */

    void ensureShaderLoaded();

    [[nodiscard]] SurfaceStyle classifyStyle(
        const Body& body
    ) const;

    /*
     * Returns the cached procedural texture for this body, generating
     * and uploading it the first time the body is asked for.
     */
    [[nodiscard]] Texture2D& getOrCreateTexture(
        const Body& body
    );

    /*
     * Walks up the parent chain to the body with no parent - the host
     * star for anything in a real system file. Used as the light source.
     */
    [[nodiscard]] const Body* findRootStar(
        const Body& body
    ) const;

    void drawShadedBody(
        const BodyVisual& visual
    );

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
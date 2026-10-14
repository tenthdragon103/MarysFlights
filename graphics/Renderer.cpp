#include "Renderer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "rlgl.h"

/*
 * ======================================================================
 * Local helpers
 * ======================================================================
 */

namespace
{

struct OrbitalInfo
{
    bool valid = false;

    double energy = 0.0;
    double altitude = 0.0;

    double periapsis = 0.0;
    double apoapsis = 0.0;

    bool hasApoapsis = false;
};

OrbitalInfo calculateOrbitalInfo(
    const Body& body
)
{
    OrbitalInfo info;

    const Body* parent =
        body.getParent();

    if (!parent)
        return info;

    const Vec3 relativePosition =
        body.getPosition() -
        parent->getPosition();

    const Vec3 relativeVelocity =
        body.getVelocity() -
        parent->getVelocity();

    const double radius =
        relativePosition.length();

    if (radius <= 0.0)
        return info;

    const double mu =
        parent->getMu();

    if (mu <= 0.0)
        return info;

    const double speedSquared =
        relativeVelocity.lengthSquared();

    info.energy =
        0.5 * speedSquared -
        mu / radius;

    info.altitude =
        radius -
        parent->getRadius();

    const Vec3 angularMomentum =
        relativePosition.cross(
            relativeVelocity
        );

    const Vec3 eccentricityVector =
        (
            relativeVelocity.cross(
                angularMomentum
            ) / mu
        ) -
        relativePosition.normalized();

    const double eccentricity =
        eccentricityVector.length();

    /*
     * Parabolic case: semi-major axis is undefined.
     */
    if (std::abs(info.energy) < 1e-12)
    {
        info.periapsis =
            (angularMomentum.lengthSquared() / mu) /
            (1.0 + eccentricity);

        info.hasApoapsis = false;
        info.valid = true;

        return info;
    }

    const double semiMajorAxis =
        -mu / (2.0 * info.energy);

    info.periapsis =
        semiMajorAxis *
        (1.0 - eccentricity);

    if (info.energy < 0.0)
    {
        info.apoapsis =
            semiMajorAxis *
            (1.0 + eccentricity);

        info.hasApoapsis = true;
    }

    info.valid = true;

    return info;
}

double distanceBetween(
    const Vector3& a,
    const Vector3& b
)
{
    const double dx =
        static_cast<double>(a.x) -
        static_cast<double>(b.x);

    const double dy =
        static_cast<double>(a.y) -
        static_cast<double>(b.y);

    const double dz =
        static_cast<double>(a.z) -
        static_cast<double>(b.z);

    return std::sqrt(
        dx * dx +
        dy * dy +
        dz * dz
    );
}

Ray getPickingRay(
    const Camera3D& camera
)
{
#if defined(RAYLIB_VERSION_MAJOR) &&                       \
    (RAYLIB_VERSION_MAJOR > 5 ||                           \
     (RAYLIB_VERSION_MAJOR == 5 && RAYLIB_VERSION_MINOR >= 5))
    return GetScreenToWorldRay(
        GetMousePosition(),
        camera
    );
#else
    return GetMouseRay(
        GetMousePosition(),
        camera
    );
#endif
}

/*
 * Returns the distance along the ray to the nearest intersection,
 * or a negative value when the sphere is missed.
 */
double raySphereDistance(
    const Ray& ray,
    const Vector3& center,
    double radius
)
{
    const double ox =
        static_cast<double>(ray.position.x) -
        static_cast<double>(center.x);

    const double oy =
        static_cast<double>(ray.position.y) -
        static_cast<double>(center.y);

    const double oz =
        static_cast<double>(ray.position.z) -
        static_cast<double>(center.z);

    const double dx = static_cast<double>(ray.direction.x);
    const double dy = static_cast<double>(ray.direction.y);
    const double dz = static_cast<double>(ray.direction.z);

    const double b =
        2.0 * (ox * dx + oy * dy + oz * dz);

    const double c =
        ox * ox + oy * oy + oz * oz -
        radius * radius;

    const double discriminant =
        b * b - 4.0 * c;

    if (discriminant < 0.0)
        return -1.0;

    const double root =
        std::sqrt(discriminant);

    const double t0 = (-b - root) * 0.5;
    const double t1 = (-b + root) * 0.5;

    if (t0 >= 0.0)
        return t0;

    if (t1 >= 0.0)
        return t1;

    return -1.0;
}

int sphereDetail(
    double screenRadius
)
{
    const int rings =
        static_cast<int>(screenRadius * 0.6);

    return std::clamp(rings, 8, 48);
}

} // namespace

/*
 * ======================================================================
 * Construction
 * ======================================================================
 */

Renderer::Renderer()
    : focusedBody(nullptr),
      hoveredBody(nullptr),
      cameraDistance(35.0),
      orbitYaw(0.0f),
      orbitPitch(20.0f)
{
    camera.position = {0.0f, 0.0f, 1.0f};
    camera.target   = {0.0f, 0.0f, 0.0f};
    camera.up       = {0.0f, 1.0f, 0.0f};

    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

/*
 * ======================================================================
 * Space conversion
 * ======================================================================
 */

double Renderer::getSceneScale() const
{
    /*
     * Everything is rendered in units of the current camera distance,
     * which keeps the camera at a fixed 1.0 units from the origin no
     * matter whether we are looking at the whole system or standing on
     * a moon's surface. This is what makes true-scale rendering
     * survive single-precision floats.
     */
    return 1.0 / std::max(cameraDistance, 1e-12);
}

Vector3 Renderer::toRaylib(
    const Vec3& position
) const
{
    return {
        static_cast<float>(position.x * kmToUnits),
        static_cast<float>(position.z * kmToUnits),
        static_cast<float>(position.y * kmToUnits)
    };
}

Vector3 Renderer::toScene(
    const Vec3& position
) const
{
    Vec3 relative = position;

    if (focusedBody)
    {
        relative =
            position -
            focusedBody->getPosition();
    }

    /*
     * The subtraction happens in double precision, in kilometres,
     * BEFORE anything is narrowed to float. That ordering is the whole
     * reason a 6 000 km planet can be rendered 40 AU from the origin.
     */
    const double scale =
        kmToUnits * getSceneScale();

    return {
        static_cast<float>(relative.x * scale),
        static_cast<float>(relative.z * scale),
        static_cast<float>(relative.y * scale)
    };
}

double Renderer::worldPerPixelAt(
    double depth
) const
{
    const double screenHeight =
        static_cast<double>(GetScreenHeight());

    if (screenHeight <= 0.0)
        return 0.0;

    const double halfFov =
        static_cast<double>(camera.fovy) *
        DEG2RAD * 0.5;

    return 2.0 * depth * std::tan(halfFov) /
           screenHeight;
}

double Renderer::getRealRadius(
    const Body& body
) const
{
    return body.getRadius() *
           kmToUnits *
           getSceneScale();
}

/*
 * ======================================================================
 * Body evaluation
 * ======================================================================
 */

bool Renderer::isMoon(
    const Body& body
) const
{
    return body.getParentType() == ParentType::Body &&
           body.getParent() != nullptr;
}

float Renderer::computeMoonAlpha(
    const Body& body,
    const Vector3& scenePosition,
    double depth
) const
{
    const Body* parent =
        body.getParent();

    if (!parent)
        return 1.0f;

    /*
     * The focused body is always drawn, otherwise focusing a moon
     * would make it vanish the moment the camera settles on it.
     */
    if (&body == focusedBody)
        return 1.0f;

    const Vector3 parentPosition =
        toScene(parent->getPosition());

    const double separation =
        distanceBetween(
            scenePosition,
            parentPosition
        );

    const double worldPerPixel =
        worldPerPixelAt(depth);

    if (worldPerPixel <= 0.0)
        return 1.0f;

    const double separationPixels =
        separation / worldPerPixel;

    /*
     * Hidden while it would overlap its parent on screen, fading in as
     * the camera gets close enough to resolve the two apart.
     */
    const double t =
        (separationPixels - moonAppearPixels) /
        (moonFullyVisiblePixels - moonAppearPixels);

    return static_cast<float>(
        std::clamp(t, 0.0, 1.0)
    );
}

Renderer::BodyVisual Renderer::computeVisual(
    const Body& body
) const
{
    BodyVisual visual;

    visual.body = &body;
    visual.position = toScene(body.getPosition());

    double depth =
        distanceBetween(
            visual.position,
            camera.position
        );

    depth = std::max(depth, 1e-9);

    visual.depth = depth;

    const double worldPerPixel =
        worldPerPixelAt(depth);

    const double realRadius =
        getRealRadius(body);

    visual.realRadius = realRadius;

    /*
     * ----------------------------------------------------------------
     * Real scale vs. visual scale
     * ----------------------------------------------------------------
     *
     * markerRadius is the world size that corresponds to a fixed number
     * of pixels at this depth. While the true radius is larger than
     * that, the body is drawn exactly as big as it really is. Once it
     * would fall under that threshold we clamp, so it stops shrinking
     * and keeps a constant dot size on screen.
     */
    const double markerPixels =
        isMoon(body)
            ? moonMarkerPixels
            : planetMarkerPixels;

    const double markerRadius =
        markerPixels * worldPerPixel;

    visual.realScale = realRadius >= markerRadius;

    visual.drawRadius =
        std::max(realRadius, markerRadius);

    visual.pickRadius =
        std::max(
            visual.drawRadius,
            pickPixels * worldPerPixel
        );

    visual.screenRadius =
        worldPerPixel > 0.0
            ? visual.drawRadius / worldPerPixel
            : 0.0;

    /*
     * ----------------------------------------------------------------
     * Visibility
     * ----------------------------------------------------------------
     */
    visual.alpha =
        isMoon(body)
            ? computeMoonAlpha(
                  body,
                  visual.position,
                  depth
              )
            : 1.0f;

    /*
     * ----------------------------------------------------------------
     * Depth remapping
     * ----------------------------------------------------------------
     *
     * Anything beyond the far plane is pulled onto a shell at
     * maxRenderDepth. Scaling the radius by the same factor keeps the
     * apparent size identical, so a distant star still reads correctly
     * while we are zoomed onto a moon.
     */
    if (depth > maxRenderDepth)
    {
        const double k = maxRenderDepth / depth;

        visual.position = {
            camera.position.x +
                static_cast<float>(
                    (visual.position.x - camera.position.x) * k
                ),

            camera.position.y +
                static_cast<float>(
                    (visual.position.y - camera.position.y) * k
                ),

            camera.position.z +
                static_cast<float>(
                    (visual.position.z - camera.position.z) * k
                )
        };

        visual.drawRadius *= k;
        visual.pickRadius *= k;
        visual.depth = maxRenderDepth;
    }

    return visual;
}

void Renderer::buildVisuals(
    const SolarSystem& solarSystem
)
{
    visuals.clear();

    for (const Body* body :
         solarSystem.getBodyPointers())
    {
        if (!body || !body->isActive())
            continue;

        BodyVisual visual =
            computeVisual(*body);

        if (visual.alpha <= 0.0f)
            continue;

        visuals.push_back(visual);
    }

    /*
     * Back to front, so the faded-in moons blend correctly.
     */
    std::sort(
        visuals.begin(),
        visuals.end(),
        [](const BodyVisual& a, const BodyVisual& b)
        {
            return a.depth > b.depth;
        }
    );
}

Color Renderer::getBodyColor(
    const Body& body,
    float alpha
) const
{
    const BodyColor& color =
        body.getColor();

    const float combined =
        (static_cast<float>(color.a) / 255.0f) *
        std::clamp(alpha, 0.0f, 1.0f);

    return Color{
        color.r,
        color.g,
        color.b,
        static_cast<unsigned char>(
            std::clamp(combined * 255.0f, 0.0f, 255.0f)
        )
    };
}

Color Renderer::toRaylibColor(
    const BodyColor& color
)
{
    return Color{
        color.r,
        color.g,
        color.b,
        color.a
    };
}

/*
 * ======================================================================
 * Selection
 * ======================================================================
 */

const Body* Renderer::getHoveredBody(
    const SolarSystem& solarSystem
) const
{
    (void)solarSystem;

    return hoveredBody;
}

void Renderer::updateSelection(
    const SolarSystem& solarSystem
)
{
    buildVisuals(solarSystem);

    hoveredBody = nullptr;

    const Ray ray =
        getPickingRay(camera);

    double bestDistance = -1.0;

    for (const BodyVisual& visual : visuals)
    {
        if (visual.alpha < 0.35f)
            continue;

        const double hit =
            raySphereDistance(
                ray,
                visual.position,
                visual.pickRadius
            );

        if (hit < 0.0)
            continue;

        if (bestDistance < 0.0 || hit < bestDistance)
        {
            bestDistance = hit;
            hoveredBody = visual.body;
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoveredBody)
    {
        focusBody(hoveredBody);
    }
}

/*
 * ======================================================================
 * Camera
 * ======================================================================
 */

double Renderer::getMinimumCameraDistance() const
{
    /*
     * Never let the camera sink inside the focused body, and never let
     * the scene scale explode. Expressed in world units.
     */
    double minimum = 1e-7;

    if (focusedBody)
    {
        const double radiusInUnits =
            focusedBody->getRadius() * kmToUnits;

        minimum =
            std::max(minimum, radiusInUnits * 1.05);
    }

    return minimum;
}

void Renderer::updateCamera(
    const Simulation& simulation
)
{
    const SolarSystem& solarSystem =
        simulation.getSolarSystem();

    if (!focusedBody)
    {
        const auto& bodies =
            solarSystem.getBodyPointers();

        if (!bodies.empty())
            focusedBody = bodies.front();
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        const Vector2 mouseDelta =
            GetMouseDelta();

        orbitYaw -= mouseDelta.x * 0.25f;
        orbitPitch += mouseDelta.y * 0.25f;

        orbitPitch =
            std::clamp(orbitPitch, -89.0f, 89.0f);
    }

    const float wheel =
        GetMouseWheelMove();

    if (std::abs(wheel) > 0.0f)
    {
        cameraDistance *=
            std::pow(0.8, static_cast<double>(wheel));
    }

    cameraDistance =
        std::clamp(
            cameraDistance,
            getMinimumCameraDistance(),
            unitsPerAu * 200.0
        );

    updateOrbitCamera();
}

void Renderer::updateOrbitCamera()
{
    const float yaw = orbitYaw * DEG2RAD;
    const float pitch = orbitPitch * DEG2RAD;

    const float horizontal =
        static_cast<float>(
            cameraUnitDistance * std::cos(pitch)
        );

    /*
     * The focused body is the origin of render space, so the camera
     * simply orbits (0, 0, 0) at a fixed unit radius.
     */
    camera.position = {
        horizontal * std::sin(yaw),

        static_cast<float>(
            cameraUnitDistance * std::sin(pitch)
        ),

        horizontal * std::cos(yaw)
    };

    camera.target = {0.0f, 0.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
}

void Renderer::focusBody(
    const Body* body
)
{
    if (!body)
        return;

    focusedBody = body;

    orbitYaw = 0.0f;
    orbitPitch = 20.0f;

    /*
     * Frame the body at roughly six radii, which lands the camera in
     * the true-scale regime immediately. Bodies without a radius
     * (barycenters) fall back to a fixed distance.
     */
    const double radiusInUnits =
        body->getRadius() * kmToUnits;

    cameraDistance =
        radiusInUnits > 0.0
            ? radiusInUnits * 6.0
            : 1.0;

    cameraDistance =
        std::clamp(
            cameraDistance,
            getMinimumCameraDistance(),
            unitsPerAu * 200.0
        );

    updateOrbitCamera();
}

void Renderer::focusNextBody(
    const SolarSystem& solarSystem
)
{
    const auto& bodies =
        solarSystem.getBodyPointers();

    if (bodies.empty())
        return;

    std::size_t currentIndex = 0;

    if (focusedBody)
    {
        for (std::size_t i = 0; i < bodies.size(); ++i)
        {
            if (bodies[i] == focusedBody)
            {
                currentIndex = i;
                break;
            }
        }
    }

    for (std::size_t offset = 1;
         offset <= bodies.size();
         ++offset)
    {
        const std::size_t index =
            (currentIndex + offset) % bodies.size();

        if (bodies[index] && bodies[index]->isActive())
        {
            focusBody(bodies[index]);
            return;
        }
    }
}

/*
 * ======================================================================
 * Rendering
 * ======================================================================
 */

void Renderer::drawGrid() const
{
    constexpr int gridSize = 40;
    constexpr float spacing = 1.0f;

    constexpr Color gridColor = {25, 25, 25, 255};

    const float halfSize =
        gridSize * spacing * 0.5f;

    for (int i = 0; i <= gridSize; ++i)
    {
        const float offset =
            -halfSize + i * spacing;

        DrawLine3D(
            {offset, 0.0f, -halfSize},
            {offset, 0.0f, halfSize},
            gridColor
        );

        DrawLine3D(
            {-halfSize, 0.0f, offset},
            {halfSize, 0.0f, offset},
            gridColor
        );
    }
}

void Renderer::draw(
    const Simulation& simulation
)
{
    const SolarSystem& solarSystem =
        simulation.getSolarSystem();

    if (!focusedBody)
    {
        const auto& bodies =
            solarSystem.getBodyPointers();

        if (!bodies.empty())
            focusedBody = bodies.front();
    }

    if (visuals.empty())
        buildVisuals(solarSystem);

    BeginMode3D(camera);

    drawGrid();

    for (const BodyVisual& visual : visuals)
    {
        const Color color =
            getBodyColor(*visual.body, visual.alpha);

        const int detail =
            sphereDetail(visual.screenRadius);

        DrawSphereEx(
            visual.position,
            static_cast<float>(visual.drawRadius),
            detail,
            detail,
            color
        );

        /*
         * A faint outline on marker-scaled bodies makes it obvious that
         * the dot is a stand-in rather than the real silhouette.
         */
        if (!visual.realScale && visual.alpha > 0.5f)
        {
            DrawSphereWires(
                visual.position,
                static_cast<float>(visual.drawRadius * 1.6),
                6,
                6,
                Color{
                    color.r,
                    color.g,
                    color.b,
                    static_cast<unsigned char>(60 * visual.alpha)
                }
            );
        }
    }

    EndMode3D();

    drawHud();

    /*
     * Cleared here so that a frame which never calls updateSelection
     * still rebuilds against fresh simulation state next time.
     */
    visuals.clear();
}

void Renderer::drawHud() const
{
    const char* focusName =
        focusedBody
            ? focusedBody->getName().c_str()
            : "None";

    DrawText(
        TextFormat(
            "RMB: Rotate | Wheel: Zoom | TAB: Focus next | Looking at: %s",
            focusName
        ),
        20,
        20,
        20,
        WHITE
    );

    if (!hoveredBody)
        return;

    const Body& body = *hoveredBody;

    const Body* parent = body.getParent();

    const OrbitalInfo orbitalInfo =
        calculateOrbitalInfo(body);

    int y = 60;

    DrawText(
        body.getName().c_str(),
        20,
        y,
        26,
        WHITE
    );

    y += 32;

    DrawText(
        parent
            ? TextFormat(
                  "Orbiting: %s",
                  parent->getName().c_str()
              )
            : "Orbiting: N/A",
        20,
        y,
        18,
        LIGHTGRAY
    );

    y += 25;

    /*
     * Report which regime the hovered body is being drawn in.
     */
    for (const BodyVisual& visual : visuals)
    {
        if (visual.body != hoveredBody)
            continue;

        DrawText(
            TextFormat(
                "Scale: %s (%.1f px)",
                visual.realScale ? "REAL" : "MARKER",
                visual.screenRadius
            ),
            20,
            y,
            18,
            visual.realScale ? GREEN : ORANGE
        );

        y += 25;
        break;
    }

    if (!orbitalInfo.valid)
        return;

    DrawText(
        TextFormat(
            "Energy: %.6e km^2/s^2",
            orbitalInfo.energy
        ),
        20, y, 18, LIGHTGRAY
    );

    y += 25;

    DrawText(
        TextFormat(
            "Altitude: %.2f km",
            orbitalInfo.altitude
        ),
        20, y, 18, LIGHTGRAY
    );

    y += 25;

    DrawText(
        TextFormat(
            "Periapsis: %.2f km",
            orbitalInfo.periapsis
        ),
        20, y, 18, LIGHTGRAY
    );

    y += 25;

    DrawText(
        orbitalInfo.hasApoapsis
            ? TextFormat(
                  "Apoapsis: %.2f km",
                  orbitalInfo.apoapsis
              )
            : "Apoapsis: N/A",
        20, y, 18, LIGHTGRAY
    );
}
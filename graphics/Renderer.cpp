#include "Renderer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>

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

/*
 * ----------------------------------------------------------------------
 * Procedural surface textures
 * ----------------------------------------------------------------------
 *
 * No texture assets are shipped or loaded from disk. Instead each body
 * gets a small equirectangular texture generated once, from a value-noise
 * fractal seeded from its name, and cached for the process lifetime
 * (Renderer::getOrCreateTexture). This is not trying to be accurate to
 * any real body's actual surface - it is there so a lit sphere reads as
 * "a planet" instead of a flat-colored ball, with every body still
 * visually distinct.
 */

std::uint32_t seedFromName(
    const std::string& name
)
{
    std::uint32_t hash = 2166136261u;

    for (char c : name)
    {
        hash ^= static_cast<unsigned char>(c);
        hash *= 16777619u;
    }

    return hash;
}

float hashNoise(
    int x,
    int y,
    std::uint32_t seed
)
{
    std::uint32_t h =
        static_cast<std::uint32_t>(x) * 374761393u +
        static_cast<std::uint32_t>(y) * 668265263u +
        seed * 2654435761u;

    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= (h >> 16);

    return (h & 0xFFFFFFu) / static_cast<float>(0xFFFFFFu);
}

float smoothNoise(
    float x,
    float y,
    std::uint32_t seed
)
{
    const int xi = static_cast<int>(std::floor(x));
    const int yi = static_cast<int>(std::floor(y));

    const float xf = x - static_cast<float>(xi);
    const float yf = y - static_cast<float>(yi);

    const float v00 = hashNoise(xi,     yi,     seed);
    const float v10 = hashNoise(xi + 1, yi,     seed);
    const float v01 = hashNoise(xi,     yi + 1, seed);
    const float v11 = hashNoise(xi + 1, yi + 1, seed);

    const float u = xf * xf * (3.0f - 2.0f * xf);
    const float v = yf * yf * (3.0f - 2.0f * yf);

    const float a = v00 + u * (v10 - v00);
    const float b = v01 + u * (v11 - v01);

    return a + v * (b - a);
}

/*
 * Fractal Brownian motion: several octaves of the noise above, each
 * higher-frequency and lower-amplitude than the last. This is what
 * turns smooth blobby noise into something that reads as terrain.
 */
float fbm(
    float x,
    float y,
    std::uint32_t seed,
    int octaves = 5
)
{
    float total = 0.0f;
    float amplitude = 0.5f;
    float frequency = 1.0f;
    float normalization = 0.0f;

    for (int i = 0; i < octaves; ++i)
    {
        total +=
            smoothNoise(
                x * frequency,
                y * frequency,
                seed + static_cast<std::uint32_t>(i) * 101u
            ) * amplitude;

        normalization += amplitude;

        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return total / normalization;
}

Color lerpColor(
    Color a,
    Color b,
    float t
)
{
    t = std::clamp(t, 0.0f, 1.0f);

    return Color{
        static_cast<unsigned char>(a.r + (b.r - a.r) * t),
        static_cast<unsigned char>(a.g + (b.g - a.g) * t),
        static_cast<unsigned char>(a.b + (b.b - a.b) * t),
        255
    };
}

/*
 * Terrestrial-looking bodies: continents/highlands above a threshold,
 * ocean-toned lowlands below it, and polar ice caps blended in near the
 * top and bottom of the texture regardless of the underlying terrain.
 */
Image buildRockyImage(
    Color base,
    std::uint32_t seed
)
{
    constexpr int width = 256;
    constexpr int height = 128;

    Color* pixels =
        static_cast<Color*>(
            RL_MALLOC(
                static_cast<size_t>(width) *
                static_cast<size_t>(height) *
                sizeof(Color)
            )
        );

    const Color highland = lerpColor(base, WHITE, 0.35f);
    const Color lowland  = base;
    const Color ocean    = lerpColor(base, Color{10, 30, 70, 255}, 0.6f);
    const Color ice      = Color{235, 240, 245, 255};

    for (int y = 0; y < height; ++y)
    {
        const float v = static_cast<float>(y) / height;
        const float latitude = (v - 0.5f) * 2.0f;

        for (int x = 0; x < width; ++x)
        {
            const float u = static_cast<float>(x) / width;

            const float elevation =
                fbm(u * 4.0f, v * 4.0f, seed, 5) * 0.7f +
                fbm(u * 11.0f, v * 11.0f, seed + 777u, 3) * 0.3f;

            Color color =
                elevation > 0.52f
                    ? lerpColor(
                          lowland,
                          highland,
                          (elevation - 0.52f) / 0.48f
                      )
                    : lerpColor(
                          ocean,
                          lowland,
                          elevation / 0.52f
                      );

            const float iceMask =
                std::max(
                    0.0f,
                    (std::abs(latitude) - 0.78f) / 0.22f
                );

            color = lerpColor(color, ice, iceMask);

            pixels[y * width + x] = color;
        }
    }

    Image image{};
    image.data = pixels;
    image.width = width;
    image.height = height;
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    return image;
}

/*
 * Gas giants: horizontal bands, gently warped so they aren't perfectly
 * straight, with a turbulence pass so the bands vary in width and blend
 * into each other rather than looking like flat stripes.
 */
Image buildGasGiantImage(
    Color base,
    std::uint32_t seed
)
{
    constexpr int width = 256;
    constexpr int height = 128;

    Color* pixels =
        static_cast<Color*>(
            RL_MALLOC(
                static_cast<size_t>(width) *
                static_cast<size_t>(height) *
                sizeof(Color)
            )
        );

    const Color bandLight = lerpColor(base, WHITE, 0.25f);
    const Color bandDark  = lerpColor(base, Color{40, 20, 10, 255}, 0.35f);

    for (int y = 0; y < height; ++y)
    {
        const float v = static_cast<float>(y) / height;

        for (int x = 0; x < width; ++x)
        {
            const float u = static_cast<float>(x) / width;

            const float warp =
                fbm(u * 6.0f, v * 20.0f, seed, 4) * 0.06f;

            const float band =
                std::sin(
                    (v + warp) * 3.14159265f * 14.0f
                ) * 0.5f + 0.5f;

            const float turbulence =
                fbm(u * 9.0f + 40.0f, v * 9.0f, seed + 555u, 4);

            Color color = lerpColor(bandLight, bandDark, band);
            color = lerpColor(color, base, 0.25f + 0.4f * turbulence);

            pixels[y * width + x] = color;
        }
    }

    Image image{};
    image.data = pixels;
    image.width = width;
    image.height = height;
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    return image;
}

/*
 * Airless bodies (moons, Mercury-like planets): mottled gray/brown
 * albedo variation with a scattering of stamped circular craters -
 * a bright rim, a slightly darkened floor.
 */
Image buildCrateredImage(
    Color base,
    std::uint32_t seed
)
{
    constexpr int width = 256;
    constexpr int height = 128;

    Color* pixels =
        static_cast<Color*>(
            RL_MALLOC(
                static_cast<size_t>(width) *
                static_cast<size_t>(height) *
                sizeof(Color)
            )
        );

    for (int y = 0; y < height; ++y)
    {
        const float v = static_cast<float>(y) / height;

        for (int x = 0; x < width; ++x)
        {
            const float u = static_cast<float>(x) / width;

            const float n = fbm(u * 10.0f, v * 10.0f, seed, 4);

            pixels[y * width + x] =
                lerpColor(
                    lerpColor(base, BLACK, 0.35f),
                    lerpColor(base, WHITE, 0.25f),
                    n
                );
        }
    }

    constexpr int craterCount = 22;

    for (int k = 0; k < craterCount; ++k)
    {
        const float cx = hashNoise(k, 0, seed) * width;
        const float cy = hashNoise(k, 1, seed) * height;
        const float radius = 4.0f + hashNoise(k, 2, seed) * 14.0f;

        const int minX = std::max(0, static_cast<int>(cx - radius - 2));
        const int maxX = std::min(width - 1, static_cast<int>(cx + radius + 2));
        const int minY = std::max(0, static_cast<int>(cy - radius - 2));
        const int maxY = std::min(height - 1, static_cast<int>(cy + radius + 2));

        for (int y = minY; y <= maxY; ++y)
        {
            for (int x = minX; x <= maxX; ++x)
            {
                const float dx = static_cast<float>(x) - cx;
                const float dy = static_cast<float>(y) - cy;
                const float d = std::sqrt(dx * dx + dy * dy);

                if (d > radius)
                    continue;

                const float rim =
                    std::abs(d - radius * 0.85f) < radius * 0.15f
                        ? 0.25f
                        : 0.0f;

                const float floorShade =
                    d < radius * 0.6f ? 0.15f : 0.0f;

                const float shade = rim - floorShade;

                Color& pixel = pixels[y * width + x];

                pixel =
                    lerpColor(
                        pixel,
                        shade > 0.0f ? WHITE : BLACK,
                        std::abs(shade)
                    );
            }
        }
    }

    Image image{};
    image.data = pixels;
    image.width = width;
    image.height = height;
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    return image;
}

/*
 * Stars: a granulated, self-luminous surface - fine bright/dark cells
 * with a few larger darker blotches standing in for starspots.
 */
Image buildStarImage(
    Color base,
    std::uint32_t seed
)
{
    constexpr int width = 256;
    constexpr int height = 128;

    Color* pixels =
        static_cast<Color*>(
            RL_MALLOC(
                static_cast<size_t>(width) *
                static_cast<size_t>(height) *
                sizeof(Color)
            )
        );

    const Color hot  = lerpColor(base, WHITE, 0.45f);
    const Color cool = lerpColor(base, BLACK, 0.25f);

    for (int y = 0; y < height; ++y)
    {
        const float v = static_cast<float>(y) / height;

        for (int x = 0; x < width; ++x)
        {
            const float u = static_cast<float>(x) / width;

            const float granulation =
                fbm(u * 24.0f, v * 12.0f, seed, 4);

            const float spots =
                fbm(u * 6.0f + 17.0f, v * 6.0f, seed + 303u, 3);

            const float t =
                granulation -
                std::max(0.0f, spots - 0.62f) * 1.5f;

            pixels[y * width + x] = lerpColor(cool, hot, t);
        }
    }

    Image image{};
    image.data = pixels;
    image.width = width;
    image.height = height;
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    return image;
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

Renderer::~Renderer()
{
    for (auto& [body, texture] : planetTextures)
    {
        (void)body;
        UnloadTexture(texture);
    }

    if (shaderReady)
    {
        /*
         * UnloadModel also releases the mesh it owns, so unitSphereMesh
         * must not be unloaded separately here.
         */
        UnloadModel(unitSphereModel);
        UnloadShader(planetShader);
    }
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
 * Surface shading
 * ======================================================================
 */

void Renderer::ensureShaderLoaded()
{
    if (shaderReady)
        return;

    planetShader =
        LoadShader(
            "shaders/planet.vs",
            "shaders/planet.fs"
        );

    /*
     * matModel and matNormal are not among raylib's automatically
     * resolved default uniforms, so their locations have to be looked
     * up and stored explicitly - once that's done, rlgl fills them in
     * for every DrawModel call without any further work here.
     */
    planetShader.locs[SHADER_LOC_MATRIX_MODEL] =
        GetShaderLocation(planetShader, "matModel");

    planetShader.locs[SHADER_LOC_MATRIX_NORMAL] =
        GetShaderLocation(planetShader, "matNormal");

    locLightPos = GetShaderLocation(planetShader, "lightPos");
    locViewPos = GetShaderLocation(planetShader, "viewPos");
    locTint = GetShaderLocation(planetShader, "tint");
    locAmbientStrength = GetShaderLocation(planetShader, "ambientStrength");
    locAtmosphereStrength = GetShaderLocation(planetShader, "atmosphereStrength");
    locAtmosphereColor = GetShaderLocation(planetShader, "atmosphereColor");

    /*
     * A single unit sphere, reused for every body: DrawModel's scale
     * parameter stretches it to each body's actual draw radius, so
     * there's no need to regenerate geometry as bodies grow or shrink
     * on screen.
     */
    unitSphereMesh = GenMeshSphere(1.0f, 64, 64);
    unitSphereModel = LoadModelFromMesh(unitSphereMesh);
    unitSphereModel.materials[0].shader = planetShader;

    shaderReady = true;
}

Renderer::SurfaceStyle Renderer::classifyStyle(
    const Body& body
) const
{
    /*
     * There's no per-body "type" or atmosphere data in the simulation
     * model to draw on, so style is inferred from what is available:
     * radius, and whether it orbits a planet rather than a star. This
     * is a coarse heuristic - if Body gains real classification data
     * (composition, atmosphere thickness, etc.) later, this is the
     * function to replace.
     */
    constexpr double gasGiantRadiusKm = 20000.0;
    constexpr double crateredRadiusKm = 3000.0;

    if (body.getMu() >= minimumStellarMu)
        return SurfaceStyle::Star;

    if (body.getRadius() >= gasGiantRadiusKm)
        return SurfaceStyle::GasGiant;

    if (isMoon(body) || body.getRadius() < crateredRadiusKm)
        return SurfaceStyle::Cratered;

    return SurfaceStyle::Rocky;
}

Texture2D& Renderer::getOrCreateTexture(
    const Body& body
)
{
    const auto existing = planetTextures.find(&body);

    if (existing != planetTextures.end())
        return existing->second;

    const std::uint32_t seed = seedFromName(body.getName());
    const Color base = getBodyColor(body, 1.0f);

    Image image;

    switch (classifyStyle(body))
    {
        case SurfaceStyle::GasGiant:
            image = buildGasGiantImage(base, seed);
            break;

        case SurfaceStyle::Cratered:
            image = buildCrateredImage(base, seed);
            break;

        case SurfaceStyle::Star:
            image = buildStarImage(base, seed);
            break;

        default:
            image = buildRockyImage(base, seed);
            break;
    }

    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    GenTextureMipmaps(&texture);
    SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);

    const auto [inserted, didInsert] =
        planetTextures.emplace(&body, texture);

    (void)didInsert;

    return inserted->second;
}

const Body* Renderer::findRootStar(
    const Body& body
) const
{
    /*
     * Walk up until we reach something star-sized (or run out of
     * parents). A planet finds its star; a star finds itself - which
     * matters in a galaxy-scale system, where the tree root is the
     * galactic centre and is not what lights anything.
     */
    const Body* current = &body;

    while (current->getMu() < minimumStellarMu && current->getParent())
        current = current->getParent();

    return current;
}

void Renderer::drawShadedBody(
    const BodyVisual& visual
)
{
    ensureShaderLoaded();

    const Body& body = *visual.body;

    Texture2D& texture = getOrCreateTexture(body);

    unitSphereModel.materials[0]
        .maps[MATERIAL_MAP_DIFFUSE]
        .texture = texture;

    const Body* star = findRootStar(body);
    const bool isStar = (star == &body);

    /*
     * A star lights itself: the exact direction doesn't matter because
     * ambientStrength is 1.0 for a star, which zeroes out the diffuse
     * term's contribution below - but the direction still has to be a
     * well-defined, non-zero vector or the shader's normalize() would
     * produce NaNs.
     */
    const Vector3 lightPosition =
        isStar
            ? Vector3{
                  visual.position.x + 1.0f,
                  visual.position.y,
                  visual.position.z
              }
            : toScene(star->getPosition());

    const Color color = getBodyColor(body, 1.0f);

    /*
     * White on purpose: the texture was already generated from the
     * body's color, so tinting by it again would square the color and
     * render everything darker than intended.
     */
    const float tint[3] = {1.0f, 1.0f, 1.0f};
    (void)color;

    const float lightPos[3] = {
        lightPosition.x,
        lightPosition.y,
        lightPosition.z
    };

    const float viewPos[3] = {
        camera.position.x,
        camera.position.y,
        camera.position.z
    };

    const float ambient = isStar ? 1.0f : 0.06f;

    const float atmosphereStrength =
        isStar
            ? 0.0f
            : classifyStyle(body) == SurfaceStyle::GasGiant
                  ? 0.35f
                  : classifyStyle(body) == SurfaceStyle::Cratered
                        ? 0.02f
                        : 0.22f;

    const float atmosphereColor[3] = {0.55f, 0.7f, 1.0f};

    SetShaderValue(planetShader, locLightPos, lightPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(planetShader, locViewPos, viewPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(planetShader, locTint, tint, SHADER_UNIFORM_VEC3);
    SetShaderValue(planetShader, locAmbientStrength, &ambient, SHADER_UNIFORM_FLOAT);
    SetShaderValue(planetShader, locAtmosphereStrength, &atmosphereStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(planetShader, locAtmosphereColor, atmosphereColor, SHADER_UNIFORM_VEC3);

    DrawModel(
        unitSphereModel,
        visual.position,
        static_cast<float>(visual.drawRadius),
        WHITE
    );
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
            maxCameraDistance
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
            maxCameraDistance
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
        /*
         * Lit, textured spheres are worth their cost only once a body
         * is both large enough on screen to show detail and already
         * drawn at its real physical size - a marker-scale dot a few
         * pixels wide gains nothing from a surface texture. Fading
         * bodies are left on the flat path too, since interpolating a
         * shaded material's alpha would need blending changes this
         * shader doesn't do.
         */
        const bool useShading =
            visual.realScale &&
            visual.screenRadius >= textureLodPixels &&
            visual.alpha >= 0.999f;

        if (useShading)
        {
            drawShadedBody(visual);
            continue;
        }

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
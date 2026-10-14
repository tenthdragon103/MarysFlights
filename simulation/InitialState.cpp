#include "InitialState.h"

#include "../simulation/Time.h"
#include "../config/ColorParser.h"

#include <cmath>
#include <stdexcept>
#include <string_view>

namespace
{
    constexpr double PI =
        3.1415926535897932384626433832795;

    constexpr double DEG_TO_RAD =
        PI / 180.0;

    constexpr std::string_view BODY_PREFIX =
        "Body.";
}

double InitialState::initialize(
    SolarSystem& solarSystem,
    const std::string& filename
)
{
    IniFile ini;

    if (!ini.load(filename))
    {
        throw std::runtime_error(
            "Failed to load INI file: " +
            filename
        );
    }

    solarSystem.clear();

    /*
     * Phase 1:
     *
     * Create every Body from the INI.
     */
    createBodiesFromIni(
        solarSystem,
        ini
    );

    if (solarSystem.getBodies().empty())
    {
        throw std::runtime_error(
            "INI contains no [Body.*] sections"
        );
    }

    /*
     * Phase 2:
     *
     * Resolve parent relationships.
     */
    resolveParents(
        solarSystem,
        ini
    );

    /*
     * Phase 3:
     *
     * Convert the orbital elements into
     * Cartesian states.
     */
    std::unordered_map<std::string, bool>
        initialized;

    std::unordered_map<std::string, bool>
        initializing;

    for (const auto& bodyPtr :
         solarSystem.getBodies())
    {
        initializeBody(
            solarSystem,
            *bodyPtr,
            ini,
            initialized,
            initializing
        );
    }

    /*
     * Phase 4:
     *
     * Shift the entire system into its
     * barycentric reference frame.
     */
    makeBarycentric(
        solarSystem
    );

    /*
     * Phase 5:
     *
     * Return the epoch as Julian Date.
     */
    return Time::julianDateFromTDB(
        ini.getString(
            "System",
            "epoch_tdb"
        )
    );
}

void InitialState::createBodiesFromIni(
    SolarSystem& solarSystem,
    const IniFile& ini
)
{
    for (const std::string& section :
         ini.getSections())
    {
        constexpr const char* prefix = "Body.";

        if (section.rfind(prefix, 0) != 0)
            continue;

        const std::string name =
            section.substr(5);

        if (name.empty())
        {
            throw std::runtime_error(
                "Invalid body section: " + section
            );
        }

        const double mass =
            ini.getDouble(section, "mass");

        const double radius =
            ini.getDouble(section, "radius");

        const double mu =
            ini.getDouble(section, "mu");

        if (mass <= 0.0)
        {
            throw std::runtime_error(
                "Body mass must be > 0: " + name
            );
        }

        if (radius <= 0.0)
        {
            throw std::runtime_error(
                "Body radius must be > 0: " + name
            );
        }

        if (mu <= 0.0)
        {
            throw std::runtime_error(
                "Body gravitational parameter must be > 0: " +
                name
            );
        }

        Body& body =
            solarSystem.createBody(
                name,
                mass,
                radius,
                mu
            );

        /*
         * Bodies are white by default.
         *
         * Only override the color if the INI
         * explicitly specifies one.
         */
        if (ini.has(section, "color"))
        {
            body.setColor(
                ColorParser::parse(
                    ini.getString(section, "color")
                )
            );
        }
    }
}

void InitialState::resolveParents(
    SolarSystem& solarSystem,
    const IniFile& ini
)
{
    for (const auto& bodyPtr :
         solarSystem.getBodies())
    {
        Body& body = *bodyPtr;

        const std::string section =
            "Body." + body.getName();

        const std::string parentName =
            ini.getString(section, "parent");

        if (parentName == "SYSTEM_BARYCENTER")
        {
            body.setParentType(
                ParentType::SystemBarycenter
            );

            continue;
        }

        Body* parent =
            solarSystem.findBody(parentName);

        if (parent == nullptr)
        {
            throw std::runtime_error(
                "Unknown parent '" +
                parentName +
                "' for body '" +
                body.getName() +
                "'"
            );
        }

        if (parent == &body)
        {
            throw std::runtime_error(
                "Body cannot be its own parent: " +
                body.getName()
            );
        }

        body.setParent(parent);
    }
}

void InitialState::initializeBody(
    SolarSystem& solarSystem,
    Body& body,
    const IniFile& ini,
    std::unordered_map<std::string, bool>& initialized,
    std::unordered_map<std::string, bool>& initializing
)
{
    const std::string name =
        body.getName();

    if (initialized[name])
        return;

    if (initializing[name])
    {
        throw std::runtime_error(
            "Circular parent relationship involving " +
            name
        );
    }

    initializing[name] = true;

    /*
     * A root body is referenced to the system
     * barycenter rather than another physical body.
     */
    if (body.getParentType() ==
        ParentType::SystemBarycenter)
    {
        const BarycentricState& barycenter =
            solarSystem.getBarycenter();

        body.setPosition(
            barycenter.position
        );

        body.setVelocity(
            barycenter.velocity
        );

        body.setAcceleration({});

        body.setHeading(
            Vec3(0.0, 0.0, 1.0)
        );

        body.setActive(true);

        initialized[name] = true;
        initializing[name] = false;

        return;
    }

    /*
     * Normal physical parent.
     */
    Body* parent =
        body.getParent();

    if (parent == nullptr)
    {
        throw std::runtime_error(
            "Body '" +
            name +
            "' has ParentType::Body but no parent"
        );
    }

    /*
     * Parent must be initialized before
     * calculating this body's state.
     */
    initializeBody(
        solarSystem,
        *parent,
        ini,
        initialized,
        initializing
    );

    const OrbitalElements elements =
        readOrbitalElements(
            ini,
            body
        );

    /*
     * Two-body relative motion uses the
     * combined gravitational parameter.
     */
    const double mu =
        parent->getMu() +
        body.getMu();

    const OrbitalState state =
        OrbitalElementsSolver::toCartesian(
            elements,
            mu
        );

    /*
     * Convert the relative state into
     * the system frame.
     */
    body.setPosition(
        parent->getPosition() +
        state.position
    );

    body.setVelocity(
        parent->getVelocity() +
        state.velocity
    );

    body.setAcceleration({});

    if (body.getVelocity().lengthSquared() > 0.0)
    {
        body.setHeading(
            body.getVelocity().normalized()
        );
    }

    body.setActive(true);

    initialized[name] = true;
    initializing[name] = false;
}

OrbitalElements InitialState::readOrbitalElements(
    const IniFile& ini,
    const Body& body
)
{
    const std::string section =
        "Body." + body.getName();

    const auto required =
        [&](const std::string& key)
        {
            if (!ini.has(section, key))
            {
                throw std::runtime_error(
                    "INI is missing [" +
                    section +
                    "]." +
                    key
                );
            }

            return ini.getDouble(
                section,
                key
            );
        };

    OrbitalElements elements;

    elements.semiMajorAxis =
        required("semi_major_axis");

    elements.eccentricity =
        required("eccentricity");

    elements.inclination =
        required("inclination") *
        DEG_TO_RAD;

    elements.longitudeOfAscendingNode =
        required(
            "longitude_of_ascending_node"
        ) *
        DEG_TO_RAD;

    elements.argumentOfPeriapsis =
        required(
            "argument_of_periapsis"
        ) *
        DEG_TO_RAD;

    elements.meanAnomaly =
        required("mean_anomaly") *
        DEG_TO_RAD;

    if (elements.semiMajorAxis <= 0.0)
    {
        throw std::runtime_error(
            "Invalid semi-major axis for " +
            body.getName()
        );
    }

    if (elements.eccentricity < 0.0 ||
        elements.eccentricity >= 1.0)
    {
        throw std::runtime_error(
            "Invalid eccentricity for " +
            body.getName()
        );
    }

    return elements;
}

void InitialState::makeBarycentric(
    SolarSystem& solarSystem
)
{
    double totalMass = 0.0;

    Vec3 centerOfMass{};
    Vec3 centerOfMassVelocity{};

    for (const auto& bodyPtr :
         solarSystem.getBodies())
    {
        Body* body = bodyPtr.get();

        if (!body->isActive())
            continue;

        const double mass =
            body->getMass();

        totalMass += mass;

        centerOfMass +=
            body->getPosition() *
            mass;

        centerOfMassVelocity +=
            body->getVelocity() *
            mass;
    }

    if (totalMass <= 0.0)
    {
        throw std::runtime_error(
            "Cannot barycentrically normalize empty system"
        );
    }

    centerOfMass /=
        totalMass;

    centerOfMassVelocity /=
        totalMass;

    for (const auto& bodyPtr :
         solarSystem.getBodies())
    {
        Body* body = bodyPtr.get();

        if (!body->isActive())
            continue;

        body->setPosition(
            body->getPosition() -
            centerOfMass
        );

        body->setVelocity(
            body->getVelocity() -
            centerOfMassVelocity
        );
    }
}
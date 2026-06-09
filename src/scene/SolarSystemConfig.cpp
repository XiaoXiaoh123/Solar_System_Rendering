#include "SolarSystemConfig.h"

#include "../utils/Constants.h"
#include "../utils/Paths.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

namespace {

struct ConfigSection {
    std::string name;
    std::map<std::string, std::string> values;
    int line = 0;
};

std::string trim(const std::string& value) {
    auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c) {
        return std::isspace(c);
    });
    auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) {
        return std::isspace(c);
    }).base();

    if (first >= last) return "";
    return std::string(first, last);
}

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

std::string stripComment(const std::string& line) {
    bool inQuotes = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '"') {
            inQuotes = !inQuotes;
        } else if (!inQuotes && (line[i] == '#' || line[i] == ';')) {
            return line.substr(0, i);
        }
    }
    return line;
}

std::string unquote(const std::string& value) {
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

const std::string* findValue(const ConfigSection& section, const std::string& key) {
    auto it = section.values.find(lower(key));
    return it == section.values.end() ? nullptr : &it->second;
}

std::string getString(const ConfigSection& section,
                      const std::string& key,
                      const std::string& fallback = "") {
    if (const std::string* value = findValue(section, key)) {
        return *value;
    }
    return fallback;
}

float parseFloat(const std::string& raw,
                 const std::string& key,
                 const ConfigSection& section) {
    try {
        std::size_t consumed = 0;
        float value = std::stof(raw, &consumed);
        if (consumed != raw.size()) {
            throw std::invalid_argument("trailing characters");
        }
        return value;
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid float for '" + key + "' in section [" +
                                 section.name + "] near line " +
                                 std::to_string(section.line) + ": " + raw);
    }
}

float getFloat(const ConfigSection& section,
               const std::string& key,
               float fallback = 0.0f) {
    if (const std::string* value = findValue(section, key)) {
        return parseFloat(*value, key, section);
    }
    return fallback;
}

bool parseBool(const std::string& raw,
               const std::string& key,
               const ConfigSection& section) {
    std::string value = lower(raw);
    if (value == "true" || value == "1" || value == "yes" || value == "on") return true;
    if (value == "false" || value == "0" || value == "no" || value == "off") return false;

    throw std::runtime_error("Invalid bool for '" + key + "' in section [" +
                             section.name + "] near line " +
                             std::to_string(section.line) + ": " + raw);
}

bool getBool(const ConfigSection& section,
             const std::string& key,
             bool fallback = false) {
    if (const std::string* value = findValue(section, key)) {
        return parseBool(*value, key, section);
    }
    return fallback;
}

glm::vec3 parseVec3(const std::string& raw,
                    const std::string& key,
                    const ConfigSection& section) {
    std::stringstream ss(raw);
    std::string part;
    float components[3] = {};

    for (int i = 0; i < 3; ++i) {
        if (!std::getline(ss, part, ',')) {
            throw std::runtime_error("Invalid vec3 for '" + key + "' in section [" +
                                     section.name + "] near line " +
                                     std::to_string(section.line) + ": " + raw);
        }
        components[i] = parseFloat(trim(part), key, section);
    }

    if (std::getline(ss, part, ',')) {
        throw std::runtime_error("Invalid vec3 for '" + key + "' in section [" +
                                 section.name + "] near line " +
                                 std::to_string(section.line) + ": " + raw);
    }

    return glm::vec3(components[0], components[1], components[2]);
}

glm::vec3 getVec3(const ConfigSection& section,
                  const std::string& key,
                  const glm::vec3& fallback) {
    if (const std::string* value = findValue(section, key)) {
        return parseVec3(*value, key, section);
    }
    return fallback;
}

BodyConfig::Type parseType(const std::string& raw, const ConfigSection& section) {
    std::string value = lower(raw);
    if (value == "star") return BodyConfig::Type::Star;
    if (value == "planet") return BodyConfig::Type::Planet;
    if (value == "moon" || value == "satellite") return BodyConfig::Type::Moon;

    throw std::runtime_error("Invalid body type in section [" + section.name +
                             "] near line " + std::to_string(section.line) +
                             ": " + raw);
}

std::vector<ConfigSection> parseSections(const std::string& path) {
    std::string resolvedPath = Paths::resolve(path);
    std::ifstream file(resolvedPath);
    if (!file.is_open()) {
        return {};
    }

    std::vector<ConfigSection> sections;
    ConfigSection* current = nullptr;
    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;
        line = trim(stripComment(line));
        if (line.empty()) continue;

        if (line.front() == '[' && line.back() == ']') {
            ConfigSection section;
            section.name = trim(line.substr(1, line.size() - 2));
            section.line = lineNumber;
            if (section.name.empty()) {
                throw std::runtime_error("Empty config section in " + path +
                                         " at line " + std::to_string(lineNumber));
            }
            sections.push_back(std::move(section));
            current = &sections.back();
            continue;
        }

        if (!current) {
            throw std::runtime_error("Config value outside a section in " + path +
                                     " at line " + std::to_string(lineNumber));
        }

        std::size_t separator = line.find('=');
        if (separator == std::string::npos) {
            throw std::runtime_error("Expected key=value in " + path +
                                     " at line " + std::to_string(lineNumber));
        }

        std::string key = lower(trim(line.substr(0, separator)));
        std::string value = unquote(trim(line.substr(separator + 1)));
        if (key.empty()) {
            throw std::runtime_error("Empty key in " + path +
                                     " at line " + std::to_string(lineNumber));
        }
        current->values[key] = value;
    }

    return sections;
}

BodyConfig makeConfig(const ConfigSection& section) {
    BodyConfig config;
    config.type = parseType(getString(section, "type", "planet"), section);

    config.params.name                = getString(section, "name", section.name);
    config.params.radius              = getFloat(section, "radius", 1.0f);
    config.params.orbitRadius         = getFloat(section, "orbitRadius", 0.0f);
    config.params.realRadiusKm        = getFloat(section, "realRadiusKm", 0.0f);
    config.params.semiMajorAxisAU     = getFloat(section, "semiMajorAxisAU", 0.0f);
    config.params.orbit.semiMajorAxis = config.params.orbitRadius;
    config.params.orbit.eccentricity  = getFloat(section, "eccentricity", 0.0f);
    config.params.orbit.inclination   = getFloat(section, "inclination", 0.0f);
    config.params.orbit.longitudeAscendingNode =
        getFloat(section, "longitudeAscendingNode", 0.0f);
    config.params.orbit.argumentPeriapsis =
        getFloat(section, "argumentPeriapsis", 0.0f);
    config.params.orbit.meanAnomalyAtEpoch =
        getFloat(section, "meanAnomalyAtEpoch", 0.0f);
    config.params.orbitPeriodDays     = getFloat(section, "orbitPeriodDays", 0.0f);
    config.params.rotationPeriodHours = getFloat(section, "rotationPeriodHours", 0.0f);
    config.params.axialTilt           = getFloat(section, "axialTilt", 0.0f);
    config.params.texturePath         = getString(section, "texturePath", "");
    config.params.hasAtmosphere       = getBool(section, "hasAtmosphere", false);
    config.params.atmosphereScale     = getFloat(section, "atmosphereScale", 1.05f);

    config.parentName = getString(section, "parent", "");
    config.drawOrbit  = getBool(section, "drawOrbit",
                                config.type != BodyConfig::Type::Star &&
                                config.params.orbitRadius > 0.0f);
    config.orbitColor = getVec3(section, "orbitColor", config.orbitColor);

    return config;
}

} // namespace

std::vector<BodyConfig> SolarSystemConfig::defaults() {
    auto makeParams = [](const std::string& name,
                         float radius,
                         float orbitRadius,
                         float realRadiusKm,
                         float semiMajorAxisAU,
                         float eccentricity,
                         float inclination,
                         float longitudeAscendingNode,
                         float argumentPeriapsis,
                         float meanAnomalyAtEpoch,
                         float orbitPeriodDays,
                         float rotationPeriodHours,
                         float axialTilt,
                         const std::string& texturePath,
                         bool hasAtmosphere = false,
                         float atmosphereScale = 1.05f) {
        CelestialParams p;
        p.name = name;
        p.radius = radius;
        p.orbitRadius = orbitRadius;
        p.realRadiusKm = realRadiusKm;
        p.semiMajorAxisAU = semiMajorAxisAU;
        p.orbit.semiMajorAxis = orbitRadius;
        p.orbit.eccentricity = eccentricity;
        p.orbit.inclination = inclination;
        p.orbit.longitudeAscendingNode = longitudeAscendingNode;
        p.orbit.argumentPeriapsis = argumentPeriapsis;
        p.orbit.meanAnomalyAtEpoch = meanAnomalyAtEpoch;
        p.orbitPeriodDays = orbitPeriodDays;
        p.rotationPeriodHours = rotationPeriodHours;
        p.axialTilt = axialTilt;
        p.texturePath = texturePath;
        p.hasAtmosphere = hasAtmosphere;
        p.atmosphereScale = atmosphereScale;
        return p;
    };

    return {
        {BodyConfig::Type::Star,
         makeParams("Sun", Constants::SUN_RADIUS, 0.0f, 695700.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, "assets/textures/sun.jpg"),
         "", false, glm::vec3(0.3f, 0.3f, 0.5f)},

        {BodyConfig::Type::Planet,
         makeParams("Mercury", Constants::MERCURY_RADIUS, Constants::MERCURY_ORBIT,
                    2439.7f, 0.387098f, 0.205630f, 7.005f, 48.331f,
                    29.124f, 174.796f, Constants::MERCURY_ORBIT_PERIOD,
                    Constants::MERCURY_ROT_PERIOD, Constants::MERCURY_TILT,
                    "assets/textures/mercury.jpg"),
         "", true, glm::vec3(0.34f, 0.32f, 0.38f)},

        {BodyConfig::Type::Planet,
         makeParams("Venus", Constants::VENUS_RADIUS, Constants::VENUS_ORBIT,
                    6051.8f, 0.723332f, 0.006772f, 3.394f, 76.680f,
                    54.922f, 50.115f, Constants::VENUS_ORBIT_PERIOD,
                    Constants::VENUS_ROT_PERIOD, Constants::VENUS_TILT,
                    "assets/textures/venus_surface.jpg", true, 1.055f),
         "", true, glm::vec3(0.48f, 0.40f, 0.30f)},

        {BodyConfig::Type::Planet,
         makeParams("Earth", Constants::EARTH_RADIUS, Constants::EARTH_ORBIT,
                    6371.0f, 1.000000f, 0.016708f, 0.000f, 0.0f,
                    102.937f, 357.517f, Constants::EARTH_ORBIT_PERIOD,
                    Constants::EARTH_ROT_PERIOD, Constants::EARTH_TILT,
                    "assets/textures/earth_daymap.jpg", true, 1.035f),
         "", true, glm::vec3(0.26f, 0.42f, 0.65f)},

        {BodyConfig::Type::Planet,
         makeParams("Mars", Constants::MARS_RADIUS, Constants::MARS_ORBIT,
                    3389.5f, 1.523679f, 0.093400f, 1.850f, 49.558f,
                    286.543f, 19.373f, Constants::MARS_ORBIT_PERIOD,
                    Constants::MARS_ROT_PERIOD, Constants::MARS_TILT,
                    "assets/textures/mars.jpg", true, 1.025f),
         "", true, glm::vec3(0.55f, 0.30f, 0.25f)},

        {BodyConfig::Type::Planet,
         makeParams("Jupiter", Constants::JUPITER_RADIUS, Constants::JUPITER_ORBIT,
                    69911.0f, 5.2044f, 0.0489f, 1.303f, 100.464f,
                    -86.133f, 20.020f, Constants::JUPITER_ORBIT_PERIOD,
                    Constants::JUPITER_ROT_PERIOD, Constants::JUPITER_TILT,
                    "assets/textures/jupiter.jpg"),
         "", true, glm::vec3(0.50f, 0.43f, 0.34f)},

        {BodyConfig::Type::Planet,
         makeParams("Saturn", Constants::SATURN_RADIUS, Constants::SATURN_ORBIT,
                    58232.0f, 9.5826f, 0.0565f, 2.485f, 113.665f,
                    -20.608f, 317.020f, Constants::SATURN_ORBIT_PERIOD,
                    Constants::SATURN_ROT_PERIOD, Constants::SATURN_TILT,
                    "assets/textures/saturn.jpg"),
         "", true, glm::vec3(0.58f, 0.52f, 0.38f)},

        {BodyConfig::Type::Planet,
         makeParams("Uranus", Constants::URANUS_RADIUS, Constants::URANUS_ORBIT,
                    25362.0f, 19.2184f, 0.0463f, 0.773f, 74.006f,
                    99.0f, 142.238f, Constants::URANUS_ORBIT_PERIOD,
                    Constants::URANUS_ROT_PERIOD, Constants::URANUS_TILT,
                    "assets/textures/uranus.jpg"),
         "", true, glm::vec3(0.35f, 0.55f, 0.60f)},

        {BodyConfig::Type::Planet,
         makeParams("Neptune", Constants::NEPTUNE_RADIUS, Constants::NEPTUNE_ORBIT,
                    24622.0f, 30.110f, 0.0090f, 1.770f, 131.784f,
                    -83.660f, 256.228f, Constants::NEPTUNE_ORBIT_PERIOD,
                    Constants::NEPTUNE_ROT_PERIOD, Constants::NEPTUNE_TILT,
                    "assets/textures/neptune.jpg"),
         "", true, glm::vec3(0.30f, 0.42f, 0.70f)},

        {BodyConfig::Type::Moon,
         makeParams("Moon", Constants::MOON_RADIUS, Constants::MOON_ORBIT,
                    1737.4f, 0.00257f, 0.0549f, 5.145f, 125.08f,
                    318.15f, 135.27f, Constants::MOON_ORBIT_PERIOD,
                    Constants::MOON_ROT_PERIOD, 0.0f,
                    "assets/textures/moon.jpg"),
         "Earth", true, glm::vec3(0.4f, 0.4f, 0.5f)}
    };
}

std::vector<BodyConfig> SolarSystemConfig::load(const std::string& path) {
    std::vector<ConfigSection> sections = parseSections(path);
    if (sections.empty()) {
        std::cerr << "Celestial config not found or empty: " << path
                  << ". Using built-in defaults." << std::endl;
        return defaults();
    }

    std::vector<BodyConfig> configs;
    configs.reserve(sections.size());
    for (const ConfigSection& section : sections) {
        configs.push_back(makeConfig(section));
    }
    return configs;
}

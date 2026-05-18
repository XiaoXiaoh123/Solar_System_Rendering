#pragma once

namespace Constants {

// --- Window ---
constexpr int   DEFAULT_WIDTH       = 1920;
constexpr int   DEFAULT_HEIGHT      = 1080;
constexpr float DEFAULT_FOV         = 45.0f;
constexpr float NEAR_PLANE          = 0.1f;
constexpr float FAR_PLANE           = 10000.0f;

// --- Time ---
// 1 real second = DAYS_PER_SECOND simulation Earth-days (at timeScale 1.0)
constexpr float DAYS_PER_SECOND     = 30.0f;
constexpr float SECONDS_PER_DAY     = 86400.0f;
constexpr float DEFAULT_TIME_SCALE  = 3.0f;
constexpr float MIN_TIME_SCALE      = 0.0f;
constexpr float MAX_TIME_SCALE      = 10.0f;

// --- Camera ---
constexpr float CAM_SPEED           = 50.0f;
constexpr float CAM_SPEED_FAST      = 250.0f;
constexpr float CAM_SENSITIVITY    = 0.1f;

// --- Sun ---
constexpr float SUN_RADIUS          = 80.0f;

// --- Planet distances & sizes (artistic scaled values) ---
constexpr float MERCURY_ORBIT       = 100.0f;
constexpr float MERCURY_RADIUS      = 1.2f;

constexpr float VENUS_ORBIT         = 140.0f;
constexpr float VENUS_RADIUS        = 2.8f;

constexpr float EARTH_ORBIT         = 180.0f;
constexpr float EARTH_RADIUS        = 3.0f;

constexpr float MARS_ORBIT          = 220.0f;
constexpr float MARS_RADIUS         = 1.8f;

constexpr float JUPITER_ORBIT       = 320.0f;
constexpr float JUPITER_RADIUS      = 24.0f;

constexpr float SATURN_ORBIT        = 400.0f;
constexpr float SATURN_RADIUS       = 20.0f;

constexpr float URANUS_ORBIT        = 480.0f;
constexpr float URANUS_RADIUS       = 12.0f;

constexpr float NEPTUNE_ORBIT       = 540.0f;
constexpr float NEPTUNE_RADIUS      = 11.5f;

// --- Moon ---
constexpr float MOON_ORBIT          = 7.0f;
constexpr float MOON_RADIUS         = 0.8f;
constexpr float MOON_ORBIT_PERIOD   = 27.32f;
constexpr float MOON_ROT_PERIOD     = 27.32f;  // tidally locked

// --- Real orbital periods (Earth days) ---
constexpr float MERCURY_ORBIT_PERIOD  = 87.97f;
constexpr float VENUS_ORBIT_PERIOD    = 224.70f;
constexpr float EARTH_ORBIT_PERIOD    = 365.25f;
constexpr float MARS_ORBIT_PERIOD     = 686.98f;
constexpr float JUPITER_ORBIT_PERIOD  = 4332.59f;
constexpr float SATURN_ORBIT_PERIOD   = 10759.22f;
constexpr float URANUS_ORBIT_PERIOD   = 30688.5f;
constexpr float NEPTUNE_ORBIT_PERIOD  = 60182.0f;

// --- Real rotation periods (hours, negative = retrograde) ---
constexpr float MERCURY_ROT_PERIOD  = 1407.5f;   // 58.6 days
constexpr float VENUS_ROT_PERIOD    = -5832.5f;  // -243 days (retrograde)
constexpr float EARTH_ROT_PERIOD    = 23.9345f;
constexpr float MARS_ROT_PERIOD     = 24.6229f;
constexpr float JUPITER_ROT_PERIOD  = 9.925f;
constexpr float SATURN_ROT_PERIOD   = 10.656f;
constexpr float URANUS_ROT_PERIOD   = -17.24f;   // retrograde
constexpr float NEPTUNE_ROT_PERIOD  = 16.11f;

// --- Axial tilts (degrees) ---
constexpr float MERCURY_TILT  = 0.03f;
constexpr float VENUS_TILT    = 2.64f;
constexpr float EARTH_TILT    = 23.44f;
constexpr float MARS_TILT     = 25.19f;
constexpr float JUPITER_TILT  = 3.13f;
constexpr float SATURN_TILT   = 26.73f;
constexpr float URANUS_TILT   = 97.77f;
constexpr float NEPTUNE_TILT  = 28.32f;

// --- Mesh ---
constexpr int   ORBIT_SEGMENTS      = 256;
constexpr int   SPHERE_SECTORS      = 64;
constexpr int   SPHERE_STACKS       = 32;

// --- Lighting ---
constexpr float AMBIENT_STRENGTH    = 0.08f;

} // namespace Constants

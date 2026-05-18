#pragma once

namespace Constants {

// --- Window ---
constexpr int   DEFAULT_WIDTH       = 1920;
constexpr int   DEFAULT_HEIGHT      = 1080;
constexpr float DEFAULT_FOV         = 45.0f;
constexpr float NEAR_PLANE          = 0.1f;
constexpr float FAR_PLANE           = 10000.0f;

// --- Time ---
constexpr float DEFAULT_TIME_SCALE  = 10.0f;   // 10x speed
constexpr float MIN_TIME_SCALE      = 0.0f;    // paused
constexpr float MAX_TIME_SCALE      = 1000.0f;

// --- Camera ---
constexpr float CAM_SPEED           = 50.0f;
constexpr float CAM_SPEED_FAST      = 250.0f;
constexpr float CAM_SENSITIVITY    = 0.1f;
constexpr float CAM_ZOOM_MIN       = 1.0f;
constexpr float CAM_ZOOM_MAX       = 90.0f;

// --- Solar System Scaling ---
// Distances are compressed by DIST_SCALE, body sizes are enlarged by SIZE_SCALE
constexpr float DIST_SCALE          = 1.0f / 500000.0f;  // 1 AU → ~300 units (real: ~150M km)
constexpr float SIZE_SCALE          = 500.0f;            // body radius multiplier
constexpr float SUN_EXTRA_SCALE     = 0.2f;              // sun rendered at 20% of scaled size
// Real data scaled:
// Sun radius:   696340 km → 696340 * DIST_SCALE * SIZE_SCALE * 0.2 ≈ 139 units
// Use explicit scaled values for better artistic control:

// --- Sun ---
constexpr float SUN_RADIUS          = 80.0f;

// --- Planets (scaled artistic values) ---
constexpr float MERCURY_ORBIT       = 100.0f;
constexpr float MERCURY_RADIUS      = 1.2f;
constexpr float MERCURY_ORBIT_SPEED = 4.74f;

constexpr float VENUS_ORBIT         = 140.0f;
constexpr float VENUS_RADIUS        = 2.8f;
constexpr float VENUS_ORBIT_SPEED   = 3.50f;

constexpr float EARTH_ORBIT         = 180.0f;
constexpr float EARTH_RADIUS        = 3.0f;
constexpr float EARTH_ORBIT_SPEED   = 2.98f;

constexpr float MARS_ORBIT          = 220.0f;
constexpr float MARS_RADIUS         = 1.8f;
constexpr float MARS_ORBIT_SPEED    = 2.41f;

constexpr float JUPITER_ORBIT       = 320.0f;
constexpr float JUPITER_RADIUS      = 24.0f;
constexpr float JUPITER_ORBIT_SPEED = 1.31f;

constexpr float SATURN_ORBIT        = 400.0f;
constexpr float SATURN_RADIUS       = 20.0f;
constexpr float SATURN_ORBIT_SPEED  = 0.97f;

constexpr float URANUS_ORBIT        = 480.0f;
constexpr float URANUS_RADIUS       = 12.0f;
constexpr float URANUS_ORBIT_SPEED  = 0.68f;

constexpr float NEPTUNE_ORBIT       = 540.0f;
constexpr float NEPTUNE_RADIUS      = 11.5f;
constexpr float NEPTUNE_ORBIT_SPEED = 0.54f;

// --- Orbit ring resolution ---
constexpr int   ORBIT_SEGMENTS      = 256;

// --- Sphere mesh subdivision ---
constexpr int   SPHERE_SECTORS      = 64;
constexpr int   SPHERE_STACKS       = 32;

// --- Lighting ---
constexpr float AMBIENT_STRENGTH    = 0.08f;
constexpr float SUN_LIGHT_COLOR[]   = {1.0f, 0.95f, 0.8f};

} // namespace Constants

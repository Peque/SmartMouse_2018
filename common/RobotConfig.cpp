#include "RobotConfig.h"

#if defined(SIM)
const RobotConfig config = {
        FRONT_ANALOG_ANGLE : 1.35255,
        BACK_ANALOG_ANGLE : 1.35255,
        FRONT_SIDE_ANALOG_X : 0.045,
        FRONT_SIDE_ANALOG_Y : 0.030,
        BACK_SIDE_ANALOG_X : -0.024,
        BACK_SIDE_ANALOG_Y : 0.030,
        FRONT_ANALOG_X : 0.055,
        MAX_SPEED : 0.18,
        MIN_SPEED : 0.018,
        FRONT_WALL_THRESHOLD : 0.15,
        SIDE_WALL_THRESHOLD : 0.08,
        WALL_CHANGED_THRESHOLD : 0.02,
        ROT_TOLERANCE : 0.14,
        TRACK_WIDTH : 0.0633,
        ANALOG_MAX_DIST : 0.18,
        MAX_FORCE : 0.013,
};
#elif defined(CONSOLE)

#else // REAL
const RobotConfig config = {
        FRONT_ANALOG_ANGLE : 1.35255,
        BACK_ANALOG_ANGLE : 1.35255,
        FRONT_SIDE_ANALOG_X : 0.042,
        FRONT_SIDE_ANALOG_Y : 0.0350,
        BACK_SIDE_ANALOG_X : -0.0206,
        BACK_SIDE_ANALOG_Y : 0.0335,
        FRONT_ANALOG_X : 0.056,
        MAX_SPEED : 0.18,
        MIN_SPEED : 0.023,
        FRONT_WALL_THRESHOLD : 0.15,
        SIDE_WALL_THRESHOLD : 0.08,
        WALL_CHANGED_THRESHOLD : 0.01,
        ROT_TOLERANCE : 0.14,
        TRACK_WIDTH : 0.0633,
        ANALOG_MAX_DIST : 0.18,
        MAX_FORCE : 0.016,
};
#endif
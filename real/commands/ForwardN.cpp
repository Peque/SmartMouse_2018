#include <real/RealMouse.h>
#include "ForwardN.h"

ForwardN::ForwardN(unsigned int n) : Command("Forward"), mouse(RealMouse::inst()), n(n) {}


void ForwardN::initialize() {
  start = mouse->getGlobalPose();
  mouse->kinematic_controller.enable_sensor_pose_estimate = true;
  const double goal_disp = KinematicController::dispToNthEdge(*mouse, n);
  const double v0 = mouse->kinematic_controller.getCurrentForwardSpeedCUPS();
  const double vf = smartmouse::kc::kVf;
  drive_straight_state = new smartmouse::kc::DriveStraightState(start, goal_disp, v0, vf);
  digitalWrite(RealMouse::LED_4, 1);
}

void ForwardN::execute() {
  double l, r;
  double t_s = static_cast<double>(getTime()) / 1000.0;
  std::tie(l, r) = drive_straight_state->compute_wheel_velocities(*mouse, t_s);
  mouse->setSpeedCps(l, r);
}

bool ForwardN::isFinished() {
  return drive_straight_state->dispError() <= 0;
}

void ForwardN::end() {
  digitalWrite(RealMouse::LED_4, 0);
  mouse->kinematic_controller.enable_sensor_pose_estimate = false;
}


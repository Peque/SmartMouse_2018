#include <common/math/math.h>
#include <sim/commands/TurnInPlace.h>

const double TurnInPlace::kP = 0.5;

TurnInPlace::TurnInPlace(Direction dir) : Command("SimTurnInPlace"), mouse(SimMouse::inst()), dir(dir) {}

void TurnInPlace::initialize() {
  goal_yaw = dir_to_yaw(dir);
  mouse->kinematic_controller.enable_sensor_pose_estimate = false;
  const auto start = mouse->getGlobalPose();
  yaw_error = smartmouse::math::yaw_diff(start.yaw, goal_yaw);
  const double goal_disp = fabs(yaw_error) * smartmouse::kc::TRACK_WIDTH_CU / 2;
  double left_v_cps, right_v_cps;
  std::tie(left_v_cps, right_v_cps) = mouse->kinematic_controller.getWheelVelocitiesCPS();
  const double vf = 0;
  std::cout << left_v_cps << ", " << right_v_cps << "\n";
  left_profile = new smartmouse::kc::VelocityProfile(start, goal_disp, left_v_cps, vf);
  right_profile = new smartmouse::kc::VelocityProfile(start, goal_disp, right_v_cps, vf);
}

void TurnInPlace::execute() {
  double t_s = getTime() / 1000.0;
  double v_l = left_profile->compute_forward_velocity(t_s);
  double v_r = right_profile->compute_forward_velocity(t_s);
  mouse->setSpeedCps(v_l, v_r);

  // when we get close to aligned, there might be a wall we can use to better estimate our angle
  // this allows us to use that
  if (fabs(yaw_error) < smartmouse::kc::ROT_TOLERANCE * 4 && mouse->kinematic_controller.enable_sensor_pose_estimate) {
    mouse->kinematic_controller.enable_sensor_pose_estimate = true;
    // FIXME: this is kind of a hack. It's needed because DriveStraight checks dir in order to compute
    // FIXME: the correct yaw. it adds dir_to_yaw(getDir()), so we must assume we're close enough
    mouse->internalTurnToFace(dir);
  }
}

bool TurnInPlace::isFinished() {
  double current_yaw = mouse->getGlobalPose().yaw;
  yaw_error = smartmouse::math::yaw_diff(current_yaw, goal_yaw);
  double vl_cps, vr_cps;
  std::tie(vl_cps, vr_cps) = mouse->getWheelVelocitiesCPS();
  return (fabs(yaw_error) < smartmouse::kc::ROT_TOLERANCE) && fabs(vl_cps) <= smartmouse::kc::MIN_SPEED_CUPS
      && fabs(vr_cps) < smartmouse::kc::MIN_SPEED_CUPS;
}

void TurnInPlace::end() {
  mouse->internalTurnToFace(dir);
  mouse->kinematic_controller.enable_sensor_pose_estimate = true;
}


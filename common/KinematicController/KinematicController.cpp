#include <cmath>
#include <algorithm>
#include <iostream>
#include <tuple>
#include <common/math/math.h>

#include <common/core/Mouse.h>
#include <common/KinematicController/KinematicController.h>


const double KinematicController::kDropSafety = 0.8;

KinematicController::KinematicController(Mouse *mouse)
    : enable_sensor_pose_estimate(true), enabled(true), kinematics_enabled(true), initialized(false),
      ignoring_left(false), ignoring_right(false), mouse(mouse),
      d_until_left_drop(0), d_until_right_drop(0) {
  current_pose_estimate_cu.col = 0;
  current_pose_estimate_cu.row = 0;
  current_pose_estimate_cu.yaw = 0;
}

GlobalPose KinematicController::getGlobalPose() {
  return current_pose_estimate_cu;
}

LocalPose KinematicController::getLocalPose() {
  LocalPose local_pose_estimate;
  local_pose_estimate.yaw_from_straight =
      smartmouse::math::yaw_diff(dir_to_yaw(mouse->getDir()), current_pose_estimate_cu.yaw);
  switch (mouse->getDir()) {
    case Direction::N: {
      local_pose_estimate.to_back = ceil(current_pose_estimate_cu.row) - current_pose_estimate_cu.row;
      local_pose_estimate.to_left = ceil(current_pose_estimate_cu.col) - current_pose_estimate_cu.col;
      break;
    }
    case Direction::S: {
      local_pose_estimate.to_back = current_pose_estimate_cu.row - floor(current_pose_estimate_cu.row);
      local_pose_estimate.to_left = current_pose_estimate_cu.col - floor(current_pose_estimate_cu.col);
      break;
    }
    case Direction::E: {
      local_pose_estimate.to_back = current_pose_estimate_cu.col - floor(current_pose_estimate_cu.col);
      local_pose_estimate.to_left = current_pose_estimate_cu.row - floor(current_pose_estimate_cu.row);
      break;
    }
    case Direction::W: {
      local_pose_estimate.to_back = ceil(current_pose_estimate_cu.col) - current_pose_estimate_cu.col;
      local_pose_estimate.to_left = ceil(current_pose_estimate_cu.row) - current_pose_estimate_cu.row;
      break;
    }
    default: exit(-1);
  }
  return local_pose_estimate;
}

std::pair<double, double> KinematicController::getWheelVelocitiesCPS() {
  std::pair<double, double> vels;
  vels.first = smartmouse::kc::radToCU(left_motor.velocity_rps);
  vels.second = smartmouse::kc::radToCU(right_motor.velocity_rps);
  return vels;
};

bool KinematicController::isStopped() {
  return left_motor.isStopped() && left_motor.isStopped();
}

void KinematicController::reset_col_to(double new_col) {
  current_pose_estimate_cu.col = new_col;
}

void KinematicController::reset_row_to(double new_row) {
  current_pose_estimate_cu.row = new_row;
}

void KinematicController::reset_yaw_to(double new_yaw) {
  current_pose_estimate_cu.yaw = new_yaw;
}

std::pair<double, double>
KinematicController::run(double dt_s, double left_angle_rad, double right_angle_rad, RangeData range_data) {
  static std::pair<double, double> abstract_forces;

  if (!initialized) {
    initialized = true;
    abstract_forces.first = 0;
    abstract_forces.second = 0;
    return abstract_forces;
  }

  // save the cycle time so other functions can use is
  // this prevents us from passing dt all over the place
  this->dt_s = dt_s;

  if (enabled) {
    if (kinematics_enabled) {
      // equations based on docs/dynamics_model.pdf
      double vl_cu = smartmouse::kc::radToCU(left_motor.velocity_rps);
      double vr_cu = smartmouse::kc::radToCU(right_motor.velocity_rps);

      GlobalPose d_pose_cu;
      d_pose_cu = forwardKinematics(vl_cu, vr_cu, current_pose_estimate_cu.yaw, dt_s);
      current_pose_estimate_cu.col += d_pose_cu.col;
      current_pose_estimate_cu.row += d_pose_cu.row;
      current_pose_estimate_cu.yaw += d_pose_cu.yaw;
      smartmouse::math::wrapAngleRadInPlace(&current_pose_estimate_cu.yaw);

      row = (unsigned int) (current_pose_estimate_cu.row);
      col = (unsigned int) (current_pose_estimate_cu.col);

      // given odometry estimate, improve estimate using sensors, then update odometry estimate to match our best estimate
      // offset_cu is measured from center wall line left of the robot (it's in the local pose frame)
      double est_yaw, offset_m;
      bool no_walls;
      std::tie(est_yaw, offset_m, no_walls) = estimate_pose(range_data, *mouse);
      double offset_cu = smartmouse::maze::toCellUnits(offset_m);

      if (enable_sensor_pose_estimate && !no_walls) {
        current_pose_estimate_cu.yaw = est_yaw;

        double d_wall_front_cu = 0;
        bool wall_in_front = false;
        if (range_data.front < 0.08) {
          double yaw_error = smartmouse::math::yaw_diff(current_pose_estimate_cu.yaw, dir_to_yaw(mouse->getDir()));
          double d_wall_front_m = cos(yaw_error) * range_data.front + smartmouse::kc::FRONT_ANALOG_X;
          d_wall_front_cu = smartmouse::maze::toCellUnits(d_wall_front_m);
          wall_in_front = true;
        }

        switch (mouse->getDir()) {
          case Direction::N: {
            current_pose_estimate_cu.col = col + offset_cu;
            if (wall_in_front) {
              current_pose_estimate_cu.row = row + d_wall_front_cu + smartmouse::maze::HALF_WALL_THICKNESS_CU;
            }
            break;
          }
          case Direction::S: {
            current_pose_estimate_cu.col = col + 1 - offset_cu;
            if (wall_in_front) {
              current_pose_estimate_cu.row = row + 1 - d_wall_front_cu - smartmouse::maze::HALF_WALL_THICKNESS_CU;
            }
            break;
          }
          case Direction::E: {
            current_pose_estimate_cu.row = row + offset_cu;
            if (wall_in_front) {
              current_pose_estimate_cu.col = col + 1 - d_wall_front_cu - smartmouse::maze::HALF_WALL_THICKNESS_CU;
            }
            break;
          }
          case Direction::W: {
            current_pose_estimate_cu.row = row + 1 - offset_cu;
            if (wall_in_front) {
              current_pose_estimate_cu.col = col + d_wall_front_cu + smartmouse::maze::HALF_WALL_THICKNESS_CU;
            }
            break;
          }
          default: break;
        }
      }
    }

    // run PID, which will update the velocities of the wheels
    abstract_forces.first = left_motor.runPid(dt_s, left_angle_rad);
    abstract_forces.second = right_motor.runPid(dt_s, right_angle_rad);
  } else {
    abstract_forces.first = 0;
    abstract_forces.second = 0;
  }

  return abstract_forces;
}

GlobalPose KinematicController::forwardKinematics(double vl_cups, double vr_cups, double yaw_rad, double dt) {
  double dcol_cu, drow_cu, dtheta_rad;
  double dyawdt = (vl_cups - vr_cups) / smartmouse::kc::TRACK_WIDTH_CU;
  double R = smartmouse::kc::TRACK_WIDTH_CU * (vr_cups + vl_cups) / (2 * (vl_cups - vr_cups));

  if (fabs(vl_cups) < 1e-5 && fabs(vr_cups) < 1e-5) {
    // this means we're stopped, so ignore it
    dcol_cu = 0;
    drow_cu = 0;
    dtheta_rad = 0;
  } else if (fabs(vl_cups - vr_cups) < 1e-5) {
    // going perfectly straight is a special condition
    dcol_cu = dt * (vr_cups + vl_cups) / 2 * cos(yaw_rad);
    drow_cu = dt * (vr_cups + vl_cups) / 2 * sin(yaw_rad);
    dtheta_rad = 0;
  } else {
    double dtheta_about_icc = dyawdt * dt; //eq 11
    dcol_cu = R * (sin(dtheta_about_icc + yaw_rad) - sin(yaw_rad)); //eq 28
    drow_cu = -R * (cos(dtheta_about_icc + yaw_rad) - cos(yaw_rad)); //eq 29
    dtheta_rad = dyawdt * dt;
  }

  return GlobalPose(dcol_cu, drow_cu, dtheta_rad);
}

std::tuple<double, double, bool> KinematicController::estimate_pose(RangeData range_data, Mouse &mouse) {
  static double last_front_left_analog_dist;
  static double last_front_right_analog_dist;
  static double last_back_left_analog_dist;
  static double last_back_right_analog_dist;
  std::tuple<double, double, bool> newest_estimate;

  double *yaw = &std::get<0>(newest_estimate);
  double *offset = &std::get<1>(newest_estimate);
  bool *ignore_walls = &std::get<2>(newest_estimate);

  double d1x_l = cos(smartmouse::kc::BACK_ANALOG_ANGLE) * range_data.back_left + smartmouse::kc::BACK_SIDE_ANALOG_X;
  double d2x_l = cos(smartmouse::kc::FRONT_ANALOG_ANGLE) * range_data.front_left + smartmouse::kc::FRONT_SIDE_ANALOG_X;
  double d1y_l = sin(smartmouse::kc::BACK_ANALOG_ANGLE) * range_data.back_left + smartmouse::kc::BACK_SIDE_ANALOG_Y;
  double d2y_l = sin(smartmouse::kc::FRONT_ANALOG_ANGLE) * range_data.front_left + smartmouse::kc::FRONT_SIDE_ANALOG_Y;
  double currentYaw_l = atan2(d2y_l - d1y_l, d2x_l - d1x_l);

  double d1x_r = cos(smartmouse::kc::BACK_ANALOG_ANGLE) * range_data.back_right + smartmouse::kc::BACK_SIDE_ANALOG_X;
  double d2x_r = cos(smartmouse::kc::FRONT_ANALOG_ANGLE) * range_data.front_right + smartmouse::kc::FRONT_SIDE_ANALOG_X;
  double d1y_r = sin(smartmouse::kc::BACK_ANALOG_ANGLE) * range_data.back_right + smartmouse::kc::BACK_SIDE_ANALOG_Y;
  double d2y_r = sin(smartmouse::kc::FRONT_ANALOG_ANGLE) * range_data.front_right + smartmouse::kc::FRONT_SIDE_ANALOG_Y;
  double currentYaw_r = -atan2(d2y_r - d1y_r, d2x_r - d1x_r);

  double d_to_wall_left = (d2x_l * d1y_l - d2y_l * d1x_l) / sqrt(pow(d2y_l - d1y_l, 2) + pow(d2x_l - d1x_l, 2));
  double d_to_wall_right = (d2x_r * d1y_r - d2y_r * d1x_r) / sqrt(pow(d2y_r - d1y_r, 2) + pow(d2x_r - d1x_r, 2));
  bool sense_left_wall = range_data.front_left < smartmouse::kc::SIDE_WALL_THRESHOLD &&
      range_data.back_left < smartmouse::kc::SIDE_WALL_THRESHOLD;
  bool sense_right_wall = range_data.front_right < smartmouse::kc::SIDE_WALL_THRESHOLD &&
      range_data.back_right < smartmouse::kc::SIDE_WALL_THRESHOLD;


  // check for walls that will fall off in the near future (geralds!)
  if (range_data.gerald_left > smartmouse::kc::GERALD_WALL_THRESHOLD) {
    d_until_left_drop = kDropSafety * tan(smartmouse::kc::GERALD_ANGLE) * d_to_wall_left;
    sense_left_wall = false;
  }

  if (range_data.gerald_right > smartmouse::kc::GERALD_WALL_THRESHOLD) {
    d_until_right_drop = kDropSafety * tan(smartmouse::kc::GERALD_ANGLE) * d_to_wall_right;
    sense_right_wall = false;
  }

  // this logic checks for walls that are "falling off" or "falling on"
  // if the change in sensor distance is above some threshold, the wall is arriving or leaving
  // so we don't yet follow the wall
  double d_back_left = fabs(range_data.back_left - last_back_left_analog_dist);
  double d_back_right = fabs(range_data.back_right - last_back_right_analog_dist);
  if (d_back_left > smartmouse::kc::WALL_CHANGED_THRESHOLD) {
    sense_left_wall = false;
  }
  if (d_back_right > smartmouse::kc::WALL_CHANGED_THRESHOLD) {
    sense_right_wall = false;
  }

  double d_front_left = fabs(range_data.front_left - last_front_left_analog_dist);
  double d_front_right = fabs(range_data.front_right - last_front_right_analog_dist);
  if (d_front_left > smartmouse::kc::WALL_CHANGED_THRESHOLD) {
    sense_left_wall = false;
  }
  if (d_front_right > smartmouse::kc::WALL_CHANGED_THRESHOLD) {
    sense_right_wall = false;
  }

  // consider the "logical" state of walls AND actual range reading
  if (sense_right_wall && mouse.isWallInDirection(right_of_dir(mouse.getDir()))) { // wall is on right
    *offset = smartmouse::maze::UNIT_DIST_M - d_to_wall_right - smartmouse::maze::HALF_WALL_THICKNESS_M;
    *yaw = dir_to_yaw(mouse.getDir()) + currentYaw_r;
    *ignore_walls = false;
  } else if (sense_left_wall && mouse.isWallInDirection(left_of_dir(mouse.getDir()))) { // wall is on left
    *offset = d_to_wall_left + smartmouse::maze::HALF_WALL_THICKNESS_M;
    *yaw = dir_to_yaw(mouse.getDir()) + currentYaw_l;
    *ignore_walls = false;
  } else { // we're too far from any walls, use our pose estimation
    *ignore_walls = true;
  }

  last_front_left_analog_dist = range_data.front_left;
  last_front_right_analog_dist = range_data.front_right;
  last_back_left_analog_dist = range_data.back_left;
  last_back_right_analog_dist = range_data.back_right;

  return newest_estimate;
};

void KinematicController::setAccelerationCpss(double acceleration_cpss) {
  this->acceleration_cellpss = acceleration_cpss;
  left_motor.setAccelerationCpss(acceleration_cpss);
  right_motor.setAccelerationCpss(acceleration_cpss);
}

void KinematicController::setSpeedCps(double left_setpoint_cps,
                                      double right_setpoint_cps) {
  left_motor.setSetpointCps(left_setpoint_cps);
  right_motor.setSetpointCps(right_setpoint_cps);
}

void KinematicController::planTraj(Waypoints waypoints) {
  TrajectoryPlanner planner(waypoints);
  Eigen::Matrix<double, 10, 1> plan = planner.plan();
}

double distance_needed(const double v_0, const double v_f, const double a_0) {
  constexpr double j_mag = 20;
  const double b = a_0;
  const double c = v_0 - v_f;
  double j = -j_mag;
  // if we are accelerating, j should be positive
  if (v_f > v_0) {
    j = j_mag;
  }
  const double a = 0.5 * j;
  const double t = (-b - sqrt(std::pow(b, 2) - 4 * a * c)) / (2 * a);
  return v_0 * t + 1 / 2 * a_0 * std::pow(t, 2) + 1 / 6 * j * std::pow(t, 3);
}

double KinematicController::fwdDisp(Direction dir, GlobalPose current_pose, GlobalPose start_pose) {
  switch (dir) {
    case Direction::N: return start_pose.row - current_pose.row;
    case Direction::E: return current_pose.col - start_pose.col;
    case Direction::S: return current_pose.row - start_pose.row;
    case Direction::W: return start_pose.col - current_pose.col;
    default: return std::numeric_limits<double>::quiet_NaN();
  }
}

double KinematicController::dispToNextEdge(Mouse &mouse) {
  GlobalPose current_pose = mouse.getGlobalPose();
  Direction dir = mouse.getDir();

  switch (dir) {
    case Direction::N: {
      return current_pose.row - mouse.getRow();
    }
    case Direction::S: {
      return mouse.getRow() + 1 - current_pose.row;
    }
    case Direction::E: {
      return mouse.getCol() + 1 - current_pose.col;
    }
    case Direction::W: {
      return current_pose.col - mouse.getCol();
    }
    default: return std::numeric_limits<double>::quiet_NaN();
  }
}

double KinematicController::dispToNthEdge(Mouse &mouse, unsigned int n) {
  // give the displacement to the nth edge like above...
  return dispToNextEdge(mouse) + (n - 1);
}

GlobalPose KinematicController::poseOfToNthEdge(Mouse &mouse, unsigned int n) {
  // give the displacement to the nth edge like above...
  double disp = dispToNthEdge(mouse, n);
  GlobalPose pose = mouse.getGlobalPose();
  pose.col += cos(pose.yaw) * disp;
  pose.row += sin(pose.yaw) * disp;

  return pose;
}

double KinematicController::sidewaysDispToCenter(Mouse &mouse) {
  // local y is sideways, increasing from left to right
  return mouse.getLocalPose().to_left - 0.5;
}

double KinematicController::fwdDispToCenter(Mouse &mouse) {
  return 0.5 - mouse.getLocalPose().to_back;
}

void KinematicController::setParams(double kP, double kI, double kD, double ff_scale, double ff_offset) {
  left_motor.setParams(kP, kI, kD, ff_scale, ff_offset);
  right_motor.setParams(kP, kI, kD, ff_scale, ff_offset);
}

double KinematicController::getCurrentForwardSpeedCUPS() {
  return smartmouse::kc::radToCU((left_motor.velocity_rps + right_motor.velocity_rps)/2);
}

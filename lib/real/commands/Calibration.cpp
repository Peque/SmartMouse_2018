#include "Calibration.h"

Calibration::Calibration() :  Command("calibrate"), mouse(RealMouse::inst()) {}

void Calibration::initialize(){
  mouse->display.setTextSize(1);
  mouse->display.setTextColor(WHITE);
}

void Calibration::execute(){
  mouse->display.clearDisplay();

  uint8_t system = 0, gyro = 0, accel = 0, mag = 0;
  mouse->imu.getCalibration(&system, &gyro, &accel, &mag);
  digitalWrite(RealMouse::LEDG, (system == 3));
  digitalWrite(RealMouse::LEDB, (mag == 3));
  mouse->display.println("IMU Calibration");
  mouse->display.print("Sys ");
  mouse->display.println(system);
  mouse->display.print("Gyro ");
  mouse->display.println(gyro);
  mouse->display.print("Accel ");
  mouse->display.println(accel);
  mouse->display.print("Mag ");
  mouse->display.println(mag);

  mouse->display.setCursor(0, 0);
  mouse->display.display();
}

bool Calibration::isFinished(){
  return mouse->goButton.fell();
}

void Calibration::end(){
  digitalWrite(RealMouse::LEDG, 0);
  digitalWrite(RealMouse::LEDB, 0);
}

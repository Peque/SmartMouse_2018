#pragma once

#include <Arduino.h>
#include "Mouse.h"
#include "AbstractMaze.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Wire.h>
#include <Encoder.h>
#include <RegulatedMotor.h>
#include "KinematicController.h"
#include <SparkFun_VL6180X.h>
#include <math.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class RealMouse : public Mouse {
public:
  /** runs setup things like pin initializes */
  void setup();

  const static int BUTTONGO = 44;

  RealMouse(AbstractMaze *maze);

  virtual SensorReading sense() override;

private:
  const static int OLED_RESET = 5;
  const static int VL6180X_ADDRESS = 0x29;

  const static int IREMITTER1 = A1;
  const static int IREMITTER2 = A4;
  const static int IREMITTER3 = A6;
  const static int IREMITTER4 = A9;
  const static int IREMITTER5 = A11;
  const static int IREMITTER6 = CANRX;

  const static int IRRECEIVER1 = A2;
  const static int IRRECEIVER2 = A5;
  const static int IRRECEIVER3 = A7;
  const static int IRRECEIVER4 = A10;
  const static int IRRECEIVER5 = DAC1;
  const static int IRRECEIVER6 =
      A3; // wired to non adc pin. this does not work.

  const static int BATTERYSENSE = A0;

  const static int VL6180EN1 = 22;
  const static int VL6180EN2 = 23;
  const static int VL6180EN3 = 24;
  const static int VL6180EN4 = 25;
  const static int VL6180EN5 = 26;
  const static int VL6180EN6 = 27;

  const static int ENCODER1A = 3;
  const static int ENCODER1B = 29;
  const static int ENCODER2A = 2;
  const static int ENCODER2B = 28;

  const static int MOTOR1A = 9;  // PWML4 PC21
  const static int MOTOR1B = 11; //
  const static int MOTOR2A = 10;
  const static int MOTOR2B = 12;

  const static int MOTORDIR1 = 30;
  const static int MOTORDIR2 = 31;

  const static int LEDGO = 13;
  const static int LEDR = 52;
  const static int LEDG = 41;
  const static int LEDB = 51;
  const static int BUZZER = 46;

  const static int BUTTON1 = 48;
  const static int BUTTON2 = 50;

  const static int SDCS = 4;
  const static int SDCARDDETECT = 42;

  Adafruit_SSD1306 display;

  /*
      static const unsigned char PROGMEM logo16_glcd_bmp[]; =
      { B00000000, B11000000,
        B00000001, B11000000,
        B00000001, B11000000,
        B00000011, B11100000,
        B11110011, B11100000,
        B11111110, B11111000,
        B01111110, B11111111,
        B00110011, B10011111,
        B00011111, B11111100,
        B00001101, B01110000,
        B00011011, B10100000,
        B00111111, B11100000,
        B00111111, B11110000,
        B01111100, B11110000,
        B01110000, B01110000,
        B00000000, B00110000 };
  */
  VL6180xIdentification identification;
  VL6180x sensor;

  Adafruit_BNO055 bno;

  Encoder encL;
  Encoder encR;

  long encLCount;
  long encRCount;

  RegulatedMotor motL;
  RegulatedMotor motR;

  KinematicController kc;

  int fwdtarget;
  int ccwtarget;
};

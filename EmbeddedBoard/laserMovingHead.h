#ifndef LASERMOVINGHEAD_H
#define LASER_MOVING_HEAD_H
#include <Servo.h>    // Add this for servo control!
#include <Arduino.h>  // Fix: Include this for LOW, HIGH, pinMode, etc.


#define LASER_PIN 7// 7
#define SERVO_X_PIN 5
#define SERVO_Y_PIN 6


// #define LOW 0
// #define HIGH 1

class LaserMovingHead {
public:
  void setup();
  void update(int pattern, float bpm, unsigned long currentTime);
  void moveServo(int xAngle, int yAngle);

private:
  Servo servoX, servoY;
  int laserPin;

  // Servo movement limits
  const int minX = 45;    // Min angle X servo
  const int maxX = 135;  // Max angle X servo
  const int minY = 45;    // Min angleY servo
  const int maxY = 135;  // Max angle Y servo

  int laserState = LOW;
  unsigned long lastBlinkTime = 0;

  void controlLaserBlink(float bpm, unsigned long currentTime);
};
#endif

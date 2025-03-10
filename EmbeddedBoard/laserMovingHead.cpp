#include "LaserMovingHead.h"
#include <Servo.h>  // ✅ Include for servo control!
#include <math.h>
#include <Arduino.h>  // ✅ Include for Arduino functions (pinMode, digitalWrite, etc.)
// #include <iostream>

// If `PI` is missing, define it
#ifndef PI
#define PI 3.14159265358979323846
#endif

unsigned long lastMoveTime = 0;
int centerAngle = 90;

void LaserMovingHead::setup() {
  pinMode(LASER_PIN, OUTPUT);
  servoX.attach(SERVO_X_PIN);
  servoY.attach(SERVO_Y_PIN);
  digitalWrite(LASER_PIN, LOW);
  moveServo(centerAngle, centerAngle);  // Center position
}

void LaserMovingHead::update(int pattern, float bpm, unsigned long currentTime) {
  controlLaserBlink(bpm, currentTime);
  // Serial.println(pattern);
  switch (pattern) {
    case 0:  // done
      {
        int sx = centerAngle + 70 * sin(currentTime * 0.002);
        int sy = centerAngle + 30 * cos(currentTime * 0.002);
        moveServo(sx, sy);
        // Serial.print(currentTime);
        // Serial.print(" : ");
        // Serial.print((centerAngle + 70 * sin(currentTime * 0.002)));
        // Serial.print(" : ");
        // Serial.println((centerAngle + 30 * cos(currentTime * 0.002)));
      }
      break;

    case 1:                            // done
      {                                // BASS_SPIKES (Random jumps)
        if (currentTime % 250 < 50) {  // Random jump every 500ms
          moveServo(random(minX, maxX), random(minY, maxY));
          // Serial.print(currentTime);
          // Serial.print(" : ");
          // Serial.print((random(minX, maxX)));
          // Serial.print(" : ");
          // Serial.println((random(minY, maxY)));
        }
      }
      break;

    case 2:  // done // SPIRAL_EXPANSION (Expanding spiral) //
      {
        float timeFactor = (currentTime % 10000) / 10000.0;  // Normalize time in 10s (0 to 1 )
        float angle = timeFactor * 2 * PI * 5;               // 5 full rotations in (2 * PI = 10s 360) 
        float radius = timeFactor * 90;                      // Expand from 0 to 90 within 10s

        // Map to servo range (90 ± 90 -> 0 to 180)
        float servoX = centerAngle + radius * cos(angle);
        float servoY = centerAngle + radius * sin(angle);

        moveServo(servoX, servoY);

        // Serial.print(currentTime % 10000);
        // Serial.print(" : ");
        // Serial.print(servoX);
        // Serial.print(" : ");
        // Serial.println(servoY);
      }
      break;

    case 3:  // done  // BPM_SCANNING (Speed adapts to BPM)
      {
        float speedFactor = bpm / 60.0;
        int circleSize = 40;
        float moreTime = 0.002;
        moveServo(centerAngle + circleSize * sin(currentTime * moreTime * speedFactor),
                  centerAngle + circleSize * cos(currentTime * moreTime * speedFactor));
        // Serial.print(currentTime);
        // Serial.print(" : ");
        // Serial.print((90 + circleSize * sin(currentTime * moreTime * speedFactor)));
        // Serial.print(" : ");
        // Serial.println((90 + circleSize * cos(currentTime * moreTime * speedFactor)));
      }
      break;

    case 4:  //done // STARBURST (Radial burst effect)
      {
        static float burstAngle = 0;
        static bool returning = false;

        if (currentTime - lastMoveTime >= 700) {  // trigger a new burst Every 700 ms
          lastMoveTime = currentTime;

          if (!returning) {
            // Burst outward in a new direction
            burstAngle = random(0, 360) * (PI / 180.0);  // Random direction and convert degrees to radians
            returning = true;
          } else {
            // Return to the center
            returning = false;
          }
        }

        // Calculate position based on the current phase
        int radius = returning ? 10 : 50;  // Go far (50) then come back (10)
        int sx = centerAngle + radius * cos(burstAngle);
        int sy = centerAngle + radius * sin(burstAngle);

        moveServo(sx, sy);

        // ----------------------------------- Debug output -----------------------------------
        // Serial.print(currentTime);
        // Serial.print(" : ");
        // Serial.print(sx);
        // Serial.print(" : ");
        // Serial.println(sy);
      }
      break;

    case 5:  // FIGURE_8_MOTION (Infinity loop)
      {
        float t = (currentTime % 4000) / 4000.0 * 2 * PI;
        int radiusSize = 45;
        moveServo(centerAngle + radiusSize * sin(t), centerAngle + radiusSize * sin(2 * t));
        // Serial.print(currentTime);
        // Serial.print(" : ");
        // Serial.print(centerAngle + radiusSize * sin(t));
        // Serial.print(" : ");
        // Serial.println(centerAngle + radiusSize * sin(2 * t));
      }
      break;

    case 6:                        // done                          // RANDOM_SWIRL
      lastMoveTime = currentTime;  // Update last move time
      int radiusSizeX = 60;
      int radiusSizeY = 30;

      float timefactor = bpm / 160.0;  // Reduce speed by increasing divisor
      int speedX = centerAngle + radiusSizeX * sin(currentTime * timefactor * 0.005); 
      int angleY = centerAngle + radiusSizeY * cos(currentTime * timefactor * 0.0025);  

      moveServo(speedX, angleY);

      // Debugging Output
      // Serial.print(currentTime);
      // Serial.print(" : ");
      // Serial.print(speedX);
      // Serial.print(" : ");
      // Serial.println(angleY);
  }
}

void LaserMovingHead::moveServo(int xAngle, int yAngle) {
  servoX.write(constrain(xAngle, minX, maxX));
  servoY.write(constrain(yAngle, minY, maxY));

  // Store the last movement time
  // lastMoveTime = millis();
}


void LaserMovingHead::controlLaserBlink(float bpm, unsigned long currentTime) {
  // unsigned long interval = 60000 / bpm / 2;  // Half-beat duration
  unsigned long interval = 60000 / bpm / 2 / 2;  // 2Half-beat duration
  if (currentTime - lastBlinkTime >= interval) {
    // Serial.print(!laserState);
    laserState = !laserState;
    digitalWrite(LASER_PIN, laserState);
    lastBlinkTime = currentTime;
  }
}

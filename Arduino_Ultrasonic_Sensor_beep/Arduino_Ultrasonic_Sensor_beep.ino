#include <Smartcar.h>
#include <TimerOne.h>
#include <TimerThree.h>

#define Debug Serial
#define Control Serial3

#define SPEED 240
#define INDEFINITELY 0

#define ENABLE_TIMER1 false
#define ENABLE_TIMER3 true

/*
 * Front proximity sensor
 */
bool FrontProximityEnabled = false;
int FrontProximityValue;
const int FrontProximityMax = 40;
SR04 FrontProximity;
const int FRONT_PROXIMITY_TRIGGER = 6;
const int FRONT_PROXIMITY_ECHO = 5;

/*
 * Rear proximity sensor
 */
bool RearProximityEnabled = false;
int RearProximityValue;
const int RearProximityMax = 40;
SR04 RearProximity;
const int REAR_PROXIMITY_TRIGGER = 24;
const int REAR_PROXIMITY_ECHO = 26;

/*
 * Infrared sensor
 */
bool InfraredEnabled = false;
int InfraredValue;
GP2Y0A21 Infrared;
const int INFRARED = A0;

/*
 * Gyroscope
 */
const int GYRO = 10;
Gyroscope Gyro(GYRO);

/*
 * Motors
 */
DCMotors Motors(STANDARD);

/*
 * Lights
 */
const int LIGHT_RIGHT = 30;
const int LIGHT_LEFT = 46;
const int LIGHT_FRONT = 28;

/*
 * Speaker
 */
const int SPEAKER_GND = A10;
const int SPEAKER_PIN = A12;

/*
 * Turning status
 */
bool TurningLeft = false;
bool TurningRight = false;

/**
 * Get cycles of counter for switching speaker and lights
 * when a proximity sensor is enabled
 */
int get_cycles(int distance) {
  if (distance < 30) return 16;
  if (distance < 35) return 8;
  if (distance < 40) return 4;
  return 1;
}

/*
 * Timer 1: Currently not used
 */
int timer1_counter = 0;
const int timer1_max = 1000;
const int timer1_us = 1000;
void timer1_func() {
  /*
   * Increase counter for this interrupt call
   */
  timer1_counter++;

  // Not used

  /*
   * Limit counter
   */
  if (timer1_counter == timer1_max) timer1_counter = 0;
}

/*
 * Timer 2: Control of lights and speaker
 */
int timer3_counter = 0;
const int timer3_max = 1000;
const int timer3_us = 1000;
void timer3_func() {
  /*
   * Increase counter for this interrupt call
   */
  timer3_counter++;

  if (TurningLeft || TurningRight) {
    /*
     * Indicate a left turn
     */
    if (TurningLeft) {
      if (timer3_counter % (timer3_max / 4) == 0) {
        digitalWrite(LIGHT_LEFT, !digitalRead(LIGHT_LEFT));
      }
    }

    /*
     * Indicate a right turn
     */
    if (TurningRight) {
      if (timer3_counter % (timer3_max / 4) == 0) {
        digitalWrite(LIGHT_RIGHT, !digitalRead(LIGHT_RIGHT));
      }
    }
  } else if (RearProximityEnabled) {
    /*
     * Indicate rear proximity
     */
    if (RearProximityValue > 0 && RearProximityValue < RearProximityMax) {
      if (timer3_counter % (timer3_max / get_cycles(RearProximityValue)) == 0) {
        digitalWrite(LIGHT_RIGHT, !digitalRead(LIGHT_RIGHT));
        digitalWrite(LIGHT_LEFT, !digitalRead(LIGHT_LEFT));
        digitalWrite(SPEAKER_PIN, !digitalRead(SPEAKER_PIN));
      }
    } else {
      digitalWrite(LIGHT_RIGHT, LOW);
      digitalWrite(LIGHT_LEFT, LOW);
      digitalWrite(SPEAKER_PIN, LOW);
    }
  } else if (FrontProximityEnabled) {
    /*
     * Indicate front proximity
     */
    if (FrontProximityValue > 0 && FrontProximityValue < FrontProximityMax) {
      if (timer3_counter % (timer3_max / get_cycles(FrontProximityValue)) == 0) {
        digitalWrite(LIGHT_RIGHT, !digitalRead(LIGHT_RIGHT));
        digitalWrite(LIGHT_LEFT, !digitalRead(LIGHT_LEFT));
        digitalWrite(SPEAKER_PIN, !digitalRead(SPEAKER_PIN));
      }
    } else {
      digitalWrite(LIGHT_RIGHT, LOW);
      digitalWrite(LIGHT_LEFT, LOW);
      digitalWrite(SPEAKER_PIN, LOW);
    }
  } else {
    /*
     * Default to disabled
     */
    digitalWrite(LIGHT_LEFT, LOW);
    digitalWrite(LIGHT_RIGHT, LOW);
    digitalWrite(SPEAKER_PIN, LOW);
  }

  /*
   * Limit counter
   */
  if (timer3_counter == timer3_max) timer3_counter = 0;
}

/**
 * Configure
 */
void setup() {
  /*
   * Serial ports
   */
  Debug.begin(9600);
  Control.begin(9600);

  /*
   * Motors
   */
  Motors.init();

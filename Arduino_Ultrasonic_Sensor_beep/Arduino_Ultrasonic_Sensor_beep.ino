#include <Smartcar.h>
#include <TimerOne.h>
#include <TimerThree.h>

#define Debug Serial
#define Control Serial3

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
 * Speed
 */
int Speed = 240;
 
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
bool InfraredEnabled = true;
int InfraredValue;
const int InfraredValueMin = 36;
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
    if (FrontProximityValue > 0 && FrontProximityValue < FrontProximityMax ) {
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
  /*
   * Proximity
   * Sensors declared and attached to the car
   */
  FrontProximity.attach(FRONT_PROXIMITY_TRIGGER, FRONT_PROXIMITY_ECHO);
  RearProximity.attach(REAR_PROXIMITY_TRIGGER, REAR_PROXIMITY_ECHO);
  Infrared.attach(INFRARED);
  //pinMode(InfraredValueMin, INPUT);

  /*
   * Gyroscope attached
   */
  Gyro.attach();
  Gyro.begin();

  /*
   * Lights attached
   * Initializing the pins for the lights
   */
  pinMode(LIGHT_RIGHT, OUTPUT);
  pinMode(LIGHT_LEFT, OUTPUT);
  pinMode(LIGHT_FRONT, OUTPUT);

  /*
   * Speaker
   * Initializing the pins for the speaker
   */
  pinMode(SPEAKER_GND, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);

  digitalWrite(SPEAKER_GND, LOW);
  digitalWrite(SPEAKER_PIN, LOW);

  /*
   * Timer 1
   */
  if (ENABLE_TIMER1) {
    Timer1.initialize(timer1_us);
    Timer1.attachInterrupt(timer1_func);
    Timer1.start();
  }

  /*
   * Timer 3
   */
  if (ENABLE_TIMER3) {
    Timer3.initialize(timer3_us);
    Timer3.attachInterrupt(timer3_func);
    Timer3.start();
  }
}

/**
 * Update sensor values
 */
void tick() { 
  FrontProximityValue = FrontProximity.getDistance();
  RearProximityValue = RearProximity.getDistance();
  InfraredValue = Infrared.getDistance();

  if (FrontProximityEnabled && (FrontProximityValue > 0 && FrontProximityValue < 20)) stop();
  //if (FrontProximityEnabled && (InfraredValue > InfraredValueMin)) stop();
  if (RearProximityEnabled && (RearProximityValue > 0 && RearProximityValue < 20)) stop();
  
}

/**
 * Wait for next byte from control port
 */
char next() {
  while (!Control.available()) {
    tick();
  }

  return Control.read();
}

/**
 * Handle commands
 */
void loop() {  
  switch (next()) {
    /*
    * Speed
    */
    case '7': Speed = 30; break;
    case '8': Speed = 70; break;
    case '9': Speed = 240; break;
    /*
     * Move
     */
    case 'm':
      switch (next()) {
        case 'f': moveForward(); break;
        case 'b': moveBackward(); break;
      }
      break;

    /*
     * Turn
     */
    case 't':
      switch (next()) {
        case 'l': turnLeft(next()); break;
        case 'r': turnRight(next()); break;
      }
      break;

    /*
     * Stop
     */
    case 's': stop(); break;

    /*
     * Toggle
     */
    case 'o':
      switch (next()) {
        case 'f': toggleFrontProximity(next() == '1'); break;
        case 'r': toggleRearProximity(next() == '1'); break;
        case 'i': toggleInfrared(next() == '1'); break;
        case 'l': toggleFrontLight(next() == '1'); break;
      }
      break;
  }
}

/**
 * Move forward
 */
void moveForward() {
  Motors.setMotorSpeed(Speed, Speed);
}

/**
 * Move backward
 */
void moveBackward() {
  Motors.setMotorSpeed(-Speed, -Speed);
}

/**
 * Wait until the specified angular displacement is met
 */
void waitUntilAngularDisplacement(int angle) {
  int initial = Gyro.getAngularDisplacement();
  int value;

  do {
    value = abs(initial - Gyro.getAngularDisplacement());
    Gyro.update();
  } while (value < angle);
}

/**
 * Turn left the specified angle or INDEFINITELY
 */
void turnLeft(int angle) {
  TurningLeft = true;

  if (angle == INDEFINITELY) {
    Motors.setMotorSpeed(Speed / 14, Speed);
  } else {
    Motors.setMotorSpeed(-Speed, Speed);
    waitUntilAngularDisplacement(angle);
    stop();
  }
}

/**
 * Turn right the specified angle or INDEFINITELY
 */
void turnRight(int angle) {
  TurningRight = true;

  if (angle == INDEFINITELY) {
    Motors.setMotorSpeed(Speed, Speed / 14);
  } else {
    Motors.setMotorSpeed(Speed, -Speed);
    waitUntilAngularDisplacement(angle);
    stop();
  }
}

/**
 * Stop all motors
 */
void stop() {
  Motors.setMotorSpeed(0, 0);

  TurningRight = false;
  TurningLeft = false;
}

/**
 * Toggle front proximity sensor
 */
void toggleFrontProximity(bool status) {
  FrontProximityEnabled = status;
}

/**
 * Toggle rear proximity sensor
 */
void toggleRearProximity(bool status) {
  RearProximityEnabled = status;
}

/**
 * Toggle infrared sensor
 */
void toggleInfrared(bool status) {
  InfraredEnabled = status;
}

/**
 * Toggle front light
 */
void toggleFrontLight(bool status) {
  digitalWrite(LIGHT_FRONT, status ? HIGH : LOW);
}

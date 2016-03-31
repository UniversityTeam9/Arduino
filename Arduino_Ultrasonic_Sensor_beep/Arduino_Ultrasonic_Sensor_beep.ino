#include <Smartcar.h>
#include <Wire.h>
#include <Servo.h>


Car car;

const int fSpeed = 90; //70% of the full speed forward
const int bSpeed = -20; //70% of the full speed backward
const int lDegrees = -75; //degrees to turn left
const int rDegrees = 75; //degrees to turn right
SR04 ultrasonicSensor;
const int TRIGGER_PIN = 6; //D6
const int ECHO_PIN = 5; //D5
SR04 ultrasonicSensor2;
const int TRIGGER_PIN2 = 24; //D24
const int ECHO_PIN2 = 26;
GP2Y0A21 infraRedSensor;
const int INFRA_PIN = A0; //analog pin A0
const int tonePin = 7; //tone pin
boolean sensorBack = false;
boolean playSound = false;
int beepSpeed = 0;

void setup() {
  Serial3.begin(9600);
  Serial.begin(9600);
  Serial.println("w forward :) ");
  Serial.println("s backward :) ");
  Serial.println("a left :) ");
  Serial.println("d right :)");
  Serial.println("to stop pres any different key :)");
  ultrasonicSensor.attach(TRIGGER_PIN, ECHO_PIN);
  ultrasonicSensor2.attach(TRIGGER_PIN2, ECHO_PIN2);
  infraRedSensor.attach(INFRA_PIN);
  pinMode(tonePin, OUTPUT);
  car.begin(); //initialize the car using the encoders and the gyro
}

void loop() {
  handleInput();
  if (playSound == true){
    digitalWrite(7, HIGH);
    delay(beepSpeed);             
    digitalWrite(7, LOW);    
    delay(beepSpeed); 
  }
}

void handleInput() { //handle serial input if there is any
  if (Serial3.available()) {
    char input = Serial3.read(); //read everything that has been received so far and log down the last entry
   // char input = Serial3.read();
    switch (input) {
      case 'a': //rotate counter-clockwise going forward
        car.setSpeed(fSpeed);
        car.setAngle(lDegrees);
        sensorBack = false;
        break;
      case 'd': //turn clock-wise
        car.setSpeed(fSpeed);
        car.setAngle(rDegrees);
        sensorBack = false;
        break;
      case 'w': //go forward
        car.setSpeed(fSpeed);
        car.setAngle(0);
        sensorBack = false;
        break;
      case  's': //go back
        car.setSpeed(bSpeed);
        car.setAngle(0);
        sensorBack = true;
        break;
      default: //if you receive any key different stop.
        car.setSpeed(0);
        car.setAngle(0);
        sensorBack = false;
        
    }
  }
  
  //beep
  int distance = ultrasonicSensor.getDistance();  
  //Serial.println(distance);
  int distanceInfra = infraRedSensor.getDistance();
    Serial.println(distanceInfra);
  if((distance < 35) && (distance > 0) || (distanceInfra < 35) && (distanceInfra > 0)){
     playSound = false;
     beepSpeed = 0;
     digitalWrite(tonePin, HIGH);
     car.setSpeed(0);
     car.setAngle(0);
    } else {
      digitalWrite(tonePin, LOW);
      }
 
  int distance2 = ultrasonicSensor2.getDistance();
  Serial.println(distance2);
  if((distance2 < 40) && (distance2 > 15) && (sensorBack == true)){
      if ((distance2 < 39) && (distance2 > 25) && (sensorBack == true)){
    playSound = true;
    beepSpeed = 400;
   }  else if((distance2 < 25) && (distance2 > 15) && (sensorBack == true)){
    playSound = true;
    beepSpeed = 200;
   }  else {
      digitalWrite(7, LOW);
      distance = 80;
      playSound = false;
      beepSpeed = 0;
      }
  }
  if (distance2 == 0 && (sensorBack == true)){
      digitalWrite(7, LOW);
      playSound = false;
      beepSpeed = 0;
      }
  if((distance2 < 15) && (distance2 > 0) && (sensorBack == true)){
    playSound = false;
    beepSpeed = 0;
    Serial.println(distance2);
    digitalWrite(7, HIGH);
    car.setSpeed(0);
    car.setAngle(0);
  } else {digitalWrite(7, LOW);}
}

#include <Smartcar.h>
#include <Wire.h>
#include <Servo.h>

Car car;

int fSpeed = 80; // 100 is 70% of the full speed forward
const int bSpeed = -70; // -70 is 70% of the full speed backward

const int lDegrees = -75; //degrees to turn left
const int rDegrees = 75; //degrees to turn right
SR04 ultrasonicSensor;
const int TRIGGER_PIN = 6; //D6
const int ECHO_PIN = 5; //D5

long previousMillis = 0;
long interval = 1000;  
int tftrefresh = 0;
boolean sensorToggle = true;

char serialData[32];
byte com = 0, error = 0, timerCounter = 0;
boolean connected;

void setup() {
  Serial3.begin(9600);
  //Serial.begin(9600);
  Serial3.println("W forward, S backward\n A left\n D  right\n P accelerate, L deccelerate\n i info, R reset, Q toggle sensors on/off");

  ultrasonicSensor.attach(TRIGGER_PIN, ECHO_PIN);
  
  car.begin(); //initialize the car using the encoders and the gyro
}

void loop() {
  handleInput();

  //Serial.println(Serial.available());
}

/*
 * Sends 0s so we know the connection is not lost
 */
//void isSonnected()
//{
//  if(millis() - lastRefreshTime >= newREFRESH_INTERVAL)
//    {
//      Serial.println(0);
//    }
//}

void handleInput() { //handle the Serial Input (if there is input)
  if (Serial3.available() > 0) {

    connected = true;
    // clear timeout
    com = timerCounter;
    
    //char input = Serial.read(); //read everything that has been received so far and log down the last entry
    Serial3.readBytesUntil('\n', serialData, 31);
    switch(serialData[0])
    {
      case '0':
        Serial3.println(0);
        break;
      case 0:
        Serial3.println(0);
        break;
      case 'a': //rotate counter-clockwise going forward
        car.setSpeed(fSpeed);
        car.setAngle(lDegrees);
        break;
      case 'd': //turn clock-wise
        car.setSpeed(fSpeed);
        car.setAngle(rDegrees);
        break;
      case 'w': //go forward
        car.setSpeed(fSpeed);
        car.setAngle(0);
        break;
      case 's': //go back
        car.setSpeed(bSpeed);
        car.setAngle(0);
        break;
      case 'p':
      // accellerate
        if(fSpeed <= 110){
          fSpeed += 10;
          car.setSpeed(fSpeed);
        }
        break;
      case 'l':
      // deccelerate
        if(fSpeed >= 70){
          fSpeed -= 10;
          car.setSpeed(fSpeed);
        }
        break;
	    case 'q':
	    //toggle sensors on/off
        toggleSensors();
        break;
      case 'i':
        // information about car
        Serial3.println("OoII  Team 9 SmartCar! \\(｡◕‿◕｡)/  IIoO");
        break;
      case 'r':
        // stop immediately
        reset();
        Serial3.println("Car has been reset!");
        break;
      default:
        // command not valid
        Serial3.println("Not a valid command!");
    }

    // clear the serialData array
    memset(serialData, 0, sizeof(serialData));
        
  }

  int distance = ultrasonicSensor.getDistance();
  if((distance < 15) && (distance > 0) && sensorToggle){
    //Serial.println(distance);
      car.setSpeed(0);
      car.setAngle(0);
    }
}

void toggleSensors(){
  if(sensorToggle){
    sensorToggle = false;
  }else{
    sensorToggle = true;
  }
}

void reset(){
  connected = false;
  car.setSpeed(0);
  car.setAngle(0);
}

/**
 * Makes Integers from the Serial Data
 * first 2 chars are not counted - they are the command char and ","
 * which separate the first var
 *
 * @params command The entire serial data that is received
 * @params returnValues array for the return values
 * @params returnCount The number of values to set inside the returnValues variable
 */
boolean parseCommand(char* command, int* returnStringValues, byte returnCount)
{
  // parsing state machine
  byte i = 1, j = 0, signByte = 0, charByte = 0, numByte;
  int tempInt = 0;
  while(i++)
  {
    switch(*(command + i))
    {
    case '\0':
    case ',':
      // set return value
      if(charByte != 0)
      {
        returnStringValues[j++] = signByte?-tempInt:tempInt;
        signByte = 0;
        tempInt = 0;
        charByte = 0;
      }else{
        return false;
      }
      break;
    case '-':
      signByte = 1;
      break;
    default:
      // convert string to int
      numByte = *(command + i) - '0';
      if(numByte < 0 || numByte > 9)
      {
        return false;
      }
      tempInt = tempInt * 10 + numByte;
      charByte++;
    }

    // enough return values have been set
    if(j == returnCount)
    {
      return true;
    }
    // end of command reached
    else if(*(command + i) == '\0')
    {
      return false;
    }
  }
}

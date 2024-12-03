#include <Servo.h>
#include <ArduinoBLE.h>
#include "ble_functions.h"
#include "buzzer_functions.h"

//have to be imported before paddle_controller
long deltaTime = 0.0f;

#include "paddle_controller.h"

//two servo for wings
Servo servo1;
Servo servo2;
//servo pins, change if you change the wiring
const int servoPin = 3;
const int servo2Pin = 5;

//Name your controller!
const char* deviceName = "FlutterHigh";

//NOT useful but keep it here in case anything breaks
const int BUZZER_PIN = 11;       // Pin for haptic feedback buzzer
const int ledPin = 13;

// Movement state tracking
int currentMovement = 0;         // Current movement value (0=none, 1=up, 2=down, 3=handshake)

//timer variable, don't assign value manually.
long clappedTime = 0.0f;

//////////////////SETTINGS//////////////////////////
//amount of time paddle go up, in milliseconds. more time, each clap/sound detection moves the paddle up more. default 100
long clappedUpTime = 100.0f;
//how long paddle stay in air before falling down in ms. more time paddle stay longer. default 400
long clappedCooldown = 400.0f;

/////////////////Servo Settings//////////////////
//how long in ms do servo need to rest/not move before moving again. This helps save energy so arduino doesn't lose power. default 1500
long servoTurnTimeCD = 500.0f;
//time in ms between each servo angle turn
long servoTurnTimerDefault = 10.0f;
//angle each servo turn turns. keep it at 1 for power saving
int servoTurnAnglePer = 1;
//servo max angle setting
int servo1High = 35;
int servo1Low = -20;

int servo2High = -35;
int servo2Low = 20;

//legacy feature, stablization, NOT in use
int HistAmount = 500;
int HistValue[500];

//sensor pin, change if you changed the wiring
int sensorPin = A4;
int val = 0;

//paddle input, used by paddle controller.
int paddleInput = 1;


//servo setup
long servoTurnTime = 0.0f;
//going up or down
int servoDir = 1;
//timer variable, don't assign value manually;
long servoTurnTimer = 10.0f;
int servo1Angle = 0;
int servo2Angle = 0;

int servo1TargetAngle = -35;
int servo2TargetAngle = 35;

//legacy function, don't touch
void pushValue(int newValue) {
  // Shift all elements in the array to the left by one
  for (int i = 0; i < HistAmount - 1; i++) {
    HistValue[i] = HistValue[i + 1];
  }

  // Add the new value at the last position
  HistValue[HistAmount - 1] = newValue;
}
//get the average, legacy function, don't touch
float calculateAverage() {
  int sum = 0;

  // Loop through the array to calculate the sum
  for (int i = 0; i < HistAmount; i++) {
    sum += HistValue[i];
  }
  
  // Calculate and return the average
  return float(sum) / HistAmount;
}

void setup(){
  //setup servos
  servo1.attach(3);
  servo2.attach(5);

  //setup sensors
  pinMode(sensorPin, INPUT);

  //begin serial communication
  Serial.begin (9600);

  //legacy, initialize history value array
  for (int i = 0; i < HistAmount; i++) {
    HistValue[i] = 0;  // Initialize array with zeroes
  }

  //setup bluetooth
  setupBLE(deviceName, 999);
  
  //setup led, this is the internal led pin 13, helpful when adjusting sensitivity for the sound sensor
  pinMode(ledPin, OUTPUT);

  //initlaize servo angle
  servo1TargetAngle = servo1High;
  servo2TargetAngle = servo2High;

}

void loop (){
  //take delta time measurement
  deltaTime = millis() - previousTime;
  previousTime = millis();

  //read sound sensor value
  val = analogRead(sensorPin);

  //if conditions are met the butterfly will flap its wing once.
  if (millis() - servoTurnTime > servoTurnTimeCD && currentMovement == 2)
  {
    //make sure it's the opposite direction
    servoDir *= -1;
    if(servo1TargetAngle == servo1High)
    {
      servo1TargetAngle = servo1Low;
    }
    else
    {
      servo1TargetAngle = servo1High;
    }
    
    if(servo2TargetAngle == servo2High)
    {
      servo2TargetAngle = servo2Low;
    }
    else
    {
      servo2TargetAngle = servo2High;
    }

    //get when the servo turned, so before the next cooldown the servo will not turn.
    //this exist because the power if very limited in this setup, if the servos turn too rapidly it will force arduino into a reboot
    servoTurnTime = millis();
  }
  
  //this ticks the servo turn time, acts as a count down timer
  servoTurnTimer -= deltaTime;
  //if it's 0, then the servo can turn again, notice this is NOT servoTurnTime, they are very similar but one is in charge of the overall flapping 
  //one is in charge of each step in a flap. 
  if (servoTurnTimer <= 0)
  {
    //use a time modifier to get an ease in and out effect.
    long timeModifier = 1 + map(sin(map(servo1TargetAngle - servo1Angle, 0, 35, 0, HALF_PI)), 0, 1, 0, 1);

    //apply the ease in and out effect to timer
    servoTurnTimer = servoTurnTimerDefault * timeModifier ;

    //constrain the angle moved so it doesn't move too much, which costs stress to power
    servo1Angle += constrain(servo1TargetAngle - servo1Angle, -servoTurnAnglePer, servoTurnAnglePer);
    servo1.write(90 + servo1Angle);
    
    servo2Angle += constrain(servo2TargetAngle - servo2Angle, -servoTurnAnglePer, servoTurnAnglePer);
    servo2.write(90 + servo2Angle);

    Serial.println(servo2Angle);
  }
  //val is the value sound sensor gets
  if(val >= 600)
  {
    //Serial.println("Clapped");
    //if sound heard, move up
    paddleInput = 2;
    clappedTime = millis();
  }
  if(paddleInput == 2 && millis() - clappedTime <= clappedUpTime)
  {
    //if sound heard and is within the clappedUpTime frame, continue to go up
    paddleInput = 2;
  }
  else if(paddleInput == 2 && millis() - clappedTime > clappedUpTime)
  {
    //if the sound heard and is not in the clappedUpTime and is in the clappedCooldown time frame, so it stays a bit before dropiing.
    paddleInput = 0;
  }
  else if(paddleInput == 0 && millis() - clappedTime > clappedUpTime + clappedCooldown)
  {
    //if the sound heard and is not within the clappedUpTime and the ClappedCooldown, it starts to fall
    paddleInput = 1;
  }

  //legacy, push sensor value to array
  pushValue(val);

  // Update BLE connection status and handle incoming data
  updateBLE();
  
  //read the inputs te determine the current state
  //results in changing the value of currentMovement
  handleInput();
  
  sendMovement(currentMovement);

  //handles internal led
  if (val >= 600)
  {
    
    digitalWrite(ledPin, HIGH);

  }
  else
  {
    digitalWrite(ledPin, LOW);
  }
  
}

void handleInput() 
{
//flipped read method because of INPUT_PULLUP 
  
  if (paddleInput == 1) 
  {
    currentMovement = 1;         // UP movement
  } 
  else if (paddleInput  == 2) 
  {
    currentMovement = 2;         // DOWN movement
  } 
  else 
  {
    currentMovement = 0;         // No movement
  }
}

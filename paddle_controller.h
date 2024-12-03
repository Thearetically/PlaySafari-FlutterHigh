int FinalOutput = 0;
float PaddleSpeed = 0.5;


float TargetPosition = 0;
float CurrentPosition = 0;

//how much difference before it moves
float PositionThreshold = 10;
//max and min position of the paddle's movement range
float MaxPosition = 700;
float MinPosition = 0;

//target framerate in p5, this is an estimation
float FrameRate = 60; //This may vary but around 60
//arbitrary value modifies how fast the time ticks
float TimeModifier = 0.5;

int isUp = 1;


unsigned long previousTime = 0;  // Variable to store the last recorded time
//unsigned long deltaTime = 0;    // Variable to store the delta time
unsigned long movedTime = 0;
long TimeToTargetPosition = 0;

void updateState(int position) 
{
  if(abs(TargetPosition - position) > 10)
  {
    TargetPosition = position;
  }

  //clampping
  if(TargetPosition > 700)
  {
    TargetPosition = 700;
  } 
  if(TargetPosition < 0)
  {
    TargetPosition = 0;
  }
  //Delta Time Calculation
  unsigned long currentTime = millis();  // Get the current time in milliseconds
  deltaTime = currentTime - previousTime; // Calculate the delta time
  previousTime = currentTime;            // Update the previous time

  //if is not staying, then tick the time, so the traveled time can reflect on the total travel time.
  if(isUp != 0)
  {
    movedTime += deltaTime;
  }
  
  //if time ticked more than 0.01666, which means p5 has gone one frame, hopefully! which means the paddle has moved up 10 pixels
  if(movedTime >= 0.01666)
  {
    movedTime -= 0.01666 * TimeModifier;
    CurrentPosition += PaddleSpeed * isUp;
  }
  //calculate time needed to go to target position.
  TimeToTargetPosition = (TargetPosition - CurrentPosition) / PaddleSpeed * TimeModifier; //* 1000;

  Serial.println("TargetPos is:" + String(TargetPosition) + "Current Pos is: " + String(CurrentPosition));


  //transfer final result to a global variable
  if (TargetPosition < CurrentPosition )
  {
    isUp = -1;
    FinalOutput = 1;
  }
  else if (TargetPosition > CurrentPosition)
  {
    isUp = 1;
    FinalOutput = 2;
  }
  else
  {
    isUp = 0;
    FinalOutput = 0;
  }
}
/* 
 To use this example code, connect the 5-way selector switch [COM-10541] 
 in the following manner:
 
 Switch                    Arduino
 ___                                  10k pull-downs
 Pin3|D---------------------pin2---------\/\/\/------GND
 Pin2|D---------------------pin3---------\/\/\/------GND
 Pin1|D---------------------pin4---------\/\/\/------GND
 COM |D---------------------VDD
 |
 
 You only need to read three pins in order to tell the position of the switch (plus Common) because
 there are actually 2 seperate wipers in the switch, each side (1,2,3,COM) is a mirror image of the other
 and they're are not interconnected. As you can (hopefully) tell from the ASCII diagram above, you're 
 going to wire each pin basically as if it were a button. (For more info on that, see the Arduino 'Button' 
 Example). 
 
 This Sketch will read the state of the three output pins on the selector and output a number on the serial 
 terminal 1-5 corresponding to the position of the lever.
 */


/* ------- 5 Way Selector Switch Vars -------*/
#define pin1 2
#define pin2 4 
#define pin3 7

int pinState1 = 0;
int pinState2 = 0;
int pinState3 = 0;

int lastPos = 0;
int switchPos = 0;

/* ------- RGB Vars -------*/

/*
#define RED_PIN 5
 #define GREEN_PIN 6
 #define BLUE_PIN 9
 */

const int bar1 = 8;
const int bar2 = 9;
const int bar3 = 10;
const int bar4 = 11;
const int bar5 = 12;
const int STATUS_PIN = 13;

const int RED_PIN = 3;
const int GREEN_PIN = 5;
const int BLUE_PIN = 6;

const int RPOT_PIN = A0;
const int GPOT_PIN = A1;
const int BPOT_PIN = A2;
const int buttonPin = A3;

int rPotVal = 0; 
int gPotVal = 0; 
int bPotVal = 0; 
int buttonState = 0;
int buttonLast = 0;
boolean userAware = false;

void read5Way()
{
  pinState1 = digitalRead(pin1);
  pinState2 = digitalRead(pin2);
  pinState3 = digitalRead(pin3);

  if(pinState1 == LOW)
  {
    if(pinState2 == LOW)
    {
      switchPos = 2;
    }
    else
    {
      switchPos = 1;
    }
  }

  else if(pinState2 == LOW)
  {
    if(pinState1 == LOW)
    {
      switchPos = 2;
    }
    else if(pinState3 == LOW)
    {
      switchPos = 4;
    }
    else
    {
      switchPos = 3;
    }
  }

  else if(pinState3 == LOW)
  {
    if(pinState2 == LOW)
    {
      switchPos = 4;
    }
    else
    {
      switchPos = 5;
    }
  }
  if(lastPos != switchPos)
  {
    lastPos = switchPos;
    Serial.print("In ");
    Serial.println(switchPos);
    if(switchPos == 1)
    {
      crossFade(255, 145, 0, 4, 0);
    }
    userAware = false;
    enableBars(switchPos);
  }
}

int calculateStep(int prevValue, int endValue) {
  int step = endValue - prevValue; // What's the overall gap?
  if (step) {                      // If its non-zero, 
    step = 1020/step;              //   divide by 1020
  } 
  return step;
}

/* The next function is calculateVal. When the loop value, i,
 *  reaches the step size appropriate for one of the
 *  colors, it increases or decreases the value of that color by 1. 
 *  (R, G, and B are each calculated separately.)
 */

int calculateVal(int step, int val, int i) {

  if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
    if (step > 0) {              //   increment the value if step is positive...
      val += 1;           
    } 
    else if (step < 0) {         //   ...or decrement it if step is negative
      val -= 1;
    } 
  }
  // Defensive driving: make sure val stays in the range 0-255
  if (val > 255) {
    val = 255;
  } 
  else if (val < 0) {
    val = 0;
  }
  return val;
}

/* crossFade() converts the percentage colors to a 
 *  0-255 range, then loops 1020 times, checking to see if  
 *  the value needs to be updated each time, then writing
 *  the color values to the correct pins.
 */
// Declare and Initialize color variables
int redVal = 0;
int grnVal = 0; 
int bluVal = 0;
int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;

void crossFade(byte R, byte G, byte B, int wait, int hold)
{
  int stepR = calculateStep(prevR, R);
  int stepG = calculateStep(prevG, G); 
  int stepB = calculateStep(prevB, B);

  for (int i = 0; i <= 1020; i+=2)
  {
    //ADD SWITCH CHECK?
    read5Way();
    if(switchPos != lastPos)
    {
      Serial.println("Breaking amber");
      break;
    }
    redVal = calculateVal(stepR, redVal, i);
    grnVal = calculateVal(stepG, grnVal, i);
    bluVal = calculateVal(stepB, bluVal, i);

    analogWrite(RED_PIN, redVal);   // Write current values to LED pins
    analogWrite(GREEN_PIN, grnVal);      
    analogWrite(BLUE_PIN, bluVal); 

    delay(wait); // Pause for 'wait' milliseconds before resuming the loop
  }
  // Update current values for next loop
  prevR = redVal; 
  prevG = grnVal; 
  prevB = bluVal;
  delay(hold); // Pause for optional 'wait' milliseconds before resuming the loop
}

void setStrips(byte r, byte g, byte b)
{
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void enableBars(int barNum)
{
  //Serial.println(barNum);
  switch(barNum)
  {
  case 1:
    digitalWrite(bar1, HIGH);
    digitalWrite(bar2, LOW);
    digitalWrite(bar3, LOW);
    digitalWrite(bar4, LOW);
    digitalWrite(bar5, LOW);
    break;
  case 2:
    digitalWrite(bar1, HIGH);
    digitalWrite(bar2, HIGH);
    digitalWrite(bar3, LOW);
    digitalWrite(bar4, LOW);
    digitalWrite(bar5, LOW);
    break;
  case 3:
    digitalWrite(bar1, HIGH);
    digitalWrite(bar2, HIGH);
    digitalWrite(bar3, HIGH);
    digitalWrite(bar4, LOW);
    digitalWrite(bar5, LOW);
    break;
  case 4:
    digitalWrite(bar1, HIGH);
    digitalWrite(bar2, HIGH);
    digitalWrite(bar3, HIGH);
    digitalWrite(bar4, HIGH);
    digitalWrite(bar5, LOW);
    break;
  case 5:
    digitalWrite(bar1, HIGH);
    digitalWrite(bar2, HIGH);
    digitalWrite(bar3, HIGH);
    digitalWrite(bar4, HIGH);
    digitalWrite(bar5, HIGH);
    break;
  }
}

void disableBars()
{
  digitalWrite(bar1, LOW);
  digitalWrite(bar2, LOW);
  digitalWrite(bar3, LOW);
  digitalWrite(bar4, LOW);
  digitalWrite(bar5, LOW);
}

boolean systemOn = false;

boolean buttonCheck()
{
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && buttonLast == LOW)
  {
    buttonLast = buttonState;
    Serial.println("Button pressed...");
    return true;
  }
  else
  {
    buttonLast = buttonState;
    return false;
  }
}

void setup()
{
  Serial.begin(9600);
  /*
  pinMode(pin1, INPUT);
   pinMode(pin2, INPUT);
   pinMode(pin3, INPUT);
   */
  pinMode(pin1, INPUT_PULLUP);
  pinMode(pin2, INPUT_PULLUP);
  pinMode(pin3, INPUT_PULLUP);

  pinMode(A1, INPUT);
  pinMode(A4, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(RPOT_PIN, INPUT);
  pinMode(GPOT_PIN, INPUT);
  pinMode(BPOT_PIN, INPUT);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);

  pinMode(bar1, OUTPUT);
  pinMode(bar2, OUTPUT);
  pinMode(bar3, OUTPUT);
  pinMode(bar4, OUTPUT);
  pinMode(bar5, OUTPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  pinMode(STATUS_PIN, OUTPUT);
}

int strobe = 200;
boolean isWhite = false;
boolean amberChanged = false;
int currentColor = 0;

void fastSwitchCheck(int delayTime)
{
  delay(delayTime);
  read5Way();
}

#define AMBER_MODE 1
#define COLOR_MODE 2
#define NORMAL_MODE 3
#define STROBE_MODE 4
#define CUSTOM_MODE 5

void loop()
{  
  buttonState = digitalRead(buttonPin);
  if(systemOn)
  {  
    rPotVal = map(analogRead(A0), 0, 1023, 0, 255); 
    delay(10);
    gPotVal = map(analogRead(A1), 0, 1023, 0, 255); 
    delay(10);
    bPotVal = map(analogRead(A2), 0, 1023, 0, 255);
    /*
    Serial.print(a0V);
     Serial.print("r, ");
     Serial.print(a1V);
     Serial.print("g, ");
     Serial.print(a2V);
     Serial.print("b, ");
     Serial.print(a3V);
     Serial.print(", ");
     Serial.print(a4V);
     Serial.print(", ");
     Serial.print(a5V);
     Serial.println("");
     */
    read5Way();

    if (userAware == false)
    {
      if(buttonCheck())
      {
        userAware = true;
        lastPos = switchPos;
      }
    }
    //check for state change
    switch(switchPos)
    {
    case AMBER_MODE:
      Serial.println("In Amber Mode");
      //changed to this in read5Way
      break;
    case COLOR_MODE:
      //Serial.println("In Color Mode");
      if(buttonCheck())
      {
        currentColor++;
      }
      if(currentColor == 0)
      {
        setStrips(rPotVal, rPotVal, rPotVal);
      }
      if(currentColor == 1)
      {
        setStrips(rPotVal, 0, 0);
      }
      else if(currentColor == 2)
      {
        setStrips(0, rPotVal, 0);
      }
      else if(currentColor == 3)
      {
        setStrips(0, 0, rPotVal);
      }
      if(currentColor > 3)
      {
        currentColor = 0;
      }
      break;
    case NORMAL_MODE:
      //Serial.println("In Normal Mode");
      setStrips(rPotVal, gPotVal, bPotVal);
      break;
    case STROBE_MODE:
      //button ON
      if(!userAware)
      {
        setStrips(0, 0, 0);

        enableBars(switchPos);
        delay(200);
        disableBars();
        delay(200);
      }
      else
      {
        //Serial.println("In Strobe Mode");
        //Serial.println(buttonState);
        enableBars(switchPos);
        if (buttonState == HIGH)
        {
          strobe = rPotVal;
          enableBars(switchPos);
          delay(strobe);
          disableBars();
          delay(strobe);
        } 
        else
        {
          setStrips(rPotVal, gPotVal, bPotVal);
          delay(strobe);
          setStrips(0, 0, 0);
          delay(strobe);
        }
      }
      break;
    case CUSTOM_MODE:
      if(!userAware)
      {
        setStrips(0, 0, 0);
        enableBars(switchPos);
        delay(200);

        disableBars();
        delay(200);
      }
      else
      {
        //Serial.println("In Custom Mode");
        enableBars(switchPos);
        //DO SHIT
        if(analogRead(RPOT_PIN) >= 512 & analogRead(GPOT_PIN) <= 512 && analogRead(BPOT_PIN) >= 512)
        {
          lastPos = switchPos;
          Serial.println("COP MODE");
          setStrips(255, 0, 0);
          fastSwitchCheck(50);
          setStrips(0, 0, 0);
          fastSwitchCheck(50);
          setStrips(255, 0, 0);
          fastSwitchCheck(50);
          setStrips(0, 0, 0);
          fastSwitchCheck(50);
          setStrips(255, 0, 0);
          fastSwitchCheck(75);
          //buttonCheck();

          setStrips(255, 255, 255);
          fastSwitchCheck(75);

          setStrips(0, 0, 255);
          fastSwitchCheck(50);
          setStrips(0, 0, 0);
          fastSwitchCheck(50);
          setStrips(0, 0, 255);
          fastSwitchCheck(50);
          setStrips(0, 0, 0);
          fastSwitchCheck(50);
          setStrips(0, 0, 255);
          fastSwitchCheck(75);
          setStrips(255, 255, 255);
          fastSwitchCheck(75);
        }
      }
      break;
    }
  }
  else
  {
    setStrips(0, 0, 0);
    if(buttonCheck())
    {
      systemOn = true;
    }
  }
}











































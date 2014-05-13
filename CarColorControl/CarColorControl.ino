// Color arrays
int black[3]  = { 
  0, 0, 0 };
int white[3]  = { 
  100, 100, 100 };
int red[3]    = { 
  100, 0, 0 };
int green[3]  = { 
  0, 100, 0 };
int blue[3]   = { 
  0, 0, 100 };
int yellow[3] = { 
  40, 95, 0 };
int dimWhite[3] = { 
  30, 30, 30 };
// Set initial color
int redVal = black[0];
int grnVal = black[1]; 
int bluVal = black[2];
int fullVal = 255;

const int REDPIN = 5;
const int GREENPIN = 6;
const int BLUEPIN = 9;

const int RECV_PIN = 11;

const int WHITEBTN_PIN = 4;
const int REDBTN_PIN = 7;
const int GREENBTN_PIN = 8;
const int BLUEBTN_PIN = 12;

const int STATUS_PIN = 10;

const int AMBER_MODE = -1;
const int NORMAL_MODE = 0;
const int FADE_MODE = 1;
const int STROBE_MODE = 2;
const int COP_MODE = 3;
const int HEART_MODE = 4;
const int SHOWOFF_MODE = 5;

int currentMode = 0;

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
  if (val > fullVal) {
    val = fullVal;
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
int wait = 1;
int hold = 0;
// Declare and Initialize color variables
int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;

void crossFade(byte color[3]) {
  // Convert to 0-255
  byte R = color[0];
  byte G = color[1];
  byte B = color[2];

  int stepR = calculateStep(prevR, R);
  int stepG = calculateStep(prevG, G); 
  int stepB = calculateStep(prevB, B);

  for (int i = 0; i <= 1020; i+=2) {
    buttonCheck();
    redVal = calculateVal(stepR, redVal, i);
    grnVal = calculateVal(stepG, grnVal, i);
    bluVal = calculateVal(stepB, bluVal, i);

    analogWrite(REDPIN, redVal);   // Write current values to LED pins
    analogWrite(GREENPIN, grnVal);      
    analogWrite(BLUEPIN, bluVal); 

    delay(wait); // Pause for 'wait' milliseconds before resuming the loop
    /*
    if (DEBUG) { // If we want serial output, print it at the 
     if (i == 0 or i % loopCount == 0) { // beginning, and every loopCount times
     Serial.print("Loop/RGB: #");
     Serial.print(i);
     Serial.print(" | ");
     Serial.print(redVal);
     Serial.print(" / ");
     Serial.print(grnVal);
     Serial.print(" / ");  
     Serial.println(bluVal); 
     } 
     DEBUG += 1;
     }
     */
  }
  // Update current values for next loop
  prevR = redVal; 
  prevG = grnVal; 
  prevB = bluVal;
  delay(hold); // Pause for optional 'wait' milliseconds before resuming the loop
}

void setStrips(boolean fade, byte r, byte g, byte b)
{
  byte arr[] = {
    r, g, b                                            };
  if(fade)
  {
    crossFade(arr);
  }
  else
  {
    analogWrite(REDPIN, map(r, 0, fullVal, 0, 1023));
    analogWrite(GREENPIN, map(g, 0, fullVal, 0, 1023));
    analogWrite(BLUEPIN, map(b, 0, fullVal, 0, 1023));
  }
}


void fadeStrips(int fadespeed)
{
  int r, g, b;
  // fade from blue to violet
  for (r = 0; r < fullVal+1; r++) { 
    analogWrite(REDPIN, r);
    buttonCheck();
    if(currentMode != 1)
      break;
    delay(fadespeed);
  } 
  // fade from violet to red
  for (b = fullVal; b > 0; b--) { 
    analogWrite(BLUEPIN, b);
    buttonCheck();
    if(currentMode != 1)
      break;
    delay(fadespeed);
  } 
  // fade from red to yellow
  for (g = 0; g < fullVal+1; g++) { 
    analogWrite(GREENPIN, g);
    buttonCheck();
    if(currentMode != 1)
      break;
    delay(fadespeed);
  } 
  // fade from yellow to green
  for (r = fullVal; r > 0; r--) { 
    analogWrite(REDPIN, r);
    buttonCheck();
    if(currentMode != 1)
      break;
    delay(fadespeed);
  } 
  // fade from green to teal
  for (b = 0; b < fullVal+1; b++) { 
    analogWrite(BLUEPIN, b);
    buttonCheck();
    if(currentMode != 1)
      break;
    delay(fadespeed);
  } 
  // fade from teal to blue
  for (g = fullVal; g > 0; g--) { 
    analogWrite(GREENPIN, g);
    buttonCheck();
    if(currentMode != 1)
      break;
    delay(fadespeed);
  }
}

void fade()
{
  if(currentMode != 1)
    return;
  //sendCode(0, fadeCode);
  //irsend.sendNEC(fadeCode, 32);
  fadeStrips(5);
}

int lastRedState = 0;
int lastGreenState = 0;
int lastBlueState = 0;

void parseSwitches(boolean checkStates, int r, int g, int b)
{
  if(checkStates && lastRedState == r && lastGreenState == g && lastBlueState == b)
  {
    return;
  }

  int redMapped = map(r, 0, 1, 0, fullVal);
  int greenMapped = map(g, 0, 1, 0, fullVal);
  int blueMapped = map(b, 0, 1, 0, fullVal);
  setStrips(true, redMapped, greenMapped, blueMapped);
  lastRedState = r;
  lastGreenState = g;
  lastBlueState = b;
}

void strobeSwitches(int r, int g, int b)
{
  Serial.println("Strobing...");
  int redMapped = map(r, 0, 1, 0, fullVal);
  int greenMapped = map(g, 0, 1, 0, fullVal);
  int blueMapped = map(b, 0, 1, 0, fullVal);
  setStrips(false, redMapped, greenMapped, blueMapped);
  delay(35);
  setStrips(false, 0, 0, 0);
  delay(35);
  lastRedState = r;
  lastGreenState = g;
  lastBlueState = b;
  // Wait a bit between retransmissions
}

void heartBeat(float tempo)
{
  for(int i=0;i<(fullVal/2);i++)
  {
    setStrips(false, i, 0, 0);
    delay(1 * tempo);
  }
  delay(100 * tempo);

  for(int i=(fullVal/2);i>80;i--)
  {
    setStrips(false, i, 0, 0);
    delay(1 * tempo);
  }
  delay(100 * tempo);

  for(int i=0;i<fullVal;i+=2)
  {
    setStrips(false, i, 0, 0);
    delay(1 * tempo);
  }
  delay(100 * tempo);
  /*
  for(int i=fullVal;i>0;i--)
   {
   setStrips(false, i, 0, 0);
   delay(2);
   }
   */
  setStrips(true, 0, 0, 0);
}

void goAmber()
{
  currentMode = AMBER_MODE;
}

int whiteButtonVal = 0; // value read from button
int whiteButtonLast = 0; // buffered value of the button's previous state
long btnDnTime; // time the button was pressed down
long btnUpTime; // time the button was released
boolean ignoreUp = false; // whether to ignore the button release because the click+hold was triggered

//values for button hold
#define debounce 20 // ms debounce period to prevent flickering when pressing or releasing the button
#define holdTime 2000 // ms hold period: how long to wait for press+hold event

void buttonCheck()
{
  // Read the state of the button
  whiteButtonVal = digitalRead(WHITEBTN_PIN);
  // Test for button pressed and store the down time
  if (whiteButtonVal == HIGH && whiteButtonLast == LOW && (millis() - btnUpTime) > long(debounce))
  {
    btnDnTime = millis();
  }
  // Test for button release and store the up time
  if (whiteButtonVal == LOW && whiteButtonLast == HIGH && (millis() - btnDnTime) > long(debounce))
  {
    Serial.println("PRESSED");
    if (ignoreUp == false)
    {
      whiteButtonLast = whiteButtonVal;
      delay(500);
      if(digitalRead(WHITEBTN_PIN) == HIGH)
      {
        Serial.println("DOUBLE PRESSED");
        buttonDoublePressed();
      }
      else
      {
        if(currentMode != STROBE_MODE || currentMode != COP_MODE || currentMode != SHOWOFF_MODE || currentMode != HEART_MODE)
        {
          currentMode++;
        }
        else
        {
          int red = digitalRead(REDBTN_PIN);
          int green = digitalRead(GREENBTN_PIN);
          int blue = digitalRead(BLUEBTN_PIN);
          parseSwitches(false, red, green, blue);
          currentMode = NORMAL_MODE;
        }
        Serial.print("Mode changed to ");
        Serial.println(currentMode);
      }
    }
    else
    { 
      ignoreUp = false;
    }
    btnUpTime = millis();
  }
  // Test for button held down for longer than the hold time
  if (whiteButtonVal == HIGH && (millis() - btnDnTime) > long(holdTime))
  {
    Serial.println("HELD");
    buttonHeld();
    ignoreUp = true;
    btnDnTime = millis();
  }

  whiteButtonLast = whiteButtonVal;
}

void buttonDoublePressed()
{
  goAmber();
}

void buttonHeld()
{
  int red = digitalRead(REDBTN_PIN);
  int green = digitalRead(GREENBTN_PIN);
  int blue = digitalRead(BLUEBTN_PIN);
  if(red && green && blue)
  {
    Serial.println("Going into showoff");
    currentMode = SHOWOFF_MODE;
  }

  if(red && !green && blue)
  {
    Serial.println("Going into cop");
    currentMode = COP_MODE;
  }

  if(red && !green && !blue)
  {
    Serial.println("Going into heart");
    setStrips(true, 0, 0, 0);
    currentMode = HEART_MODE;
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(REDBTN_PIN, INPUT);
  pinMode(GREENBTN_PIN, INPUT);
  pinMode(BLUEBTN_PIN, INPUT);
  pinMode(WHITEBTN_PIN, INPUT);

  //enable internal pull up resistors
  digitalWrite(REDBTN_PIN, HIGH);
  digitalWrite(GREENBTN_PIN, HIGH);
  digitalWrite(BLUEBTN_PIN, HIGH);
  digitalWrite(WHITEBTN_PIN, HIGH);

  pinMode(STATUS_PIN, OUTPUT);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  Serial.println("INIT");
}

void loop()
{
  buttonCheck();
  if(currentMode == NORMAL_MODE)
  {
    Serial.println("In Normal");
    int red = digitalRead(REDBTN_PIN);
    int green = digitalRead(GREENBTN_PIN);
    int blue = digitalRead(BLUEBTN_PIN);
    parseSwitches(true, red, green, blue);
  }
  else if(currentMode == FADE_MODE)
  {
    Serial.println("In Fade");
    setStrips(true, 0, 0, fullVal);
    fade();
  }
  else if(currentMode == STROBE_MODE)
  {
    Serial.println("In Strobe");
    int red = digitalRead(REDBTN_PIN);
    int green = digitalRead(GREENBTN_PIN);
    int blue = digitalRead(BLUEBTN_PIN);
    strobeSwitches(red, green, blue);
  }
  else if(currentMode == COP_MODE)
  {
    Serial.println("In Cop");
    setStrips(false, fullVal, 0, 0);
    delay(75);
    setStrips(false, 0, 0, fullVal);
    delay(75);
  }
  else if(currentMode == HEART_MODE)
  {
    Serial.println("In Heart");
    heartBeat(1.0); 
  }
  else if(currentMode == SHOWOFF_MODE)
  {
    Serial.println("In ShowOff");
    setStrips(false, fullVal, 0, 0);
    delay(100);
    setStrips(false, 0, fullVal, 0);
    delay(100);
    setStrips(false, 0, 0, fullVal);
    delay(100);
  }
  else if(currentMode == AMBER_MODE)
  {
    Serial.println("In Amber");
    int oldWait = wait;
    wait = 2;
    setStrips(true, 255, 128, 0);
    wait = oldWait;
  }
}




















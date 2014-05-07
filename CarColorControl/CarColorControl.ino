#include <IRremote.h>
#include "IRCode.h"

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

const int REDPIN = 5;
const int GREENPIN = 6;
const int BLUEPIN = 9;

const int RECV_PIN = 11;

const int WHITEBTN_PIN = 4;
const int REDBTN_PIN = 7;
const int GREENBTN_PIN = 8;
const int BLUEBTN_PIN = 12;

const int STATUS_PIN = 10;

const int NORMAL_MODE = 0;
const int FADE_MODE = 1;
const int STROBE_MODE = 2;
const int COP_MODE = 3;
const int HEART_MODE = 4;

boolean isAmber = false;
int currentMode = 0;

IRrecv irrecv(RECV_PIN);
//irsend //irsend;

decode_results results;

// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results *results, IRCode& store) {
  store.codeType = results->decode_type;
  int count = results->rawlen;
  if (store.codeType == UNKNOWN) {
    Serial.println("Got unknown, saving as raw");
    store.codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= store.codeLen; i++) {
      if (i % 2) {
        // Mark
        store.rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        // Space
        store.rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(store.rawCodes[i - 1], DEC);
    }
    Serial.println("");
  }
  else {
    if (store.codeType == NEC) {
      Serial.print("Got NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (store.codeType == SONY) {
      Serial.print("Got SONY: ");
    } 
    else if (store.codeType == RC5) {
      Serial.print("Got RC5: ");
    } 
    else if (store.codeType == RC6) {
      Serial.print("Got RC6: ");
    } 
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(store.codeType, DEC);
      Serial.println("");
    }
    Serial.println(results->value, HEX);
    store.codeValue = results->value;
    store.codeLen = results->bits;
  }
}

void sendCode(int repeat, IRCode code) {
  if (code.codeType == NEC) {
    if (repeat) {
      //irsend.sendNEC(REPEAT, code.codeLen);
      Serial.println("Sent NEC repeat");
    } 
    else {
      //irsend.sendNEC(code.codeValue, code.codeLen);
      Serial.print("Sent NEC ");
      Serial.println(code.codeValue, HEX);
    }
  } 
  else if (code.codeType == SONY) {
    //irsend.sendSony(code.codeValue, code.codeLen);
    Serial.print("Sent Sony ");
    Serial.println(code.codeValue, HEX);
  } 
  else if (code.codeType == RC5 || code.codeType == RC6) {
    if (!repeat) {
      // Flip the toggle bit for a new button press
      code.toggle = 1 - code.toggle;
    }
    // Put the toggle bit into the code to send
    code.codeValue = code.codeValue & ~(1 << (code.codeLen - 1));
    code.codeValue = code.codeValue | (code.toggle << (code.codeLen - 1));
    if (code.codeType == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(code.codeValue, HEX);
      //irsend.sendRC5(code.codeValue, code.codeLen);
    } 
    else {
      //irsend.sendRC6(code.codeValue, code.codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(code.codeValue, HEX);
    }
  } 
  else if (code.codeType == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    //irsend.sendRaw(code.rawCodes, code.codeLen, 38);
    Serial.println("Sent raw");
  }
}

void dumpCode(IRCode code)
{
  Serial.println("---DUMPING IRCODE---\n");
  Serial.print("IRCode name = { ");
  Serial.print(code.codeType);
  Serial.print(", 0x");
  Serial.print(code.codeValue, HEX);
  Serial.print(", {0}, ");
  Serial.print(code.codeLen);
  Serial.print(", ");
  Serial.print(code.toggle);
  Serial.println("};");
}

unsigned long getIRCode(int r, int g, int b)
{
  if(r == 1 && g == 1 && b == 1)
    return whiteCode;
  if(r == 1 && g == 0 && b == 0)
    return redCode;
  if(r == 0 && g == 1 && b == 0)
    return greenCode;
  if(r == 0 && g == 0 && b == 1)
    return blueCode;

  if(r == 1 && g == 0 && b == 1)
    return purpleCode;
  if(r == 1 && g == 1 && b == 0)
    return yellowCode;
  if(r == 0 && g == 1 && b == 1)
    return cyanCode;
  if(r == 0 && g == 0 && b == 0)
    return offCode;
}

/*
 *
 * END OF IR CODE
 *
 */


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
    r, g, b                              };
  if(fade)
  {
    crossFade(arr);
  }
  else
  {
    analogWrite(REDPIN, map(r, 0, 255, 0, 1023));
    analogWrite(GREENPIN, map(g, 0, 255, 0, 1023));
    analogWrite(BLUEPIN, map(b, 0, 255, 0, 1023));
  }
}


void fadeStrips(int fadespeed)
{
  int r, g, b;
  // fade from blue to violet
  for (r = 0; r < 256; r++) { 
    analogWrite(REDPIN, r);
    buttonCheck();
    if(isAmber || currentMode != 1)
      break;
    delay(fadespeed);
  } 
  // fade from violet to red
  for (b = 255; b > 0; b--) { 
    analogWrite(BLUEPIN, b);
    buttonCheck();
    if(isAmber || currentMode != 1)
      break;
    delay(fadespeed);
  } 
  // fade from red to yellow
  for (g = 0; g < 256; g++) { 
    analogWrite(GREENPIN, g);
    buttonCheck();
    if(isAmber || currentMode != 1)
      break;
    delay(fadespeed);
  } 
  // fade from yellow to green
  for (r = 255; r > 0; r--) { 
    analogWrite(REDPIN, r);
    buttonCheck();
    if(isAmber || currentMode != 1)
      break;
    delay(fadespeed);
  } 
  // fade from green to teal
  for (b = 0; b < 256; b++) { 
    analogWrite(BLUEPIN, b);
    buttonCheck();
    if(isAmber || currentMode != 1)
      break;
    delay(fadespeed);
  } 
  // fade from teal to blue
  for (g = 255; g > 0; g--) { 
    analogWrite(GREENPIN, g);
    buttonCheck();
    if(isAmber || currentMode != 1)
      break;
    delay(fadespeed);
  }
}

void fade()
{
  if(isAmber || currentMode != 1)
    return;
  //sendCode(0, fadeCode);
  //irsend.sendNEC(fadeCode, 32);
  fadeStrips(5);
}

void interiorOverride(IRCode results)
{
  if(results.codeValue == redCode)
  {
    setStrips(true, 255, 0, 0);
  }
  else if(results.codeValue == greenCode)
  {
    setStrips(true, 0, 255, 0);
  }
  else if(results.codeValue == blueCode)
  {
    setStrips(true, 0, 0, 255);
  }
  else if(results.codeValue == fadeCode)
  {
    fade();
  }
}

int lastRedState = 0;
int lastGreenState = 0;
int lastBlueState = 0;

unsigned long lastCode;

void parseSwitches(boolean checkStates, int r, int g, int b)
{
  if(checkStates && lastRedState == r && lastGreenState == g && lastBlueState == b)
  {
    return;
  }

  int redMapped = map(r, 0, 1, 0, 255);
  int greenMapped = map(g, 0, 1, 0, 255);
  int blueMapped = map(b, 0, 1, 0, 255);
  if(!isAmber)
  {
    ////irsend.sendNEC(onCode, 32);
    delay(50);
    unsigned long buttonsCode = getIRCode(r, g, b);
    ////irsend.sendNEC(buttonsCode, 32);
    Serial.println(buttonsCode, HEX);
    lastCode = buttonsCode;
    setStrips(true, redMapped, greenMapped, blueMapped);
  }
  /*
  if(r == 1 && g == 0 && b == 0)
   {
   sendCode(0, redCode);
   //setStrips(255, 0, 0);
   }
   else if(r == 0 && g == 0 && b == 1)
   {
   sendCode(0, blueCode);
   //setStrips(0, 255, 0);
   }
   else if(r == 0 && g == 1 && b == 0)
   {
   sendCode(0, greenCode);
   //setStrips(0, 0, 255);
   }
   else if(r == 0 && g == 0 && b == 0)
   {
   //sendCode(0, offCode);
   //setStrips(0, 0, 0);
   }
   */
  delay(50);
  irrecv.enableIRIn();
  lastRedState = r;
  lastGreenState = g;
  lastBlueState = b;
}

void strobeSwitches(int r, int g, int b)
{
  if(!isAmber)
  {
    Serial.println("Strobing...");
    int redMapped = map(r, 0, 1, 0, 255);
    int greenMapped = map(g, 0, 1, 0, 255);
    int blueMapped = map(b, 0, 1, 0, 255);
    setStrips(false, redMapped, greenMapped, blueMapped);
    //irsend.sendNEC(onCode, 32);
    unsigned long buttonsCode = getIRCode(r, g, b);
    //irsend.sendNEC(buttonsCode, 32);
    delay(25);
    setStrips(false, 0, 0, 0);
    //irsend.sendNEC(offCode, 32);
    delay(25);
  }
  else
  {
    return;
  }

  /*
  if(r == 1 && g == 0 && b == 0)
   {
   Serial.println("RED PRESSED");
   sendCode(0, redCode);
   setStrips(255, 0, 0);
   delay(25);
   setStrips(255, 0, 0);
   delay(25);
   }
   else if(r == 0 && g == 0 && b == 1)
   {
   Serial.println("BLUE PRESSED");
   sendCode(0, blueCode);
   setStrips(0, 255, 0);
   delay(25);
   setStrips(0, 255, 0);
   delay(25);
   }
   else if(r == 0 && g == 1 && b == 0)
   {
   Serial.println("GREEN PRESSED");
   sendCode(0, greenCode);
   setStrips(0, 0, 255);
   delay(25);
   setStrips(0, 0, 255);
   delay(25);
   }
   else if(r == 0 && g == 0 && b == 0)
   {
   Serial.println("NONE PRESSED");
   //sendCode(0, offCode);
   setStrips(0, 0, 0);
   }
   */
  lastRedState = r;
  lastGreenState = g;
  lastBlueState = b;
  // Wait a bit between retransmissions
  delay(50);
  irrecv.enableIRIn();
}

void blinkLED(int pin, int amount)
{
  for(int i=0;i<amount;i++)
  {
    digitalWrite(pin, LOW);
    delay(500);
    digitalWrite(pin, HIGH);
    delay(500);
  }
}

void heartBeat()
{
    //in progress
}

void goAmber()
{
  isAmber = true;
  //irsend.sendNEC(amberCode, 32);
  //sendCode(0, amberCode);
  delay(100);
  irrecv.enableIRIn();
  setStrips(true, 255, 150, 0);
}

IRCode repeatCode;

int whiteButtonVal = 0; // value read from button
int whiteButtonLast = 0; // buffered value of the button's previous state
long btnDnTime; // time the button was pressed down
long btnUpTime; // time the button was released
boolean ignoreUp = false; // whether to ignore the button release because the click+hold was triggered

//values for button hold
#define debounce 20 // ms debounce period to prevent flickering when pressing or releasing the button
#define holdTime 2000 // ms hold period: how long to wait for press+hold event

void changeMode()
{
  Serial.print("Mode changed to ");
  Serial.println(currentMode + 1);
  if(isAmber)
  {
    isAmber = false;
    currentMode = 0;
    Serial.println("Broke amber");
    int red = digitalRead(REDBTN_PIN);
    int green = digitalRead(GREENBTN_PIN);
    int blue = digitalRead(BLUEBTN_PIN);
    parseSwitches(false, red, green, blue);
  }
  else
  {
    if(currentMode < 2)
    {
      currentMode++;
    }
    else
    {
      int red = digitalRead(REDBTN_PIN);
      int green = digitalRead(GREENBTN_PIN);
      int blue = digitalRead(BLUEBTN_PIN);
      parseSwitches(false, red, green, blue);
      currentMode = 0;
    }
  }
  blinkLED(STATUS_PIN, currentMode + 1);
}

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
    if (ignoreUp == false) buttonPressed();
    else ignoreUp = false;
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

void buttonPressed()
{
  delay(500);
  if(digitalRead(WHITEBTN_PIN) == HIGH)
  {
    Serial.println("DBL PRS");
    buttonDoublePressed();
  }
  else
  {
    changeMode();
  }
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

  }

  if(red && !green && blue)
  {
    currentMode = COP_MODE;
  }

  if(red && !green && !blue)
  {
    setStrips(true, 0, 0, 0);
    currentMode = HEART_MODE;
  }
}

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
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
  Serial.println("INITed");
  //irsend.sendNEC(onCode, 32);
  delay(50);
  irrecv.enableIRIn();
}

void loop()
{
  buttonCheck();
  if(!isAmber)
  {
    if(currentMode == NORMAL_MODE)
    {
      int red = digitalRead(REDBTN_PIN);
      int green = digitalRead(GREENBTN_PIN);
      int blue = digitalRead(BLUEBTN_PIN);
      parseSwitches(true, red, green, blue);
      /*
    if (irrecv.decode(&results))
       {
       Serial.println("Got IR");
       digitalWrite(STATUS_PIN, HIGH);
       Serial.println(results.value, HEX);
       storeCode(&results, repeatCode);
       //if a ir code is received from remote, use that instead of buttons
       interiorOverride(repeatCode);
       delay(50);
       digitalWrite(STATUS_PIN, LOW);
       irrecv.resume(); // resume receiver
       }
       */
    }
    else if(currentMode == FADE_MODE)
    {
      setStrips(true, 0, 0, 0);
      fade();
    }
    else if(currentMode == STROBE_MODE)
    {
      int red = digitalRead(REDBTN_PIN);
      int green = digitalRead(GREENBTN_PIN);
      int blue = digitalRead(BLUEBTN_PIN);
      strobeSwitches(red, green, blue);
    }
    else if(currentMode == COP_MODE)
    {
      setStrips(false, 255, 0, 0);
      delay(75);
      setStrips(false, 0, 0, 255);
      delay(75);
    }
    else if(currentMode == HEART_MODE)
    {
      heartBeat(); 
    }
  }
  else
  {
    wait = 5;
    setStrips(true, 255, 100, 0);
    wait = 1;
  }
}













/*
 * IRrecord: record and play back IR signals as a minimal 
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * An IR LED must be connected to the output PWM pin 3.
 * A button must be connected to the input BUTTON_PIN; this is the
 * send button.
 * A visible LED can be connected to STATUS_PIN to provide status.
 *
 * The logic is:
 * If the button is pressed, send the IR code.
 * If an IR code is received, record it.
 *
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */
 
#include <IRremote.h>
#include "IRCode.h"

int RECV_PIN = 11;
int BUTTON_PIN = 12;
int STATUS_PIN = 13;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

void setup()
{
  Serial.begin(57600);
  Serial.println("init");
  irrecv.enableIRIn(); // Start the receiver
  pinMode(BUTTON_PIN, INPUT);
  pinMode(STATUS_PIN, OUTPUT);
}

// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results *results, IRCode& store) {
  store.codeType = results->decode_type;
  int count = results->rawlen;
  if (store.codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
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
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (store.codeType == SONY) {
      Serial.print("Received SONY: ");
    } 
    else if (store.codeType == RC5) {
      Serial.print("Received RC5: ");
    } 
    else if (store.codeType == RC6) {
      Serial.print("Received RC6: ");
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
      irsend.sendNEC(REPEAT, code.codeLen);
      Serial.println("Sent NEC repeat");
    } 
    else {
      irsend.sendNEC(code.codeValue, code.codeLen);
      Serial.print("Sent NEC ");
      Serial.println(code.codeValue, HEX);
    }
  } 
  else if (code.codeType == SONY) {
    irsend.sendSony(code.codeValue, code.codeLen);
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
      irsend.sendRC5(code.codeValue, code.codeLen);
    } 
    else {
      irsend.sendRC6(code.codeValue, code.codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(code.codeValue, HEX);
    }
  } 
  else if (code.codeType == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    irsend.sendRaw(code.rawCodes, code.codeLen, 38);
    Serial.println("Sent raw");
  }
}

void dumpCode(IRCode code)
{
  Serial.println("---DUMPING IRCODE---");
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

int lastButtonState;

IRCode testCode;

void loop() {
  // If button pressed, send the code.
  int buttonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("Released");
    irrecv.enableIRIn(); // Re-enable receiver
  }
  /*
  if (buttonState) {
   Serial.println("Pressed, sending");
   digitalWrite(STATUS_PIN, HIGH);
   sendCode(lastButtonState == buttonState);
   digitalWrite(STATUS_PIN, LOW);
   delay(50); // Wait a bit between retransmissions
   } 
   else if (irrecv.decode(&results)) {
   digitalWrite(STATUS_PIN, HIGH);
   storeCode(&results);
   
   irrecv.resume(); // resume receiver
   digitalWrite(STATUS_PIN, LOW);
   }
   */
  if (irrecv.decode(&results))
  {
    digitalWrite(STATUS_PIN, HIGH);
    storeCode(&results, testCode);
    dumpCode(testCode);
    irrecv.resume(); // resume receiver
    digitalWrite(STATUS_PIN, LOW);
  }
  lastButtonState = buttonState;
}



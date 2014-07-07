#include <Servo.h>

Servo trunkServo;
int RFIDResetPin = 13;

char tag1[13] = "------------";
char tag2[13] = "------------";

void setup()
{
  Serial.begin(9600);
  trunkServo.attach(8);
  trunkServo.write(178);
  pinMode(RFIDResetPin, OUTPUT);
  digitalWrite(RFIDResetPin, HIGH);
}

void loop()
{
  char tagString[0];
  int index = 0;
  boolean reading = false;

  while(Serial.available())
  {
    int readByte = Serial.read();
    if(readByte == 2) reading = true; //begining of tag
    if(readByte == 3) reading = false; //end of tag

    if(reading && readByte != 2 && readByte != 10 && readByte != 13){
      //store the tag
      tagString[index] = readByte;
      index++;
    }
  }
  checkTag(tagString); //Check if it is a match
  clearTag(tagString); //Clear the char of all value
  resetReader(); //reset the RFID reader
}

void checkTag(char tag[])
{
  if(strlen(tag) == 0) return;
  if(compareTag(tag, tag1) || compareTag(tag, tag2))
  {
    openTrunk();
  }
  else
  {
    Serial.print("Tag: ");
    Serial.print(tag);
    Serial.println(" didn't match...");
  }
}

void openTrunk()
{                                     
  trunkServo.write(20);
  delay(1000);
  trunkServo.write(178);
  delay(2000);
}

/*---------------------------*/
/*--------RFID Code----------*/
/*---------------------------*/
void resetReader()
{
  digitalWrite(RFIDResetPin, LOW);
  digitalWrite(RFIDResetPin, HIGH);
  delay(150);
}

void clearTag(char one[])
{
  for(int i = 0; i < strlen(one); i++)
  {
    one[i] = 0;
  }
}

boolean compareTag(char one[], char two[])
{
  if(strlen(one) == 0) return false; //empty

  for(int i = 0; i < 12; i++)
  {
    if(one[i] != two[i]) return false;
  }
  return true; //no mismatches
}





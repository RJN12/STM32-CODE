#include <Wire.h>

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
}

void loop()
{
  
  delay(100);
}


void receiveEvent(int howMany)
{
  while(0 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);
  }
Serial.print('\n');
   
  
}
####Code ref: https://www.arduino.cc/en/Tutorial/LibraryExamples/MasterWriter

// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

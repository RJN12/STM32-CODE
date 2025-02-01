#include<SoftwareSerial.h>

#define GSM_Rx 2 
#define GSM_Tx 3
#define LED 13

SoftwareSerial gsm(GSM_Rx,GSM_Tx);

void setup()
{
pinMode(LED,OUTPUT);
digitalWrite(LED,LOW);
gsm.begin(9600);
Serial.begin(9600);
gsm.println("AT+CMGF=1");
delay(1000);
gsm.println("AT+CNMI=1,2,0,0,0");
delay(1000);

}

void loop()
{
  if(gsm.available())
  {
   String test = gsm.readString();
  
  if(test.indexOf("Yes") != -1)
 {
 digitalWrite(LED,HIGH);
 Serial.println("LED is ON now");
 } 
 else if (test.indexOf("No") != -1)
 {
 digitalWrite(LED,LOW);
 Serial.print("LED is OFF now");
 }
  }
  
}

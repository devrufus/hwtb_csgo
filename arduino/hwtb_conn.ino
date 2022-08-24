#include <Keyboard.h>
#include <Mouse.h>

//const int ledPin = 13;

void setup() 
{
  Keyboard.begin();
  Mouse.begin();
  Serial.begin(115200);
  //pinMode(ledPin, OUTPUT);
}

void loop() 
{
  while(Serial.available())
  {
int data = Serial.read();
    if(data == '.')
    {
   Keyboard.write(' ');
    }
       if(data == '-')
    {
    Mouse.click();
    }
   }
  }

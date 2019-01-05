#include <Servo.h>

#define MAX_SIGNAL 2000
#define MIN_SIGNAL 1000

Servo motor[4];

void setup() 
{
    unsigned long     _gCurrTime = 0;  
    int               i = 0;
    
    Serial.begin(115200);
    Serial.println("Program begin...");
    Serial.println("This program will calibrate the ESC.");

    for(i=0 ; i<4 ; i++)
        motor[i].attach(i + 8);

    {
        Serial.println("Now writing maximum output.");
        Serial.println("Turn on power source, then wait 2 seconds and press any key.");
    
        motor[0].writeMicroseconds(MAX_SIGNAL);
        motor[1].writeMicroseconds(MAX_SIGNAL);
        motor[2].writeMicroseconds(MAX_SIGNAL);
        motor[3].writeMicroseconds(MAX_SIGNAL);

        _gCurrTime = micros();
      
        // Wait for input
        //while((micros() - _gCurrTime) < 2000000);
        delay(4000);
      
        // Send min output
        Serial.println("Sending minimum output");
        motor[0].writeMicroseconds(MIN_SIGNAL);
        motor[1].writeMicroseconds(MIN_SIGNAL);
        motor[2].writeMicroseconds(MIN_SIGNAL);
        motor[3].writeMicroseconds(MIN_SIGNAL);
    }
}

void loop() {  

}

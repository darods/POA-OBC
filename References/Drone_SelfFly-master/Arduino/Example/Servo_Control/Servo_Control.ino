#include <Servo.h> 
 
Servo nServo0;
Servo nServo1;
 
int nServoPin0 = 9;
int nServoPin1 = 10;
int angle = 90; // servo position in degrees 
 
void setup() 
{    
    Serial.begin(115200);
    
    nServo0.attach(nServoPin0, 800, 2200);
    nServo1.attach(nServoPin1, 800, 2200);

    delay(500); 
    nServo0.writeMicroseconds(1000); 
    delay(10); 
    nServo1.writeMicroseconds(1000); 
    delay(500); 

    delay(500); 
    nServo0.writeMicroseconds(1500); 
    delay(10); 
    nServo1.writeMicroseconds(1500); 
    delay(500); 

    delay(500); 
    nServo0.writeMicroseconds(2000); 
    delay(10); 
    nServo1.writeMicroseconds(2000); 
    delay(500); 

    delay(500); 
    nServo0.writeMicroseconds(1500); 
    delay(10); 
    nServo1.writeMicroseconds(1500); 
    delay(500); 
} 
 
void loop() 
{ 
    if(Serial.available())
    {
        int nAngleH = Serial.read();
        int nAngleV = Serial.read();

        if('\n' == Serial.read())
        {          
            nAngleH = (int)(map(nAngleH, 0, 180, 800, 2200));
            nAngleV = (int)(map(nAngleV, 0, 180, 800, 2200));

            nServo0.writeMicroseconds(nAngleV);
            delay(2);            
            nServo1.writeMicroseconds(nAngleH);
            delay(2);
        }
        else
        {
            nServo0.writeMicroseconds(1500); 
            delay(2); 
            nServo1.writeMicroseconds(1500); 
            delay(2);
        }
    }
    
    delay(10);
}

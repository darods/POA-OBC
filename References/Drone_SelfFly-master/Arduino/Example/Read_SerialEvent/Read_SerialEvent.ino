#include <Servo.h>

Servo nServo0;
Servo nServo1;

int nServoPin0 = 9;
int nServoPin1 = 10;

void setup() {
    Serial.begin(115200);
    
    nServo0.attach(nServoPin0, 600, 2400);
    nServo1.attach(nServoPin1, 600, 2400);

    delay(1000); 
    nServo0.writeMicroseconds(1000); 
    delay(10); 
    nServo1.writeMicroseconds(1000); 
    delay(1000); 

    delay(1000); 
    nServo0.writeMicroseconds(1500); 
    delay(10); 
    nServo1.writeMicroseconds(1500); 
    delay(1000); 

    delay(1000); 
    nServo0.writeMicroseconds(2000); 
    delay(10); 
    nServo1.writeMicroseconds(2000); 
    delay(1000); 
}


void loop() 
{
    if(Serial.available())
    {
        int nAngleH = Serial.read();
        int nAngleV = Serial.read();

        if('\n' == Serial.read())
        {          
            nAngleH = (int)(map(nAngleH, 0, 180, 600, 2400));
            nAngleV = (int)(map(nAngleV, 0, 180, 600, 2400));

            nServo0.writeMicroseconds(nAngleH);
            delay(2);            
            nServo1.writeMicroseconds(nAngleV);
            delay(2);
        }
    }
    
    delay(10);
}


//void serialEvent()
//{
//    int nAngle = Serial.parseInt();
//    
//    //if(Serial.read() == '\n')
//    {
//        nServo0.write(nAngle);
//        delay(1);
//    }
//}


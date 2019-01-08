const int chipSelect = 4;
/*
 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4   
 */
#include <SdFat.h>
SdFat sd;
SdFile myFile;

char fileName[] = "2468.txt";
int sdPower = 3;
void setup() 
{ Serial.begin(9600); 
  pinMode(sdPower,OUTPUT);
}

void loop() 
{  
  digitalWrite(sdPower,HIGH);
  sd.begin(chipSelect, SPI_HALF_SPEED);
  myFile.open(fileName, O_RDWR | O_CREAT | O_AT_END);

  myFile.println("Hello World");


  myFile.close();  

  // re-open the file for reading:
  if (!myFile.open(fileName, O_READ)) {
    sd.errorHalt("opening test.txt for read failed");
  }

  // read from the file until there's nothing else in it:
  int data;
  while ((data = myFile.read()) >= 0) Serial.write(data);
  // close the file:
  myFile.close();

  digitalWrite(sdPower,LOW);
 delay(1000);  
}

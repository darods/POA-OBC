/*   codigo de ejemplo, obtenido de https://www.instructables.com/id/How-to-Use-XBee-Modules-As-Transmitter-Receiver-Ar/
 *    ~ Simple Arduino - xBee Transmitter sketch ~

  Read an analog value from potentiometer, then convert it to PWM and finally send it through serial port to xBee.
  The xBee serial module will send it to another xBee (resiver) and an Arduino will turn on (fade) an LED.
  The sending message starts with '<' and closes with '>' symbol. 
  
  Dev: Michalis Vasilakis // Date:2/3/2016 // Info: www.ardumotive.com // Licence: CC BY-NC-SA                    */

//Constants: 
const int potPin = A0; //Pot at Arduino A0 pin 
//Variables:
int value ; //Value from pot

void setup() {

  //Start the serial communication
  Serial.begin(9600); //Baud rate must be the same as is on xBee module

}

void loop() {
    
    //Read the analog value from pot and store it to "value" variable
    value = analogRead(A0);
    //Map the analog value to pwm value
    value = map (value, 0, 1023, 0, 255);
  //Send the message:
  Serial.print('<');  //Starting symbol
  Serial.print(value);//Value from 0 to 255
  Serial.println('>');//Ending symbol


}
  

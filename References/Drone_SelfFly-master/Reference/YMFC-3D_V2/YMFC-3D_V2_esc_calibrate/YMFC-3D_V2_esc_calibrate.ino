///////////////////////////////////////////////////////////////////////////////////////
//Terms of use
///////////////////////////////////////////////////////////////////////////////////////
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////////////
//Safety note
///////////////////////////////////////////////////////////////////////////////////////
//Always remove the propellers and stay away from the motors unless you 
//are 100% certain of what you are doing.
///////////////////////////////////////////////////////////////////////////////////////

#include <EEPROM.h>                                  //Include the EEPROM.h library so we can store information onto the EEPROM

//Declaring global variables
byte last_channel_1, last_channel_2, last_channel_3, last_channel_4;
byte eeprom_data[36];
int receiver_input_channel_1, receiver_input_channel_2, receiver_input_channel_3, receiver_input_channel_4;
int counter_channel_1, counter_channel_2, counter_channel_3, counter_channel_4, start;
int receiver_input[5];
//int temp;
unsigned long timer_channel_1, timer_channel_2, timer_channel_3, timer_channel_4, esc_timer, esc_loop_timer;
unsigned long zero_timer, timer_1, timer_2, timer_3, timer_4, current_time;

//Setup routine
void setup(){  
  //Arduino Uno pins default to inputs, so they don't need to be explicitly declared as inputs
  DDRB |= B00011111;                                 //Configure digital poort 8, 9, 10, 11, 12 as output
  
  PCICR |= (1 << PCIE2);    // set PCIE0 to enable PCMSK0 scan
  PCMSK2 |= (1 << PCINT18);  // set PCINT18 (digital input 2) to trigger an interrupt on state change
  PCMSK2 |= (1 << PCINT19);  // set PCINT19 (digital input 3) to trigger an interrupt on state change
  PCMSK2 |= (1 << PCINT20);  // set PCINT20 (digital input 4) to trigger an interrupt on state change
  PCMSK2 |= (1 << PCINT21);  // set PCINT21 (digital input 5) to trigger an interrupt on state change
  PCMSK2 |= (1 << PCINT22);  // set PCINT22 (digital input 6) to trigger an interrupt on state change

  //Serial.begin(57600);      //Start the serial connetion @ 57600bps
  
  //Read EEPROM for fast access data
  for(start = 0; start <= 35; start++)
  {
    eeprom_data[start] = EEPROM.read(start);
    //Serial.println(eeprom_data[start]);
  }
  
  //Check the EEPROM signature to make sure that the setup program is executed
  while(eeprom_data[33] != 'J' || eeprom_data[34] != 'M' || eeprom_data[35] != 'B'){
    delay(500);
    digitalWrite(12, !digitalRead(12));              //Change the led status to indicate error.
  }
  //Serial.println("Step1");
  wait_for_receiver();                               ///Wait until the receiver is active.
  //Serial.println("Step2");
  zero_timer = micros();                             //Set the zero_timer for the first loop.
}

//Main program loop
void loop(){
  receiver_input_channel_3 = convert_receiver_channel(3);    //Convert the actual receiver signals for throttle to the standard 1000 - 2000us
    
  while(zero_timer + 4000 > micros());                       //Start the pulse after 4000 micro seconds.
  zero_timer = micros();                                     //Reset the zero timer.
  PORTB |= B00001111;                                        //Set port 8, 9, 10 11, and 12 high at once
  timer_channel_1 = receiver_input_channel_3 + zero_timer;   //Calculate the time when digital port 8 is set low
  timer_channel_2 = receiver_input_channel_3 + zero_timer;   //Calculate the time when digital port 9 is set low
  timer_channel_3 = receiver_input_channel_3 + zero_timer;   //Calculate the time when digital port 10 is set low
  timer_channel_4 = receiver_input_channel_3 + zero_timer;   //Calculate the time when digital port 11 is set low
  
  while(PORTB & B00001111){                                        //Execute the loop until digital port 8 to 11 is low
    esc_loop_timer = micros();                               //Check the current time
    if(timer_channel_1 <= esc_loop_timer)PORTB &= B11111110; //When the delay time is expired, digital port 8 is set low
    if(timer_channel_2 <= esc_loop_timer)PORTB &= B11111101; //When the delay time is expired, digital port 9 is set low
    if(timer_channel_3 <= esc_loop_timer)PORTB &= B11111011; //When the delay time is expired, digital port 10 is set low
    if(timer_channel_4 <= esc_loop_timer)PORTB &= B11110111; //When the delay time is expired, digital port 11 is set low
  }
  
}

//This routine is called every time input 8, 9, 10 or 11 changed state
ISR(PCINT2_vect){
  current_time = micros();
  //Channel 1=========================================
  if(PIND & B00000100){                                        //Is input 8 high?
    if(last_channel_1 == 0){                                   //Input 8 changed from 0 to 1
      last_channel_1 = 1;                                      //Remember current input state
      timer_1 = current_time;                                  //Set timer_1 to current_time
    }
  }
  else if(last_channel_1 == 1){                                //Input 8 is not high and changed from 1 to 0
    last_channel_1 = 0;                                        //Remember current input state
    receiver_input[1] = current_time - timer_1;                 //Channel 1 is current_time - timer_1
  }
  //Channel 2=========================================
  if(PIND & B00001000 ){                                       //Is input 9 high?
    if(last_channel_2 == 0){                                   //Input 9 changed from 0 to 1
      last_channel_2 = 1;                                      //Remember current input state
      timer_2 = current_time;                                  //Set timer_2 to current_time
    }
  }
  else if(last_channel_2 == 1){                                //Input 9 is not high and changed from 1 to 0
    last_channel_2 = 0;                                        //Remember current input state
    receiver_input[2] = current_time - timer_2;                 //Channel 2 is current_time - timer_2
  }
  //Channel 3=========================================
  if(PIND & B00010000 ){                                       //Is input 10 high?
    if(last_channel_3 == 0){                                   //Input 10 changed from 0 to 1
      last_channel_3 = 1;                                      //Remember current input state
      timer_3 = current_time;                                  //Set timer_3 to current_time
    }
  }
  else if(last_channel_3 == 1){                                //Input 10 is not high and changed from 1 to 0
    last_channel_3 = 0;                                        //Remember current input state
    receiver_input[3] = current_time - timer_3;                 //Channel 3 is current_time - timer_3
  }
  //Channel 4=========================================
  if(PIND & B00100000 ){                                       //Is input 11 high?
    if(last_channel_4 == 0){                                   //Input 11 changed from 0 to 1
      last_channel_4 = 1;                                      //Remember current input state
      timer_4 = current_time;                                  //Set timer_4 to current_time
    }
  }
  else if(last_channel_4 == 1){                                //Input 11 is not high and changed from 1 to 0
    last_channel_4 = 0;                                        //Remember current input state
    receiver_input[4] = current_time - timer_4;                 //Channel 4 is current_time - timer_4
  }
}

//Checck if the receiver values are valid within 10 seconds
void wait_for_receiver(){
  byte zero = 0;                                                                //Set all bits in the variable zero to 0
  while(zero < 15){                                                             //Stay in this loop until the 4 lowest bits are set
    if(receiver_input[1] < 2100 && receiver_input[1] > 900)zero |= 0b00000001;  //Set bit 0 if the receiver pulse 1 is within the 900 - 2100 range
    if(receiver_input[2] < 2100 && receiver_input[2] > 900)zero |= 0b00000010;  //Set bit 1 if the receiver pulse 2 is within the 900 - 2100 range
    if(receiver_input[3] < 2100 && receiver_input[3] > 900)zero |= 0b00000100;  //Set bit 2 if the receiver pulse 3 is within the 900 - 2100 range
    if(receiver_input[4] < 2100 && receiver_input[4] > 900)zero |= 0b00001000;  //Set bit 3 if the receiver pulse 4 is within the 900 - 2100 range
    delay(500);                                                                 //Wait 500 milliseconds
  }
}

//This part converts the actual receiver signals to a standardized 1000 – 1500 – 2000 microsecond value.
//The stored data in the EEPROM is used.
int convert_receiver_channel(byte function){
  byte channel, reverse;                                                       //First we declare some local variables
  int low, center, high, actual;
  int difference;
  
  channel = eeprom_data[function + 23] & 0b00000111;                           //What channel corresponds with the specific function
  if(eeprom_data[function + 23] & 0b10000000)reverse = 1;                      //Reverse channel when most significant bit is set
  else reverse = 0;                                                            //If the most significant is not set there is no reverse
  
  actual = receiver_input[channel];                                            //Read the actual receiver value for the corresponding function
  low = (eeprom_data[channel * 2 + 15] << 8) | eeprom_data[channel * 2 + 14];  //Store the low value for the specific receiver input channel
  center = (eeprom_data[channel * 2 - 1] << 8) | eeprom_data[channel * 2 - 2]; //Store the center value for the specific receiver input channel
  high = (eeprom_data[channel * 2 + 7] << 8) | eeprom_data[channel * 2 + 6];   //Store the high value for the specific receiver input channel
  
  if(actual < center){                                                         //The actual receiver value is lower than the center value
    if(actual < low)actual = low;                                              //Limit the lowest value to the value that was detected during setup
    difference = ((long)(center - actual) * (long)500) / (center - low);       //Calculate and scale the actual value to a 1000 - 2000us value
    if(reverse == 1)return 1500 + difference;                                  //If the channel is reversed
    else return 1500 - difference;                                             //If the channel is not reversed
  }
  else if(actual > center){                                                                        //The actual receiver value is higher than the center value
    if(actual > high)actual = high;                                            //Limit the lowest value to the value that was detected during setup
    difference = ((long)(actual - center) * (long)500) / (high - center);      //Calculate and scale the actual value to a 1000 - 2000us value
    if(reverse == 1)return 1500 - difference;                                  //If the channel is reversed
    else return 1500 + difference;                                             //If the channel is not reversed
  }
  else return 1500;
}


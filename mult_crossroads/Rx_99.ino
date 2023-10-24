#include <SPI.h>
#include <RF22.h>
#include <RF22Router.h>
#include "traffic_lights.h"

#define MY_ADDRESS 99 // define my unique address
#define TRAFFIC 1 // define who I can talk to
#define FINE 2 // define who I can talk to
#define LIGHT 69 // define who I can talk to
#define PREV_LIGHT 4
#define NEXT_LIGHT 5

enum state {
  red = 0,
  green,
  orange
};
  
// Singleton instance of the radio
RF22Router rf22(MY_ADDRESS); // initiate the class to talk to my radio with MY_ADDRESS
int received_value=0;

int number_of_bytes=0; // will be needed to measure bytes of message

float throughput=0; // will be needed for measuring throughput
int flag_measurement=0;

int counter=0;

int current_state;
unsigned long current_time;
unsigned long next_state_time;

int car_counter; // Current index to track the position in the dataPoints array
int car_fine;
long randNumber; 
bool red_to_green_sent;
int other_state;
int previous_sens_val;
int sensor_value;

void setup() {
  Serial.begin(9600);
  if (!rf22.init())
    Serial.println("RF22 init failed");
  // Defaults after init are 463.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  if (!rf22.setFrequency(463.0)) // The frequency should be the same as that of the transmitter. Otherwise no communication will take place
    Serial.println("setFrequency Fail");
  rf22.setTxPower(RF22_TXPOW_20DBM);
  //1,2,5,8,11,14,17,20 DBM
  rf22.setModemConfig(RF22::OOK_Rb40Bw335  );// The modulation should be the same as that of the transmitter. Otherwise no communication will take place
  //modulation

  // Manually define the routes for this network
  rf22.addRouteTo(TRAFFIC, TRAFFIC); // tells my radio card that if I want to send data to TRAFFIC then I will send them directly to TRAFFIC and not to another radio who would act as a relay 
  rf22.addRouteTo(FINE, FINE);
  rf22.addRouteTo(LIGHT, LIGHT);
  rf22.addRouteTo(PREV_LIGHT, PREV_LIGHT);
  rf22.addRouteTo(NEXT_LIGHT, NEXT_LIGHT);

  led_initialize(green);
  current_state = green;
  current_time = millis();
  next_state_time = current_time + timer(current_state);
  car_counter = 0;
  car_fine = 0;
  red_to_green_sent = false;
  other_state=-1;
  previous_sens_val = 0;
  sensor_value = 0;

  // Set random seed
  randomSeed(analogRead(A0));
  
  delay(100); 
}

void loop() 
{
  // Should be a message for us now  
  uint8_t buf[RF22_ROUTER_MAX_MESSAGE_LEN];
  char incoming[RF22_ROUTER_MAX_MESSAGE_LEN];
  memset(buf, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
  memset(incoming, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
  uint8_t len = sizeof(buf); 
  uint8_t from;
  //digitalWrite(5, LOW);
  if (rf22.recvfromAck(buf, &len, &from)) // I'm always expecting to receive something from any legitimate transmitter. If I do, I'm sending an acknowledgement
  {
 //   digitalWrite(5, HIGH);
    buf[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0';
    memcpy(incoming, buf, RF22_ROUTER_MAX_MESSAGE_LEN); // I'm copying what I have received in variable incoming
    //Serial.print("got request from : ");
    //Serial.println(from, DEC);
    received_value=atoi((char*)incoming); // transforming my data into an integer
  }
    
  current_time = millis();
  sensor_value = 0;

  if(from == FINE){
    // Data came from sensor after traffic light
    if (current_state == red && received_value){
      Serial.println("ILLEGAL! BUSTED!");
    }
  }
    
  else if(from == LIGHT && other_state != received_value){
    // Data came from another traffic light in the junction
    other_state = received_value;
    Serial.println(other_state);

    switch(other_state) {
      case red:
        //Serial.println("Other light is red, wants to go green. Turn orange!");
        orange_on();

        current_state = orange;
        next_state_time = current_time + timer(current_state);
        
        break;
      case orange: 
        //Serial.println("Other light is red, wants to go red. Turn green!");
        green_on();
        car_counter = 0;
        red_to_green_sent = false;
        current_state = green;
        
        break;
    }  
  }

  // Data came from original sensor or no data
  switch(current_state) {
    case red:
      for(int i=0;i<1000;i++){
        sensor_value += analogRead(A0);
      }

      if (sensor_value > 500) {
        sensor_value = 1;
      }
      else {
        sensor_value = 0;
      }
      Serial.println(sensor_value);
      if (!red_to_green_sent){
        if (previous_sens_val != sensor_value){
            car_counter += sensor_value;
            previous_sens_val = sensor_value;
        }
        if (current_time >= next_state_time || car_counter >= 2 || from == PREV_LIGHT){
          //Serial.println("I am red, i wants to go green. Send to other light!");
          if (car_counter >= 2){
            sendToTrafficLights(current_state,PREV_LIGHT);
            sendToTrafficLights(current_state,NEXT_LIGHT);
            }
          sendToTrafficLights(current_state,LIGHT);
          red_to_green_sent = true;
        }
      }
      
      break;
    case green:
       if (from == NEXT_LIGHT){
        orange_on();
        current_state = orange;
        next_state_time = current_time + timer(current_state);
       }
       break;
    case orange:
      if (current_time >= next_state_time){
        //Serial.println("I am orange and time ended. Go red!");
        sendToTrafficLights(current_state,LIGHT);
        red_on();
        red_to_green_sent = false;
        current_state = red;
        next_state_time = current_time + timer(current_state);
      }
      break;
  }
  
  
}

void sendToTrafficLights(int state, int DESTINATION_ADDRESS){
  bool send_failed = true;

  char data_read[RF22_ROUTER_MAX_MESSAGE_LEN];
  uint8_t data_send[RF22_ROUTER_MAX_MESSAGE_LEN];
  memset(data_read, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
  memset(data_send, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);    
  sprintf(data_read, "%d", state); // I'm copying the measurement sensorVal into variable data_read
  data_read[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0'; 
  memcpy(data_send, data_read, RF22_ROUTER_MAX_MESSAGE_LEN); // now I'm copying data_read to data_send
  number_of_bytes=sizeof(data_send); // I'm counting the number of bytes of my message
  //Serial.print("Number of Bytes= ");
  //Serial.println(number_of_bytes);  // and show the result on my monitor

  // just demonstrating that the string I will send, after those transformation from integer to char and back remains the same
  int sensorVal2=0;
  sensorVal2=atoi(data_read);
  Serial.print("The string I'm ready to send is= ");
  Serial.println(sensorVal2);

  while (send_failed) {
    if (rf22.sendtoWait(data_send, sizeof(data_send), DESTINATION_ADDRESS) != RF22_ROUTER_ERROR_NONE) // I'm sending the data in variable data_send to DESTINATION_ADDRESS... cross fingers
    {
      //Serial.println("sendtoWait failed"); // for some reason I have failed
      randNumber=random(100, 1000); 
      delay(randNumber);
    }
    else
    {
      counter=counter+1;
      //Serial.println("sendtoWait Successful"); // I have received an acknowledgement from DESTINATION_ADDRESS. Data have been delivered!
      send_failed = false;
    }
  }
}

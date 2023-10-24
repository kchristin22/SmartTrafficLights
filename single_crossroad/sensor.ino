#include <SPI.h>
#include <RF22.h>
#include <RF22Router.h>

#define MY_ADDRESS 49 // define my unique address
#define DESTINATION_ADDRESS_1 69 // traffic light address

// Singleton instance of the radio
RF22Router rf22(MY_ADDRESS); // initiate the class to talk to my radio with MY_ADDRESS
int number_of_bytes=0; // will be needed to measure bytes of message

float throughput=0; // will be needed for measuring throughput
int flag_measurement=0;

int counter=0;
int initial_time=0;
int final_time=0;

long randNumber; 
int sensorVal;
bool send_failed;

// named constant for the pin the sensor is connected to
const int sensorPin = A0; // will be needed to measure something from pin A0

void setup() {
  Serial.begin(9600); // to be able to view the results in the computer's monitor
  if (!rf22.init()) // initialize my radio
    Serial.println("RF22 init failed");
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  if (!rf22.setFrequency(463.0)) // set the desired frequency
    Serial.println("setFrequency Fail");
  rf22.setTxPower(RF22_TXPOW_20DBM); // set the desired power for my transmitter in dBm
  //1,2,5,8,11,14,17,20 DBM
  rf22.setModemConfig(RF22::OOK_Rb40Bw335); // set the desired modulation
  //modulation

  // Manually define the routes for this network
  rf22.addRouteTo(DESTINATION_ADDRESS_1, DESTINATION_ADDRESS_1); // tells my radio card that if I want to send data to DESTINATION_ADDRESS_1 then I will send them directly to DESTINATION_ADDRESS_1 and not to another radio who would act as a relay

  // Set random seed
  randomSeed(analogRead(A0));
  
  delay(1000); // delay for 1 s
}

void loop() 
{
  counter=0;
  sensorVal = 0;
  send_failed = true;
  initial_time=millis();

  // measure force sensor value
  for(int i=0;i<1000;i++){
    sensorVal += analogRead(A0);
  }

  if (sensorVal > 500) {
    sensorVal = 1;
    Serial.println("here");
  }
  else {
    sensorVal = 0;
  }

  //Serial.print("My measurement is: ");
  //Serial.println(sensorVal); // and show it on Serial

  // the following variables are used in order to transform my integer measured value into a uint8_t variable, which is proper for my radio
  char data_read[RF22_ROUTER_MAX_MESSAGE_LEN];
  uint8_t data_send[RF22_ROUTER_MAX_MESSAGE_LEN];
  memset(data_read, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
  memset(data_send, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);    
  sprintf(data_read, "%d", sensorVal); // I'm copying the measurement sensorVal into variable data_read
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

  // TODO: implement ALOHA
  while (send_failed) {
    if (rf22.sendtoWait(data_send, sizeof(data_send), DESTINATION_ADDRESS_1) != RF22_ROUTER_ERROR_NONE) // I'm sending the data in variable data_send to DESTINATION_ADDRESS_1... cross fingers
    {
      //Serial.println("sendtoWait failed"); // for some reason I have failed
      randNumber=random(100, 1000); 
      delay(randNumber);
    }
    else
    {
      counter=counter+1;
      //Serial.println("sendtoWait Successful"); // I have received an acknowledgement from DESTINATION_ADDRESS_1. Data have been delivered!
      send_failed = false;
    }
  }
  final_time=millis();
  throughput=(float)counter*number_of_bytes*1000.0/(final_time-initial_time); // *1000 is because time is measured in ms. This is not the communication throughput, but rather each measurement-circle throughput.
  // Serial.print("Throughput=");
  // Serial.print(throughput);
  // Serial.println("Bytes/s");
  // Serial.print("Initial time= ");  
  // Serial.print(initial_time);
  // Serial.print("     Final time= ");  
  // Serial.println(final_time);

}

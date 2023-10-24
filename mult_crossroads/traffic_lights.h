#define RED_LED_PIN 4
#define GREEN_LED_PIN 9
#define ORANGE_LED_PIN 7

#include <TimerOne.h>

void led_initialize(int initialState){
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(ORANGE_LED_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(ORANGE_LED_PIN, LOW);
  
  if(initialState == 0){
    digitalWrite(RED_LED_PIN, HIGH);  
  }
  else if(initialState == 1){
    digitalWrite(GREEN_LED_PIN, HIGH);  
  }
}


void red_on(){
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(ORANGE_LED_PIN, LOW);
}


void green_on(){
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(ORANGE_LED_PIN, LOW);
}

void orange_on(){
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(ORANGE_LED_PIN, HIGH);
}

unsigned long timer(int state){
  switch(state){
     case 0:
        //red
        return 10000;
     case 2:    
        //orange
        return 1000;
   } ;  
  
}

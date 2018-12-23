#include <SoftwareSerial.h>

boolean enableControl = false;
boolean enableAutonomous = true;

const uint8_t TRIG = 7;
const uint8_t ECHO = 8;

int turningDelay = 290;
int minDist = 40;//minimum distance that the sensor should detect obstacle
const uint8_t inA1 = 2; 
const uint8_t inA2 = 3;
const uint8_t inB1 = 4;
const uint8_t inB2 = 5;
const uint8_t INTERNAL_FAN = 12;

long breakTime = 5;//(min)How long will the bot rest?
long maxRunningTime = 5;//(min)How long will the bot run before rest?

long time;
long previousTime = 0;
int distance;

char DAT;
char AutoCMD = 'F'; //command for autoPiloting

SoftwareSerial HC06(10, 11);//Rx Tx

void setup() {
  Serial.begin(9600);
  ADCSRA &= ~(1 << 7);//Turn of ADC to save power
  
    //Turn all OUTPUTS to "Output" to save power
  for(uint8_t i = 0; i < 20; i++){    
    if(i != ECHO)pinMode(i, OUTPUT);
  }

  if(enableControl){
    pinMode(10, INPUT);
    HC06.begin(9600);
  }
  
  bot_Move(false, false, false, false);
  delay(2000);
}

void loop() {
  if(enableControl){
    if (HC06.available() > 0){
      DAT = HC06.read();
    }

    switch(DAT){
        case 'A':
        //FORWARD     F      B      R      L
        if(!bot_Move(true, false, false, false))return;
        break;

        case 'B':
          if(!bot_Move(false, true, false, false))return;
          break;

        case 'C':
          if(!bot_Move(false, false, false, true))return;
          break;

        case 'D':
          if(!bot_Move(false, false, true, false))return;
          break;

        case 'z':
          if(!bot_Move(false, false, false, false))return;
          break;
      }
  
    while(AUTONOMOUS_ENABLE(AutoCMD)){
      goAutonomous(turningDelay);
    }
    return;
  }

  if(enableAutonomous){
    goAutonomous(turningDelay);
  }
}
//   FORWARD     F      B      R      L
//if(!bot_Move(true, false, false, false))return;
boolean bot_Move(bool FOR, bool BACK, bool RIGH, bool LEF){
  if(FOR){
    digitalWrite(inA1, HIGH);
    digitalWrite(inA2, LOW);
    digitalWrite(inB1, HIGH);
    digitalWrite(inB2, LOW);
    return true;
  }

  if(BACK){
    digitalWrite(inA1, LOW);
    digitalWrite(inA2, HIGH);
    digitalWrite(inB1, LOW);
    digitalWrite(inB2, HIGH);
    return true;
  }

  if(RIGH){
    digitalWrite(inA1, LOW);
    digitalWrite(inA2, HIGH);
    digitalWrite(inB1, HIGH);
    digitalWrite(inB2, LOW);
    return true;
  }

  if(LEF){
    digitalWrite(inA1, HIGH);
    digitalWrite(inA2, LOW);
    digitalWrite(inB1, LOW);
    digitalWrite(inB2, HIGH);
    return true;
  }

  else{
    digitalWrite(inA1, LOW);
    digitalWrite(inA2, LOW);
    digitalWrite(inB1, LOW);
    digitalWrite(inB2, LOW);
    return false;
  }
}

void restBot(long duration){
  while(!isTime(duration * 1000 * 60)){
    bot_Move(false, false, false, false);
    digitalWrite(INTERNAL_FAN, HIGH);
  }
  digitalWrite(INTERNAL_FAN, LOW);
  return;
}

void sendStatus(bool RANGE, bool ACTION){
  if(RANGE){
    HC06.println("GF");
  }

  if(ACTION){
    HC06.println("AO");
  }
}

void goAutonomous(int turnDelay){
  //go forward
  if(isTime(maxRunningTime * 1000 * 60)){
    restBot(breakTime);
    return;
  }
  
  if(!bot_Move(true, false, false, false))return;
  sendStatus(true, false);
  
  if(obstaclePresent(minDist)){ //If obstacle is in front
    sendStatus(false, true); 
    if(!bot_Move(false, false, true, false))return; //turn right
    delay(turnDelay);
    
    if(obstaclePresent(minDist)){
      sendStatus(false, true);
      if(!bot_Move(false, false, false, true))return; //go around
      delay(turnDelay * 2);

      if(obstaclePresent(minDist)){
        sendStatus(false, true);
        if(!bot_Move(false, false, false, true))return;
        delay(turnDelay);
        return;
      }
      return;
    }
     return;
  }
}

boolean obstaclePresent(uint8_t minimumDist){
  //return one if obstacle is present
  int nowdist = getDistance();
  if(nowdist <= minimumDist)return true;
  else return false;
}

int getDistance(){
  digitalWrite(TRIG,LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG,LOW);
 
  time = pulseIn(ECHO,HIGH);
  //distance=time*340/20000;

  // Calculating the distance
  distance = time * 0.034 / 2;
  return distance;
}

int AUTONOMOUS_ENABLE(char cmd){
  DAT = HC06.read();
  if(DAT == cmd)return 1;
  else return 0;
}

boolean isTime(long interval){
   unsigned long now = millis();
   if(now - previousTime > interval){
      previousTime = now;
      return true; 
   }

   else{
      return false;
   }
}


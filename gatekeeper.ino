//**************************************************************//
//  Name    : gatekeeper                                        //
//  Author  : Karsten Schlachter                                //
//  Date    : Oktober 2015 letztes update Nov 16                //
//  Version : 1.2                                               //
//  Notes   : Software fuer Tuer-Arduino                        //
//  Web   : https://wiki.schaffenburg.org/Projekt:Tuerschloss   // 
//****************************************************************


//TODO: der ganze zustandsspass mit fwd als variable stammt noch aus dem konzept mit dem cd-laufwerk..sollte iwann sinnvoller gemacht werden

#include <Wire.h>
#include <Servo.h> 
#define SLAVE_ADDRESS 0x04
 
#define ONTIME_LEVEL1 5000
#define ONTIME_LEVEL2 10000 
 
 //VORSICHT: pin 2(sda)&3(scl) enstprechen beim leonardo den genutzten scl und sda pins für i2c (bei uno a4(sda) u a5(scl))
int pinServo=11,pinServoPower=8;
int pinRGB_R=5,pinRGB_G=13;
int pinDoorSense=4;
int pinMotionSense=0;
int pinButton=9;


void sendData();
void receiveData(int byteCount);


enum LockState {ERR=-1,MOVING,LOCKED,OPEN,DOLOCK,PUBLIC,GOPUBLIC};

struct RGBColor{int r; int g; int b;};
bool operator==(const RGBColor& c1,const RGBColor& c2){
  return (c1.r==c2.r && c1.g==c2.g && c1.b==c2.b);
  }
RGBColor statusLight={0,0,0};

unsigned long lastLightChange=0;

int fwd=0;
LockState lockState=MOVING;//-1=fehler 0=gestoppt 1=verriegelt 2=offen
int lastButtonState=0,buttonState=0;
int lastDoorState=0,doorState=0;
int stopped=0;
int lastServoPos=0;


boolean position1=false;
boolean position2=false;


boolean bewegungsstatus;
unsigned long curMillis=0, lastMotion=0,lastAction=0;
int curLightLevel=0;

Servo servo;

void   updateLastAction(){
      lastAction=millis();
}
 
 
void lightOn(){
  curLightLevel=1;
}
 
void lightOff(){
  curLightLevel=0;
}
 

void setServoPos(int pos){
        //Servo wegen jitter nur während bewegung mit strom versorgen
        if(pos!=lastServoPos){
          digitalWrite(pinServoPower,HIGH);
          delay(100);
          servo.write(pos);
          //TODO: zeit nach betrag der änderung abschätzen
          delay(2000);//servo zeit geben um position einzunehmen
      //   if(pos==0){//stromversorgung nur bei stellung 0 deaktivieren
          digitalWrite(pinServoPower,LOW);
        //  }
          lastServoPos=pos;
        }
}

void checkMotion(){
  bewegungsstatus=digitalRead(pinMotionSense);
 
 
  if (bewegungsstatus == HIGH)
  {
 //   digitalWrite(MOTIONLEDPIN, HIGH);

    updateLastAction();
    lastMotion=lastAction;
    Serial.println("bewegung!");
    }
  else
  {
  //digitalWrite(MOTIONLEDPIN, LOW);
  }
  
  
  if(lastAction!=0 ){
    curMillis=millis();
  
     if(curMillis<ONTIME_LEVEL1 || lastAction>(curMillis-ONTIME_LEVEL1)){
 //  Serial.println((curMillis-onTime));
         if(curLightLevel==0){
     
         lightOn();
         }
     }else if(curLightLevel==1){
  // if(curLightLevel){
     lightOff();
  // }
 }
  }
  
  
}



void setup() {

 
  pinMode(pinRGB_R, OUTPUT);
  pinMode(pinRGB_G, OUTPUT);
  
  pinMode(9,INPUT_PULLUP);

  pinMode(pinDoorSense,INPUT_PULLUP);
  pinMode(pinMotionSense,INPUT);
  
  servo.attach(pinServo);
  pinMode(pinServoPower, OUTPUT);
    
  Serial.begin(9600);
  
  //i2c einrichten
  Wire.begin(SLAVE_ADDRESS);  
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
  lockState=LOCKED;
  
}


void loop() {
  static unsigned long buttonPressStart=0;
  checkMotion();
  
  //steigende flanke vom button
  buttonState=digitalRead(pinButton);
  if(buttonState != lastButtonState){
   updateLastAction();
        if(buttonState==0){
          buttonPressStart=millis();
          Serial.println("Button betaetigt");
          while(digitalRead(pinButton)==0){};
          
              if(lockState==OPEN){//wenn offen auf verriegeln vorbereiten
                  lockState=DOLOCK;
                  Serial.println("bereite verriegeln vor");
                }else if(lockState==DOLOCK){//verriegeln abbrechen
                  lockState=OPEN;
                  Serial.println("verriegeln abgebrochen");
                }else{//im verriegelten zustand tuer entsperren
                    //TODO: dauerhaftes halten für 0.5 oder 1s voraussetzen
                     lockState=MOVING;
                     fwd=1;
                     Serial.println("tuer entriegeln - derzeit deaktiviert");
                  }
                /*  fwd=!fwd;
              Serial.println("fwd-change");
              Serial.println(fwd);*/
          }
    }

/*
   position1=! digitalRead(pinPosSense1);
   position2=! digitalRead(pinPosSense2);
   
   //paarueberwachung
   if(position1 && position2){
     //da stimmt was nicht
     }
  */   
     
    //statuslicht
    switch(lockState){
      case MOVING: //orange
      statusLight={150,120,0};
            break;
            
      case LOCKED:
      statusLight={200,0,0};
          break;
          
      case OPEN:
      statusLight={0,200,0};
          break;
          
      case DOLOCK://blinken
          if((millis()-lastLightChange)>1000){
            Serial.println("blink");
              if(statusLight==((RGBColor){0,0,0})){
              statusLight={100,0,0};
              }else{
              statusLight={0,0,0};
              }
              lastLightChange=millis();
            }
          break;   
          
      default://aus
      
        statusLight={0,0,0};
      };
      
      if(curLightLevel==1 || lockState==DOLOCK){
          analogWrite(pinRGB_R,statusLight.r);
          analogWrite(pinRGB_G,statusLight.g);
     }else{
        analogWrite(pinRGB_R,0);
         analogWrite(pinRGB_G,0);
       }
    
  lastButtonState=buttonState;

  if(lockState==DOLOCK){

    //bei tuer von offen nach zu verriegeln
      doorState=digitalRead(pinDoorSense);
  if(doorState != lastDoorState){
    Serial.println("door state changed");
        if(doorState==0){//tuer geschlossen
              //etwas zeit zum schliessen der tür lassen (und um es sich nochmal anders zu ueberlegen weil man was vergessen hat)
              delay(3000);
              //tuer immernoch geschlossen oder zwischenzeitlich wieder geoeffnet?
              doorState=digitalRead(pinDoorSense);//status updaten       
              if(doorState==0){

                  lockState=MOVING;
                  fwd=0;
               }

          }
    }
    lastDoorState=doorState;
    
  }else{
 
    if(fwd==1){
        lockState=MOVING;
        setServoPos(0);
        lockState=OPEN;
    }else{
        lockState=MOVING;
        setServoPos(120);
 
        lockState=LOCKED;
     }
    /*
    if(fwd && position2 || !fwd && position1){
      //motor stoppen
      digitalWrite(pinMotor1,LOW);
      digitalWrite(pinMotor2,LOW);
   //   Serial.println("endposition");
      lockState=(position1)?OPEN:LOCKED;
        
   }else{
      digitalWrite(pinMotor1,fwd);
      digitalWrite(pinMotor2,!fwd);
      lockState=MOVING;
    }
    */
    
  }
//  digitalWrite(8, !digitalRead(9));  

  delay(100);             
}


static int INPUT_BUFF_LEN = 10;
char input_buff[10];
int input_len=0;



boolean exec_cmd(char cmd[]){

  if (strcmp(cmd,"open") == 0){//oeffnen orignal code: 1
    fwd=1;
    updateLastAction();
  
  }else if(strcmp(cmd,"close") == 0){//code 2
    fwd=0;
    updateLastAction();
    
  }else {//status uebermitteln
   //nichts tun - status wird immer zurueckgegeben
    return false;
  }
  return true;
}

// i2c callbacks 
void receiveData(int byteCount){

  while(Wire.available()) {
  
   char c=Wire.read();
   
   Serial.print("data received: ");
   Serial.println(c);
   Serial.print("cmd buff: ");
   Serial.println(input_buff);
  
   if(c=='\n'){//cmd ggf ausfuehren, dann buffer "leeren"
      exec_cmd(input_buff); 
      input_len=0;
   }else if(input_len<(INPUT_BUFF_LEN-1)){
      input_buff[input_len++]=c;
      input_buff[input_len]='\0';
   }



  }
}

void sendData(){
  Wire.write(lockState);
  Serial.println("gesendet:");
  Serial.println(lockState);
}

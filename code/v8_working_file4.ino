#include <SoftwareSerial.h>
SoftwareSerial Serial1(4, 3); // RX, TX

#define SELPIN 12 //Selection Pin 
#define DATAOUT 11//MOSI 
#define DATAIN  10//MISO 
#define SPICLOCK  9//Clock 
#define butt1  6// 
#define butt2  7// 

int volt_range = 0; // 0 = 0-5v range, !0 = 0-1v range
int fiveV_range[2] = {4096, 1023};
int oneV_range[2] = {819, 204};
int range_spi = 0;
int range_an = 0;

int readvalue;
int ledC = 13;
int ledV = 8;
int potVal[16];
int OLDpotVal[16];

//address 0 contains nothing!
int midiCCselect[16] = {0,3,4,5,7,8,9,10,12,11,1,2,17,20,18,19}; // select the midi Controller Number for each input 


void setup(){ 

 //  Set MIDI baud rate:
 Serial1.begin(31250); // MIDI OUT
 //set pin modes 
 pinMode(SELPIN, OUTPUT); 
 pinMode(DATAOUT, OUTPUT); 
 pinMode(DATAIN, INPUT); 
 pinMode(SPICLOCK, OUTPUT); 

 pinMode(ledC, OUTPUT);
 pinMode(ledV, OUTPUT);

 pinMode(butt1, INPUT);  
 pinMode(butt2, INPUT);  
 
 //disable device to start with 
 digitalWrite(SELPIN,HIGH); 
 digitalWrite(DATAOUT,LOW); 
 digitalWrite(SPICLOCK,LOW); 

 //check for mode change to 0-1v range//
if (digitalRead(butt1) && digitalRead(butt2)){
  
  int countMode = 0;
  for(int i = 0; i <=50; i++){
    if (butt1 && butt2){countMode ++;}
  }
  if (countMode > 30){
    digitalWrite(ledV,HIGH);
    volt_range = 1;
  }
}

 ///

 Serial.begin(9600); 
 delay(10);
 Serial.println("V4/V8 Test Program");
 delay(10);

 //read and update knobs so that we dont burst send all paramiters at starup
  readAllPots();
  mapPots();
  readButts();
 copyA(potVal, OLDpotVal, sizeof(potVal));
} 

void loop() { 
  readAllPots();
  mapPots();
  readButts();
  checkForValChange();
  
  copyA(potVal, OLDpotVal,  sizeof(potVal)); //copy new vals into old vals
  
  for (int i = 0; i <= 15; i++) {
  //Serial.print(OLDpotVal[i]);
  //Serial.print(" ");
  }
  //Serial.println(" ");
  delay(3);
  
  
} 

void copyA(int* src, int* dst, int len) {
    memmove(dst, src, sizeof(src[0])*len);
}

void readAllPots() {

for (int i = 1; i <= 8; i++) {
  potVal[i] = read_adc(i); 
}
  potVal[9] = analogRead(A0); 
  potVal[10] = analogRead(A1); 
  potVal[11] = analogRead(A2); 
  potVal[12] = analogRead(A3); 
  potVal[13] = analogRead(A4); 

}

void readButts() {
  potVal[14] = digitalRead(butt1)*127;
  potVal[15] = digitalRead(butt2)*127;

  
}

void mapPots() {
  
  if(volt_range == 0){ // handle 0-5v and 0-1v mapping
     range_spi = fiveV_range[0];
     range_an = fiveV_range[1];
    } else {
     range_spi = oneV_range[0];
     range_an = oneV_range[1];
    }

    for (int i = 1; i <= 13; i++) {
        if ((potVal[i] > range_spi)&&(i <= 8))
          {potVal[i] = range_spi; }
        if ((potVal[i] > range_an)&&(i > 8))
          {potVal[i] = range_an; }
    }
   Serial.print(potVal[12]);
  Serial.print(" ");

  for (int i = 0; i <= 8; i++) {
  potVal[i] = map(potVal[i], 0, range_spi, 0, 127 ); 
}

  
  potVal[9] = map(potVal[9], 0, range_an, 0, 127 ); 
  for (int i = 10; i <= 11; i++) {
  potVal[i] = map(potVal[i], 0, range_an, 0, 7 ); 
}

  potVal[12] = map(potVal[12], 0, range_an, 0, 2 );
  potVal[13] = map(potVal[13], 0, range_an, 0, 127 ); 
  



}

void checkForValChange(){
  for (int i = 0; i <= 15; i++) {
  if((OLDpotVal[i] != potVal[i])){//if the new value is different from the old value 
    midiCC(0xB0, midiCCselect[i], potVal[i]);
    //Serial.print("OLD VAL ");
    //Serial.print(OLDpotVal[i]);
    //Serial.print(" ");
    //Serial.print("NEW VAL ");
    //Serial.println(potVal[i]);
    
    }
  }
  
}

int read_adc(int channel){
  int adcvalue = 0;
  byte commandbits = B11000000; //command bits - start, mode, chn (3), dont care (3)

  //allow channel selection
  commandbits|=((channel-1)<<3);

  digitalWrite(SELPIN,LOW); //Select adc
  // setup bits to be written
  for (int i=7; i>=3; i--){
    digitalWrite(DATAOUT,commandbits&1<<i);
    //cycle clock
    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);    
  }

  digitalWrite(SPICLOCK,HIGH);    //ignores 2 null bits
  digitalWrite(SPICLOCK,LOW);
  digitalWrite(SPICLOCK,HIGH);  
  digitalWrite(SPICLOCK,LOW);

  //read bits from adc
  for (int i=11; i>=0; i--){
    adcvalue+=digitalRead(DATAIN)<<i;
    //cycle clock
    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);
  }
  digitalWrite(SELPIN, HIGH); //turn off device
  return adcvalue;
}

void midiCC(int CC_data, int c_num, int c_val){
  digitalWrite(ledC, !digitalRead(ledC));
  Serial1.write(CC_data);
  Serial1.write(c_num);
  Serial1.write(c_val);
  Serial.println(c_val);
  digitalWrite(ledC, !digitalRead(ledC));
}



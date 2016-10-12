//FIXME
const int programPin=3;
const int readPin=4;
const int enablePin=2;

const unsigned long romSize=1024*1024;


//The pinout from the eprom is different from the snes pinout 
int adrPins[20]={22,//eprom A0  snes A0
                  23,//eprom A1  snes A1
                  24,//eprom A2  snes A2
                  25,//eprom A3  snes A3
                  26,//eprom A4  snes A4
                  27,//eprom A5  snes A5
                  28,//eprom A6  snes A6
                  29,//eprom A7  snes A7
                  30,//eprom A8  snes A8
                  31,//eprom A9  snes A9
                  32,//eprom A10 snes A10
                  33,//eprom A11 snes A11
                  34,//eprom A12 snes A12
                  35,//eprom A13 snes A13
                  36,//eprom A14 snes A14
                  37,//eprom A15 snes A15
                  40,//38,//eprom A16 snes A18 *
                  41,//39,//eprom A17 snes A19 *
                  38,//40,//eprom A18 snes A16 *
                  39,//41 //eprom A19 snes A17 * 
                  };
                  
char dataPins[8]={5,6,7,8,9,10,11,12};
/*
Frame Format
program:
|preamble|opt|addr0|addr1|addr2|numbbutes|bytes|checksum|end
read:
|preamble|opt|addr0|addr1|addr2|numbbutes|end


*/
byte inByte=0;
unsigned int secH=0,secL=0;

void setup() {
  pinMode(programPin,OUTPUT);
  pinMode(readPin,OUTPUT);
  pinMode(enablePin,OUTPUT);
  //FIXME 
  for(int i=0;i<20;i++){
    pinMode(adrPins[i],OUTPUT);
  }  
  digitalWrite(programPin,LOW);
  digitalWrite(readPin,LOW);
  digitalWrite(enablePin,HIGH);
  Serial.begin(250000);
  delay(1000);
  programMode();
//  setAddress(0);
//  programByte(0x78);
//  //writeSector(5);
}
int index=0;
void loop() {
  if(Serial.available()){
    inByte=Serial.read();
    if(inByte==0x55){
      while(Serial.available()==0);
      inByte=Serial.read();
      switch(inByte){
        case 'w':
          programMode();
          while(Serial.available()<2);
          secH=Serial.read();
          secL=Serial.read();
          writeSector(secH,secL);
          break;
        case 'r':
          readMode();
          readROM();
          break;
      }
    }
  }
}


//low level functions, direct ccontact with hardware pins
void programMode(){
  //data as output
  for(int i=0;i<8;i++){
    pinMode(dataPins[i],OUTPUT);
  }
  digitalWrite(readPin,LOW);
  digitalWrite(programPin,HIGH);
}
void readMode(){
  //data as input
  for(int i=0;i<8;i++){
    pinMode(dataPins[i],INPUT);
  }
//  for(int i=0;i<8;i++){
//    digitalWrite(dataPins[i],HIGH);
//  }
  digitalWrite(programPin,LOW);
  digitalWrite(readPin,LOW);
  
}
void setAddress(uint32_t Addr){
    for(int i=0;i<8;i++){
      digitalWrite(adrPins[i],Addr&(1<<i));
    }
    Addr=Addr>>8;
    for(int i=0;i<8;i++){
      digitalWrite(adrPins[i+8],Addr&(1<<i));
    }
    Addr=Addr>>8;
    for(int i=0;i<4;i++){
      digitalWrite(adrPins[i+16],Addr&(1<<i));
    }
}
byte readByte(unsigned long adr){
    byte data;
    setAddress(adr);
    digitalWrite(enablePin,LOW);
    delayMicroseconds(1);
    for(int i=7;i>=0;i--){
        data=data<<1;
        data|=digitalRead(dataPins[i])&1;
    }
    digitalWrite(enablePin,HIGH);
    return data;
}
void setData(char Data){
  for(int i=0;i<8;i++){
      digitalWrite(dataPins[i],Data&(1<<i));
  }
}
void programByte(byte Data){
  //select address
  //
  //setAddress(adr); 
  setData(Data);
  //Vpp pulse
  delayMicroseconds(4);
  digitalWrite(enablePin,LOW);
  delayMicroseconds(60);
  digitalWrite(enablePin,HIGH);
}

void writeSector(unsigned char sectorH,unsigned char sectorL){
  byte dataBuffer[128];
  unsigned long address=0;
  byte CHK=sectorH,CHKreceived;
  CHK^=sectorL;
  
  address=sectorH;
  address=(address<<8)|sectorL;
  address*=128;

  for(int i=0;i<128;i++){
        while(Serial.available()==0);
        dataBuffer[i]=Serial.read();
        CHK ^= dataBuffer[i];
  }
  while(Serial.available()==0);
  CHKreceived=Serial.read();
  programMode();
  //only program the bytes if the checksum is equal to the one received
  if(CHKreceived==CHK){
    for (int i = 0; i < 128; i++){
      setAddress(address++);
      programByte(dataBuffer[i]);
    }
  Serial.write(CHK);
  }
  readMode();
  
}
int readROM(){
  unsigned long num=1024*1024;
  unsigned long address;
  byte data,checksum=0;
  address=0;
  //read mode
  readMode();
  //start frame
  digitalWrite(readPin,LOW);
  digitalWrite(programPin,LOW);
  for(long i;i<1048576;i++){//1048576
    data=readByte(address++);
    Serial.write(data);
    //checksum^=data;
  }
  digitalWrite(readPin,HIGH);
    
  //Serial.write(checksum);
  //Serial.write(0xAA);
}



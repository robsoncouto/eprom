const int programPin=3;
const int readPin=4;
const int enablePin=2;

const unsigned long romSize=1024*1024;


//The pinout from the eprom is different from the snes pinout
int adrPins[20]={22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40};

char dataPins[8]={5,6,7,8,9,10,11,12};
byte inByte=0;
unsigned int secH=0,secL=0;

void setup() {
  // put your setup code here, to run once:
  pinMode(programPin,OUTPUT);
  pinMode(readPin,OUTPUT);
  pinMode(enablePin,OUTPUT);
  for(int i=0;i<20;i++){
    pinMode(adrPins[i],OUTPUT);
  }
  digitalWrite(programPin,LOW);
  digitalWrite(readPin,LOW);
  digitalWrite(enablePin,HIGH);
  Serial.begin(230400);
  delay(1000);
  programMode();
}
int index=0;
void loop() {
  // put your main code here, to run repeatedly:
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
    delayMicroseconds(10);
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
  //only program the bytes into eprom if the checksum is equal to the one received
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

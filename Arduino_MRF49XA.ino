/*
    Розробив Roman
    Канал на YouTube: https://goo.gl/x8FL2o
    Відео з проектом: https://youtu.be/5U3ED4m1bmw
    Спроба подружити Arduino з MRF49XA. 
    За основу було взято схему піротехнічного пульта на 31 команду з цієї статі: https://goo.gl/GNG4hi
    2018 Roman
*/

#include<SPI.h>

//--------------------------------------------------------------------
// MRF49XA SPI commands:
//--------------------------------------------------------------------
#define    CREG                  0x801F         
#define    FSREG                 0xAE10         
#define    TCREG                 0x98F0
#define    AFCREG                0xC400        
#define    PMREG0                0x8239         
#define    PMREG1                0x8201         
//--------------------------------------------------------------------
//  FSK/DATA/FSEL:
//--------------------------------------------------------------------
#define    DATA                  9
#define    PERIOD                170
//--------------------------------------------------------------------
//  Default data:
//--------------------------------------------------------------------
byte DataABCDEiD[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF};



void setup() {
  pinMode(DATA, OUTPUT);
  digitalWrite(DATA, HIGH);
  digitalWrite(SS, HIGH);
  Serial.begin(9600);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV128);  
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode (SPI_MODE0);
}

void loop() {
 Serial.println("The transmitter is running");
 Serial.println("Started test transmission...");
 TestTransmission();
 Serial.println("Finished test transmission");
 SerialPrint();
 Serial.println("The sequence of bytes: A B C D E id");
 while(true){
  if (ReadData()) {
    SerialPrint();
    Serial.println("Data transfer is started");
    for(uint8_t i = 0; i < 15; i++){
    StartTransmission();
    SynchronizingSignal();
    for(uint8_t j = 0; j < sizeof(DataABCDEiD); BitsTransfer(DataABCDEiD[j++]));
    delayMicroseconds(PERIOD);
    EndTransmission();
    delay(10);
    }
    Serial.println("Data transfer is finished");
    Serial.println("The sequence of bytes: A B C D E id");
    } else {
      Serial.println("Invalid input Bytes!");
      Serial.println("The sequence of bytes: A B C D E id");
      }
  }
}

void SerialPrint(){
    Serial.println("Bytes transfer:");
    Serial.print("A = ");
    Serial.println(DataABCDEiD[0], HEX);
    Serial.print("B = ");
    Serial.println(DataABCDEiD[1], HEX);
    Serial.print("C = ");
    Serial.println(DataABCDEiD[2], HEX);
    Serial.print("D = ");
    Serial.println(DataABCDEiD[3], HEX);
    Serial.print("E = ");
    Serial.println(DataABCDEiD[4], HEX);
    Serial.print("ID = ");
    Serial.println(DataABCDEiD[5], HEX);
  }


boolean ReadData(){
 byte tmp;
 uint8_t count = 0; 
 while(true){
 if(Serial.available() > 0){
 tmp = Serial.read();
 if(tmp == 0x0A) return true;
 if(tmp == 0x20) continue;
 if(((tmp >= 0x30) && (tmp <= 0x39)) or ((tmp >= 0x41) && (tmp <= 0x46)) or ((tmp >= 0x61) && (tmp <= 0x66))) {
  } else return false;
  if ((tmp >= 0x61) && (tmp <= 0x66)) tmp = tmp & 0xDF;
  if(count >= 12) count = 0;
  if ((tmp >= 0x30) && (tmp <= 0x39)) {
    tmp = tmp & 0x0F;
    } else {
    tmp = tmp + 0x09;
    tmp = tmp & 0x0F;
      }
  if((count%2) == 0) {
    DataABCDEiD[count / 2] = DataABCDEiD[count / 2] & 0x0F;
    DataABCDEiD[count / 2] = DataABCDEiD[count / 2] | (tmp << 4);
    } else {
    DataABCDEiD[(count - 1) / 2] = DataABCDEiD[(count - 1) / 2] & 0xF0;
    DataABCDEiD[(count - 1) / 2] = DataABCDEiD[(count - 1) / 2] | tmp;
      }
  count++;
 }
 }
 } 
  
void BitsTransfer(byte bits){
  BitsTransferRRF(bits);
  BitsTransferRLF(bits);
  }

void BitsTransferRRF(byte bits){
  for(uint8_t i = 0; i < 8; i++){
  if(bits & 0x80) {
    TransferDataR();
    }
    else {
    TransferDataL();
    }
  bits = bits << 1;
  }
  }
void BitsTransferRLF(byte bits){
  for(uint8_t i = 0; i < 8; i++){
  if(bits & 0x01) {
    TransferDataL();
    }
    else {
    TransferDataR();
    }
  bits = bits >> 1;
  }
  }

void TransferDataR(){
    digitalWrite(DATA, HIGH);
    delayMicroseconds(PERIOD);
    digitalWrite(DATA, LOW);
    delayMicroseconds(PERIOD);
    delayMicroseconds(PERIOD);
  }
  
void TransferDataL(){
    digitalWrite(DATA, HIGH);
    delayMicroseconds(PERIOD);
    delayMicroseconds(PERIOD);
    digitalWrite(DATA, LOW);
    delayMicroseconds(PERIOD);
  }
  
  

void SynchronizingSignal(){
  delay(2);
  for(uint8_t i = 0; i < 12; i++){
  digitalWrite(DATA, HIGH);
  delayMicroseconds(PERIOD);
  digitalWrite(DATA, LOW);
  delayMicroseconds(PERIOD);
  }
  delay(2);
  }

void StartTransmission(){
  digitalWrite(SS, LOW);
  SPI.transfer16(CREG);
  SPI.transfer16(FSREG);
  SPI.transfer16(TCREG);
  SPI.transfer16(AFCREG);
  SPI.transfer16(PMREG0);
  digitalWrite(SS, HIGH);
  }

void EndTransmission(){
  digitalWrite(SS, LOW);
  SPI.transfer16(PMREG1);
  digitalWrite(SS, HIGH);
  }

void TestTransmission(){
  delay(1000);
  for(uint8_t i = 0; i < 2; i++){
  digitalWrite(SS, LOW);
  SPI.transfer16(PMREG1);
  SPI.transfer16(CREG);
  SPI.transfer16(FSREG);
  SPI.transfer16(TCREG);
  SPI.transfer16(AFCREG);
  SPI.transfer16(PMREG0);
  digitalWrite(SS, HIGH);
  delay(1000);
  EndTransmission();
  delay(1000);
  }
  }

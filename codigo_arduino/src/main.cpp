#include <Arduino.h>
#include <SoftwareSerial.h>
#include<Wire.h>    
#include <RTClib.h>
#include <SPI.h>

#define disk1 0x50
//Address of 24LC256 eeprom chip

void writeEEPROM(int deviceaddress, unsigned int eeaddress, byte data );
byte readEEPROM(int deviceaddress, unsigned int eeaddress );
void nextionCommand(String nextionName, long value);



SoftwareSerial SerialNextion(2, 3); // RX, TX
String  endChar = String(char(0xff)) + String(char(0xff)) + String(char(0xff));
RTC_DS3231 RTC;

String dfd;
byte lastMin=0;


unsigned int address_red = 0;
unsigned int address_green = 1;
unsigned int address_blue = 2;


long red_value;
long green_value;
long blue_value;
int motor_state = 0;
int motor_state6 = 0;

int alarmNumber = 0;

int ledRojo = 9;
int ledVerde = 10;
int ledAzul = 11;


unsigned long periodo = 2000;
unsigned long TiempoAhora = 0;

int NumeroElementosAlarma =4;

unsigned int ArrayAddresHora[] = {6,7,8,9};
unsigned int ArrayAddresMinuto[] = {12,13,14,15};

int ArrayHoraAlarma[4];
int ArrayMinutosAlarma[4];

int alarmid=0;

int delayAsincronoMinuto[] = {0,0,0,0};
int delayAsincronoHora[] = {0,0,0,0};

union main
{
  char charByte[4];
  long valLong;
}value;


void setup() {
  SerialNextion.begin(9600);                       //The default baud rate of the Nextion TFT is 9600.         
  Serial.begin(9600);
  Wire.begin();  
  RTC.begin();

  //RTC.adjust(DateTime(__DATE__, __TIME__));

  pinMode(LED_BUILTIN, OUTPUT); 
  pinMode(ledRojo,OUTPUT);
  pinMode(ledVerde,OUTPUT);
  pinMode(ledAzul,OUTPUT);

  red_value = int(readEEPROM(disk1, address_red));
  green_value = int(readEEPROM(disk1, address_green));
  blue_value = int(readEEPROM(disk1, address_blue));

  //for( int i=0; i<NumeroElementosAlarma; i++){
  //  writeEEPROM(disk1, ArrayAddresHora[i], ArrayHoraAlarma[i]);
  //  writeEEPROM(disk1, ArrayAddresMinuto[i], ArrayMinutosAlarma[i]);
  //}

  for( int i=0; i<NumeroElementosAlarma; i++){
    ArrayHoraAlarma[i] = readEEPROM(disk1, ArrayAddresHora[i]);
    ArrayMinutosAlarma[i]= readEEPROM(disk1, ArrayAddresMinuto[i]);
  }

  analogWrite(ledRojo,red_value); 
  analogWrite(ledVerde,green_value); 
  analogWrite(ledAzul,blue_value); 
  digitalWrite(LED_BUILTIN, LOW);

  //writeEEPROM(disk1, address_red, 123);
  //writeEEPROM(disk1, address_green, 0);
  //writeEEPROM(disk1, address_blue, 0);
  Serial.print("Configurandose....");
  while ((unsigned long) (millis() - TiempoAhora) > periodo){
        Serial.print(".");
        TiempoAhora = millis();
      }
  Serial.println(".");
}

void loop() {



DateTime now = RTC.now();

if(lastMin!=now.minute()){
    
    lastMin=now.minute();
    Serial.print(now.hour());
    Serial.print(':');
    Serial.println(now.minute());

  for (int j = 0; j<NumeroElementosAlarma; j++){
    if((now.hour()==delayAsincronoHora[j]) & (now.minute()==delayAsincronoMinuto[j])) {
      if((unsigned long) (millis() - TiempoAhora) > 500){
        Serial.println("Fin de Alarma");
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        Serial.println(j);
        TiempoAhora = millis();
      }
      
    }
  }

  for (int i = 0; i<NumeroElementosAlarma; i++){
    if((now.hour()==ArrayHoraAlarma[i]) & (now.minute()==ArrayMinutosAlarma[i])) {
      if((unsigned long) (millis() - TiempoAhora) > periodo){
        Serial.println("Alarma");
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        Serial.println(i);
        TiempoAhora = millis();
      }
      
    }
  }

  

}

if(SerialNextion.available()>0)                  //If we receive something...
  {
  dfd += char(SerialNextion.read());
  Serial.println(dfd);
  Serial.println(dfd.length());

  if((dfd.substring(0,5)=="alarm")& (dfd.length()==9)){
    value.charByte[0]=char(dfd[5]);
     alarmid = value.valLong;

    switch (alarmid)
    {
    case 0:
      Serial.println("Alarma1");
      alarmNumber = 1;
      break;
    case 1:
      Serial.println("Alaram2");
      alarmNumber =2;
      break;
    case 2:
      Serial.println("Alarma3");
      alarmNumber =3;
      break;
    case 3:
      Serial.println("Alarma4");
      alarmNumber=4;
    default:
      break;
    }
  
    dfd="";
  }
  
    if((dfd.substring(0,4)=="time") & (dfd.length()==9)){
    
    switch (alarmNumber)
    {
    case 1:
      value.charByte[0]= char(dfd[5]);

      if(dfd.substring(4,5)=="1")  ArrayHoraAlarma[0]= value.valLong;
      if(dfd.substring(4,5)=="2")  ArrayMinutosAlarma[0]= value.valLong;

      delayAsincronoMinuto[0] = ArrayMinutosAlarma[0] +1 ;
      delayAsincronoHora[0] = ArrayHoraAlarma[0];
      Serial.println("Alarma1Set");
      break;
    case 2:
      value.charByte[0]= char(dfd[5]);

      if(dfd.substring(4,5)=="1")  ArrayHoraAlarma[1]= value.valLong;
      if(dfd.substring(4,5)=="2")  ArrayMinutosAlarma[1]= value.valLong;

      delayAsincronoMinuto[1] = ArrayMinutosAlarma[1] +1 ;
      delayAsincronoHora[1] = ArrayHoraAlarma[1];
      Serial.println("Alarma2Set");
      break;
    case 3:
      value.charByte[0]= char(dfd[5]);

      if(dfd.substring(4,5)=="1")  ArrayHoraAlarma[2]= value.valLong;
      if(dfd.substring(4,5)=="2")  ArrayMinutosAlarma[2]= value.valLong;

      delayAsincronoMinuto[2] = ArrayMinutosAlarma[2] +1 ;
      delayAsincronoHora[2] = ArrayHoraAlarma[2];
      Serial.println("Alarma3Set");
      break;
    case 4:
      value.charByte[0]= char(dfd[5]);

      if(dfd.substring(4,5)=="1")  ArrayHoraAlarma[3]= value.valLong;
      if(dfd.substring(4,5)=="2")  ArrayMinutosAlarma[3]= value.valLong;

      delayAsincronoMinuto[3] = ArrayMinutosAlarma[3] +1 ;
      delayAsincronoHora[3] = ArrayHoraAlarma[3];
      Serial.println("Alarma4Set");
      break;

    default:
      break;
    }
    dfd="";
  }

  

  if((dfd.substring(0,3)=="red")& (dfd.length()==7)){

    value.charByte[0]= char(dfd[3]);

    red_value = value.valLong; 
    analogWrite(ledRojo,red_value); 
    dfd="";
  }

  if((dfd.substring(0,5)=="green")& (dfd.length()==9)){

    value.charByte[0]= char(dfd[5]);

    green_value = value.valLong;  
    analogWrite(ledVerde,green_value); 

    dfd="";
  }

  if((dfd.substring(0,4)=="blue")& (dfd.length()==8)){

    value.charByte[0]= char(dfd[4]);

    blue_value = value.valLong;  
    analogWrite(ledAzul,blue_value); 

    dfd="";
  }

  if((dfd.substring(0,5)=="motor")&(dfd.length()==10)){

    value.charByte[0]= char(dfd[6]);

    if(dfd.substring(4,5)=="1")  motor_state = value.valLong;
    if(dfd.substring(4,5)=="2")  motor_state = value.valLong;
    if(dfd.substring(4,5)=="3")  motor_state = value.valLong;
    if(dfd.substring(4,5)=="4")  motor_state = value.valLong;
    if(dfd.substring(4,5)=="5")  motor_state = value.valLong;
    
    if(motor_state==1) digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    if(motor_state==0) digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    dfd="";
  }

  if((dfd.substring(0,2)=="m6")&(dfd.length()==6)){
    value.charByte[0]= char(dfd[2]);
    motor_state6 = value.valLong;
    Serial.println(motor_state6);

    dfd="";
  }

  if((dfd.substring(0,6)=="tmeset")&(dfd.length()==10)) {
      value.charByte[0]=char(dfd[6]);
     alarmid = value.valLong;
   switch (alarmid)
    {
    case 0:
      nextionCommand("n0.val", ArrayHoraAlarma[0]);
      nextionCommand("n1.val", ArrayMinutosAlarma[0]);
      break;
    case 1:    
      nextionCommand("n0.val", ArrayHoraAlarma[1]);
      nextionCommand("n1.val", ArrayMinutosAlarma[1]);
      break;
    case 2:
      nextionCommand("n0.val", ArrayHoraAlarma[2]);
      nextionCommand("n1.val", ArrayMinutosAlarma[2]);
      break;
    case 3:
      nextionCommand("n0.val", ArrayHoraAlarma[3]);
      nextionCommand("n1.val", ArrayMinutosAlarma[3]);
    default:
      dfd="";
      break;
    }
  dfd="";
  }

  if ((dfd.substring(0,3)=="set") & (dfd.length()==3))
  {
    red_value = int(readEEPROM(disk1, address_red));
    green_value = int(readEEPROM(disk1, address_green));
    blue_value = int(readEEPROM(disk1, address_blue));

    analogWrite(ledRojo,red_value); 
    analogWrite(ledVerde,green_value); 
    analogWrite(ledAzul,blue_value); 

    nextionCommand("h0.val", red_value);
    nextionCommand("h1.val", green_value);
    nextionCommand("h2.val", blue_value);
    dfd="";
  }
  
  if((dfd.substring(0,4)=="save")&(dfd.length()==4)){

    writeEEPROM(disk1, address_red, int(red_value));
    writeEEPROM(disk1, address_green, int(green_value));
    writeEEPROM(disk1, address_blue, int(blue_value));
    
    dfd="";
  }

  if((dfd.substring(0,6)=="tmsave")&(dfd.length()==6)){
    for( int i=0; i<NumeroElementosAlarma; i++){
    
    if((ArrayHoraAlarma[i] == readEEPROM(disk1, ArrayAddresHora[i]))& (ArrayMinutosAlarma[i]==readEEPROM(disk1, ArrayAddresMinuto[i]))){
      Serial.println("No se modifico");
      Serial.println(i);
      }
    else{
      writeEEPROM(disk1, ArrayAddresHora[i], ArrayHoraAlarma[i]);
      writeEEPROM(disk1, ArrayAddresMinuto[i], ArrayMinutosAlarma[i]);  
      }
    }
    dfd="";
  }
  }
}


void nextionCommand(String nextionName, long value){
  SerialNextion.print(nextionName);
  SerialNextion.print("=");
  SerialNextion.print(String(value));
  SerialNextion.print(endChar);
}

void writeEEPROM(int deviceaddress, unsigned int eeaddress, byte data ) 
{
 Wire.beginTransmission(deviceaddress);
 Wire.write((int)(eeaddress >> 8));   // MSB
 Wire.write((int)(eeaddress & 0xFF)); // LSB
 Wire.write(data);
 Wire.endTransmission();

  delay(5);
}
 
byte readEEPROM(int deviceaddress, unsigned int eeaddress ) 
{
 byte rdata = 0xFF;
 
 Wire.beginTransmission(deviceaddress);
 Wire.write((int)(eeaddress >> 8));   // MSB
 Wire.write((int)(eeaddress & 0xFF)); // LSB
 Wire.endTransmission();

 Wire.requestFrom(deviceaddress,1);
 
 if (Wire.available()) rdata = Wire.read();

 return rdata;
}


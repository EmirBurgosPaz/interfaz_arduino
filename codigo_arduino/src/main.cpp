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
void estadoMotor(int motor_state, int pinMotor);
bool prendidoAlarma(DateTime now, int ArrayHoraAlarma[4], int ArrayMinutosAlarma[4], bool FlagOnce);
bool apagadoAlarma(DateTime now, int ArrayHoraAlarma[4], int ArrayMinutosAlarma[4], int delayAsincronoHora[4],int delayAsincronoMinuto[4],bool FlagOnce);


RTC_DS3231 RTC;
SoftwareSerial SerialNextion(2, 3); // RX, TX

String  end_char = String(char(0xff)) + String(char(0xff)) + String(char(0xff));
String dfd;
byte last_min=0;
byte last_second =0;

int auto_state;
bool flag_once = true;
bool flag_auto= true;

int motor_state = 0;
int motor_state6 = 0;

int alarm_number = 0;

const byte led_rojo = 9;
const byte led_verde = 10;
const byte led_azul = 11;

const int tiempo_prendido_motor = 1;

const int periodo = 2000;
const int period_100ms = 100;
unsigned long tiempo_ahora = 0;

const int numero_elementos_alarma =4;

const  int array_address_hora[] = {6,7,8,9};
const  int array_addres_minuto[] = {12,13,14,15};

int array_hora_alarma[4];
int array_minuto_alarma[4];

int alarma_id=0;

int array_RGB[3];
const int array_addres_RGB[] = {0,1,2};
const int numero_elementos_RGB = 3;

int delay_asincrono_minuto[] = {0,0,0,0};
int delay_asincrono_hora[] = {0,0,0,0};

union main
{
  char char_byte[4];
  long val_long;
}value;


void setup() {
  SerialNextion.begin(9600);                       //The default baud rate of the Nextion TFT is 9600.         
  Serial.begin(9600);
  Wire.begin();  
  RTC.begin();

  //RTC.adjust(DateTime(__DATE__, __TIME__));

  pinMode(LED_BUILTIN, OUTPUT); 
  pinMode(led_rojo,OUTPUT);
  pinMode(led_verde,OUTPUT);
  pinMode(led_azul,OUTPUT);

 for( int i=0; i<numero_elementos_RGB; i++){
    array_RGB[i] = readEEPROM(disk1, array_addres_RGB[i]);
    //Serial.println(ArrayRGB[i]);
  }

  for( int i=0; i<numero_elementos_alarma; i++){
    array_hora_alarma[i] = readEEPROM(disk1, array_address_hora[i]);
    array_minuto_alarma[i]= readEEPROM(disk1, array_addres_minuto[i]);
  }

  analogWrite(led_rojo,array_RGB[0]); 
  analogWrite(led_verde,array_RGB[1]); 
  analogWrite(led_azul,array_RGB[2]); 
  digitalWrite(LED_BUILTIN, LOW);

  //writeEEPROM(disk1, address_red, 123);
  //writeEEPROM(disk1, address_green, 0);
  //writeEEPROM(disk1, address_blue, 0);
  Serial.print("Configurandose....");
  while ((unsigned long) (millis() - tiempo_ahora) > periodo){
        Serial.print(".");
        tiempo_ahora = millis();
      }
  Serial.println(".");

  //for( int i=0; i<NumeroElementosAlarma; i++){
  //  writeEEPROM(disk1, ArrayAddresHora[i], ArrayHoraAlarma[i]);
  //  writeEEPROM(disk1, ArrayAddresMinuto[i], ArrayMinutosAlarma[i]);
  //}
}

void loop() {



DateTime now = RTC.now();

if(last_second!=now.second()){
    if(last_min != now.minute()){
      Serial.print(now.hour());
      Serial.print(':');
      Serial.println(now.minute());
      last_min=now.minute();
    }    
    
  if(!flag_once & flag_auto){
    flag_once = apagadoAlarma(now, array_hora_alarma, array_minuto_alarma, delay_asincrono_hora, delay_asincrono_minuto, flag_once);
  }

  if(flag_once & flag_auto){
    flag_once = prendidoAlarma(now, array_hora_alarma, array_minuto_alarma, flag_once);
  }
  
  
  last_second=now.second();
}

if(SerialNextion.available()>0)                  //If we receive something...
  {
  dfd += char(SerialNextion.read());
  Serial.println(dfd);
  Serial.println(dfd.length());
  
  if((dfd.substring(0,5)=="index")&(dfd.length()==9)){
    value.char_byte[0]= char(dfd[5]);
    auto_state = value.val_long;
    if(auto_state==1){ Serial.println("Automatico"); flag_auto =true; }
    if(auto_state==2){ Serial.println("Manual"); flag_auto = false;}

    dfd = "";
  }

  if((dfd.substring(0,5)=="alarm")& (dfd.length()==9)){
    value.char_byte[0]=char(dfd[5]);
     alarma_id = value.val_long;

    switch (alarma_id)
    {
    case 0:
      Serial.println("Alarma1");
      alarm_number = 1;
      break;
    case 1:
      Serial.println("Alaram2");
      alarm_number =2;
      break;
    case 2:
      Serial.println("Alarma3");
      alarm_number =3;
      break;
    case 3:
      Serial.println("Alarma4");
      alarm_number=4;
    default:
      break;
    }
  
    dfd="";
  }
  
    if((dfd.substring(0,4)=="time") & (dfd.length()==9)){
    
    switch (alarm_number)
    {
    case 1:
      value.char_byte[0]= char(dfd[5]);

      if(dfd.substring(4,5)=="1")  array_hora_alarma[0]= value.val_long;
      if(dfd.substring(4,5)=="2")  array_minuto_alarma[0]= value.val_long;
      Serial.println("Alarma1Set");
      break;
    case 2:
      value.char_byte[0]= char(dfd[5]);

      if(dfd.substring(4,5)=="1")  array_hora_alarma[1]= value.val_long;
      if(dfd.substring(4,5)=="2")  array_minuto_alarma[1]= value.val_long;

      Serial.println("Alarma2Set");
      break;
    case 3:
      value.char_byte[0]= char(dfd[5]);

      if(dfd.substring(4,5)=="1")  array_hora_alarma[2]= value.val_long;
      if(dfd.substring(4,5)=="2")  array_minuto_alarma[2]= value.val_long;
      Serial.println("Alarma3Set");
      break;
    case 4:
      value.char_byte[0]= char(dfd[5]);

      if(dfd.substring(4,5)=="1")  array_hora_alarma[3]= value.val_long;
      if(dfd.substring(4,5)=="2")  array_minuto_alarma[3]= value.val_long;

      Serial.println("Alarma4Set");
      break;

    default:
      break;
    }
    dfd="";
  }

  

  if((dfd.substring(0,3)=="red")& (dfd.length()==7)){

    value.char_byte[0]= char(dfd[3]);

    array_RGB[0] = value.val_long; 
    analogWrite(led_rojo,array_RGB[0]); 
    dfd="";
  }

  if((dfd.substring(0,5)=="green")& (dfd.length()==9)){

    value.char_byte[0]= char(dfd[5]);

    array_RGB[1] = value.val_long;  
    analogWrite(led_verde,array_RGB[1]); 

    dfd="";
  }

  if((dfd.substring(0,4)=="blue")& (dfd.length()==8)){

    value.char_byte[0]= char(dfd[4]);

    array_RGB[2] = value.val_long;  
    analogWrite(led_azul,array_RGB[2]); 

    dfd="";
  }



  if((dfd.substring(0,5)=="motor")&(dfd.length()==10)){

    if(dfd.substring(5,6)=="1"){ motor_state = value.val_long; estadoMotor(motor_state, LED_BUILTIN); }
    if(dfd.substring(5,6)=="2"){ motor_state = value.val_long; estadoMotor(motor_state, LED_BUILTIN); }
    if(dfd.substring(5,6)=="3"){ motor_state = value.val_long; estadoMotor(motor_state, LED_BUILTIN); }
    if(dfd.substring(5,6)=="4"){ motor_state = value.val_long; estadoMotor(motor_state, LED_BUILTIN); }
    if(dfd.substring(5,6)=="5"){ motor_state = value.val_long; estadoMotor(motor_state, LED_BUILTIN); }
    
    dfd="";
  }

  if((dfd.substring(0,2)=="m6")&(dfd.length()==6)){
    value.char_byte[0]= char(dfd[2]);
    motor_state6 = value.val_long;
    Serial.println(motor_state6);

    dfd="";
  }

  if ((dfd.substring(0,4)=="exit")& (dfd.length()==4))
  {
    int estado_apagado = 3;
    estadoMotor(estado_apagado, LED_BUILTIN);
    dfd="";
  }
  



  if((dfd.substring(0,6)=="tmeset")&(dfd.length()==10)) {
    value.char_byte[0]=char(dfd[6]);
    alarma_id = value.val_long;
   switch (alarma_id)
    {
    case 0:
      nextionCommand("n0.val", readEEPROM(disk1, array_address_hora[0]));
      nextionCommand("n1.val", readEEPROM(disk1, array_addres_minuto[0]));
      break;
    case 1:    
      nextionCommand("n0.val", readEEPROM(disk1, array_address_hora[1]));
      nextionCommand("n1.val", readEEPROM(disk1, array_addres_minuto[1]));
      break;
    case 2:
      nextionCommand("n0.val", readEEPROM(disk1, array_address_hora[2]));
      nextionCommand("n1.val", readEEPROM(disk1, array_addres_minuto[2]));
      break;
    case 3:
      nextionCommand("n0.val", readEEPROM(disk1, array_address_hora[3]));
      nextionCommand("n1.val", readEEPROM(disk1, array_addres_minuto[3]));
    default:
      dfd="";
      break;
    }
  dfd="";
  }

  if ((dfd.substring(0,3)=="set") & (dfd.length()==3))
  {
    for( int i=0; i<numero_elementos_RGB; i++){
    array_RGB[i] = int(readEEPROM(disk1, array_addres_RGB[i]));
    Serial.println(array_RGB[i]);
    }
    analogWrite(led_rojo,array_RGB[0]); 
    analogWrite(led_verde,array_RGB[1]); 
    analogWrite(led_azul,array_RGB[2]); 

    nextionCommand("h0.val", array_RGB[0]);
    nextionCommand("h1.val", array_RGB[1]);
    nextionCommand("h2.val", array_RGB[2]);
    dfd="";
  }
  
  if((dfd.substring(0,4)=="save")&(dfd.length()==4)){

    for( int i=0; i<numero_elementos_RGB; i++){
    if((array_RGB[i] == readEEPROM(disk1, array_addres_RGB[i]))){
      Serial.println("No se modifico");
      Serial.println(i);
      }
    else{
      writeEEPROM(disk1, array_addres_RGB[i], array_RGB[i]);
      Serial.println("Guardado");
      }
    }
    
    dfd="";
  }

  if((dfd.substring(0,6)=="tmsave")&(dfd.length()==6)){
    for( int i=0; i<numero_elementos_alarma; i++){
    
    if((array_hora_alarma[i] == readEEPROM(disk1, array_address_hora[i]))& (array_minuto_alarma[i]==readEEPROM(disk1, array_addres_minuto[i]))){
      Serial.println("No se modifico");
      Serial.println(i);
      }
    else{
      delay_asincrono_hora[i] = array_hora_alarma[i];
      delay_asincrono_minuto[i]= array_minuto_alarma[i] + tiempo_prendido_motor;
      writeEEPROM(disk1, array_address_hora[i], array_hora_alarma[i]);
      writeEEPROM(disk1, array_addres_minuto[i], array_minuto_alarma[i]);
      Serial.println("Guardado");  
      }
    }
    dfd="";
  }
  if(dfd.length()==20) dfd="";
  }
}


void nextionCommand(String nextionName, long value){
  SerialNextion.print(nextionName);
  SerialNextion.print("=");
  SerialNextion.print(String(value));
  SerialNextion.print(end_char);
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

bool prendidoAlarma(DateTime now, int ArrayHoraAlarma[4], int ArrayMinutosAlarma[4], bool FlagOnce){
    for (int i = 0; i<numero_elementos_alarma; i++){
      if((now.hour()==ArrayHoraAlarma[i]) & (now.minute()==ArrayMinutosAlarma[i])) {
        Serial.println("Alarma");
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        Serial.println(i);
        FlagOnce = false;
      }
    }
  return FlagOnce;
}

bool apagadoAlarma(DateTime now, int ArrayHoraAlarma[4], int ArrayMinutosAlarma[4], int delayAsincronoHora[4],int delayAsincronoMinuto[4],bool FlagOnce){
  for (int j = 0; j<numero_elementos_alarma; j++){
    if((now.hour()==delayAsincronoHora[j]) & (now.minute()==delayAsincronoMinuto[j])) {
      Serial.println("Fin de Alarma");
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      Serial.println(j);  
      FlagOnce=true;
    }
  }
  return FlagOnce;
}

void estadoMotor(int motor_state, int pinMotor){
    if(motor_state==1) digitalWrite(pinMotor, !digitalRead(pinMotor));
    if(motor_state==0) digitalWrite(pinMotor, !digitalRead(pinMotor));
    if(motor_state==3) digitalWrite(pinMotor, LOW);
}
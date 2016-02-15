//EstacaoMeteorologicaV1.ino
// 05/05/2015
// Bruno Victor de M. Ferreira  (brunovf1 at gmail dot com)
/*
V1.0
Estação Meteorologica Simples utilizando Shield SD SparFun,
Sensor de temperatura e humidade DHT11, Sensor de Temperatura DS18B20,
e uma placa solar com um divisor de tensao.

V1.1
Adicionado a campacidade de ajuste da hora em tempo de execuçao
para tal e necessario enviar via serial uma String com o seguinte formato:
"T"+timestamp

O log dos dados e salvo no arquivo log.txt, com os seguintes dados:
data 				==> dia/mes/ano|hora:min:seg
temperatura do sensor DHT11 	==> celsius
temperatura do sensor DS18B20 	==> celsius
humidade do sensor DHT11 	==> humidade relativa do ar
luminosidade 			==> Valor de tensao da placa solar

As conexoes sao as seguintes:
pino A0	==> placa solar
pino A1	==> DHT11
pino 4	==> Shield SD
pino 5	==> DS18B20
pino 8	==> Led de erro
*/
#include <dht.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include <OneWire.h>
#include <Time.h>  
#include <Wire.h>  
#include <SD.h>
#include <SPI.h>

#define dht_dpin A1 //Pino DATA do Sensor DHT11 na porta Analogica A1

dht DHT; //Inicializa o sensor
byte SensorPin = 5; //pino do sensor de temperatura
byte LumidPin = A0; //pino da placa solar
OneWire ds(SensorPin); //objeto do sensor de temperatura
File myFile;

void setup() {
	Serial.begin(9600);// Serial para debug
	Wire.begin();// porta i2c para o RTC
	pinMode(4, OUTPUT); //pino de controle do cartão SD no baramento ISP
	pinMode(8,OUTPUT); // pino indicando erro na inicializacao do cartao SD
	setSyncProvider(RTC.get); // funcao para configurar a biblioteca Time
	if(timeStatus()!= timeSet){
//		Serial.println("Unable to sync with the RTC");
		while(1){ //loop de erro de inicializacao de hora
			digitalWrite(8,HIGH);
			delay(100);
			digitalWrite(8, LOW);
			delay(100);
		}
	}
	else{
//		Serial.println("RTC has set the system time");
	}

	if (!SD.begin(4)) {// verificaçao dd cartao SD
//		Serial.println("initialization failed!");
		while(1){// loop de erro
			digitalWrite(8,HIGH);
			delay(100);
			digitalWrite(8, LOW);
			delay(100);
		}
		
	}
	myFile = SD.open("Log.txt", FILE_WRITE);// abrir arquivo de log
	
	if (myFile) {// aquivo aberto corretamente
		myFile.println("Date, DHTTemp, DSTemp, Humd, Lumid");//cabeçalho
		myFile.close();
	}

	delay(1000);//Aguarda 1 seg antes de acessar as informações do sensor   
}


void loop() {
	int readlumid = analogRead(LumidPin);//leituta da placa solar
	float lumid = 3*readlumid * (5.0 / 1023.0);// conversao para Tensao
	float ds_temp = getDSTemp();// coleta da temperatura
	DHT.read11(dht_dpin); //Lê as informações do sensor de temp e humd
	time_t time_n = now();// captura da hora
	String now_s = String(day(time_n))+'/'+String(month(time_n));
	now_s += '/'+String(year(time_n))+'|'+String(hour(time_n))+':';
	now_s += String(minute(time_n))+':'+String(second(time_n));//converçao de time para String
//	Serial.println(now_s);//begin debug
//	Serial.print(',');
//	Serial.print(ds_temp);
//	Serial.print(',');
//	Serial.print(DHT.temperature);
//	Serial.print(',');
//	Serial.print(DHT.humidity);
//	Serial.print(',');
//	Serial.print(lumid);
//	Serial.println(';');//end debug
	datatoCSV(now_s, ds_temp, DHT.temperature, DHT.humidity, lumid);//escita no cartao
	delay(1000);//delay de leituras
}

#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by unix time_t as ten ascii digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message

void serialEvent() {
  while(Serial.available() >=  TIME_MSG_LEN ){  // time message consists of a header and ten ascii digits
    char c = Serial.read() ; 
//    Serial.print(c); 
    if( c == TIME_HEADER ) {       
      time_t pctime = 0;
      for(int i=0; i < TIME_MSG_LEN -1; i++){   
        c = Serial.read();          
        if( c >= '0' && c <= '9'){   
          pctime = (10 * pctime) + (c - '0') ; // convert digits to a numb
        }
      }   
//      Serial.println(pctime);
      RTC.set( (time_t) pctime);
      setTime( (time_t) pctime);
    }  
  }
}

void datatoCSV(String now_s, float ds_temp, int dht_temp, int dht_humidity, float lumid){
	myFile = SD.open("Log.txt", FILE_WRITE);//abre o arquivo
	if (myFile) {//se abrio o arquivo corretamente
//		Serial.print("Writing...");
		myFile.print(now_s);
		myFile.print(",");
		myFile.print(ds_temp);
		myFile.print(",");
		myFile.print(dht_temp);
		myFile.print(",");
		myFile.print(dht_humidity);
		myFile.print(",");
		myFile.print(lumid);
		myFile.println(";");
		myFile.close();
//		Serial.println("Done");
	}
	else{
//		Serial.print("Error!");//debug
	}
}

float getDSTemp(){ // funçao para ler o sensor de temperatura
	byte data[12];
	byte addr[8];
	if ( !ds.search(addr)) {
		//no more sensors on chain, reset search
		ds.reset_search();
		return -1000;
	}
	if ( OneWire::crc8( addr, 7) != addr[7]) {
		Serial.println("CRC is not valid!");
		return -1000;
	}
	if ( addr[0] != 0x10 && addr[0] != 0x28) {
		Serial.print("Device is not recognized");
		return -1000;
	}
	ds.reset();
	ds.select(addr);
	ds.write(0x44,1); 
	byte present = ds.reset();
	ds.select(addr); 
	ds.write(0xBE); 
	for (int i = 0; i < 9; i++) { 
		data[i] = ds.read();
	}
	ds.reset_search();
	byte MSB = data[1];
	byte LSB = data[0];
	float TRead = ((MSB<<8) | LSB); 
	float Temperature = TRead / 16;
	return Temperature;
}

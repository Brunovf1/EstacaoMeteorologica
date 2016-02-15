//EstacaoMeteorologicaV1.ino
// 29/05/2015
/*
Estação Meteorologica Simples utilizando Shield SD SparFun,
Sensor de temperatura e humidade DHT11, Sensor de Temperatura DS18B20,
e uma placa solar com um divisor de tensao.

O log dos dados e salvo no arquivo log.txt, com os seguintes dados:
data 				==> "%Y%m%dT%H%M%SZ"
temperatura do sensor DHT11 	==> celsius
temperatura do sensor DS18B20 	==> celsius
humidade do sensor DHT11        ==> humidade relativa do ar
luminosidade 			==> Valor de tensao da placa solar

As conexoes sao as seguintes:
pino A0	==> placa solar
pino A1	==> DHT11
pino 4	==> Shield SD
pino 5	==> DS18B20
pino 8	==> Led de erro
*/
#include <dht.h>
#include <OneWire.h>
#include <FileIO.h>
#include "Wire.h"
#include "Adafruit_BMP085.h"

#define dht_dpin A1 //Pino DATA do Sensor DHT11 na porta Analogica A1
#define SensorPin 5
#define LumidPin A0

Adafruit_BMP085 bmp;
dht DHT; //Inicializa o sensor
OneWire ds(SensorPin); //objeto do sensor de temperatura

void setup() {
  Wire.begin();// porta i2c para o RTC
  bmp.begin();
  Bridge.begin();
  FileSystem.begin();
  writeHeader();
  delay(1000);//Aguarda 1 seg antes de acessar as informações do sensor   
}

void writeHeader(){
  String HEADER = "Date, DHTTemp, DSTemp, BMPTemp, Humd, Lumid, Pressure";
  String fbuffer = getFileName();
  char fileName[fbuffer.length()+1];
  fbuffer.toCharArray(fileName,fbuffer.length()+1);
  File dataFile = FileSystem.open(fileName, FILE_APPEND);
  if (dataFile) {
    dataFile.println(HEADER);        
  }  
}

void loop() {
  DHT.read11(dht_dpin); //Lê as informações do sensor de temp e humd
  String dataString;
  dataString += getTimeStamp();
  dataString += String(',');
  dataString += String(DHT.temperature);
  dataString += String(',');
  dataString += String(getDSTemp());
  dataString += String(',');
  dataString += String(bmp.readTemperature());
  dataString += String(',');
  dataString += String(bmp.readPressure());
  dataString += String(',');
  dataString += String(DHT.humidity);
  dataString += String(',');
  dataString += String((3*(analogRead(LumidPin)) * (5.0 / 1023.0)));
  String fbuffer = getFileName();
  char fileName[fbuffer.length()+1];
  fbuffer.toCharArray(fileName,fbuffer.length()+1);
  File dataFile = FileSystem.open(fileName, FILE_APPEND);
  
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  }
  
  delay(1000);//delay de leituras
}

String getTimeStamp(){
  //date +"%Y%m%dT%H%M%SZ"
  String result;
  Process time;
  // date is a command line utility to get the date and the time
  // in different formats depending on the additional parameter
  time.begin("date");
  time.addParameter("-u");
  time.addParameter("+%Y%m%dT%H%M%SZ");
  time.run();  // run the command

  // read the output of the command
  while (time.available() > 0) {
    char c = time.read();
    if (c != '\n')
      result += c;
  }

  return result;
}

String getFileName(){
  //date +%Y.%m.%d-Estacao-666.csv
  String result;
  Process time;
  // date is a command line utility to get the date and the time
  // in different formats depending on the additional parameter
  time.begin("date");
  time.addParameter("+/mnt/sd/%Y.%m.%d-Estacao-666.csv");
  time.run();  // run the command

  // read the output of the command
  while (time.available() > 0) {
    char c = time.read();
    if (c != '\n')
      result += c;
  }

  return result;
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
		return -1000;
	}
	if ( addr[0] != 0x10 && addr[0] != 0x28) {
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

//EstacaoMeteorologicaV1.ino
// 29/05/2015
/*
Estação Meteorologica Simples utilizando Shield SD SparFun,
Sensor de temperatura e humidade DHT11, Sensor de Temperatura DS18B20,
e uma placa solar com um divisor de tensao.

O log dos dados e salvo no arquivo log.txt, com os seguintes dados:
data 							==> dia/mes/ano|hora:min:seg
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
//#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include <OneWire.h>
#include <Time.h>  
#include <Wire.h>
#include <FileIO.h>

#define dht_dpin A1 //Pino DATA do Sensor DHT11 na porta Analogica A1

dht DHT; //Inicializa o sensor
byte SensorPin = 5; //pino do sensor de temperatura
byte LumidPin = A0; //pino da placa solar
OneWire ds(SensorPin); //objeto do sensor de temperatura
String HEADER = "Date, DHTTemp, DSTemp, Humd, Lumid";

void setup() {
        Bridge.begin();
	Serial.begin(9600);// Serial para debug
	Wire.begin();// porta i2c para o RTC
        FileSystem.begin();
//	setSyncProvider(RTC.get); // funcao para configurar a biblioteca Time
        while (!Serial);
//	if(timeStatus()!= timeSet){
//		Serial.println("Unable to sync with the RTC");
//	}
//	else{
//		Serial.println("RTC has set the system time");
//	}
        File dataFile = FileSystem.open("/mnt/sd/datalog.txt", FILE_APPEND);
        if (dataFile) {
           dataFile.println(HEADER);
        }
        
	delay(1000);//Aguarda 1 seg antes de acessar as informações do sensor   
}


void loop() {
	int readlumid = analogRead(LumidPin);//leituta da placa solar
	float lumid = 3*readlumid * (5.0 / 1023.0);// conversao para Tensao
	float ds_temp = getDSTemp();// coleta da temperatura
	DHT.read11(dht_dpin); //Lê as informações do sensor de temp e humd
//	time_t ds1307 = now();// captura da hora        
        time_t linux = getTimeStamp();
        String dataString;
        dataString += String(linux);
        dataString += String(',');
        dataString += String(DHT.temperature);
        dataString += String(',');
        dataString += String(ds_temp);
        dataString += String(',');
        dataString += String(DHT.humidity);
        dataString += String(',');
        dataString += String(lumid);
        
//        if(ds1307 >= linux){
          //TODO verificar se o relogio esta acertado ou se o rtc resetou
//        }
        File dataFile = FileSystem.open("/mnt/sd/datalog.txt", FILE_APPEND);
        if (dataFile) {
          dataFile.println(dataString);
          dataFile.close();
          // print to the serial port too:
          Serial.println(dataString);
        }
	delay(1000);//delay de leituras
}

//bool verifyTime(time_t ds1307, time_t linux){
//}

time_t getTimeStamp() {
  String result;
  Process time;
  // date is a command line utility to get the date and the time
  // in different formats depending on the additional parameter
  time.begin("date");
  time.addParameter("+%s");
//  time.addParameter("+%D-%T");  // parameters: D for the complete date mm/dd/yy
  //             T for the time hh:mm:ss
  time.run();  // run the command

  // read the output of the command
  while (time.available() > 0) {
    char c = time.read();
    if (c != '\n')
      result += c;
  }

  return result.toInt();
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

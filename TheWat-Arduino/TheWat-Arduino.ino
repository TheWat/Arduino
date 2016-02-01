#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <SPI.h>
#include "APmode.h"
/****
 * Vreal = (2Vin-3.3/2)*(.512/3.3)*1001
 * 
 * Ireal = (Vcurrent-3.3/2)*(.512/3.3)*10
 * 
 */
 /********************
  * SPI CLK - Pin 14
  * SPI Input - Pin 12
  * CS - Pin 5
  */
#define NOWIFI


bool isconnected;

const int chipSelectPin = 5;
const int chipSelectPin2 = 16;

const int spi_clk = 100000;


 
void setup() {
  //Serial.begin(115200);
  Serial.begin(9600);
  EEPROM.begin(512);
  SPI.begin();
  
  //use 125 kHz. Courtesy of Arduino SPI.h source
  SPI.beginTransaction(SPISettings(spi_clk, MSBFIRST, SPI_MODE0));
  pinMode(chipSelectPin, OUTPUT);
  
  
  delay(100);
 
  // We start by connecting to a WiFi network
  #ifdef NOWIFI
  
  isconnected = true;
  pinMode(2,OUTPUT);
  for(int i=0; i < 4; i++){
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
    delay(1000);
  }
  Serial.println("Your ESP is Starting!");
  delay(500);
  
  
  #else
  String ssid = loadSSID();
  String pwd = loadPwd();
  Serial.println(ssid);
  Serial.println(pwd);
  WiFi.begin(ssid.c_str(), pwd.c_str());
  Serial.print("Connecting to " + ssid + "...");
  isconnected = testWifi();
  if(isconnected){
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
      
    SPI.begin();
    //use 125 kHz. Courtesy of Arduino SPI.h source
    SPI.beginTransaction(SPISettings(spi_clk, MSBFIRST, SPI_MODE0));
    pinMode(chipSelectPin, OUTPUT);
  }
  else{
    Serial.println("");
    Serial.println("Setting up AP Config point");
    apmode();
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }
  #endif
}
 




void getPage(String host, String url, int port){
  WiFiClient client;
  if (!client.connect(host.c_str(), port)) {
    Serial.println("connection failed");
    return;
  }

   client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(500);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}

uint16_t switchBytes(uint16_t input){
  uint16_t temp = input << 8;
  //Serial.println(temp,HEX);
  input = input >> 8;
  //Serial.println(input,HEX);
  return ((temp | input)&0x3fff);
}
double opampInputVoltage(double in, double vref){
  return .512*(in/vref-.5);
}

int maxCount = 3000;
int count = 0;

double currentVREF = 3.3;
double voltageVREF = 3.3;
float CRMSsum = 0;
float VRMSsum = 0;
double powerSum = 0;
int accuracy = 3;



void loop() {

  if(isconnected){
    yield();
    //getPage("wat.jabez.org","/",3000);
    
    digitalWrite(chipSelectPin, LOW);
    uint16_t readval = SPI.transfer16(0x0000);    
    digitalWrite(chipSelectPin, HIGH);

    /*digitalWrite(chipSelectPin2, LOW);
    uint16_t readval2 = switchBytes(SPI.transfer16(0x0000));
    float volV = opampInputVoltage(float(readval2)/4096.0*(3.3/2.0),voltageVREF);
    digitalWrite(chipSelectPin2, HIGH);*/
    float volV = 0;
    
    float spiV = (float(switchBytes(readval))/4096.0)*3.3/2.0;

    float curV = opampInputVoltage(spiV,currentVREF);

    if(count==maxCount){
            
      Serial.print("Current RMS: ");
      float intermediate = sqrt(CRMSsum/float(maxCount));
      Serial.print(5*intermediate,accuracy);
      Serial.println(" Amps");
      Serial.println(switchBytes(readval));
      
      
      Serial.print("Voltage RMS: ");
      intermediate = sqrt(VRMSsum/float(maxCount));
      Serial.print(1001*intermediate,accuracy);
      Serial.println(" Volts");

      //Serial.println(readval2);

      Serial.print("Power: ");
      Serial.print(powerSum/float(maxCount));
      Serial.println(" Watts");

      
      CRMSsum = 0;
      VRMSsum = 0;
      powerSum = 0;
      count = 0;
    }
    
    
    CRMSsum += curV*curV;
    VRMSsum += volV*volV;
    powerSum += curV*volV;
    count++;
    
   
  }
  else{
    //Serial.println(".");
    // Check if a client has connected
   APServer();
  }
  
}

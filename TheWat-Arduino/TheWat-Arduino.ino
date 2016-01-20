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



bool isconnected;

const int chipSelectPin = 5;

const int spi_clk = 100000;

float currentMultiplier = 1;
float voltageMultiplier = 1;
float currentOffset = 0;
float voltageOffset = 0;



 
void setup() {
  //Serial.begin(115200);
  Serial.begin(9600);
  EEPROM.begin(512);
  
  
  
  delay(100);
 
  // We start by connecting to a WiFi network
  
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

int value = 0;
int maxCount = 1000;
int count = 0;
float voltageSum = 0;
float powerSum = 0;


void loop() {
  ++value;
  

  if(isconnected){
    yield();
    
    //delay(500);  
    //getPage("wat.jabez.org","/",3000);
    digitalWrite(chipSelectPin, LOW);
    uint16_t readval = SPI.transfer16(0x0000);
    digitalWrite(chipSelectPin, HIGH);
    float spiV = (float(switchBytes(readval))/4096.0)*3.3/2.0;
    //Serial.print("SPI: ");
    
    //Serial.println(readval,HEX);
  //  Serial.print("  ");
    //Serial.println((float(switchBytes(readval))/4096.0)*3.3/2.0);
    float internalV = float(analogRead(A0))/1024;
    //Serial.print("ADC: ");
    //Serial.println(float(a)/1024);
    /*
    if(count==maxCount){
      Serial.print("Running sum: ");
      Serial.println(voltageSum/float(maxCount));
      //Serial.print("Instant Voltage");
      //Serial.println((float(switchBytes(readval))/4096.0)*3.3/2.0);
      voltageSum = 0;
      count = 0;   
    }
    voltageSum += (float(switchBytes(readval))/4096.0)*3.3/2.0;
    */
    float realVoltage = (2*internalV-3.3/2)*(.512/3.3)*1001;
    float realCurrent = (spiV-3.3/2)*10*(.512/3.3);
    float power = realVoltage*realCurrent;
    powerSum += power;
    count++;
    if(count == maxCount){
      Serial.print("Running sum: ");
      Serial.println(powerSum/float(maxCount));
      Serial.print("Instant Power: ");
      Serial.println(power);
      powerSum = 0;
      count = 0;  
    }    
  }
  else{
    //Serial.println(".");
    // Check if a client has connected
   APServer();
  }
  
}

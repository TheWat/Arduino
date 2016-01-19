#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "APmode.h"

WiFiServer server(80);

int saveWifiConfig(String ssid, String pwd)
{
  for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
  for (int i = 0; i < ssid.length(); ++i){
    EEPROM.write(i, ssid[i]);
    //Serial.print("Wrote: ");
    //Serial.println(ssid[i]); 
  }
  for (int i = 0; i < pwd.length(); ++i)
  {
    EEPROM.write(32+i, pwd[i]);
    //Serial.print("Wrote: ");
    //Serial.println(pwd[i]); 
   }    
  EEPROM.end();
  return 0;
}
String loadSSID()
{
  String esid;
  //max SSID length is 32 characters
  for(int i = 0; i < 32; i++){
    esid += char(EEPROM.read(i));
  }
  return esid;
}
String loadPwd(){
  String epwd;
  //most passwords are less than 64 characters
  for(int i = 32; i < 96; i++){
    epwd += char(EEPROM.read(i));
  }
  return epwd;
}

bool testWifi(){
  for(int i = 0; i < 20; i++){
    if(WiFi.status() == WL_CONNECTED){ return true;}
    delay(500);
    Serial.print(".");
  }
  return false;
}

void apmode(){
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP Arduino","password");
  server.begin();//so that was important 
}

void APServer(){
    WiFiClient client = server.available();  // Check if a client has connected
    if (!client) {
        return;
    }
    Serial.println("");
    Serial.println("New client");

    // Wait for data from client to become available
    while (client.connected() && !client.available()) {
      delay(1);
    }
    
    // Read the first line of HTTP request
    String req = client.readStringUntil('\r');
    Serial.println(req);
    String s;
    String gsid;
    String gpwd;
    //if the form has been submitted
    if(req.indexOf("ssid")!=-1){
      //TODO Make this a POST request
      //I'm basically hoping they're being nice
      int x = req.indexOf("ssid")+5;
      gsid = req.substring(x,req.indexOf("&",x));
      x = req.indexOf("pwd")+4;
      gpwd = req.substring(x,req.indexOf(" ",x));
      s = gsid;
      saveWifiConfig(gsid,gpwd);
      Serial.print("Restarting");
      client.print("Thanks! Restarting to new configuration");
      client.flush();
      ESP.restart();
    }
    else{
    s =  "<h1>Welcome to your ESP</h1><form action='/' method='get'><table><tr><td>SSID</td><td><input type='text' name='ssid'></td></tr><tr><td>Password</td><td><input type='text' name='pwd'></td><tr><td /><td><input type='submit' value='Submit'></td></tr></form>";
    client.print(s);
    }
    client.flush();
    delay(1);
    //Serial.println("Client disonnected");
}

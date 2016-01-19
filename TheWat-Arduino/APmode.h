#ifndef AP_MODE
#define AP_MODE

int saveWifiConfig(String,String);
String loadSSID();
String loadPwd();
bool testWifi();
void apmode();
void APServer();

#endif

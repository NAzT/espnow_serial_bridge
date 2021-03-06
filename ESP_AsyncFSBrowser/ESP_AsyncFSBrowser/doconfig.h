
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
extern void printMacAddress(uint8_t* macaddr);
extern int dhtType;
extern char myName[];
extern uint8_t master_mac[];


bool saveConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["name"] = myName;
  json["mac"] = "0a0b0c0d0d0d";
  json["dhtType"] = dhtType;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  return true;
}

bool loadConfig(char *myName, int *dhtType) {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  uint32_t sleep = json["sleepS"];

  if (json.containsKey("mac")) {
    const char* mac = json["mac"];
    String macStr = String(mac);
    Serial.printf("Loaded mac %s\r\n", mac);
    for (size_t i = 0; i < 12; i+=2) {
      String mac = macStr.substring(i, i+2);
      byte b = strtoul(mac.c_str(), 0, 16);
      master_mac[i/2] = b;
    }
    // strcpy(master_mac, mac);
  }

  if (json.containsKey("dhtType")) {
    *dhtType = json["dhtType"].as<int>();
    Serial.printf("Loaded dhtType %d\r\n", *dhtType);
  }
  else {
    *dhtType = 11;
  }

  if (json.containsKey("name")) {
    const char* name = json["name"];
    if (strlen(name) > 11) {
      strncpy(myName, name, 11);
      myName[11] = '\0';
    }
    else {
      strcpy(myName, name);
    }
    Serial.printf("Loaded myName = %s\r\n", myName);
  }
  else {
    strcpy(myName, "Noname");
  }



  // String macStr = String(mac);
  // for (size_t i = 0; i < 12; i+=2) {
  //   String mac = macStr.substring(i, i+2);
  //   byte b = strtoul(mac.c_str(), 0, 16);
  //   master_mac[i/2] = b;
  // }
  //
  // printMacAddress(master_mac);

  return true;
}

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define DEBUG

#ifdef DEBUG
  #define debug(x)     Serial.print(x)
  #define debugln(x)   Serial.println(x)
#else
  #define debug(x)     // define empty, so macro does nothing
  #define debugln(x)
#endif

const char* ssid = "xxx";
const char* password = "xxx";
const char* dom_interval = "http://192.168.1.xx:xxxx/json.htm?type=command&param=getuservariable&idx=1";
const char* dom_device = "http://192.168.1.xx:xxxx/json.htm?type=devices&rid=149";

StaticJsonBuffer<2200> jsonBuffer;
int httpCode;
String payload;
byte interval;
String dev_status;

byte sleepCount = 0;


void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  debug("Connecting to ");
  debugln(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debug(".");
  }

  debugln("");
  debugln("WiFi connected"); 
  debugln("IP address: ");
  debugln(WiFi.localIP());

  debugln("I'm awake and getting Domoticz data about Airwick");

  getDomoticzData();
  
  EEPROM.begin(1); // initialise EEPROM to store only one byte
  sleepCount = EEPROM.read(0);
  if(sleepCount > 250) sleepCount = 1;
  debugln("Current SleepCount: "+ String(sleepCount));

  if( (sleepCount >= interval) && (dev_status == "On")){
    debugln("Airwick papurskimas!!");
    sleepCount = 0;
  }
  if( (sleepCount >= interval) && (dev_status == "Off")){
    sleepCount = 0;
  }
  EEPROM.write(0, sleepCount+1);
  EEPROM.commit();
  debugln("Going into deep sleep for 59 seconds");
  ESP.deepSleep(59e6); // 59e6 is 59 seconds
}

// the loop function runs over and over again forever
void loop() {
}

void getDomoticzData(){
  if (WiFi.status() == WL_CONNECTED) 
  {
    HTTPClient http; //Object of class HTTPClient
    http.begin(dom_interval);
    httpCode = http.GET();
    payload = http.getString();

//    debug("Response Code:"); //200 is OK
//    debugln(httpCode);   //Print HTTP return code
//    debug("Returned data from Server:");
//    debugln(payload);    //Print request response payload

    if (httpCode > 0) 
    {
      JsonObject& root = jsonBuffer.parseObject(payload);
 
      interval = root["result"][0]["Value"]; 

      debug("Idx_interval:");
      debugln(interval);
    }
    jsonBuffer.clear();
    http.end(); //Close connection

    http.begin(dom_device);
    httpCode = http.GET();
    payload = http.getString();

    if (httpCode > 0) 
    {
      JsonObject& root1 = jsonBuffer.parseObject(payload);

      dev_status = root1["result"][0]["Data"].as<String>(); 
      
      debug("Dev_status:");
      debugln(dev_status);
    }
    jsonBuffer.clear();
    http.end(); //Close connection
  }
}

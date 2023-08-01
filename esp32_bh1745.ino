#include <WiFi.h>
#include <HTTPClient.h>
#include <BH1745.h>
#include <ArduinoJson.h>
const char* ssid = "WITLAB";
const char* password = "******";
const char* ntpServer = "kr.pool.ntp.org";
uint8_t timeZone = 9;
uint8_t summerTime = 0; // 3600
char timeYear[5];
char timeMonth[3];
char timeDate[3];
char timeHour[3];
char timeMinute[3];
HTTPClient http;

BH1745 bh = BH1745();
// for ESP32 Change the SDA and SCl
#ifdef ESP32
#define SDA 18
#define SCL 19
#endif


char datetime[128];
StaticJsonDocument<300> jsonDocument;
void setup() {
  Serial.begin(9600);
  // WiFi 연결
  WiFi.begin(ssid, password);
  Serial.println("WiFi Connecting");
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  
  Serial.println(WiFi.localIP());
  configTime(3600 * timeZone, 3600 * summerTime, ntpServer);

  #ifdef ESP32
  bool result = bh.begin(SDA, SCL);
  #else
  bool result = bh.begin(18,19);
  #endif
  if (!result){
    Serial.println("Device Error");
    while (1){;;}
  }
  bh.setGain(bh.GAIN_16X);
  bh.setRgbcMode(bh.RGBC_16_BIT);

}

void loop() {
  // put your main code here, to run repeatedly:
  if(WiFi.status()==WL_CONNECTED){
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return;
    }
    strftime(timeYear,5, "%Y", &timeinfo);
    strftime(timeMonth,3, "%m", &timeinfo);
    strftime(timeDate,3, "%d", &timeinfo);
    strftime(timeHour,3, "%H", &timeinfo);
    strftime(timeMinute,3, "%M", &timeinfo);

    sprintf(datetime,"%s-%s-%s %s:%s:00",timeYear, timeMonth, timeDate, timeHour, timeMinute);
    jsonDocument.clear();
    bh.read();
    jsonDocument["r"] = bh.red;
    jsonDocument["g"] = bh.green;
    jsonDocument["b"] = bh.blue;
    jsonDocument["c"] = bh.clear;
    jsonDocument["datetime"] = datetime; // Replace with actual date and time
    jsonDocument["sensorId"] = 1; // Replace with the sensor ID

    String jsonString;
    serializeJson(jsonDocument, jsonString);


    http.begin("http://******/light/save");
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonString);

    if(httpResponseCode>0){
  
    String response = http.getString();                       //Get the response to the request
  
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);           //Print request answer
  
   }else{
  
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  
   }

   http.end();  //Free resources
  }
  else{
    Serial.println("Error in WiFi Connection");
  }
  delay(10000);
}

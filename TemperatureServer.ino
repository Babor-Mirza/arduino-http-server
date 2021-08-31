#include<OneWire.h>
#include<DallasTemperature.h>
#include<WiFi.h>

const unsigned int BIT_RATE = 115200;
/*GPIO where DS18B20 temperature Sensor is connected */
const unsigned int TEMPERATURE_PIN01 = 15;
/* Setup OneWire instance to communicate with One-Wire devices*/
OneWire oneWiredHandler(TEMPERATURE_PIN01);
/* Setup Temperature Calculator and Converter Instance*/
DallasTemperature temperatureHandler(&oneWiredHandler);
/* WiFi credentials */
const char* WiFi_SSID = "R-One";
const char* WiFi_PASSWORD = "kics@iot";
/* WifiServer to handle Temperature Request*/
WiFiServer webServerHandler(80);// Listening on Port 8080

/*Wifi conncection function*/
void initWiFiConnection(void);
/* Handler Response */
void tramistTemperature(float, float, WiFiClient*);

void setup() {
  Serial.begin(BIT_RATE);
  while (!Serial); /* wait for serial port to connect*/
  initWiFiConnection(); /* Connect to Wifi*/
  webServerHandler.begin(); /* start webServer */
  temperatureHandler.begin(); /* Start Computation and Conversion */

}

void loop() {
  WiFiClient handleClientRequest = webServerHandler.available();
  if (handleClientRequest) {
    temperatureHandler.requestTemperatures();
    float temperatureInCelsius = temperatureHandler.getTempCByIndex(0);
    float temperatureInFahrenheit = temperatureHandler.getTempFByIndex(0);
    /* prepare and send reponse*/
    transmitTemperature(temperatureInCelsius, temperatureInFahrenheit, &handleClientRequest);
    //while(!handleClientRequest.disconnect());
    /* print to UART */
    Serial.print(temperatureInCelsius);
    Serial.print(" Celsius---");
    Serial.print(temperatureInFahrenheit);
    Serial.println(" Fahrenheit");
    Serial.println("-------------------------------------");
  } else {
    Serial.println("Waiting for client...");
  }
}


void initWiFiConnection(void) {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WiFi_SSID, WiFi_PASSWORD);
    Serial.print("Connecting to WiFi: ");
    Serial.println(WiFi_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }
  }
  Serial.println();
  Serial.print("Connected at ");
  Serial.println(WiFi.localIP());
  delay(1000);


}
void transmitTemperature(float temperatureInCelsius, float temperatureInFahrenheit, WiFiClient* clientHandler) {
  if (clientHandler->connected()) {
     clientHandler->flush();
      String responseBody=""
      "<!DOCTYPE HTML>"
      "<HTML>"
      "<HEAD><TITLE>Temperature Sensor</TITLE></HEAD>"
      "<BODY>"
      "<H2>Temperature in Celsius</H2><P>";
      responseBody += String(temperatureInCelsius) + "</P>";
      responseBody += "<H2>Temperature in Fahrenheit</H2><P>";
      responseBody += String(temperatureInFahrenheit) + "</P></BODY></HTML>\n";
      
      String responseHeader = "HTTP/1.1 200 OK\n"
      "Content-Type: text/html; charset=ascii\n"
      "Keep-Alive: timeout=30, max=1\n"
      "Connection: Keep-Alive\n"
      "Content-Length:"+String(responseBody.length());
      clientHandler->println(responseHeader);
      clientHandler->println();
      clientHandler->print(responseBody);
      clientHandler->println();
      Serial.println(responseHeader + responseBody);
      //clientHandler->flush();
      /* Response End*/
  }
  delay(1000*30);
  clientHandler->stop(); /* disconnect client */
  Serial.println("Response has been sent...");
}

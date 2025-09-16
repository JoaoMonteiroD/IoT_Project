#include <ThingSpeak.h>
#include <WiFi.h>

//Wi-Fi Credentials
const char* ssid = "BS Sala Aulas";
const char* password = "2024184500";

unsigned long myChannelNumber = 2747643;
const char*myWriteAPIKey = "ELGWBFAV145UEKLF";

WiFiClient client;

int pin4 = 4;
int mq= A0;

void setup() {
  pinMode(pin4, OUTPUT);

  Serial.begin(115200);

  //Connect to wifi
  Wifi.begin(ssid, password);
  Serial.print("Connecting to the WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to the WiFi!");

  ThingSpeak.begin(client);
}

void loop() {
  //Start comunication with the 10b20
  ds.reset();
  ds.write(0xCC); //Skip ROM (address all devices on the bus)
  ds.write(0x44); //Start temperature conversion

  //Wait dor the temperature conversion complete
  delay(750);

  ds.reset();
  ds.write(0xCC); //Skip ROM (address all devices on the bus)
  ds.write(0xBE); //Read scratchpad 






  //Print temperature to Serial Monitor
  Serial.print("Temperature");
  Serial.print(tempeartureC);
  Serial.println(" ºC");

  //Send temperature data to ThingSpeak
  ThingSpeak.setField(1, temperatureC);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  //Wait 15 seconds before sending the next data
  delay(1500);
}

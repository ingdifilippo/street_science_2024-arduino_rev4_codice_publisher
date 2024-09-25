#include <ArduinoMqttClient.h>
#include <LiquidCrystal_I2C.h>
// To use ArduinoGraphics APIs, please include BEFORE Arduino_LED_Matrix
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include "Adafruit_SHTC3.h"
#include <WiFi.h>
#include <string>

#include "secret.h"

// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate
//    flashed in the WiFi module.

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
ArduinoLEDMatrix matrix;

const char broker[] = "broker.hivemq.com";
int port = 1883;
const char topic[] = "elettronica/lab20_tx";

const long interval = 10000;
unsigned long previousMillis = 0;

char countString[3];



void setup() {
  Serial.begin(115200);
  matrix.begin();
  while (!Serial)
    delay(10);  // will pause until serial console opens

  lcd.init();  // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Quarta Elettronica");
  lcd.setCursor(2, 1);
  lcd.print("IIS AOSTA AQ");
  lcd.setCursor(0, 2);
  lcd.print("Street science 2024");
  delay(4000);
  lcd.clear();
  lcd.setCursor(0, 0);



  Serial.println("SHTC3 test ");
  lcd.print("SHTC3 test... ");
  lcd.setCursor(0, 1);

  if (!shtc3.begin()) {
    Serial.println("Couldn't find SHTC3");
   
    lcd.print("SHTC3 NON TROVATO!!");
    while (1) delay(1);
  }

  Serial.println("Found SHTC3 sensor");
  Serial.println("waiting WiFi link...");
  lcd.print("sensore SHTC3 ok...");
  delay(3000);
  lcd.clear();
  lcd.print("Connessione al WiFi ");
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.println(".");
    lcd.print(".");
    delay(5000);
  }

  Serial.println("connected to WiFi...");
  lcd.setCursor(0, 2);
  lcd.print("ok");
  delay(2000);
  lcd.clear();
  lcd.print("Conn. al broker ");

  if (!mqttClient.connect(broker, port)) {
     lcd.setCursor(0, 3);
    lcd.print("Connessione fallita ");
    Serial.println("Connection failed ");
    Serial.println("Stop!");

    while (1)
      ;
  }

  Serial.println("connected to broker..");
  
  lcd.print("ok");
  delay(1000);
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // to avoid having delays in loop, we'll use the strategy from BlinkWithoutDelay
  // see: File -> Examples -> 02.Digital -> BlinkWithoutDelay for more info
  unsigned long currentMillis = millis();

  sensors_event_t humidity, temp;

  shtc3.getEvent(&humidity, &temp);  // populate temp and humidity objects with fresh data


  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;
    lcd.clear();
    lcd.print("Temp: ");
    lcd.setCursor(0, 1);
    lcd.print(temp.temperature);
    lcd.print(" C");
    lcd.setCursor(0, 2);
    lcd.print("Umidita': ");
    lcd.setCursor(0, 3);
    lcd.print(humidity.relative_humidity);
    lcd.print("%");

    Serial.print("Temperature: ");
    Serial.print(temp.temperature);
    Serial.println(" degrees C");
    Serial.print("Humidity: ");
    Serial.print(humidity.relative_humidity);
    Serial.println("% rH");

    mqttClient.beginMessage(topic);
    //mqttClient.print("weather,location=us-midwest temperature=82 1465839830100400200"); //dataformat influx in telegraf
    mqttClient.print("monitoraggio,ambiente=lab20_tx rh=");  //dataformat senza timestamp
    mqttClient.print(humidity.relative_humidity);

    mqttClient.endMessage();
    mqttClient.beginMessage(topic);

    mqttClient.print("monitoraggio,ambiente=lab20_tx temp=");  //dataformat senza timestamp
    mqttClient.print(temp.temperature);
    mqttClient.endMessage();
  }
  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(100);

  const char text_1[] = "  Sharper 2024 Elettronica ";
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text_1);
  matrix.endText(SCROLL_LEFT);

  matrix.endDraw();
}

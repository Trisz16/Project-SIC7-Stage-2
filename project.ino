#include <WiFi.h>
#include "UbidotsESPMQTT.h"
#include <DHT11.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define TOKEN "BBUS-iNrhDO5etGL3dduT7nfGz7ClrmiPAh"
#define WIFINAME "Geliathus_4G"
#define WIFIPASS "AL123AGG"

#define PinDHT11 2
#define PinHumidityMoisture 35
#define PinBuzzer 14
#define PinLED1 27
#define PinLED2 25
#define PinLED3 26
#define PinMotor1 5
#define PinPIR 19
#define PinLDR 34
#define PinButton1 32
#define PinButton2 33

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT11 dht11(PinDHT11);
Ubidots client(TOKEN);

// Variabel kontrol
int nilaiSwitch1 = 0;
int nilaiSwitch2 = 0;
int lastMenu = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = '\0';
  float value = atof(p);

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Value: ");
  Serial.println(value);

  // Deteksi variable mana yang diterima berdasarkan topic
  if (strstr(topic, "switch1") != NULL) {
    nilaiSwitch1 = value;
    Serial.print("Switch1 diubah menjadi: ");
    Serial.println(nilaiSwitch1);
  }else if (strstr(topic, "switch2") != NULL) {
    nilaiSwitch2 = value;
    Serial.print("Switch2 diubah menjadi: ");
    Serial.println(nilaiSwitch2);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PinButton1,INPUT_PULLUP);
  pinMode(PinButton2,INPUT_PULLUP);
  pinMode(PinLDR,INPUT);
  pinMode(PinHumidityMoisture,INPUT);
  pinMode(PinBuzzer,OUTPUT);
  pinMode(PinLED1,OUTPUT);
  pinMode(PinLED2,OUTPUT);
  pinMode(PinLED3,OUTPUT);
  pinMode(PinMotor1,OUTPUT);
  pinMode(PinPIR,INPUT);

  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(WIFINAME, WIFIPASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup MQTT
  client.begin(callback);
  client.ubidotsSubscribe("sic7", "switch1");
  client.ubidotsSubscribe("sic7", "switch2");

  Serial.println("Menunggu data dari Ubidots...");
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("GLIATHUS - POLMAN");
  display.println("ALI DZAKWAN RACHMAT");
  display.println("AGAM GUNATA");
  display.println("NAUFAL");
  display.println("ALI ZIYAD");
  display.display();
  delay(2000); 
}

void loop() {
  //reconnect ke ubidots
  if (!client.connected()) {
    client.reconnect();
    client.ubidotsSubscribe("sic7", "switch1");
    client.ubidotsSubscribe("sic7", "switch2");
  }

  //inisialisasi
  int temperature = 0; //temperature dht11
  int humidity = 0; //humidity dht11
  int result = dht11.readTemperatureHumidity(temperature,humidity); //DHT11
  int humidityM = analogRead(PinHumidityMoisture); //soil humidity moisture sensor
  int PIRState = digitalRead(PinPIR); //PIR
  int LDRState = analogRead(PinLDR); //LDR
  int button1State = digitalRead(PinButton1); //button1
  int button2State = digitalRead(PinButton2); //button2

  //button ganti interface
  if(button1State == 0){
    digitalWrite(PinBuzzer,HIGH);
    delay(100);
    client.add("button1",1);
    client.add("buzzer",1);
    client.ubidotsPublish("sic7");
    if(lastMenu == 2){
      lastMenu = -1;
    }
    digitalWrite(PinBuzzer,LOW);
    client.add("buzzer",0);
    client.ubidotsPublish("sic7");
    lastMenu+=1;
  }else{
    digitalWrite(PinBuzzer,LOW);
    client.add("button1",0);
    client.add("buzzer",0);
    client.ubidotsPublish("sic7");
  }

  //interface menu
  if(lastMenu == 0){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    // Display static text
    display.println("GLIATHUS - POLMAN");
    display.println("");
    display.println("DATA DHT11");
    display.print("Temperature : ");
    display.print(temperature);
    display.println(" C");
    display.print("Humidity: ");
    display.print(humidity);
    display.println(" %");
    display.display();
  }else if(lastMenu == 1){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    // Display static text
    display.println("GLIATHUS - POLMAN");
    display.println("");
    display.println("DATA SOIL MOISTURE SENSOR");
    display.print("Humidity: ");
    display.println(humidityM);
    display.display();
  }else if(lastMenu == 2){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    // Display static text
    display.println("GLIATHUS - POLMAN");
    display.println("");
    display.println("DATA LDR");
    display.print("LDR: ");
    display.println(LDRState);
    display.display();
  }
  
  //sensor soil humidity moisture
  if(humidityM > 3000 || button2State == 0 || nilaiSwitch1 == 1){
    digitalWrite(PinMotor1,LOW);
    digitalWrite(PinLED2,HIGH);
    if(button2State == 0){
      client.add("button2",1);
    }
    client.add("humiditymoisture",humidityM);
    client.add("led2",1);
    client.add("motor",1);
    client.ubidotsPublish("sic7");
  }else{
    digitalWrite(PinMotor1,HIGH);
    digitalWrite(PinLED2,LOW);
    client.add("humiditymoisture",humidityM);
    client.add("button2",0);
    client.add("led2",0);
    client.add("motor",0);
    client.ubidotsPublish("sic7");
  }

  //
  if(PIRState == 1){
    digitalWrite(PinLED1,HIGH);
    digitalWrite(PinBuzzer,HIGH);
    client.add("led1",1);
    client.add("pir",1);
    client.add("buzzer",1);
    client.ubidotsPublish("sic7");
    delay(100);
    digitalWrite(PinBuzzer,LOW);
    digitalWrite(PinLED1,LOW);
    client.add("led1",0);
    client.add("buzzer",0);
    client.ubidotsPublish("sic7");
  }else{
    client.add("pir",0);
    client.ubidotsPublish("sic7");
  }

  if(LDRState == 4095 || nilaiSwitch2 == 1){
    digitalWrite(PinLED3,HIGH);
    client.add("ldr",LDRState);
    client.add("led3",1);
    client.ubidotsPublish("sic7");
  }else{
    digitalWrite(PinLED3,LOW);
    client.add("ldr",LDRState);
    client.add("led3",0);
    client.ubidotsPublish("sic7");
  }

  client.add("temperature",temperature);
  client.add("humiditydht11",humidity);
  client.ubidotsPublish("sic7");
  Serial.print("temp : ");
  Serial.println(temperature);
  Serial.print("humi : ");
  Serial.println(humidity);
  Serial.print("result : ");
  Serial.println(result);
  Serial.print("PIR : ");
  Serial.println(PIRState);
  Serial.print("LDR : ");
  Serial.println(LDRState);
  Serial.print("HUMI : ");
  Serial.println(humidityM);
  Serial.print("B1 : ");
  Serial.println(button1State);
  Serial.print("B2 : ");
  Serial.println(button2State);
  Serial.println("=========================================");
  client.loop();
}
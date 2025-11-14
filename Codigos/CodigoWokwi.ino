#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ========================= CONFIG WiFi/MQTT =========================
const char* SSID = "Wokwi-GUEST";//Nome do wifi que sera usado
const char* PASSWORD = "";//Senha do wifi
const char* BROKER_MQTT = "44.223.43.74"; //Ip do servidor fiware
const int BROKER_PORT = 1883;

// ========================= TOPICOS FIWARE =========================
const char* TOPICO_SUBSCRIBE  = "/TEF/device070/cmd";
const char* TOPICO_PUBLISH_1  = "/TEF/device070/attrs";    // stressIndex
const char* TOPICO_PUBLISH_2  = "/TEF/device070/attrs/p";  // luminosidade
const char* TOPICO_PUBLISH_H  = "/TEF/device070/attrs/h";  // umidade
const char* TOPICO_PUBLISH_T  = "/TEF/device070/attrs/t";  // temperatura
const char* ID_MQTT           = "fiware_001";

const char* topicPrefix = "device070";

// ========================= PINOS =========================
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const int LED_ONBOARD = 2;
const int POT_PIN     = 35;   // potenciômetro -> stress bruto
const int LUM_PIN = 34; // luminosidade
const int BUZZER_PIN  = 27;

// ========================= LCD =========================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ========================= MQTT =========================
WiFiClient espClient;
PubSubClient MQTT(espClient);

char EstadoSaida = '0'; // ligado/desligado via comando FIWARE

// ========================= PROTÓTIPOS =========================
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();
void reconectWiFi();
void VerificaConexoesWiFIEMQTT();
void publishStressIndex(float stressIndex);

// ========================= SETUP =========================
void setup() {
  Serial.begin(115200);

  pinMode(LED_ONBOARD, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.print("StressBreak IoT");

  dht.begin();

  WiFi.begin(SSID, PASSWORD);
  delay(2000);

  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

// ========================= LOOP =========================
void loop() {
  VerificaConexoesWiFIEMQTT();

  // ==== LEITURAS ====
  int lumRaw = analogRead(LUM_PIN);
  int potRaw = analogRead(POT_PIN);

  int luminosidade = map(lumRaw, 0, 4095, 0, 100);
  int potStress    = map(potRaw, 0, 4095, 0, 100);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h)) MQTT.publish(TOPICO_PUBLISH_H, String(h).c_str());
  if (!isnan(t)) MQTT.publish(TOPICO_PUBLISH_T, String(t).c_str());

  MQTT.publish(TOPICO_PUBLISH_2, String(luminosidade).c_str());

  // ==== CÁLCULO DO ÍNDICE DE ESTRESSE ====
  float desconforto = abs(22 - t) * 3.5;  // longe de 22°C = desconforto
  float stressIndex = (0.4 * potStress) +
                      (0.3 * desconforto) +
                      (0.3 * (100 - luminosidade));

  if (stressIndex > 100) stressIndex = 100;
  if (stressIndex < 0)   stressIndex = 0;

  publishStressIndex(stressIndex);

  // ==== FEEDBACK LOCAL ====
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stress: ");
  lcd.print((int)stressIndex);
  lcd.print("%");

  lcd.setCursor(0, 1);
  if (stressIndex < 40) {
    lcd.print("OK");
  } else if (stressIndex < 70) {
    lcd.print("Atencao");
    tone(BUZZER_PIN, 800, 100);
  } else {
    lcd.print("ALTO!");
    tone(BUZZER_PIN, 2000, 200);
  }

  MQTT.loop();
  delay(1500);
}

// ========================= PUBLICA INDEX =========================
void publishStressIndex(float stressIndex) {
  String payload = "s|" + String((int)stressIndex);
  MQTT.publish(TOPICO_PUBLISH_1, payload.c_str());
}

// ========================= CALLBACK FIWARE =========================
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  String cmdON  = String(topicPrefix) + "@on|";
  String cmdOFF = String(topicPrefix) + "@off|";

  if (msg == cmdON) {
    digitalWrite(LED_ONBOARD, HIGH);
    EstadoSaida = '1';
  }

  if (msg == cmdOFF) {
    digitalWrite(LED_ONBOARD, LOW);
    EstadoSaida = '0';
  }
}

// ========================= RECONEXÃO =========================
void VerificaConexoesWiFIEMQTT() {
  if (!MQTT.connected()) reconnectMQTT();
  if (WiFi.status() != WL_CONNECTED) reconectWiFi();
}

void reconectWiFi() {
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(100);
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    if (MQTT.connect(ID_MQTT)) {
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    } else {
      delay(2000);
    }
  }
}

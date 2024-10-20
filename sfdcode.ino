#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>

#define WIFI_SSID "123456789"
#define WIFI_PASSWORD "123456789"
#define API_KEY "AIzaSyC0gPSHesz3RxIsbFM48OkKK_zCBhfbtmc"
#define DATABASE_URL "https://test-26075-default-rtdb.firebaseio.com/"


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

#define DHTPIN D4          // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11      // DHT 11
#define IRPIN D2           // Digital pin connected to the IR sensor
#define MQ4PIN A0          // Analog pin connected to the MQ-4 sensor
#define RED_LED_PIN D5     // Digital pin connected to the red LED
#define GREEN_LED_PIN D6   // Digital pin connected to the green LED

DHT dht(DHTPIN, DHTTYPE);

float temperatureThreshold = 35.0; // Temperature threshold in Celsius
float humidityThreshold = 70.0;   // Humidity threshold in percentage

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(IRPIN, INPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  // Configure Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Firebase authentication successful");
    signupOK = true;
  }
  else{
    Serial.printf("Firebase signup error: %s\n", config.signer.signupError.message.c_str());
  }
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  delay(2000);  // Wait for 2 seconds between measurements
  
  int irValue = digitalRead(IRPIN);
  int mq4Value = analogRead(MQ4PIN);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  Serial.print("IR Sensor Value: ");
  Serial.println(irValue);
  Serial.print("MQ-4 Sensor Value: ");
  Serial.println(mq4Value);
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%  Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
  
  // Check if the temperature or humidity exceeds the threshold
  if (temperature > temperatureThreshold || humidity > humidityThreshold || irValue == HIGH || mq4Value > 200) {
    Serial.println("Food might be spoiled! Take necessary actions.");
    digitalWrite(RED_LED_PIN, HIGH);   // Turn on the red LED
    digitalWrite(GREEN_LED_PIN, LOW);  // Turn off the green LED
  } else {
    Serial.println("Food is safe.");
    digitalWrite(RED_LED_PIN, LOW);    // Turn off the red LED
    digitalWrite(GREEN_LED_PIN, HIGH); // Turn on the green LED
  }
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.setInt(&fbdo, "main/irValue", irValue)){
      Serial.println("irValue data sent to Firebase");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("Failed to send irValue data to Firebase"+ fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "main/temperature", temperature)){
      Serial.println("temperature data sent to Firebase");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("Failed to send temperature data to Firebase"+ fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "main/humidity", humidity)){
      Serial.println("humidity data sent to Firebase");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("Failed to send humidity data to Firebase"+ fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "main/mq4Value", mq4Value)){
      Serial.println("mq4Value data sent to Firebase");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("Failed to send mq4Value data to Firebase"+ fbdo.errorReason());
    }
}
}

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>

/* 
// Replace the following variables with your Wifi and Telegram Bot Token

#define BOT_TOKEN " "
#define CHAT_ID " "
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
*/


// DHT & LED
#define DHT_TYPE DHT22    // Type of DHT sensor
#define DHT_PIN 27        // DHT22 pin connected to GPIO pin 27
#define LED_PIN_1 26      // LED pin connected to GPIO pin 26   <- for monitoring LED (red)
#define LED_PIN_2 25      // LED pin connected to GPIO pin 25   <- for standby LED (green)

#define intervalTime 1200000      // settings this value with your desired interval
// Interval for 45 minutes (45 minutes = 45 * 60 * 1000 ms = 2700000 ms)
// Interval for 20 minutes (20 minutes = 20 * 60 * 1000 ms = 1200000 ms)
// Interval for 10 minutes (10 minutes = 10 * 60 * 1000 ms = 100000 ms)
// Interval for 1 minute (1 minute = 1 * 60 * 1000 ms = 60000 ms)
// For LED testing purposes, use 10 seconds (10000 ms)

unsigned long previousMillisLED2 = 0; // variable to store previous time for LED_PIN_2
const long intervalLED2 = 2250; // interval time for LED_PIN_2 to blink (2250 ms)

unsigned long previousMillisDHT = 0; // variable to store previous time for temperature monitoring
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

/*
use this if you had root certificate for telegram

Telegram root certificate (replace with the appropriate root certificate)
const char TELEGRAM_CERTIFICATE_ROOT[] = \
  "-----BEGIN CERTIFICATE-----\n" \
  "YOUR_CERT_HERE\n" \
  "-----END CERTIFICATE-----\n";

*/

bool isFirstConnect = true;
bool wasDisconnected = false;

DHT dht(DHT_PIN, DHT_TYPE);

///// CODE BEGIN /////

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  
  // Connect to WiFi network
  connectWiFi();
  
  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Function to control the blinking of LED_PIN_2
  if (currentMillis - previousMillisLED2 >= intervalLED2) {
    previousMillisLED2 = currentMillis;
    digitalWrite(LED_PIN_2, HIGH);
    delay(250);
    digitalWrite(LED_PIN_2, LOW);
  }

  // Check WiFi connection status
  if (WiFi.status() != WL_CONNECTED) {
    wasDisconnected = true;
    connectWiFi();
  } else {
    if (wasDisconnected) {
      wasDisconnected = false;
      sendReconnectMessage();
    }

    // Monitor temperature at every intervalTime
    if (currentMillis - previousMillisDHT >= intervalTime) {
      previousMillisDHT = currentMillis;
      float temperature = dht.readTemperature(); // Read temperature from DHT sensor
      if (!isnan(temperature)) { 
        // Check if the temperature value is valid
        // LED blinks once shortly when temperature is read
        digitalWrite(LED_PIN_1, HIGH);
        delay(250);
        digitalWrite(LED_PIN_1, LOW);
        delay(250);
        // Send message to Telegram bot
        sendTelegramMessage(temperature);
      }
    }
  }
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // LED blinks continuously while searching for WiFi connection
    digitalWrite(LED_PIN_1, HIGH);
    delay(500);
    digitalWrite(LED_PIN_1, LOW);
    delay(250);
    Serial.println("Connecting to WiFi '" + String(WIFI_SSID) + "'");
  }

  Serial.println("Connected to WiFi '" + String(WIFI_SSID) + "'");
  Serial.println("WiFi IP: " + WiFi.localIP().toString());

  // LED stays on for 5 seconds when WiFi is successfully connected
  digitalWrite(LED_PIN_1, HIGH);
  delay(5000);
  digitalWrite(LED_PIN_1, LOW);

  // Set the Telegram root certificate
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  // Send message when first successfully connected
  if (isFirstConnect) {
    isFirstConnect = false;
    sendOnlineMessage();
  }
}

void sendTelegramMessage(float temperature) {
  String message = "Current temperature: " + String(temperature) + " °C";
  Serial.println(message);

  // Send message to Telegram bot
  if (!bot.sendMessage(CHAT_ID, message, "")) {
    Serial.println("Failed to send temperature message to Telegram!");
    // LED blinks three times shortly when message sending fails
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN_1, HIGH);
      delay(250);
      digitalWrite(LED_PIN_1, LOW);
      delay(250);
    }
  } else {
    Serial.println("Current temperature successfully sent to Telegram!");
    // LED blinks twice shortly when message sending is successful
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_PIN_1, HIGH);
      delay(250);
      digitalWrite(LED_PIN_1, LOW);
      delay(250);
    }
  }
}

void sendOnlineMessage() {
  String message = "Temperature monitor is online. \nModule connected to '" + String(WIFI_SSID) + "'";
  Serial.println("\nTemperature monitor is online. \nModule connected to '" + String(WIFI_SSID) + "'\n");
  bot.sendMessage(CHAT_ID, message, "");
}

void sendReconnectMessage() {
  String message = "Temperature monitor is back online. Module reconnected to '" + String(WIFI_SSID) + "'";
  Serial.println(message);
  bot.sendMessage(CHAT_ID, message, "");
}

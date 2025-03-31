#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <DHT.h> 
#include "image_data.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// MQTT Configuration
static const char* mqtt_server = "mqtt-dashboard.com"; // Địa chỉ broker HiveMQ Cloud
static const int mqtt_port = 1883;  // Cổng MQTT bảo mật
static const char* mqtt_topic = "quang/home"; // Chủ đề MQTT
static const char* mqtt_username = "minhquang"; // Username
static const char* mqtt_password = "quang123"; // Mật khẩu

WiFiClient espClient;
PubSubClient client(espClient);

// WiFi Configuration
const char* ssid = "milu-misa"; // Thay bằng SSID Wi-Fi của bạn
const char* password = "19551963luna"; // Thay bằng mật khẩu Wi-Fi

// TFT Configuration
#define TFT_CS        14   // Chip Select
#define TFT_RST       33   // Reset
#define TFT_DC        27   // Data/Command
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// DHT11 Configuration
#define DHT_PIN  4        // Pin connected to DHT11 data pin
#define DHT_TYPE DHT11    // Define DHT type as DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// Image dimensions
#define IMAGE_WIDTH   50
#define IMAGE_HEIGHT  50
#define DEVICE1_PIN   16
#define DEVICE2_PIN   17
// PWM Configuration for GPIO13
#define LED_PIN 13            // GPIO13
#define PWM_CHANNEL 0         // PWM channel
#define PWM_RESOLUTION 8      // Resolution (8-bit: 0-255)
#define PWM_FREQUENCY 5000    // PWM frequency (5 kHz)

// Gas Sensor Configuration
#define MQ2_PIN 35       // Chân ADC kết nối với MQ2
#define GAS_THRESHOLD 600  // Ngưỡng phát hiện khí gas

// Student Information
#define STUDENT_NAME "LE MINH QUANG"
#define STUDENT_ID   "MSSV: 21139045"

// Global variables
float temperature = 30.0;
float humidity = 50.0;
float gasValue = 0.0;
unsigned long previousMillis = 0;
const long interval = 2000;

void setup_mqtt();
void reconnectMQTT();
void callback(char* topic, byte* payload, unsigned int length);
void updateTemperatureImage();
void updateHumidityImage();
void updateGasDisplay();
void displayStudentInfo();

void setup() {
  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  // Initialize MQTT
  setup_mqtt();

  // Initialize the display
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  // Initialize the DHT sensor
  dht.begin();

  // Configure PWM for LED
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 153);
  pinMode(DEVICE1_PIN, OUTPUT);
  pinMode(DEVICE2_PIN, OUTPUT);
  // Display initial images and text
  tft.drawRGBBitmap(10, 10, humi, IMAGE_WIDTH, IMAGE_HEIGHT);
  updateTemperatureImage();
  updateHumidityImage();
  displayStudentInfo();
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  unsigned long currentMillis = millis();

  // Cập nhật nhiệt độ và độ ẩm
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float newTemperature = dht.readTemperature();
    float newHumidity = dht.readHumidity();
    gasValue = analogRead(MQ2_PIN);
    char tempStr[8];

    if (!isnan(newTemperature) && !isnan(newHumidity)) {
      temperature = newTemperature;
      humidity = newHumidity;
    }
    dtostrf(temperature, 6, 2, tempStr);
    client.publish("quang/home/temperature", tempStr);
    dtostrf(humidity, 6, 2, tempStr);
    client.publish("quang/home/humidity", tempStr);
    dtostrf(gasValue, 6, 2, tempStr);
    client.publish("quang/home/gas", tempStr);
    updateTemperatureImage();
    updateHumidityImage();
    updateGasDisplay();
  }

  // Kích hoạt LED cảnh báo nếu vượt ngưỡng
  if (gasValue > GAS_THRESHOLD) {
    ledcWrite(PWM_CHANNEL, 255);  // Tăng sáng LED (cảnh báo)
    client.publish(mqtt_topic, "Gas Alert!"); // Gửi cảnh báo qua MQTT
  } else {
    ledcWrite(PWM_CHANNEL, 10);  // Giảm sáng LED (bình thường)
  }
}

void setup_mqtt() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_Client", mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT");
      client.subscribe(mqtt_topic); // Subscribe to topic
    } else {
      Serial.print("Failed to connect. Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (message == "0") {
    digitalWrite(DEVICE1_PIN, HIGH); // DEVICE1_PIN output HIGH
    Serial.println("DEVICE1_PIN turned ON");
  } else if (message == "1") {
    digitalWrite(DEVICE1_PIN, LOW); // DEVICE1_PIN output LOW
    Serial.println("DEVICE1_PIN turned OFF");
  } else if (message == "2") {
    digitalWrite(DEVICE2_PIN, HIGH); // DEVICE2_PIN output HIGH
    Serial.println("DEVICE2_PIN turned ON");
  } else if (message == "3") {
    digitalWrite(DEVICE2_PIN, LOW); // DEVICE2_PIN output LOW
    Serial.println("DEVICE2_PIN turned OFF");
  } else {
    Serial.println("Unknown command received");
  }
}


void updateTemperatureImage() {
  // Clear previous temperature image area
  tft.fillRect(80, 10, IMAGE_WIDTH, IMAGE_HEIGHT, ST77XX_BLACK);

  // Display the appropriate image based on the temperature
  if (temperature > 30) {
    tft.drawRGBBitmap(80, 10, temp_hot, IMAGE_WIDTH, IMAGE_HEIGHT);
  } else {
    tft.drawRGBBitmap(80, 10, temp_cold, IMAGE_WIDTH, IMAGE_HEIGHT);
  }

  // Clear previous temperature text area
  tft.fillRect(80, 70, 60, 20, ST77XX_BLACK);

  // Display the current temperature
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(80, 70);
  tft.print("Temp: ");
  tft.print(temperature, 1);  // Display with one decimal place
  tft.println(" C");
}

void updateHumidityImage() {
  // Clear previous humidity text area
  tft.fillRect(10, 70, 60, 20, ST77XX_BLACK);

  // Display the current humidity
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 70);
  tft.print("Humi: ");
  tft.print(humidity, 1);  // Display with one decimal place
  tft.println("%");
}

void updateGasDisplay() {
  // Xóa thông tin khí gas cũ
  tft.fillRect(10, 120, 100, 20, ST77XX_BLACK);

  // Hiển thị giá trị khí gas mới
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 80);
  tft.print("Gas: ");
  tft.print(gasValue, 0);  // Hiển thị giá trị ADC (không có chữ số thập phân)

  // Hiển thị cảnh báo nếu vượt ngưỡng
  if (gasValue > GAS_THRESHOLD) {
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 90);
    tft.print("ALERT!");
  } else {
    tft.fillRect(10, 90, 60, 10, ST77XX_BLACK);  // Xóa cảnh báo cũ nếu có
  }
}

void displayStudentInfo() {
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  
  // Hiển thị tên
  tft.setCursor(10, 100);
  tft.print(STUDENT_NAME);
  
  // Hiển thị mã số sinh viên
  tft.setCursor(10, 110);
  tft.print(STUDENT_ID);
}
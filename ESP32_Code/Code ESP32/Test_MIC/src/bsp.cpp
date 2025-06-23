#include "bsp.h"

// Khởi tạo các đối tượng
WiFiClientSecure espClient;
PubSubClient client(espClient);
WebSocketsServer webSocket(WEBSOCKET_PORT);

/*
//-------------------set up cho MQTT----------------//
static const char* mqtt_server = "4be2d2486f5d49378e37387b5ab071a2.s1.eu.hivemq.cloud"; // Địa chỉ broker HiveMQ Cloud
static const int mqtt_port = 8883;  // Cổng MQTT không bảo mật
static const char* mqtt_topic = "test/topic"; // Chủ đề MQTT
static const char* mqtt_username = "hivemq.webclient.1731080373406"; // Username của bạn
static const char* mqtt_password = "60W.>lvV@B9fGmzUA*4j"; // Mật khẩu của bạn từ HiveMQ Cloud

// Định nghĩa hàm reconnectMQTT
void reconnectMQTT() {
  // Thêm logic kết nối lại MQTT sau một thời gian dài nếu bị mất kết nối
  while (!client.connected()) {
    
    Serial.print("Connecting to MQTT...");
    espClient.setInsecure();
    if (client.connect("ESP32_Client", mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT");
      client.subscribe(mqtt_topic); // Đăng ký chủ đề
    } else {
      Serial.print("Failed to connect. Retrying in 5 seconds...");
      delay(5000);  // Chờ 5 giây trước khi thử lại
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (message == "1") {
    digitalWrite(GPIO_PIN_LIGHT, HIGH); // Bật GPIO19
    Serial.println("GPIO19 turned ON");
  } else if (message == "0") {
    digitalWrite(GPIO_PIN_LIGHT, LOW); // Tắt GPIO19
    Serial.println("GPIO19 turned OFF");
  } else if (message =="2"){
    digitalWrite(GPIO_PIN_MOTOR, HIGH);
    Serial.println("GPIO21 turned ON");
  } else if (message =="3"){
    digitalWrite(GPIO_PIN_MOTOR, LOW);
    Serial.println("GPIO21 turned OFF");
  }
}

void setup_mqtt(){
  client.setServer(mqtt_server, mqtt_port); // Sử dụng cổng MQTT đã cập nhật
  client.setCallback(callback); // Đăng ký hàm callback
}
*/


// Cấu hình I2S không sử dụng DMA
void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_CLK_PIN,
    .ws_io_num = I2S_LRCL_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_DATA_IN_PIN
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}


void setupWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.print("Địa chỉ IP của ESP32: ");
  Serial.println(WiFi.localIP()); // In địa chỉ IP ra màn hình
}


void setupWebSocket() {
  webSocket.begin();
  webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_TEXT) {
      Serial.println("Received message from WebSocket client");
      // Xử lý thông điệp nhận được từ WebSocket tại đây
    }
  });
}

void sendDataWebSocket(const int16_t* buffer, size_t bufferSize) {
  // Kiểm tra nếu WebSocket client có kết nối trước khi gửi dữ liệu
  if (webSocket.connectedClients()) {
    webSocket.broadcastBIN((uint8_t*)buffer, bufferSize * sizeof(int16_t));
  }
}

void readDataTask(void *parameter) {
  int16_t buffer[BUFFER_SIZE];
  size_t bytes_read;

  while (1) {
    i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);

    if (DEBUG_MODE == 1) {
      Serial.println("Received Audio Data:");
      for (int i = 0; i < 10; i++) {
        Serial.print(buffer[i]);
        Serial.print(" ");
      }
      Serial.println();
    }
    sendDataWebSocket(buffer, BUFFER_SIZE);
  }
}


void setup_gpio(){
  pinMode(GPIO_PIN_LIGHT, OUTPUT); // Cấu hình chân GPIO4 làm OUTPUT
  pinMode(GPIO_PIN_MOTOR, OUTPUT);
}
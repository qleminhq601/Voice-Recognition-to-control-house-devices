#include <bsp.h>

void setup() {
  Serial.begin(9600);
  setupWiFi();
 // setup_mqtt();
  setupWebSocket();
  setup_gpio();
  setupI2S();
  xTaskCreate(readDataTask, "ReadData", 4096, NULL, 1, NULL);
}

void loop() {
  /*if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();*/
  webSocket.loop();
}

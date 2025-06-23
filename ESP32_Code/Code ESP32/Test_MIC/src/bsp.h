/* Mấy phần #ifndef BSP_H, #define BSP_H, #endif: Đây là phần chỉ thị tiền xử lý 
(preprocessor directive) trong C/C++ được gọi là Include Guard. 
Mục đích của phần này là ngăn chặn việc bao gồm nhiều lần file header trong một project, 
tránh việc khai báo trùng lặp dẫn đến lỗi biên dịch.*/


/* File header này bao gồm các khai báo về:
  + Các thư viện cần thiết (#include <Arduino.h>, #include <driver/i2s.h>, ...).
  + Các định nghĩa hằng số (ví dụ: #define I2S_DATA_IN_PIN 34).
  + Khai báo các đối tượng toàn cục (extern WiFiClientSecure espClient;), giúp các file khác 
    biết đến các biến hoặc đối tượng mà không cần phải định nghĩa lại.
  + Các prototype hàm (như void setupWiFi();, void setupI2S();), tức là các hàm 
    sẽ được triển khai trong file bsp.cpp
*/


#ifndef BSP_H  //Kiểm tra xem có định nghĩa BSP_H hay chưa. ifndef có nghĩa là "if not defined"
#define BSP_H  //Nếu chưa có định nghĩa BSP_H, thì định nghĩa nó. Sau đó, nếu file này được bao gồm lại trong chương trình, phần mã này sẽ không được thực hiện nữa.

#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <PubSubClient.h> //cho mqtt

#define I2S_DATA_IN_PIN  34
#define I2S_CLK_PIN      14
#define I2S_LRCL_PIN     25
#define SAMPLE_RATE      16000  // Tần số mẫu 16 kHz
#define BUFFER_SIZE      1024   // Kích thước buffer
#define DEBUG_MODE       1

#define WIFI_SSID        "milu-misa"
#define WIFI_PASSWORD    "19551963luna"

#define WEBSOCKET_PORT   81     // Cổng WebSocket
#define GPIO_PIN_LIGHT   19
#define GPIO_PIN_MOTOR   21

extern WiFiClientSecure espClient;  // Khai báo biến WiFiClientSecure
extern PubSubClient client;         // Khai báo biến PubSubClient
extern WebSocketsServer webSocket;  // Khai báo biến WebSocketsServer

/* Chứng chỉ SSL của HiveMQ
const char* certificate = R"(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)";*/

void setupWiFi();
void setupI2S();
//void setup_mqtt();
void setup_gpio();
//void reconnectMQTT();
//void callback(char* topic, byte* payload, unsigned int length);
void setupWebSocket();
void sendDataWebSocket(const int16_t* buffer, size_t bufferSize);
void readDataTask(void *parameter);

#endif // BSP_H
/*Đây là chỉ thị kết thúc khối #ifndef/#define. Nó đánh dấu kết thúc của phần bảo vệ này, 
đảm bảo rằng mã trong file header chỉ được bao gồm một lần trong quá trình biên dịch.*/
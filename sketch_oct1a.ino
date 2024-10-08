#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Thông tin mạng WiFi
const char* ssid = "SHP.202";        
const char* password = "22222222";   

// Thông tin MQTT broker
const char* mqtt_server = "192.168.0.103"; 
const char* mqtt_user = "hoang";  
const char* mqtt_password = "b21dccn393";  

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

// Khai báo cảm biến DHT và cảm biến ánh sáng
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define LED_PIN1 D5  // Chân điều khiển đèn LED 1
#define LED_PIN2 D7  // Chân điều khiển đèn LED 2
#define LED_PIN3 D8  // Chân điều khiển đèn LED 3
#define LDR_PIN A0  // Chân nhận tín hiệu analog từ cảm biến ánh sáng
#define LDR_DIGITAL_PIN D6  // Chân nhận tín hiệu số từ cảm biến ánh sáng

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Chuyển đổi payload sang kiểu String
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Điều khiển từng đèn theo từng lệnh riêng lẻ
  if (message == "den 1 bat") {
    digitalWrite(LED_PIN1, HIGH);  
    Serial.println("LED 1 ON");
  } 
  if (message == "den 1 tat") {
    digitalWrite(LED_PIN1, LOW);
    Serial.println("LED 1 OFF");
  }
  if (message == "den 2 bat") {
    digitalWrite(LED_PIN2, HIGH);
    Serial.println("LED 2 ON");
  }
  if (message == "den 2 tat") {
    digitalWrite(LED_PIN2, LOW);
    Serial.println("LED 2 OFF");
  }
  if (message == "den 3 bat") {
    digitalWrite(LED_PIN3, HIGH);
    Serial.println("LED 3 ON");
  }
  if (message == "den 3 tat") {
    digitalWrite(LED_PIN3, LOW);
    Serial.println("LED 3 OFF");
  }

  // Điều khiển kết hợp các đèn theo nhiều lệnh
  if (message == "den 1 bat den 2 tat den 3 tat") {
    digitalWrite(LED_PIN1, HIGH);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
    Serial.println("LED 1 ON, LED 2 OFF, LED 3 OFF");
  }
  if (message == "den 1 tat den 2 bat den 3 bat") {
    digitalWrite(LED_PIN1, LOW);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);
    Serial.println("LED 1 OFF, LED 2 ON, LED 3 ON");
  }
  if (message == "den 1 bat den 2 bat den 3 tat") {
    digitalWrite(LED_PIN1, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, LOW);
    Serial.println("LED 1 ON, LED 2 ON, LED 3 OFF");
  }
  if (message == "den 1 tat den 2 tat den 3 bat") {
    digitalWrite(LED_PIN1, LOW);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, HIGH);
    Serial.println("LED 1 OFF, LED 2 OFF, LED 3 ON");
  }
  if (message == "den 1 bat den 2 tat den 3 bat") {
    digitalWrite(LED_PIN1, HIGH);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, HIGH);
    Serial.println("LED 1 ON, LED 2 OFF, LED 3 ON");
  }
  
  // Thêm các trường hợp tương tự cho các kết hợp khác, ví dụ:
  if (message == "den 1 tat den 2 tat den 3 tat") {
    digitalWrite(LED_PIN1, LOW);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
    Serial.println("All LEDs OFF");
  }
  if (message == "den 1 bat den 2 bat den 3 bat") {
    digitalWrite(LED_PIN1, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);
    Serial.println("All LEDs ON");
  }
  
  // Các lệnh khác (bật/tắt tất cả đèn)
  if (message == "tat tat ca den") {
    digitalWrite(LED_PIN1, LOW);
    digitalWrite(LED_PIN2, LOW);
    digitalWrite(LED_PIN3, LOW);
    Serial.println("All LEDs OFF");
  }
  if (message == "bat tat ca den") {
    digitalWrite(LED_PIN1, HIGH);
    digitalWrite(LED_PIN2, HIGH);
    digitalWrite(LED_PIN3, HIGH);
    Serial.println("All LEDs ON");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.publish("outTopic", "hello world");
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_PIN1, OUTPUT);  
  pinMode(LED_PIN2, OUTPUT);
  pinMode(LED_PIN3, OUTPUT);
  pinMode(LDR_DIGITAL_PIN, INPUT);  // Chân tín hiệu số của cảm biến ánh sáng
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1884);
  client.setCallback(callback);
  dht.begin(); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    // Đọc dữ liệu từ cảm biến DHT
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Lỗi đọc cảm biến DHT!");
      return;
    }

    // Đọc dữ liệu từ cảm biến ánh sáng (analog)
    int lightIntensity = 1030 - analogRead(LDR_PIN);

    // Gửi dữ liệu cảm biến ánh sáng và DHT qua MQTT
    char tempString[8];
    char humString[8];
    char lightString[8];
    dtostrf(t, 1, 2, tempString);  
    dtostrf(h, 1, 2, humString);  
    dtostrf(lightIntensity, 1, 2, lightString); 

    client.publish("temperature", tempString);
    client.publish("humidity", humString);
    client.publish("light_intensity", lightString);
  }
}

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "son";
const char* password = "ayobangun@";
const char* mqtt_server = "broker.emqx.io";
const char* topic = "person/status";

WiFiClient espClient;
PubSubClient client(espClient);
int ledPin = D4;
int trigPin = D8;
int echoPin = D7;
bool ledState = false;
int personCount = 0;
bool outStatus = false;

long duration;
float distance;

#define SOUND_SPEED 0.034

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
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '0') {
    digitalWrite(ledPin, LOW);
  } else {
    digitalWrite(ledPin, HIGH);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.publish(topic, "0"); // Publikasikan pesan saat terkoneksi
      client.subscribe(topic); // Berlangganan ke topik
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * SOUND_SPEED / 2;

  if(distance > 0 && distance < 50){
    if(personCount > 0 && ledState){
      if(personCount < 0){
        personCount = 0;
      }else if(personCount == 3){
        outStatus = true;
      }else{
        personCount++;
      }
    }else{
      if(personCount > 3) {
        personCount = 3;
      }else{
        personCount++;
      }
    }
  }

  if(outStatus){
    personCount--;
  }

  if(personCount > 0){
    ledState = true;
  }else{
    outStatus = false;
    ledState = false;
  }

  Serial.print("Person Count: ");
  Serial.print(personCount);
  Serial.print(" LED State: ");
  Serial.println(ledState);

  if(ledState){
    client.publish(topic, "1");
    digitalWrite(ledPin, HIGH);
  } else {
    client.publish(topic, "0");
    digitalWrite(ledPin, LOW);
    
  }

  delay(350);
}
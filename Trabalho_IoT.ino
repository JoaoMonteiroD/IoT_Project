#include <ThingSpeak.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Configuração do Broker MQTT
const char* mqtt_server = "192.168.1.127"; // IP do broker MQTT
const int mqtt_port = 1883;                // Porta MQTT
const char* topic_1 = "IoT/Distancia";     // Tópico MQTT
const char* topic_2 = "IoT/Gas";

// Wi-Fi
const char* ssid = "MEO-9BFDC0"; // Nome Wi-Fi
const char* password = "b49d14e8e6";      // Senha Wi-Fi

// Credenciais ThingSpeak
unsigned long myChannelNumber = 2747644;  // Número do canal ThingSpeak
const char* myWriteAPIKey = "Y61T1BX0OSJAZT3V"; // API do ThingSpeak

WiFiClient espClient;       // Cliente Wi-Fi
PubSubClient client(espClient); // Cliente MQTT
unsigned long lastMsg = 0;

// Pinos para os sensores
#define TRIG_PIN 12         // Pino Trigger do sensor de distância
#define ECHO_PIN 14         // Pino Echo do sensor de distância
#define GAS_SENSOR_PIN 34   // Pino analógico do sensor de gás (MQ-6)

// Função para medir a distância
float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duracao = pulseIn(ECHO_PIN, HIGH);
  float distancia = (duracao * 0.034) / 2;
  return distancia;
}

// Callback do MQTT
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  Serial.print("Mensagem: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();
}

// Reconnect ao MQTT Broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao broker MQTT...");
    if (client.connect("ESP32_Client")) {
      Serial.println("Conectado!");
      client.subscribe(topic_1);
      client.subscribe(topic_2);
    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configuração dos pinos dos sensores
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GAS_SENSOR_PIN, INPUT);

  // Conexão ao Wi-Fi
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");

  // Configuração do MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Inicialização do ThingSpeak
  ThingSpeak.begin(espClient);

  Serial.println("Sistema inicializado!");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 10000) { // Enviar a cada 10 segundos
    lastMsg = now;

    // Leitura dos sensores
    float distancia = medirDistancia();
    int valorGas = analogRead(GAS_SENSOR_PIN);

    // Enviar os valores para MQTT
    String distanceMessage = String(distancia) + " cm";
    String gasMessage = String(valorGas) + " ppm";

    client.publish(topic_1, distanceMessage.c_str());
    client.publish(topic_2, gasMessage.c_str());

    // Enviar valores ao ThingSpeak
    ThingSpeak.setField(1, distancia);
    ThingSpeak.setField(2, valorGas);
    int resposta = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if (resposta == 200) {
      Serial.println("Dados enviados ao ThingSpeak com sucesso.");
    } else {
      Serial.print("Erro ao enviar ao ThingSpeak: ");
      Serial.println(resposta);
    }

    // Imprimir no monitor serial
    Serial.print("Distância: ");
    Serial.println(distanceMessage);
    Serial.print("Gás: ");
    Serial.println(gasMessage);
  }
}

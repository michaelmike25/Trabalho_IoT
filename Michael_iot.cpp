#include "DHT.h" // Biblioteca para o sensor DHT11
#include <PubSubClient.h> // biblioteca pub sub mqtt para ESP8266
#include <ESP8266WiFi.h> // biblioteca de especificações da ESP8266
#include <WiFiClientSecure.h>  // Cliente seguro para TLS

#define DHTPIN 2       // Pino de conexão do DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT11

// Credenciais Wi-Fi
const char* ssid = "rede"; //Rede
const char* password = "senha"; // Senha da rede

// Configurações do broker HiveMQ
const char* mqtt_server = "chave do servidor"; // chave do servidor que recebe as informações enviadas via MQTT
const int mqtt_port = porta;
const char* mqtt_user = "usuario";  // usuário HiveMQ
const char* mqtt_password = "senha";  // senha HiveMQ

// Tópicos MQTT para temperatura e umidade
const char* temp_topic = "mestrado/iot/michael/temperatura";
const char* hum_topic = "mestrado/iot/michael/umidade";

// Inicializa o sensor DHT, cliente Wi-Fi seguro e cliente MQTT
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure espClient;  // uso para conexão com segurança
PubSubClient client(espClient); // conexão pub/su mqtt

void setup() {
  Serial.begin(9600);
  Serial.println(F("Iniciando DHT11 e MQTT..."));
  
  dht.begin();       // Inicializa o sensor DHT
  wifi_conexao();    // Conecta ao Wi-Fi

  espClient.setInsecure();  // Desativa a verificação do certificado TLS (somente para testes)
  client.setServer(mqtt_server, mqtt_port);  // Configura o servidor MQTT
  client.setCallback(mqttCallback);  // Define a função de callback para mensagens recebidas
}

void loop() {
  if (!client.connected()) {
    mqtt_reconnect();  // Tenta reconectar ao broker, se desconectado
  }
  
  client.loop();  // Processa mensagens MQTT recebidas

  // Leitura do sensor a cada 2 segundos (tempo minimo 1s)
  delay(2000);

  float h = dht.readHumidity(); // variavel umidade
  float t = dht.readTemperature(); // variavel temperatura

  // Verifica se a leitura foi das variaveis foi bem-sucedida
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Falha na leitura do sensor DHT!"));
    return;
  }

  // Exibe os dados no Serial Monitor
  Serial.print("Umidade: "); Serial.print(h);
  Serial.print("%  Temperatura: "); Serial.print(t); Serial.println("°C");

  // Publica os dados de temperatura e umidade em tópicos separados
  client.publish(temp_topic, String(t).c_str(), true);  // Publica a temperatura
  client.publish(hum_topic, String(h).c_str(), true);   // Publica a umidade

  Serial.println("Dados publicados:"); // publica os dados no broker
  Serial.print("Temperatura em "); Serial.print(temp_topic); Serial.print(": "); Serial.println(t);
  Serial.print("Umidade em "); Serial.print(hum_topic); Serial.print(": "); Serial.println(h);
}

// Função para conectar ao Wi-Fi
void wifi_conexao() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Função para reconectar ao broker MQTT, caso a conexão caia retorna o estado atual da conexão
void mqtt_reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT... ");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado!");
      client.subscribe(temp_topic);  // Inscreve-se no tópico de temperatura
      client.subscribe(hum_topic);   // Inscreve-se no tópico de umidade
    } else {
      Serial.print("Falha na conexão, estado: ");
      Serial.println(client.state());
      delay(5000);  // Aguarda 5s antes de tentar novamente
    }
  }
}

// Função de callback para tratar mensagens recebidas confirmando o envio e recebimento no terminal
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  Serial.print("Mensagem: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);  // Converte cada byte em caractere e concatenar na saída
  }
  Serial.println();
}
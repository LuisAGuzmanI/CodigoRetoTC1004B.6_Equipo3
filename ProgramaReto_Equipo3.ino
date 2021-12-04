// Programa para el reto Equipo 3
// Luis Ángel Guzmán Iribe - A01741757
// Esteban Martínez Aguiar - A00831789
// Paola De La Rosa - A01233794
// Emiliano Limón Cantú - A01177520

// ----- Definiciones de pins de NodeMCU (se cuenta de arriba hacia abajo)
#define D0 16   // Arriba derecha 1
#define D1 5    // Arriba derecha 2
#define D2 4    // Arriba derecha 3
#define D3 0    // Arriba derecha 4
#define D4 2    // Arriba derecha 5
// Posiciones 6 3V3, 7 GND
#define D5 14   // Arriba derecha 8
#define D6 12   // Arriba derecha 9
#define D7 13   // Arriba derecha 10
#define D8 15   // Arriba derecha 11
#define SD3 10  // Arriba izquierda 4
#define SD2  9  // Arriba izquierda 5
float h, t, f;
float d;  //m

// ----- Definiciones del sensor de temperatura DHT11
#include "DHT.h"
#define DHTPIN 5  // Posicion equivalente a D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int identificador = 1;

// ----- Definiciones de fotorresistor
int photoPin = A0;
int ledphotoResistor = 16;  // Equivale a D0
int x = -1;  //0..1023

// ----- Definiciones de ESP8266MQTT
/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>  // Si no existe la biblioteca
                           // Nota bajar .zip https://www.arduinolibraries.info/libraries/pub-sub-client
                           // Instalar Menu: Programa / Submenu: Incluir libreria / Anadir biblioteca PubSubClientxxx.zip
const char* ssid = "Samsung Galaxy S20 5G 7604"; // Nombre de la red
const char* password = "ballade1"; // Contrasena
const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* topico_salida = "NodeRED_Test_MQTT";
const char* topico_entrada = "NodeRED_Salida"; //Clima o aire acondicionado
char sTopicoOutDatos[50];

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// =================================================================
// ----- Funcion que Conecta a Wifi del Router
// We start by connecting to a WiFi network
void setup_wifi() {
  delay(10);
  Serial.println();    
  //Serial.print("Connecting to ");  
  Serial.println(ssid);
  WiFi.mode(WIFI_STA); WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.print(".");
  Serial.println(""); Serial.println("WiFi connected"); Serial.print("IP address: "); Serial.println(WiFi.localIP());
}

// ----- Funcion que Conecta a Broker MQTT 
void setup_mqtt() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  if (!client.connected()) {
    reconnect();
  }
}


// ----- Funcion que Conecta a Broker MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived ["); Serial.print(topic); Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
  if ((char)payload[0] == '1') {
    digitalWrite(D4, HIGH);
    delay(1000);
    digitalWrite(D4, LOW);
  } 

  if ((char)payload[0] == '2') {
    digitalWrite(D2, HIGH);
    delay(1000);
    digitalWrite(D2, LOW);
  } 

  if ((char)payload[0] == '3') {
    digitalWrite(D0, HIGH);
    delay(1000);
    digitalWrite(D0, LOW);
  } 

}

// ----- Funcion que Conecta o reconecta a Broker MQTT
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      client.subscribe(topico_entrada);
      // client.subscribe(topico_salida);   //OJO Quitar el comentario para hacerlo suscribe
      client.publish(topico_salida, sTopicoOutDatos);      

      // ... and resubscribe
      //client.subscribe("inTopic");
     
    } else {
      Serial.print("failed, rc="); Serial.print(client.state()); Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void conectarMQTT() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
           
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    ++value;

 //   snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    
    // Serial.print(now); Serial.print("Publish message: "); Serial.println(sTopicoOutDatos);
    client.publish(topico_salida, sTopicoOutDatos);
    
 //REDV   client.publish(topico_salida, msg);
//    client.publish(topico_entrada, msg);
  }
}

// =================================================================
void enviarMediciones(int id){
  // Leer Datos del sensor DHT11
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.print("Falló al leer el sensor DHT\n");
    return;
  }
  
  Serial.print("Humedad: "); Serial.print(h);
  Serial.print(", Temperatura: "); Serial.print(t);
  Serial.print("(C), "); 
  Serial.print("\n");

  // Leer Datos de la fotoresistencia
  int il = analogRead(photoPin);
  Serial.print("Valor de voltaje ");
  Serial.println(il);

  // Enviar información al broker MQTT
  snprintf(sTopicoOutDatos, MSG_BUFFER_SIZE, "{\"id\":%d,\"t\":%5.2f,\"h\":%5.2f,\"il\":%d}",id, t, h, il);
  client.publish(topico_salida, sTopicoOutDatos);
}

// =================================================================
void setup() {
  pinMode(D0, OUTPUT); // LED actuador de fotorresistor
  pinMode(D2, OUTPUT); // LED actuador de Humedad
  pinMode(D4, OUTPUT); // LED actuador de Temperatura
   
   // ----- Iniciar consola: Herramientas, Monitor serie
   Serial.begin(9600);  // Iniciar consola
  
  // ----- Abrir Wifi y MQTT
  setup_wifi();
  setup_mqtt();
  
  // ----- Inicializacion sensor temperatura - humedad
  Serial.println(F("DHTxx test!"));
  dht.begin();

  // ----- Inicializacion fotorresistor
  pinMode(ledphotoResistor, OUTPUT);
  pinMode(photoPin, INPUT);
  Serial.println("Iniciando lectura fotorresistor");
}

void loop() {
  identificador = identificador + 1;
  
   
  conectarMQTT();
  enviarMediciones(identificador);
   
  Serial.println("**************");
 
}

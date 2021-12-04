// ----- Definiciones de LEDs
#define D0 16 // LED del fotorresistor (LED Azul)
#define D2 4 // LED des Sensor de temperatura (LED Amarillo)
#define D4 2 // Blinking (LED Rojo) 

#define D1 5 // Sensor de temperatura

#define D5 14 // trigPin
#define D6 12 // echoPin

// ----- Definiciones de fotorresistor
int photoPin = A0;
int ledphotoResistor = 16;  // Equivale a D0
int x = -1;  //0..1023

// ----- Definiciones del sensor de temperatura DHT11
#include "DHT.h"
#define DHTPIN 5  // Posicion equivalente a D1
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ----- Definiciones del sensor de distancia HC-SR04
int trigPin = D5;
int echoPin = D6;
float v = 331.5 + 0.6 * 20; // m/s

// ----- Definiciones de ESP8266MQTT
#include <ESP8266WiFi.h>
#include <PubSubClient.h>  // Si no existe la biblioteca
// Nota bajar .zip https://www.arduinolibraries.info/libraries/pub-sub-client
// Instalar Menu: Programa / Submenu: Incluir libreria / Anadir biblioteca PubSubClientxxx.zip
const char* ssid = "Samsung Galaxy S20 5G 7604"; // Nombre de la Red
const char* password = "ballade1"; // Contraseña
const char* mqtt_server = "broker.mqtt-dashboard.com";
char* topico_salida = "TopicoOutTempA01741757";
char* topico_salida2 = "TopicoOutPhotoResitorA01741757";
char* topico_entrada = "topEntradaA01741757"; //Clima o aire acondicionado
char sTopicoOutTemp[50];
char sTopicoPhtoResistor[50];

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
  Serial.println();    Serial.print("Connecting to ");   Serial.println(ssid);
  WiFi.mode(WIFI_STA); WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
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
  digitalWrite(D5, HIGH); delay(100); digitalWrite(D5, LOW);  // Aviso Led azul en D5 Pin 8

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(D6, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01). No es cierto en D6.
  } else {
    digitalWrite(D6, LOW);    // Turn the LED off by making the voltage HIGH
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
      client.subscribe(topico_salida);   //OJO Quitar el comentario para hacerlo suscribe
      client.publish(topico_salida, sTopicoOutTemp);

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

    Serial.print(now); Serial.print("Publish message: "); Serial.println(sTopicoOutTemp);
    client.publish(topico_salida, sTopicoOutTemp);

    //REDV   client.publish(topico_salida, msg);
    //    client.publish(topico_entrada, msg);
  }
}

// Función obtenida de: https://vimalb.github.io/IoT-ESP8266-Starter/Lesson_08/lesson.html 
// Falta por moverle a esto, aún no jala
void myMessageArrived(char* topic, byte* payload, unsigned int length) {
  // Convert the message payload from bytes to a string
  String message = "";
  for (unsigned int i=0; i< length; i++) {
    message = message + (char)payload[i];
  }
   
  // Print the message to the serial port
  Serial.println(message);
}

// =================================================================
// ----- Funcion detector de iluminacion mediante un fotorresitor
// Puerto A0, lee valores de 0 a 1023 dependiendo de la resistencia
void medirLuzledPhotoResistor() {
  v = analogRead(photoPin);
  Serial.print("Valor de voltaje ");
  Serial.println(v);
  if (v > 90) // Se modificó este valor para poder encender el LED, ahora se enciende con la luz de la habitación y se apaga cuando le pongo mi mano como sombra
    digitalWrite(ledphotoResistor, HIGH);
  else
    digitalWrite(ledphotoResistor, LOW);

  // Uso futuro: Cuando se use MQTT para formar el JSON de temperatura y humedad
  snprintf (sTopicoPhtoResistor, MSG_BUFFER_SIZE, "{\"v\":%5.2f}", v);
  client.publish(topico_salida2, sTopicoPhtoResistor);
}

// =================================================================
// ----- Funcion que lee temperatura y humedad con el sensor DHT11
void medirTemperatura() {
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.print("Falló al leer el sensor DHT\n");
    return;
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humedad: "); Serial.print(h);
  Serial.print(", Temperatura: "); Serial.print(t);
  Serial.print("(C), "); Serial.print(f);
  Serial.print("(F), Indice de calor (C)"); Serial.print(hic);
  Serial.print(", Indice de calor (F)"); Serial.print(hif);
  Serial.print("\n");

  //Threshold o nivel de umbral para prender led
  Serial.println(t);
  if (t > 26)
    digitalWrite(D2, HIGH);
  else
    digitalWrite(D2, LOW);

  // Uso futuro: Cuando se use MQTT para formar el JSON de temperatura y humedad
  snprintf(sTopicoOutTemp, MSG_BUFFER_SIZE, "{\"t\":%5.2f,\"h\":%5.2f}", t, h);
  client.publish(topico_salida, sTopicoOutTemp);
}

// =================================================================
// ----- Funcion que lee el sensor ultrasonico

void medirDistanciaCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin, LOW);

  //listen for echo
  float tUs = pulseIn(echoPin, HIGH);  //microseconds

  float t = tUs / 1000.0 / 1000.0 / 2.0;    //s
  float d = t * v; //m
  float dCm = d * 100; // cm
  Serial.print("Distancia(cm): ");
  Serial.println(dCm);
  delay(200);  //ms
}


void setup() {
  // put your setup code here, to run once:
  pinMode(D0, OUTPUT); // LED del fotorresistor
  pinMode(D2, OUTPUT); // LED del sensor de temperatura
  pinMode(D4, OUTPUT); // LED del blinking

  Serial.begin(9600);  // Iniciar consola

  // ----- Abrir Wifi y MQTT
  setup_wifi();
  setup_mqtt();

  // ----- Inicializacion sensor temperatura - humedad
  Serial.println(F("DHTxx test!"));
  Serial.println("DHTxx test!");
  dht.begin();

  // ----- Inicializacion fotorresistor
  pinMode(ledphotoResistor, OUTPUT);
  pinMode(photoPin, INPUT);
  Serial.println("Iniciando lectura fotorresistor");

}
void loop() {
  //Ejercicio 2
  digitalWrite(D4, HIGH);
  delay(500);
  digitalWrite(D4, LOW);
  delay(500);

  medirLuzledPhotoResistor();
  medirTemperatura();
  //medirDistanciaCm();
  Serial.println("******************************************");
}

// ----- Definiciones de LEDs
#define D0 16 // LED del fotorresistor (LED Azul)
#define D2 4 // LED des Sensor de temperatura (LED Amarillo)
#define D4 2 // Blinking (LED Rojo) 

#define D1 5 // Sensor de temperatura

#define D5 5 // trigPin
#define D6 5 // echoPin

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
float v = 331.5+0.6*20;   // m/


// =================================================================
// ----- Funcion detector de iluminacion mediante un fotorresitor
// Puerto A0, lee valores de 0 a 1023 dependiendo de la resistencia
void medirLuzledPhotoResistor() {
  x = analogRead(photoPin);
  Serial.print("Valor de voltaje ");
  Serial.println(x);
  if (x > 90) // Se modificó este valor para poder encender el LED, ahora se enciende con la luz de la habitación y se apaga cuando le pongo mi mano como sombra
    digitalWrite(ledphotoResistor, HIGH);
  else
    digitalWrite(ledphotoResistor, LOW);
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
     digitalWrite(D2,HIGH);
  else 
     digitalWrite(D2,LOW);
     
  // Uso futuro: Cuando se use MQTT para formar el JSON de temperatura y humedad
  // snprintf (sTopicoOutTemp, MSG_BUFFER_SIZE, "{\"t\":%5.2f,\"h\":%5.2f}",t,h);
}

// =================================================================
// ----- Funcion que lee el sensor ultrasonico

void medirDistanciaCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin,LOW);

  //listen for echo
  float tUs = pulseIn(echoPin, HIGH);  //microseconds

  float t = tUs / 1000.0 / 1000.0 / 2.0;    //s
  float d = t*v;  //m
  float dCm = d*100;  // cm  
  Serial.print("Distancia(cm): ");
  Serial.println(dCm);
  delay(200);  //ms
}


void setup() {
  // put your setup code here, to run once:
   pinMode(D0,OUTPUT); // LED del fotorresistor
   pinMode(D2,OUTPUT); // LED del sensor de temperatura
   pinMode(D4,OUTPUT); // LED del blinking

   Serial.begin(9600);  // Iniciar consola

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
  digitalWrite(D4,HIGH);
   delay(500);
   digitalWrite(D4,LOW);
   delay(500);

   medirLuzledPhotoResistor(); 
   medirTemperatura();
   medirDistanciaCm();
}

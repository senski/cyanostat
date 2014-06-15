/*
 * Algal CyanoStat
 * Stephanie Conley
 * June 14, 2014
 */

#include <Ethernet.h>
#include <SPI.h>

#define numTubes 6

struct sensors {
  int lightPin;
  int valvePin;
};

struct dataPoint {
  int data;
  unsigned long drain;
};

// Google Drive connection variables
char formKey[] = "dDc1N1liaGhNYkxxSDhVZ04takJjLWc6MQ";
char entryKey0[] = "0";
char entryKey1[] = "9";
byte mac[] = {
  0x90,0xA2,0xDA,0x0D,0x52,0x49};
byte server[] = {
  74,125,239,9};
EthernetClient client;

// Automation variables
struct sensors tube[numTubes] = {
  {
    9, 31    }
};
struct dataPoint dataPoint[numTubes] = {
  {
    0, 0    }
};
unsigned long checkDelay = 3600000UL;  // check interval
unsigned long evapDelay = 14400000UL;   // evaporation compensation interval
unsigned long nightDelay = 86400000UL; // Delay 12 hours for every 24 hours
unsigned long offset = 43200000UL;
unsigned long timer0 = 0UL;
unsigned long timer1 = 0UL;
unsigned long timer2 = 0UL;
unsigned long flushTime = 8000UL;      // how long to flush
unsigned long settleTime = 0UL;     // let the bubbles settle
unsigned long evapTime = 4000UL;       // how long to flush for evaporation compensation
unsigned long nightTime = 43200000UL;  // 12 hour night timer
int airPin = 41;                       // air pump
int bulbPin = 43;
int flushThreshold = 400;              // flushes when lightLevel is <
int numReadings = 100;                 // number of photoresistor readings to average

void setup()
{
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); // Disable SD card
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW); // Enable w5100 ethernet ic
  Ethernet.begin(mac);
  delay(1000);
  Serial.println("connecting...");
  for (int i = 0; i < numTubes; i++) {
    pinMode(tube[i].valvePin, OUTPUT); 
  }
  pinMode(airPin, OUTPUT); // sets the air pump pin to output
  pinMode(bulbPin, OUTPUT);
  digitalWrite(airPin, HIGH); // start the air pump
  digitalWrite(bulbPin, HIGH);
  Serial.println("Finished setup");
}

void loop(){
  if (millis() - timer0 > checkDelay) {
    timer0 = millis();
    digitalWrite(airPin, LOW);
    delay(settleTime);
    for (int i = 0; i < numTubes; i++) {
      int lightLevel = readLightLevel(tube[i].lightPin);
      dataPoint[i].data = lightLevel;

      for (int j = 0; lightLevel < flushThreshold && j < 2; j++) {
        digitalWrite(tube[i].valvePin, HIGH);
        delay(flushTime);
        digitalWrite(tube[i].valvePin, LOW);
        lightLevel = readLightLevel(tube[i].lightPin);
        dataPoint[i].data = lightLevel;
        dataPoint[i].drain += flushTime;
      }
    }
    digitalWrite(airPin, HIGH);
    sendData(dataPoint);

    for (int i = 0; i < numTubes; i++)
      dataPoint[i].drain = 0L;
  }

  // compensate for evaporation
  if (millis() - timer1 > evapDelay) {
    timer1 = millis();
    digitalWrite(airPin, LOW);
    delay(settleTime);
    for (int i = 0; i < numTubes; i++) {
      digitalWrite(tube[i].valvePin, HIGH);
      delay(evapTime);
      digitalWrite(tube[i].valvePin, LOW);
      dataPoint[i].drain += evapTime;
    }
    digitalWrite(airPin, HIGH);
  }

  // Night cycle
  if ((millis() - timer2 + offset) > nightDelay) {
    timer2 = millis();
    offset = 0;
    digitalWrite(airPin, LOW);
    digitalWrite(bulbPin, LOW);
    delay(nightTime);
    digitalWrite(bulbPin, HIGH);
    digitalWrite(airPin, HIGH);
    delay(0);
  }

  // fix unsigned long overflow
  if (timer0 > millis())
    timer0 = millis();
  if (timer1 > millis())
    timer1 = millis();
  if (timer2 > millis())
    timer2 = millis();
}

int readLightLevel(int lightPin)
{
  long lightLevelSum = 0L;
  for (int i = 0; i < numReadings; i++)
  {
    lightLevelSum += analogRead(lightPin);
    delay(5); // 5ms delay between intermediate readings
  }
  int lightLevel = lightLevelSum / numReadings;
  return lightLevel;
}

void sendData(struct dataPoint *data) {
  Ethernet.maintain();
  delay(1000);
  String dataString;
  for (int i = 0; i < numTubes; i++) {
    if (i > 0)
      dataString += "&";
    dataString += "entry.";
    dataString += entryKey0;
    dataString += ".single=";
    dataString += data[i].data;
    dataString += "&entry.";
    dataString += entryKey1;
    dataString += ".single=";
    dataString += data[i].drain;
  }
  dataString += "&submit=Submit";

  if(client.connect(server, 80)) {
    Serial.println("connected");

    client.print("POST /spreadsheet/formResponse?formkey=");
    client.print(formKey);
    client.println("&ifq HTTP/1.1");
    client.println("Host: docs.google.com");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(dataString.length());
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();
    client.println(dataString);

    Serial.print("POST /spreadsheet/formResponse?formkey=");
    Serial.print(formKey);
    Serial.println("&ifq HTTP/1.1");
    Serial.println("Host: docs.google.com");
    Serial.println("Connection: close");
    Serial.print("Content-Length: ");
    Serial.println(dataString.length());
    Serial.println("Content-Type: application/x-www-form-urlencoded");
    Serial.println();
    Serial.println(dataString);

    Serial.println("disconnecting");
    delay(1000);
    client.stop();
    client.flush();
  }
  else{
    Serial.println("failed to connect");
  }
}

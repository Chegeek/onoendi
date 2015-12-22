// ONOENDI GPS Data sending
// Copyright (c) 2015 Yugan Studio Works
// Luqman Sahidin <lsahidin@yahoo.com>
// Release to Public Domain, Enjoy!

#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// GPS Class and Serial
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 4800;
static const uint32_t SerialBaud = 115200;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

// depends on you, debuging or not (0 is not)
#define DEBUG 1

// Device Preparation
String DeviceId = "GP2136EF";
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //physical mac address

////////////////////////////////////////////////
// seting untuk IP static
byte ip[] = { 192, 168, 1, 15 }; // ip in lan
byte gateway[] = { 192, 168, 1, 1 }; // internet access via router
byte dns_ip[] = {192, 168, 1, 1};
byte subnet[] = { 255, 255, 255, 0 }; //subnet mask

EthernetClient client;
String device_location,contentlength;
static void smartdelay(unsigned long ms);
unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 5L * 1000L; // delay between updates, in milliseconds
static void print_float(float val, float invalid, int len, int prec);

//////////////////////

void data_toSend(){

  //double GPSspeed = gps.speed.kmph();
  TinyGPSTime &t = gps.time;
  char sz[32];
  sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());

  //if (GPSspeed < 1)
    //GPSspeed = 0;

  if (gps.location.isValid()){
    Serial.println(F("Collecting data.."));
    device_location = "deviceid=" + DeviceId + "&latitude=" + String(gps.location.lat(), 6) + "&longitude=" + String(gps.location.lng(), 6) + "&gpstime=" + sz;
    contentlength = "Content-Length: " + String(device_location.length());
    //device_location = "deviceid=GP2136EF";
    //contentlength = "Content-Length: 17";
    Serial.println(device_location);
    Serial.println(F("ready to send."));

    client.stop();
    // if you get a connection, report back via serial:
    if (client.connect("www.kreatifitas.com", 80)) {
      if (DEBUG){
        Serial.println("contacting and sending to server..");
        Serial.println();
      }
      // Make a HTTP request:
      client.println("POST /onoendi/locking HTTP/1.1");
      client.println("Host: kreatifitas.com");
      client.println("User-Agent: GeoBrowser");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.println(contentlength);
      client.println();
      client.println(device_location);
      client.println("Connection: close");
      client.println();
  
      // note the time that the connection was made:
      lastConnectionTime = millis();
    } else {
      // If you didn't get a connection to the server:
      if (DEBUG){
        Serial.println(F("connection failed!"));
        Serial.println();
      }
      // do restart after 2 seconds
      delay(2000);
      setup();
    }
  } else {
    Serial.println(F("WAITING FOR GPS DATA.."));
  }
}

void setup(){
  if (DEBUG){
    Serial.begin(SerialBaud);
    Serial.println("ONOENDI " + DeviceId);
    Serial.println(F("starting up.."));
  }
  ss.begin(GPSBaud);

  // start the Ethernet connection:
  Ethernet.begin(mac, ip, dns_ip, gateway, subnet);
  // print your local IP address:
  if (DEBUG){
    Serial.print(F("IP: "));
    String titik = "";
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
      // print the value of each byte of the IP address:
      Serial.print(titik);
      Serial.print(Ethernet.localIP()[thisByte], DEC);
      titik = ".";
    }
    Serial.println(" waiting for GPS Data..");
  }
}

void loop(){ 
  if (client.available()) {
    char c = client.read();
    if (DEBUG)
      Serial.print(c);
  }

  // if five seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    smartDelay(1000); // delay for gps availability
    if (millis() > 5000 && gps.charsProcessed() < 10)
      Serial.println(F("No GPS data received: check wiring"));
    data_toSend();
  }
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

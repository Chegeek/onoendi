// ONONENDI GPS TRACKER
// OCT 15 (c) Yugan Studio Works

// Latest update on 13 March 16 13:37 GMT+7
// ~ Bug Fixed modem_get_imei()
// ~ Manage data length
// ~ Change the way command handler

// Update on 13 Jan 16 06:54 GMT+7
// ~ ON/OFF and GPS_SPEED_TO_DETECT definition
// ~ data mode function


#include "config.h"
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

/* GLOBAL VARIABLES */
uint8_t x = 0;            // current data index
byte answer = 0;          // get answer command
byte cmd = 0;             // notify have command
uint8_t eeAddress;        // Memory location we want the data to be put.
uint8_t data_index = 0;   // current data index
uint8_t interval = 0;     // interval count in loop()
uint8_t interval_sms = 0; // interval checking sms

bool gpsfix = false;      // fix gps indication
bool constate = false, reload = false;
char iso_date_time[20];   // time attached to every data line
char modem_reply[150];    // data received from modem, max 150 chars
char data_current[60];    // data bucket collected from gps
char msg[110];            // buffer command message
bool power_reboot = false;
bool save_config = false;
byte sending = 0;

char lat_current[12];
char lon_current[12];

float flat, flon;
unsigned long age;

unsigned long last_time_gps, last_date_gps;

TinyGPS t_gps;
SoftwareSerial gps(3,4);
SoftwareSerial modem(6,7);

static void gpsdelay(unsigned long ms);

#ifdef DEBUG
  #define debug_print(x)  Serial.print(x)
  #define debug_println(x)  Serial.println(x)
#else
  #define debug_print(x)
  #define debug_println(x)
#endif

struct settings {
  char imei[16];
  char key[12];
  char smspass[10];
  char apn[15];
  byte interval;
  uint8_t data_log; // data mode is active or not
  uint8_t power_ignition;
  bool loginPacket = false;  // Try to send login packet to server, need to save when device restart
};

settings config;

void setup(){
  pinMode(RELAYPIN, OUTPUT);
  
  #ifdef DEBUG
    Serial.begin(115200);
  #endif

  modem.begin(9600);
  gps.begin(4800);

  debug_println(F("ONONENDI GPS STARTING ...\r\n"));

  // if there any exist configuration on eeprom?
  loadeeprom();
  
  // check power state? 1 is relay off means power on and 0 is relay on.
  if (config.power_ignition == ON){
    digitalWrite(RELAYPIN, LOW);
  } else {
    digitalWrite(RELAYPIN, HIGH);
  }
  
  // Modem initialization
  modem.listen();
  delay(500);
  modem_startup();  // starting modem availability

  // Set with default config if not exits configuration
  if (load_default_config() == 1){
      save2eeprom();  // save config
  }
  
  debug_print(F("\r\n"));  
}

void loop() {
  save_config = false;

  if (interval > config.interval){   // counter for interval to collecting gps data
    
    gpsdelay(2000);
    collect_gps_data();

    debug_println(data_current);

    if (config.loginPacket) { // send login packet to the server when get command
      gpsfix = true;
      config.loginPacket = false;
      save_config = true;
    }

      modem.listen(); // set port to listen modem
      delay(500);
      // gsm ready or not
      if (modem_check_at("AT+CREG?","+CREG: 0,1",200) == 0){ // if not
        pinMode(MODEMRST, OUTPUT);
        reboot_mcu();
      }

    if (config.data_log == ON){
      // get fix
      if (gpsfix) {
          modem_establish_connection();
          modem_send_data();
        }
    }
      
    LedStat();
    interval = 0;
  }
  
  if (interval_sms >= SMS_INTERVAL){   // check sms every SMS_INTERVAL seconds
    modem.listen();
    delay(500);
    sms_check();
    interval_sms = 0;
  }

  if (save_config) save2eeprom();
  if (power_reboot) reboot_mcu();
  
  interval++;
  interval_sms++;
  delay(200); // breath
  
}

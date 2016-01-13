void reboot_mcu(){
  // also reset modem
  pinMode(MODEMRST, OUTPUT);
  digitalWrite(MODEMRST, LOW);
  delay(30);
  // and gps
//  pinMode(GPSRST, OUTPUT);
//  digitalWrite(GPSRST, LOW);
//  delay(30);
  // now reset the MCU
//  asm volatile ("  jmp 0"); // this function is not affective reset
  pinMode(MCURST, OUTPUT);
  digitalWrite(MCURST, LOW);
}

void LedStat(){
  digitalWrite(LEDPIN,HIGH);     // let's give some LED starting indication
  delay(100);
  digitalWrite(LEDPIN, LOW);
  delay(100);
  digitalWrite(LEDPIN,HIGH);     // twice to make it like sophisticated device :D
  delay(100);
  digitalWrite(LEDPIN, LOW);
  delay(100);
}

void generate_key(){
  debug_println(F("generate_key() started"));
  
  String key(config.imei);
  char buf[8];
  
  key.remove(0,10);
  key = "ys" + key;
  key.toCharArray(buf, 8);

  debug_print(F("Geokey: "));
  debug_println(buf);
  
  fill_preconfig(config.key, buf);

  debug_println(F("generate_key() completed"));
}

void save2eeprom(){
  debug_println(F("Clean memory first . ."));
  for ( int i = 0 ; i < EEPROM.length() ; i++ )
    EEPROM.write(i, 0);
    
  debug_println(F("save2eeprom() started"));
  eeAddress = 0; //go to zero
  float mem_id = 'yuganstudio';  // Variable to store in EEPROM.
  // One simple call, with the address first and the object second.
  EEPROM.put(eeAddress, mem_id);
  eeAddress += sizeof(float); // Move address to the next byte
  
  EEPROM.put(eeAddress, config);
  debug_println(F("save2eeprom() completed"));
}

void loadeeprom(){
  debug_println(F("loadeeprom() started"));
  eeAddress = 0; //go to zero
  eeAddress = sizeof(float); //Move address to the next byte after float.
  EEPROM.get(eeAddress, config);

  debug_println(F("=== Existing Config ==="));
  debug_println(config.imei);
  debug_println(config.key);
  debug_println(config.smspass);
  debug_println(config.interval);
  debug_println(config.apn);
  debug_println(config.power_ignition);
  debug_println(config.loginPacket);
  
  debug_println(F("loadeeprom() completed"));
}

void fill_preconfig(char * param, char * pvalue){
  int i;
  for(i=0; i<strlen(pvalue); i++) {
    param[i] = pvalue[i];
  }
}

uint8_t load_default_config(){
  uint8_t need_save = 0;
  
  debug_println(F("load_default_config() started"));
  
  // config.imei
  if (strlen(config.imei) == 0){
    modem_get_imei(); // save result to config.imei
    need_save = 1;
  }    
    
  // config.key
  if (strlen(config.key) == 0){
    generate_key();   // generate key for geolink from last 5 digits IMEI
    need_save = 1;
  }
    
  // config.apn
  if (strlen(config.apn) == 0){
    fill_preconfig(config.apn, DEFAULT_APN);
    need_save = 1;
  }
    
  // config.smspass
  if (strlen(config.smspass) == 0){
    fill_preconfig(config.smspass, DEFAULT_SMS_KEY);
    need_save = 1;
  }
    
  // config.interval
  if (config.interval == 0){
    config.interval = DEFAULT_INTERVAL;
    need_save = 1;
  }

  // config.power_ignition
  if (config.power_ignition == 0){
    config.power_ignition = DEFAULT_POWERIGNITION;
    need_save = 1;
  }  

  return need_save;
    
  debug_println(F("load_default_config() completed"));
}

static void gpsdelay(unsigned long ms){
  gps.listen();     // set port to listen on gps
  delay(500);
  
  unsigned long start = millis();
  do 
  {
    while (gps.available())
      t_gps.encode(gps.read());
  } while (millis() - start < ms);
}


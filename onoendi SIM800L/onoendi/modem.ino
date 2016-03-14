void modem_startup(){
  debug_println(F("modem_startup() started"));
  // MCU reset, pin mode reset to default :D
  digitalWrite(LEDPIN, HIGH);
  // checks if the module is started
  if (modem_check_at("AT", "OK", 1000) == 0){
    // waits for an answer from the module  
    // Send AT every two seconds and wait for the answer
    while (modem_check_at("AT", "OK", 2000) == 0);
  }
  
  while (modem_check_at("AT+CREG?","+CREG: 0,1",1000) == 0);
  digitalWrite(LEDPIN, LOW);     // let's give some LED starting indication
  delay(500);
  digitalWrite(LEDPIN, HIGH);
  delay(500);
  digitalWrite(LEDPIN, LOW);

  //set SMS as text format
  modem_check_at("AT+CMGF=1", "OK", 1000);
  
  debug_println(F("modem_startup() completed"));
}

void modem_get_imei() {
  debug_println(F("modem_get_imei() started"));
  
  unsigned long previous;
  int i = 0;
  if (modem_check_at("AT+CGSN","OK",1000) == 1){
    char *tmp = strtok(modem_reply, "AT+CGSN\r\n");
    for(i=0; i<strlen(tmp); i++) {
      config.imei[i] = tmp[i];
      if(i > 15) { //imei can not exceed 15 chars
        break;
      }
    }
  }
  config.imei[i+1] = '\0';
//  debug_println(config.imei);
  debug_println(F("modem_get_imei() completed"));
}

uint8_t set_apn() {
  debug_println(F("set_apn() started"));

  //set APN
  modem_check_at("AT+SAPBR=3,1,\"Contype""\",\"GPRS\"", "OK", 1000);
  debug_print(F("Set APN: "));
  debug_println(config.apn);
  modem.print("AT+SAPBR=3,1,\"APN""\",\"");
  modem.print(config.apn);
  if (modem_check_at("\"", "OK", 1000) == 1){
    debug_println(F("Set APN success"));
    return 1;
  } else {
    return 0;
  }
  debug_println(F("set_apn() completed"));
}

void modem_establish_connection(){
    debug_println(F("modem_establish_connection() started"));
    constate = false;

  if (set_apn()==1){
    // cek bearer status
    modem_check_at("AT+SAPBR=0,1","OK",2000);
    
      // Bearer configure
      if (modem_check_at("AT+SAPBR=1,1","OK", 3000) == 1){
        debug_println(F("Connected to remote server"));
        
        constate = true;
      } else {
        debug_println(F("Cannot connect to remote server"));
        
        constate = false;
        reboot_mcu();
      }
  }
    
    debug_println(F("modem_establish_connection() completed"));
    
    if (reload) modem_send_data();
}

void modem_send_data(){
  debug_println(F("modem_send_data() started"));
  reload = false;
  
  //opening connection
  if (constate){
    delay(3000); // waiting connection establish
    if (modem_check_at("AT+HTTPINIT","OK",2000) == 1){
      if (modem_check_at("AT+HTTPPARA=\"CID""\",1","OK",1000) == 1){

          modem.print("AT+HTTPPARA=\"URL""\",\"");
          modem.print(URL);
          modem.print("imei=");
          modem.print(config.imei);
          modem.print("&key=");
          modem.print(config.key);
          modem.print("&d=");
          modem.print(iso_date_time);
          modem.print("[");
          modem.print(data_current);
          modem.print("]{}");
//          modem.println("\"");
//          delay(500); // processing AT


          if (modem_check_at("\"","OK",2000) == 1){
            if (modem_check_at("AT+HTTPACTION=0","+HTTPACTION: ",10000) == 1){
              if (modem_check_at("AT+HTTPREAD","OK",2000) == 1){
                    delay(2000);
                    modem_check_at("AT+HTTPTERM","OK",2000);
              }
            }

            modem_check_at("AT+SAPBR=0,1","OK",1000); // disconnect gprs connection
          }
      }
    }
      else{
      reload = true;
      reboot_mcu();
    }
  }
    else{
    reload = true;
    modem_establish_connection();
  }
  
  debug_println(F("modem_send_data() completed"));
}

int8_t modem_check_at(const char* ATcommand, const char* expected_answer, unsigned int timeout){
  unsigned long previous;
  
  x = 0;
  answer = 0;

  memset(modem_reply, '\0', 150);    // Clear modem_reply

  delay(100);

  while( modem.available() > 0) modem.read();    // Clean the input buffer

  modem.println(ATcommand);    // Send the AT command 

//  debug_println(ATcommand);
  x = 0;
  previous = millis();

  // this loop waits for the answer
  do{
      if(modem.available() != 0){    
          modem_reply[x] = modem.read();
          x++;
          // check if the desired answer is in the response of the module
          if (strstr(modem_reply, expected_answer) != NULL){
              answer = 1;
              break;
          }
          modem_reply[x] = '\0'; // Null terminate the string
      }
      // Waits for the asnwer with time out
  }
  while((answer == 0) && ((millis() - previous) < timeout));    
  debug_println(modem_reply);
 
  return answer;
}

void modem_wait_at() {
  unsigned long timeout = millis();

  memset(modem_reply, '\0', 150);    // Initialize the string

  while (!strncmp(modem_reply,"AT+",3) == 0) {
    if((millis() - timeout) >= 2000) {
      debug_println(F("Warning: timed out waiting for last modem reply"));
      break;
    }
    modem_get_reply(0);

    delay(50);
  }
}

void modem_get_reply(int fullBuffer) {
  //get reply from the modem
  byte index = 0;
  char inChar=-1; // Where to store the character read

  while(modem.available()) {
    if(index < 100) { // One less than the size of the array
      inChar = modem.read(); // Read a character
      modem_reply[index] = inChar; // Store it
      index++; // Increment where to write next

      if(index == 100 || (!fullBuffer && inChar == '\n')) { //some data still available, keep it in serial buffer
        break;
      }
    }
  }

  modem_reply[index] = '\0'; // Null terminate the string

  if(strlen(modem_reply) >0) {
    debug_print(F("Modem Reply: "));
    debug_println(modem_reply);
  }
}


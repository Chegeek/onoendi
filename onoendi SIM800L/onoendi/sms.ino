//check SMS
void sms_check() {
  char index;
  char phonecpy[15];  // phone buffer
  byte cmd = 0;
  int reply_index = 0;
  char *tmp, *phone;

  sending = 0;

  modem_check_at("AT+SAPBR=0,1","OK",2000);  // to make sure not in data mode.

  debug_println(F("sms_check() started"));

  modem.println("AT+CMGL=\"REC UNREAD\"");

  modem_wait_at();

  for(int i=0;i<30;i++) {
    if(modem.available()) {
      while(modem.available()) {
        index = modem.read();

//        debug_print(index);

        if(index == '#') {
          //next data is probably command till \r
          //all data before "," is sms password, the rest is command
          debug_println(F("SMS command found"));
          cmd = 1;
          modem_reply[reply_index] = '\0';

          //Getting phone number. phone info will look like this: +CMGL: 10,"REC READ","+436601601234","","5 12:13:17+04"
          //phone will start from ","+  and end with ",
          tmp = strstr(modem_reply, "+CMGL:");
          if(tmp!=NULL) {
            tmp = strstr(modem_reply, "\",\"+");
            tmp += strlen("\",\"+");
            phone = strtok(tmp, "\",\"");
            strcpy(phonecpy, phone);  // copy phone -> phonecpy, modem_reply will reset while process ussd

            debug_print(F("Phone:"));
            debug_println(phonecpy);
            
            reply_index = 0;
          }
        } else {
            modem_reply[reply_index] = index;
            reply_index++;
        }
          
        if (cmd == 1){
          if (index == '\r' || index == '\n'){
            modem_reply[reply_index] = '\0';
            debug_print(F("command received:"));
            debug_println(modem_reply);
            sms_auth_cmd(modem_reply);
            cmd = 0;
            reply_index = 0;
          }
        }
      } //--- end while ---
    } // --- end if modem ---
    delay(10);
  } //--- end for ---

//  combine all message to be sent in 1 sms
  if (sending == 1) {
    sms_send_msg(msg, phonecpy);
  }
  memset(msg, '\0', msg_char);
  debug_println(F("Deleting READ SMS"));

  modem_check_at("AT+CMGDA=\"DEL READ\"", "OK", 1000); //remove all READ SMS

  debug_println(F("sms_check() completed"));
}

void sms_auth_cmd(char *cmd) {
  char *tmp;
  int i=0;

  debug_println(F("sms_cmd() started"));

  //command separated by "," format: password,command=value
  while ((tmp = strtok_r(cmd, ",", &cmd)) != NULL) {
    if(i == 0) {
      //checking password
      if(strcmp(tmp, config.smspass) == 0) {
        debug_println(F("sms_cmd(): SMS password accepted, executing command"));
        sms_cmd_run(cmd);
        break;
      } else {
        debug_print(F("sms_cmd(): SMS password failed, ignoring command. Failed password: "));
        debug_println(tmp);  // to trace your password sent from sms
      }
    }
    i++;
  }

  debug_println(F("sms_cmd() completed"));
}

void sms_cmd_run(char *cmd) {
  char *tmp;
  char *cusdd, *cusddval;
  long val;
  uint8_t xy;
  uint8_t k=0, z=0;

  debug_println(F("sms_cmd_run() started"));

  //checking what command to execute
  
  //Send Login Packet
  tmp = strstr(cmd, "login");
  if(tmp != NULL) {
    debug_println(F("sms_cmd_run(): Login command detected"));
    config.loginPacket = true;
    save_config = true;
    
    snprintf(msg,msg_char,"%s\r\nLogin request sent",msg);
    sending = 1;
  }
  
  //set APN
  tmp = strstr(cmd, "apn=");
  if(tmp != NULL) {
    debug_println(F("sms_cmd_run(): Set APN command detected"));

    //setting new APN
    tmp += strlen("apn=");

    debug_print(F("sms_cmd_run(): Last APN: "));
    debug_println(config.apn);

    //updating APN in config
    for(k=0;k<strlen(tmp);k++) {
      config.apn[k] = tmp[k];
    }

    config.apn[k] = '\0';  //null terminate APN

    debug_print(F("New APN configured: "));
    debug_println(config.apn);

    snprintf(msg,msg_char,"%s\r\nNew APN saved",msg);
    sending = 1;

    save_config = true;
    power_reboot = true;
  }

  //SMS pass
  tmp = strstr(cmd, "smspass=");
  if(tmp != NULL) {
    debug_println(F("sms_cmd_run(): Set smspass command detected"));

    //setting new APN
    tmp += strlen("smspass=");

    debug_print(F("sms_cmd_run(): New smspass: "));
    debug_println(tmp);

    //updating sms password
    for(k=0;k<strlen(tmp);k++) {
      config.smspass[k] = tmp[k];
    }

    config.smspass[k] = '\0';  //null terminate APN

    debug_print(F("New smspass configured: "));
    debug_println(config.smspass);
    
    snprintf(msg,msg_char,"%s\r\nNew SMS password saved",msg);
    sending = 1;

    save_config = true;
  }

  // Set Data Mode
  tmp = strstr(cmd, "data=");
  if(tmp != NULL) {
    debug_println(F("sms_cmd_run(): Set Data mode detected"));
    
    tmp += strlen("data=");
    debug_print(F("sms_cmd_run(): Set data mode to: "));
    debug_println(tmp);
//    if(tmp == "off") {  //validation is hard to compare value using equals (==) dont know why?
    if (strstr(tmp, "off") != NULL){
      config.data_log = OFF;
    } else {
      config.data_log = ON;
    }

    if (config.data_log == ON){
      snprintf(msg,msg_char,"%s\r\nData Mode ON",msg);
    } else {
      snprintf(msg,msg_char,"%s\r\nData Mode OFF",msg);
    }
    sending = 1;
    
    save_config = true;
  }

  // Set Power Cut Relay
  tmp = strstr(cmd, "power=");
  if(tmp != NULL) {
    debug_println(F("sms_cmd_run(): Set power command detected"));
    
    tmp += strlen("power=");
    debug_print(F("sms_cmd_run(): Set power to: "));
    debug_println(tmp);
    if (strstr(tmp, "off") != NULL){
      config.power_ignition = OFF;
    } else {
      config.power_ignition = ON;
    }

    if (config.power_ignition == ON){
      snprintf(msg,msg_char,"%s\r\nPower ignition ON",msg);
    } else {
      snprintf(msg,msg_char,"%s\r\nPower ignition OFF",msg);
    }
    sending = 1;
      
    save_config = true;
    power_reboot = true;
  }
  
  // Set Interval
  tmp = strstr(cmd, "int=");
  if(tmp != NULL) {
    debug_println(F("sms_cmd_run(): Set interval command detected"));

    //setting new Interval
    tmp += strlen("int=");
    val = atol(tmp);

    if(val > 0) {
      //updating interval in config
      config.interval = val;

      debug_print(F("New interval configured: "));
      debug_println(config.interval);

      snprintf(msg,msg_char,"%s\rInterval changed",msg);
      sending = 1;

      save_config = true;
      power_reboot = true;
    } else debug_println(F("sms_cmd_run(): invalid value"));
  }

  ////////////////////////////// end of group for multiple command /////////////////////////////

// Get request position
  tmp = strstr(cmd, "pos");
  if(tmp != NULL) {
    
    debug_println(F("sms_cmd_run(): Position request detected"));

    snprintf(msg,msg_char,"https://maps.google.com/maps/place/%s,%s",lat_current,lon_current);
    sending = 1;

  }

// Get IMEI
  tmp = strstr(cmd, "imei");
  if(tmp != NULL) {
    
    debug_println(F("sms_cmd_run(): IMEI request detected"));

    snprintf(msg,msg_char,"Your IMEI number is %s",config.imei);
    sending = 1;

  }

// Get Key
  tmp = strstr(cmd, "key");
  if(tmp != NULL) {
        
    debug_println(F("sms_cmd_run(): Key request detected"));

    snprintf(msg,msg_char,"Your access key is %s",config.key);
    sending = 1;

  }

  /* to add more commands add above this line */
  /* avoid to create filter command below "Send USSD Code" */
  /* because of "modem_reply" will be clear on CUSD */
  
// Send USSD Code
  tmp = strstr(cmd, "ussd=");  
  if(tmp != NULL) {
    debug_println(F("sms_cmd_run(): Request for USSD Code detected"));
    int xy=0;
    
    tmp += strlen("ussd=");
    if(tmp){
      modem.print("AT+CUSD=1,\"*");
      val = atol(tmp);
      modem.print(val);

      if (modem_check_at("#\",15","OK",2000) == 1){
        if (modem_check_at("AT","\", ",5000) == 1){ // ", 0 end of CUSD data received. send AT to avoid last command by clear modem_reply
          cusdd = strstr(modem_reply, "\"");
          cusddval = strtok(cusdd, "\"");
                    
          debug_println(F("CUSD String:"));
          debug_println(cusddval);
       }
      } else {
        debug_println(F("Error! check result on command"));
      }
    }

      snprintf(msg,msg_char,cusddval);
      sending = 1;
      delay(100);
      modem_check_at("AT+CUSD=2","OK",2000); // cancel hooked USSD session

 }

  debug_println(F("sms_cmd_run() completed"));
}

void sms_send_msg(const char* txt, char* phone){
  debug_println(F("sms_send_msg() started"));
  
  modem.print("AT+CMGS=\"");
  modem.print("+");
  modem.print(phone);
  if (modem_check_at("\"", ">", 2000) == 1){
    debug_println(txt);
    modem.print(txt);
    if (modem_check_at("\x1A", "OK", 5000) == 1){ //ctrl+z
      debug_println(F("text message sent!"));
      delay(1000);
      modem_check_at("AT+CMGDA=\"DEL SENT\"", "OK", 1000);
    } else {
      debug_println(F("Error! check result on command"));
    }
  }
  
  debug_println(F("sms_send_msg() completed"));
}

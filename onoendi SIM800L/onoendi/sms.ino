//check SMS
void sms_check() {
  char index;
  byte cmd;
  int reply_index = 0;
  char *tmp, *tmpcmd;

  modem_check_at("AT+SAPBR=0,1","OK",2000);  // to make sure not in data mode.

  debug_println(F("sms_check() started"));

  modem.println("AT+CMGL=\"REC UNREAD\"");

  modem_wait_at();

  for(int i=0;i<30;i++) {
    if(modem.available()) {
      while(modem.available()) {
        index = modem.read();

//          Serial.print(index);

        if(index == '#') {
          //next data is probably command till \r
          //all data before "," is sms password, the rest is command
          debug_println(F("SMS command found"));
          cmd = 1;

          //get phone number
          modem_reply[reply_index] = '\0';

          //phone info will look like this: +CMGL: 10,"REC READ","+436601601234","","5 12:13:17+04"
          //phone will start from ","+  and end with ",
          tmp = strstr(modem_reply, "+CMGL:");
          if(tmp!=NULL) {
//            debug_println(F("Getting phone number"));

            tmp = strstr(modem_reply, "\",\"+");
            tmp += strlen("\",\"+");
            tmpcmd = strtok(tmp, "\",\"");

            debug_print(F("Phone:"));
            debug_println(tmpcmd);

          }

          reply_index = 0;
        } else if(index == '\r') {
          if(cmd == 1) {
            debug_println(F("SMS command received"));

            modem_reply[reply_index] = '\0';

            debug_print(F("New line received after command: "));
            debug_println(modem_reply);

            sms_cmd(modem_reply,tmpcmd);
            reply_index = 0;
            cmd = 0;
          }
        } else {
          if(cmd == 1) {
            modem_reply[reply_index] = index;
            reply_index++;
          } else {
            if(reply_index < 200) {
              modem_reply[reply_index] = index;
              reply_index++;
            } else {
              reply_index = 0;
            }
          }
        }
      }
    }

    delay(10);
  }

  debug_println(F("Deleting READ SMS"));

  //remove all READ SMS

  modem_check_at("AT+CMGDA=\"DEL READ\"", "OK", 1000);

  debug_println(F("sms_check() completed"));
}

void sms_cmd(char *cmd, char *phone) {
  char *tmp;
  int i=0;

  debug_println(F("sms_cmd() started"));

  //command separated by "," format: password,command=value
  while ((tmp = strtok_r(cmd, ",", &cmd)) != NULL) {
    if(i == 0) {
      //checking password
      if(strcmp(tmp, config.smspass) == 0) {
        debug_println(F("sms_cmd(): SMS password accepted, executing command"));
        sms_cmd_run(cmd,phone);
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

void sms_cmd_run(char *cmd, char *phone) {
  char *tmp;
  char *cusdd, *cusddval;
  char msg[130];
  long val;
  int k=0, z=0;

  debug_println(F("sms_cmd_run() started"));

  //checking what command to execute
  
  // Send Login Packet
  //set APN
  tmp = strstr(cmd, "login");
  if(tmp != NULL) {
    debug_println(F("sms_cmd_run(): Login command detected"));
    config.loginPacket = true;
    save_config = true;

    sms_send_msg("Login request sent", phone);
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

    //send SMS reply
    sms_send_msg("New APN saved", phone);

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
    
    //send SMS reply
    sms_send_msg("New SMS password saved", phone);

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
      config.data_log = false;  // using 2, cannot use 0. because zero is default value after eeprom erase
    } else {
      config.data_log = true;
    }
    
    //send SMS reply
    if(config.data_log == true) {
      sms_send_msg("Data Mode ON", phone);
    } else {
      sms_send_msg("Data Mode OFF", phone);
    }
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
    
    //send SMS reply
    if(config.power_ignition == ON) {
      sms_send_msg("Power ignition ON", phone);
    } else {
      sms_send_msg("Power ignition OFF", phone);
    }
    save_config = true;
    power_reboot = true;
  }
  
  // Set Interval
  tmp = strstr(cmd, "int=");
  if(tmp != NULL) {
    debug_println(F("sms_cmd_run(): Set interval command detected"));

    //setting new Interval
    tmp += strlen("int=");

//    debug_println(F("sms_cmd_run(): New interval"));

    val = atol(tmp);

    if(val > 0) {
      //updating interval in config
      config.interval = val;

      debug_print(F("New interval configured: "));
      debug_println(config.interval);

      //send SMS reply
      sms_send_msg("Interval data transfer changed", phone);

      save_config = true;
      power_reboot = true;
    } else debug_println(F("sms_cmd_run(): invalid value"));
  }

// Get request position
  tmp = strstr(cmd, "pos");
  if(tmp != NULL) {
    
    debug_println(F("sms_cmd_run(): Position request detected"));

    snprintf(msg,130,"https://maps.google.com/maps/place/%s,%s",lat_current,lon_current);

    sms_send_msg(msg, phone);
  }

// Get IMEI
  tmp = strstr(cmd, "imei");
  if(tmp != NULL) {
    
    debug_println(F("sms_cmd_run(): IMEI request detected"));

    snprintf(msg,130,"Your IMEI number is %s",config.imei);

    sms_send_msg(msg, phone);
  }

// Get Key
  tmp = strstr(cmd, "key");
  if(tmp != NULL) {
        
    debug_println(F("sms_cmd_run(): Key request detected"));

    snprintf(msg,130,"Your access key is %s",config.key);

    sms_send_msg(msg, phone);
  }

  memset(msg, '\0', 130);

  /* to add more commands, avoid create filter below Send USSD Code */
  
// Send USSD Code
  tmp = strstr(cmd, "ussd=");  
  if(tmp != NULL) {
    debug_println(F("sms_cmd_run(): Request for USSD Code detected"));
    
    char phonecpy[20];
    char r_cusd[140];
    int xy=0;
    strcpy(phonecpy, phone);  // copy phone -> phonecpy, modem_reply will reset counter to process ussd
    
    tmp += strlen("ussd=");
    if(tmp){
      modem.print("AT+CUSD=1,\"*");
      modem.print(tmp);

      if (modem_check_at("#\",15","OK",5000) == 1){
        if (modem_check_at("AT","\", ",5000) == 1){ // ", 0 end of CUSD data received. send AT to avoid last command by clear modem_reply
          cusdd = strstr(modem_reply, "\"");
          cusddval = strtok(cusdd, "\"");

          memset(r_cusd, '\0', 140);

          for (xy=0; xy < strlen(cusddval); xy++){
            r_cusd[xy] = cusddval[xy];
            if (xy > 140){
              break;
            }
          }
          r_cusd[xy+1] = '\0';
          
          debug_println(F("result of CUSD:"));
          debug_println(r_cusd);
        }
      } 
   }
   
      sms_send_msg(r_cusd, phonecpy);
      modem_check_at("AT+CUSD=2","OK",2000); // cancel USSD session

 }

  debug_println(F("sms_cmd_run() completed"));
}

void sms_send_msg(char* txt, char* phone){
  debug_println(F("sms_send_msg() started"));
  
  modem.print("AT+CMGS=\"");
  modem.print("+");
  modem.print(phone);

  debug_println(txt);
  
  if (modem_check_at("\"", ">", 2000) == 1){
      modem.print(txt);
      if (modem_check_at("\x1A", "OK", 5000) == 1){ //ctrl+z
        debug_println(F("text message sent!"));
        delay(1000);
        modem_check_at("AT+CMGDA=\"DEL SENT\"", "OK", 1000);
      }
  }
  
  debug_println(F("sms_send_msg() completed"));
}

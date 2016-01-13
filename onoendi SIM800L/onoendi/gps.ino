// collect all information from gps then standarize the format. final data to be sent.
void collect_gps_data(){
  char tmpgps[15];
  gpsfix = false;
  data_index = 0;

  float flat, flon;
  unsigned long chars = 0;
  unsigned long fix_age, time_gps, date_gps, speed, course, alt;
  unsigned short sentences = 0, failed = 0;

  float falt = t_gps.f_altitude(); // +/- altitude in meters
  float fc = t_gps.f_course(); // course in degrees
  float fkmph = t_gps.f_speed_kmph(); // speed in km/hr

  // time in hhmmsscc, date in ddmmyy
  t_gps.get_datetime(&date_gps, &time_gps, &fix_age);

  // Get current lat/lon, then store to data_current
  t_gps.f_get_position(&flat, &flon, &fix_age);

  if(fix_age == TinyGPS::GPS_INVALID_AGE)
    debug_println(F("No fresh fix detected"));
    
    else if(fix_age > 1000)
      debug_println(F("Warning: possible stale data!"));
      
      else {
        //update current time var - format 04/12/98,00:35:45+00
        // Add 1000000 to ensure the position of the digits
        ltoa(date_gps + 1000000, tmpgps, 10);  //1ddmmyy
        iso_date_time[0] = tmpgps[1];
        iso_date_time[1] = tmpgps[2];
        iso_date_time[2] = '/';
        iso_date_time[3] = tmpgps[3];
        iso_date_time[4] = tmpgps[4];
        iso_date_time[5] = '/';
        iso_date_time[6] = tmpgps[5];
        iso_date_time[7] = tmpgps[6];
        iso_date_time[8] = ',';
    
        // Add 1000000 to ensure the position of the digits
        ltoa(time_gps + 100000000, tmpgps, 10);  //1hhmmssms
        iso_date_time[9] = tmpgps[1];
        iso_date_time[10] = tmpgps[2];
        iso_date_time[11] = ':';
        iso_date_time[12] = tmpgps[3];
        iso_date_time[13] = tmpgps[4];
        iso_date_time[14] = ':';
        iso_date_time[15] = tmpgps[5];
        iso_date_time[16] = tmpgps[6];
        iso_date_time[17] = '+';
        iso_date_time[18] = '0';
        iso_date_time[19] = '0';
        iso_date_time[20] = '\0';        
    
        debug_print(F("iso_date_time set from GPS time: "));
        debug_println(iso_date_time);
      
        int first_gps_item = 0;
      
        // GPS DATE
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }
      
          //converting date to data packet
          ltoa(date_gps, tmpgps, 10);
          for(int i=0;i<strlen(tmpgps);i++) {
            data_current[data_index] = tmpgps[i];
            data_index++;
          }
      
        // GPS TIME
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }
      
          //time
          ltoa(time_gps, tmpgps, 10);
          for(int i=0;i<strlen(tmpgps);i++) {
            data_current[data_index] = tmpgps[i];
            data_index++;
          }
      
        // GPS LATITUDE
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }
      
          dtostrf(flat,1,6,tmpgps);
          dtostrf(flat,1,6,lat_current);
          for(int i=0;i<strlen(tmpgps);i++) {
            data_current[data_index++] = tmpgps[i];
          }
      
        // GPS LONGITUDE
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }
      
          dtostrf(flon,1,6,tmpgps);
          dtostrf(flon,1,6,lon_current);
          for(int i=0;i<strlen(tmpgps);i++) {
            data_current[data_index++] = tmpgps[i];
          }
      
        // GPS SPEED
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }
      
          dtostrf(fkmph,1,2,tmpgps);
          for(int i=0;i<strlen(tmpgps);i++) {
            data_current[data_index++] = tmpgps[i];
          }
      
        // GPS ALTITUDE
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }
      
          dtostrf(falt,1,2,tmpgps);
          for(int i=0;i<strlen(tmpgps);i++) {
            data_current[data_index++] = tmpgps[i];
          }
      
        // GPS HEADING
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }
      
          dtostrf(fc,1,2,tmpgps);
          for(int i=0;i<strlen(tmpgps);i++) {
            data_current[data_index++] = tmpgps[i];
          }
      
        // GPS HDOP
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }
      
          long hdop = t_gps.hdop(); //hdop
      
          ltoa(hdop, tmpgps, 10);
          for(int i=0;i<strlen(tmpgps);i++) {
            data_current[data_index++] = tmpgps[i];
          }
      
        // GPS SATELITES
          if(first_gps_item) {
            data_current[data_index++] = ',';
          } else {
            first_gps_item = 1;
          }
      
          long sats = t_gps.satellites(); //satellites
      
          ltoa(sats, tmpgps, 10);
          for(int i=0;i<strlen(tmpgps);i++) {
            data_current[data_index++] = tmpgps[i];
          }
      
      }

  //get fix in case no valid altitude or course supplied
  if(falt == 1000000){
    debug_println(F("can't get altitude!"));
    
    } else if(fc == 0){
        debug_println(F("no course!"));
      
      } else if(date_gps == 0){
          debug_println(F("Date is not valid!"));
      
        } else if(fkmph < GPS_SPEED_TO_DETECT){
            debug_print(F("Speed under human walking/unstable position : "));
            debug_println(fkmph);
      
            //check if this fix is already received or yet
          } else if((last_time_gps == time_gps) && (last_date_gps == date_gps)) {
            debug_println(F("This fix date/time already logged"));
            // repeat collect_gps_data() in the next session
    
            } else {
              gpsfix = true;
              debug_println(F("GPS fix received."));
            
              //save last gps data date/time
              last_time_gps = time_gps;
              last_date_gps = date_gps;
              
            }
            
  if(gpsfix) {
    //fix was found
    debug_println(data_current);
    
    } else {
      debug_println(F("collect_gps_data(): fix not yet acquired"));
      t_gps.stats(&chars, &sentences, &failed);
      debug_print(F("Failed checksums: "));
      debug_println(failed);
      
    }
    
}

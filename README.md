# ONOENDI 
## V2.0 - SIM800L
In this version I'm using GSM shield (SIM800L) small and cheap :) there is some feature addition.
with 2 model communication, Text Message and HTTP Data. Now the gps device is more complex:<br>
1. Arduino Pro Mini 328 (in this version)<br>
2. SIM800L GSM Shield<br>
3. GPS Module VK2828U7G5LF ublox 7<br>
4. LM2596 Power Module (GSM shield need 3.7V instead of 5V, so I need this module)

### Features [October 03, 2015]
- Track movement
- Display track log - support by Geolink
- Engine shutdown/cut-off power remotely from sms
- Environment monitoring/reporting (will..)
- Monitor and Control from mobile device (will..)

### SMS Format
Format: #password,command=<value>
e.g. #12345,apn=internet
send command to device cellular number
more info can read from manual pdf file include


## V1 - Wiznet W5100
GPS Tracker Arduino Based, this is normally Geo Location until now, there is no special function. 
Built from shield by shield but not using GSM shield, I mean this way is alternative that I have:<br>
1. Arduino UNO R3, when I was built in my 1st test<br>
2. W5100 Wiznet Ethernet Shield<br>
3. Of course we need GPS shield, I'm using ublox neo-6m gps<br>
4. Small 3G Router, will hack the board (OS can be using with Open-WRT)<br>
5. 3G Modem GPRS/CDMA

### Features [September 05, 2015]
- Track movement
- Lost-mode
- Engine shutdown remotely (will..)
- Environment monitoring/reporting (will..)


### Copyright
Luqman Sahidin<br>
Yugan Studio Works

This file was downloaded from http://www.github.com/lsahidin/onoendi

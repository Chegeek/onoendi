//#define DEBUG
#define DEFAULT_APN "internet"
#define DEFAULT_SMS_KEY "12345"

// PIN MAPPING
#define LEDPIN  13
#define RELAYPIN  11
#define MODEMRST  10
//#define GPSRST  10 //no need to reset
#define MCURST  5

#define ON  1
#define OFF 2 // using 2, cannot use 0. because zero is default value after eeprom erase

#define msg_char  110
#define GPS_SPEED_TO_DETECT 5 // means move 5 kmph, human walking.

#define DEFAULT_INTERVAL  5       // 5 seconds
#define DEFAULT_POWERIGNITION  ON // relay LOW
#define SMS_INTERVAL  3           // 3 seconds
#define URL "updates.geolink.io/index.php?"

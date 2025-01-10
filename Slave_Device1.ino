// Slave 6.0
// Now emergency is independent
// Added "Invalid" string for Empty json extract

#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <StreamUtils.h>

#define LBS 1 // LOAD BALANCING SYSTEM
#define EVSE_3S_ADC_VALUES 1
#define EVSE_7S_ADC_VALUES 1
#define EVSE_6_6_ADC_VALUES 1

uint16_t load_value = 0;
uint16_t relay_change = 0;

/*
    This creates two empty databases, populates values, and retrieves them back
    from the SPIFFS file
*/
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <SPI.h>
#include <FS.h>
#include "SPIFFS.h"
#include "FFat.h"
#include "src/LocalListHandler.h"
#include "src/AuthorizationCache.h"
#include "src/LocalStorage.h"

#if LBS
#include "src/evse_espnow_slave.h"
extern volatile uint8_t phase_Type;
extern uint8_t received_action;
extern uint8_t phase_change_flag;
extern   uint8_t phase_change_flag_3;
extern  uint8_t phase_change_flag_6;
extern  uint8_t phase_change_flag_7;
extern uint8_t load_change_flag;
extern uint8_t dis_phase_type;
extern volatile float device_load;
uint8_t previous_phase = 0;
bool charge_request = 0;
volatile  uint8_t relay_restart_flag = 0;

bool sus_active = 0;
float previous_load = 0;

bool load_receive = 0;

extern volatile uint8_t action_phase_type;
extern uint8_t device_id;
extern uint8_t phase_assign_load_change;

bool send_power_flag = 0;

uint16_t count = 0;

// #define EVSE_3S_CP_SR (1)

#if EVSE_3S_ADC_VALUES
volatile uint32_t evse_state_a_upper_threshold = 4096;
volatile uint32_t evse_state_a_lower_threshold = 3900;
volatile uint32_t evse_state_b_upper_threshold = 3500;
volatile uint32_t evse_state_b_lower_threshold = 2600;
volatile uint32_t evse_state_c_upper_threshold = 747;
volatile uint32_t evse_state_c_lower_threshold = 626;
volatile uint32_t evse_state_d_upper_threshold = 625;
volatile uint32_t evse_state_d_lower_threshold = 500;
volatile uint32_t evse_state_e_threshold = 499;
volatile uint32_t evse_state_sus_upper_threshold = 877;
volatile uint32_t evse_state_sus_lower_threshold = 748;
volatile uint32_t evse_state_e2_upper_threshold = 2399;
volatile uint32_t evse_state_e2_lower_threshold = 2100;
volatile uint32_t evse_state_dis_upper_threshold = 1050;
volatile uint32_t evse_state_dis_lower_threshold = 878;

#endif

#if EVSE_6_6_ADC_VALUES

volatile uint32_t evse_state_a_upper_threshold_6 = 4096;
volatile uint32_t evse_state_a_lower_threshold_6 = 3900;
volatile uint32_t evse_state_b_upper_threshold_6 = 3450;
volatile uint32_t evse_state_b_lower_threshold_6 = 2600;
volatile uint32_t evse_state_c_upper_threshold_6 = 1300;
volatile uint32_t evse_state_c_lower_threshold_6 = 1140;
volatile uint32_t evse_state_d_upper_threshold_6 = 625;
volatile uint32_t evse_state_d_lower_threshold_6 = 500;
volatile uint32_t evse_state_e_threshold_6 = 499;
volatile uint32_t evse_state_sus_upper_threshold_6 = 1520;
volatile uint32_t evse_state_sus_lower_threshold_6 = 1360;
volatile uint32_t evse_state_e2_upper_threshold_6 = 2399;
volatile uint32_t evse_state_e2_lower_threshold_6 = 2100;
volatile uint32_t evse_state_dis_upper_threshold_6 = 1770;
volatile uint32_t evse_state_dis_lower_threshold_6 = 1600;



#endif

#if EVSE_7S_ADC_VALUES

volatile uint32_t evse_state_a_upper_threshold_7 = 4096;
volatile uint32_t evse_state_a_lower_threshold_7 = 3500;
volatile uint32_t evse_state_b_upper_threshold_7 = 3500;
volatile uint32_t evse_state_b_lower_threshold_7 = 2900;
volatile uint32_t evse_state_c_upper_threshold_7 = 1551;
volatile uint32_t evse_state_c_lower_threshold_7 = 1150;
volatile uint32_t evse_state_d_upper_threshold_7 = 1150;
volatile uint32_t evse_state_d_lower_threshold_7 = 850;
volatile uint32_t evse_state_e_threshold_7 = 850;
volatile uint32_t evse_state_sus_upper_threshold_7 = 1850;
volatile uint32_t evse_state_sus_lower_threshold_7 = 1550;
volatile uint32_t evse_state_e2_upper_threshold_7 = 2800;
volatile uint32_t evse_state_e2_lower_threshold_7 = 2100;
volatile uint32_t evse_state_dis_upper_threshold_7 = 2100;
volatile uint32_t evse_state_dis_lower_threshold_7 = 1850;




#endif



// #if EVSE_3S_CP_SR

// evse_state_a_upper_threshold = 4096;
// evse_state_a_lower_threshold = 3900;
// evse_state_b_upper_threshold = 3500;
// evse_state_b_lower_threshold = 2600;
// evse_state_c_upper_threshold = 747;
// evse_state_c_lower_threshold = 626;
// evse_state_d_upper_threshold = 625;
// evse_state_d_lower_threshold = 500;
// evse_state_e_threshold = 499;
// evse_state_sus_upper_threshold = 877;
// evse_state_sus_lower_threshold = 748;
// evse_state_e2_upper_threshold = 2399;
// evse_state_e2_lower_threshold = 2100;
// evse_state_dis_upper_threshold = 1050;
// evse_state_dis_lower_threshold = 878;

// #endif

#endif

#include <nvs_flash.h>
#include <Preferences.h>

#include <time.h>
#include <Wire.h>
#include <RTCx.h>

#define V_charge_lite1_4 1

#define STM_ENABLE 0
#if LBS
DynamicJsonDocument rxM_doc(200);
DynamicJsonDocument txM_doc(200);

#endif

#include "driver/ledc.h"
#include "esp_err.h"

#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO (5) // Define the output GPIO
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY (4095)                // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY (1000)           // Frequency in Hertz. Set frequency at 5 kHz

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true

#define RED 00
#define GREEN 01
#define BLUE 02
#define WHITE 03
#define YELLOW 04
#define BLINKYGREEN 05
#define BLINKYBLUE 06
#define BLINKYRED 07

#define ORANGE 9
#define VOILET 10

   // Blinking

#define BLINKYGREEN 05
#define BLINKYBLUE 06
#define BLINKYRED 07
#define BLINKYWHITE 11
#define BLINKYWHITE_ALL 12
#define BLINKYBLUE_ALL 13

#define CP_ENABLE 1 // for control pilot

/* States for EVSE state machine */
typedef enum
{
  STARTUP,
  GFCI_CHECK_START,
  GFCI_CHECK_EXPECTED,
  GFCI_PASSED,
  FAULT,
  STATE_A,   // 5
  STATE_B,   // 6
  STATE_C,   // 7
  STATE_D,   // 8
  STATE_E,   // 9
  STATE_SUS, // 10
  STATE_DIS, // 11
  V_UNKNOWN
  // STATE_F
} EVSE_states_enum;

EVSE_states_enum EVSE_state;
EVSE_states_enum PRV_EVSE_state;

volatile uint8_t gu8state_ctob = 0;
volatile uint16_t gu16state_ctob_count = 0;

volatile uint16_t gu16relay_weld_scanning_count = 0;
volatile uint16_t gu16relay_weld_scanning_event = 0;

volatile int ADC_Result = 0;

uint8_t gu8_dBloop_flag = 0;
String db_getparam;
uint8_t gu8_get_type = 0;

uint8_t gu8_authCaheloop_flag = 0;
String authCache_getparam;
uint8_t gu8_get_auth_type = 0;
extern bool clearcache_flag;
extern bool get_authCache_flag;
extern bool set_authCache_flag;

bool flag_blinkygreen_eins = false;
bool flag_blinkyblue_eins = false;
bool flag_blinkyred_eins = false;
bool flag_blinkywhite_eins = false;

bool flag_blinkygreen_zwei = false;
bool flag_blinkyblue_zwei = false;
bool flag_blinkyred_zwei = false;
bool flag_blinkywhite_zwei = false;

bool flag_blinkygreen_drei = false;
bool flag_blinkyblue_drei = false;
bool flag_blinkyred_drei = false;
bool flag_blinkywhite_drei = false;

bool flag_green_eins = false;
bool flag_blue_eins = false;
bool flag_red_eins = false;
bool flag_white_eins = false;
bool flag_orange_eins = false;
bool flag_violet_eins = false;

bool flag_green_zwei = false;
bool flag_blue_zwei = false;
bool flag_red_zwei = false;
bool flag_white_zwei = false;
bool flag_orange_zwei = false;
bool flag_violet_zwei = false;

bool flag_green_drei = false;
bool flag_blue_drei = false;
bool flag_red_drei = false;
bool flag_white_drei = false;
bool flag_orange_drei = false;
bool flag_violet_drei = false;

// Sflags

bool sflag_blinkygreen_eins = false;
bool sflag_blinkyblue_eins = false;
bool sflag_blinkyred_eins = false;
bool sflag_blinkywhite_eins = false;

bool sflag_blinkygreen_zwei = false;
bool sflag_blinkyblue_zwei = false;
bool sflag_blinkyred_zwei = false;
bool sflag_blinkywhite_zwei = false;

bool sflag_blinkygreen_drei = false;
bool sflag_blinkyblue_drei = false;
bool sflag_blinkyred_drei = false;
bool sflag_blinkywhite_drei = false;

bool sflag_green_eins = false;
bool sflag_blue_eins = false;
bool sflag_red_eins = false;
bool sflag_white_eins = false;
bool sflag_orange_eins = false;
bool sflag_violet_eins = false;

bool sflag_green_zwei = false;
bool sflag_blue_zwei = false;
bool sflag_red_zwei = false;
bool sflag_white_zwei = false;
bool sflag_orange_zwei = false;
bool sflag_violet_zwei = false;

bool sflag_green_drei = false;
bool sflag_blue_drei = false;
bool sflag_red_drei = false;
bool sflag_white_drei = false;
bool sflag_orange_drei = false;
bool sflag_violet_drei = false;

extern int counter;

bool flag_emgy_once = false;
bool slave_emgy_flag = false;

bool prev_flag_blinkygreen_eins = false;
bool prev_flag_blinkyblue_eins = false;
bool prev_flag_blinkyred_eins = false;
bool prev_flag_blinkywhite_eins = false;

bool prev_flag_blinkygreen_zwei = false;
bool prev_flag_blinkyblue_zwei = false;
bool prev_flag_blinkyred_zwei = false;
bool prev_flag_blinkywhite_zwei = false;

bool prev_flag_blinkygreen_drei = false;
bool prev_flag_blinkyblue_drei = false;
bool prev_flag_blinkyred_drei = false;
bool prev_flag_blinkywhite_drei = false;

volatile uint8_t relay_weld_fault_occ = 0;
volatile uint8_t relay_weld_pwm_stop_flag = 0;

bool ledOff = false;
bool rfid = false;
unsigned long onTime, offTime, startOfflineWhite; // the last time the output pin was toggled

uint8_t gu8_cp_relay_flag = 0;

#define PB1 13
#define PB2 15
#define PB3 4

#if V_charge_lite1_4
#undef EMGY
#undef EARTH_DISCONNECT
#define EMGY 39 // Sensor VN
#define EARTH_DISCONNECT 35
#else
#undef EMGY
#define EMGY 2
#endif

#define GFCI 36

#define RELAY1 27 // 5 old design
#define RELAY2 18
#define RELAY3 19

#define RED1 32
#define GREEN1 33
#define BLUE1 25

#define RED2 26
#define GREEN2 5
#define BLUE2 14

#define RED3 21
#define GREEN3 22
#define BLUE3 23

#define CPIN 34

#if STM_ENABLE
#define STM_PWM
#endif

#define CPOUT 12 //@anesh commit to 12

Preferences txnid;
Preferences rfid_table;

// ID TAG dB for Local list database
IdTagInfoClass* idTagObject_db;

#if IDTAG_INFO_DB
extern Local_Authorization_List_t get_Local_Authorization_List;
extern Local_Authorization_List_t Local_Authorization_List;
extern LinkedList<Local_Authorization_List_t> LL_Local_Authorization_List;

#endif

// ID TAG dB for Authorization cache database
AuthorizationCacheInfoClass* AuthorizationCacheObject_db;

#if AUTHORIZATION_CACHE_DB
extern Authorization_Cache_List_t get_Authorization_Cache_List;
extern Authorization_Cache_List_t Authorization_Cache_List;
extern LinkedList<Authorization_Cache_List_t> LL_Authorization_Cache_List;

#endif

#if LOCAL_STORAGE_DB
extern LinkedList<String> LL_StoredRFIDs;
extern LinkedList<stored_session> LL_Local_Stored_List;
#endif

/*
 * @brief : Instantiate an object pointer for LocalStorage class
 */

LocalStorageClass* localStoreObject;
uint8_t gu8_storage_Loop = 100;

// Control Pilot Setup
const int pwm_pin = CPOUT; // CPIN
const int pwm_freq = 1000;
const int pwm_channel = 0;
const int pwm_resolution = 8;
float chargingLimit_7S = 32.0f;
float cp_chargelimit_3S = 16.0f;
float chargingLimit_6S = 27.6f;
// float chargingLimit = 14.0f;

// have to increase serial buffer size from 64 bytes to 256 bytes
// For this goto Arduino/Library/EspSoftwareSerial and update BufCapacity to 256

#if 0
SoftwareSerial slaveSerial(16, 17); // Rx ->16, Tx ->17
#else
#define slaveSerial Serial2 // Rx ->16, Tx ->17
#endif

// void slaveLed(StaticJsonDocument rx_doc);
int CalculateDutyCycle(float);
void slaveCP_IN();
void slaveCP_OUT(bool);
void slaveConnectorId();
void slaveRelay(bool, char);
void slaveEmgy();
#if V_charge_lite1_4
void slaveEarthDisc();
#endif
void slaveLed(char, bool, int);
void slavesendlocallist();
void slavegetlistVersion();
void slavegetidtag();
void slavegetupdateType();
void slavegetresponses(uint8_t lu8_get_type);
#if LBS
bool receivePower();
float received_power = 0;
#endif

void slave_SendAuthCache();
void slave_GetAuthCache();
void slave_ClearCache();
void syncTime(String, String, String, String, String, String);
/*
 * @brief : Additions for local storage
 */
void slaveStoreRFID();
void slaveStoreRFID_resp(uint8_t i);

StaticJsonDocument<1000> tx_doc; // 350 changed to 1000
StaticJsonDocument<1000> rx_doc; // 350 changed to 1000
int state_timer = 0;

void stateEINS();
void stateZWEI();
void stateDREI();
void stateTimer();
void dBLoop();
void AuthCacheLoop();
void storageLoop();
void slaveRtc();
bool setVal = false;
void checkRelay_Weld();
/*******************/
// RTC Implementation

#if V_charge_lite1_4
void slaveEarthDisc()
{
  bool stat = digitalRead(EARTH_DISCONNECT);
  Serial.println("Status:-> " + String(stat));
  tx_doc["type"] = "response";
  tx_doc["object"] = "earth_disc";
  tx_doc["status"] = stat;
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}
#endif

void rtc_setup()
{

  // Wire.begin();
  Wire.begin(13, 15); // PB1 and PB2
  // Wire.begin(18,19); // DS1307 optional.
  Serial.println();
  Serial.println("Autoprobing for a RTC...");
  if (rtc.autoprobe())
  {
    // Found something, hopefully a clock.
    Serial.print("Autoprobe found ");
    Serial.print(rtc.getDeviceName());
    Serial.print(" at 0x");
    Serial.println(rtc.getAddress(), HEX);
  }

  // Enable the battery backup. This happens by default on the DS1307
  // but needs to be enabled on the MCP7941x.
  rtc.enableBatteryBackup();

  // rtc.clearPowerFailFlag();

  // Ensure the oscillator is running.
  rtc.startClock();

  if (rtc.getDevice() == RTCx::MCP7941x)
  {
    Serial.print("Calibration: ");
    Serial.println(rtc.getCalibration(), DEC);
    // rtc.setCalibration(-127);
  }

  rtc.setSQW(RTCx::freq4096Hz);
}

const uint8_t bufLen = 30;
char buffer[bufLen + 1] = { '\0' };
uint8_t bufPos = 0;
unsigned long last = 0;
struct RTCx::tm tm;
RTCx::time_t rtc_timestamp()
{

  last = millis();
  rtc.readClock(tm);

  RTCx::printIsotime(Serial, tm).println();
  RTCx::time_t t = RTCx::mktime(&tm);

  Serial.print("unixtime = ");
  Serial.println(t);
  Serial.println("-----");

  return t;
}

void syncTime(String hr, String min, String sec, String dt, String mon, String yr)
{
  tx_doc["type"] = "response";
  tx_doc["object"] = "TimeSync";
  tm.tm_mon = mon.toInt();
  tm.tm_mday = dt.toInt();
  tm.tm_year = yr.toInt() - 1900; // Years since 1900.
  tm.tm_sec = sec.toInt();        // Seconds [0..59]
  tm.tm_min = min.toInt();        // Minutes [0..59].
  tm.tm_hour = hr.toInt();        // Hour [0..23].
  bool suc = rtc.setClock(&tm);
  if (suc)
  {
    Serial.println(F("Time was set"));
    Serial.println(suc);
    tx_doc["status"] = "Accepted";
  }
  else
  {
    Serial.println(F("Failed"));
    Serial.println(suc);
    tx_doc["status"] = "Rejected";
  }
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}

void getTime(char* datestring)
{
  rtc.readClock(tm);
  /*sprintf(datestring, "%04d-%02d-%02dT%02d:%02d:%02d.000Z", tm.tm_year + 1900,
      tm.tm_mon + 1,
      tm.tm_mday,
      tm.tm_hour,
      tm.tm_min,
      tm.tm_sec);*/
      /*sprintf(datestring, "%02d%02d%02d%02d%02d%02d", tm.tm_mday,tm.tm_mon + 1, ((tm.tm_year + 1900)-2000),tm.tm_hour,
      tm.tm_min,
      tm.tm_sec);*/

  for (int i = 0; i < 6; i++)
  {
    switch (i)
    {
    case 0:
      datestring[i] = tm.tm_mday;
      break;
    case 1:
      datestring[i] = tm.tm_mon + 1;
      break;
    case 2:
      datestring[i] = ((tm.tm_year + 1900) - 2000);
      break;
    case 3:
      datestring[i] = tm.tm_hour;
      break;
    case 4:
      datestring[i] = tm.tm_min;
      break;
    case 5:
      datestring[i] = tm.tm_sec;
      break;
    default:
      break;
    }
  }
  Serial.print(F("***Debug Date***"));
  for (int i = 0; i < 6; i++)
  {
    Serial.println(datestring[i], HEX);
  }
}

void slaveRtc()
{
  tx_doc.clear();

  // String timestamp = String(rtc_timestamp());
  char datestring[6];
  getTime(datestring);
  tx_doc["type"] = "response";
  tx_doc["object"] = "rtc";
  tx_doc["timestamp"] = datestring;
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}

void slave_loop()
{
  STATE_check();
  EMGY_check(); // So as to make it independent.

  gu16relay_weld_scanning_count++;
  if (gu16relay_weld_scanning_count >= 50)
  {
    gu16relay_weld_scanning_count = 0;
    /*relay weld is commented */
    // checkRelay_Weld();
  }

  tx_doc.clear();
  rx_doc.clear();

  const char* type = "";
  const char* object = "";

  int startTime = 0;
  bool success = false;
  if (slaveSerial.available())
  {
    Serial.println(F("Slave:"));
    ReadLoggingStream loggingStream(slaveSerial, Serial);
    DeserializationError err = deserializeJson(rx_doc, loggingStream);
    switch (err.code())
    {
    case DeserializationError::Ok:
      success = true;
      break;
    case DeserializationError::InvalidInput:
      Serial.print(F(" Invalid input! Not a JSON\n"));
      break;
    case DeserializationError::NoMemory:
      Serial.print(F("Error: Not enough memory\n"));
      break;
    default:
      Serial.print(F("Deserialization failed\n"));
      break;
    }

    if (!success)
    {
      rx_doc.clear();
      delay(1000);
      while (slaveSerial.available())
      { // testing for flushing garbage values :: although while loop is strictly forbidden
        slaveSerial.read();
      }
    }
    else
    {
      type = rx_doc["type"] | "Invalid";
      object = rx_doc["object"] | "Invalid";

      /************************************************* Authentication cache  (SQL DataBase)************************************/
      if ((strcmp(object, "SendAuthCache") == 0))
      {
        // Recevied packet format from Master to Slave
        // {"type":"request","object":"SendAuthCache","idTag":"8037d79e","expiryTime,":"1660042275"}

        uint8_t lu8_authcachelist_id = 0;
        int listVersion = 0;
        time_t rec_expirytime = 0;
        LL_Authorization_Cache_List.clear();

        lu8_authcachelist_id++;
        Serial.println();
        Serial.println("SendAuthCache");
        Authorization_Cache_List.id = lu8_authcachelist_id;
        Serial.print("lu8_authcachelist_id: ");
        Serial.println(Authorization_Cache_List.id);

        Serial.print(F("idTag: "));
        Authorization_Cache_List.idTag = rx_doc["idTag"].as<char*>();
        Serial.println(Authorization_Cache_List.idTag);
        rec_expirytime = rx_doc["expiryTime"];
        Authorization_Cache_List.expiry_time = rec_expirytime;
        Serial.println(Authorization_Cache_List.expiry_time);

        LL_Authorization_Cache_List.add(Authorization_Cache_List);

        gu8_authCaheloop_flag = SET_AUTH_CACHE_DB_PKT; // 1 for Set the data to Table
        return;
      }

      if ((strcmp(object, "GetAuthCache") == 0))
      {
        // Recevied packet format from Master to Slave
        // {"type":"request","object":"GetAuthCache","idTag":"8037d79e"}
        authCache_getparam = rx_doc["idTag"].as<char*>();
        gu8_authCaheloop_flag = GET_AUTH_CACHE_DB_PKT; // 2 for Get the data from Table

        return;
      }

      if ((strcmp(object, "ClearCache") == 0))
      {
        // Recevied packet format from Master to Slave
        // {"type":"request","object":"ClearCache"}
        gu8_authCaheloop_flag = CLEAR_AUTH_CACHE_DB_PKT; // 3 for Clear cache or data from Table

        return;
      }

      /************************************************* Authentication cache (SQL DataBase) ************************************/

      /************************rtc*******************/
      if (strcmp(object, "rtc") == 0)
      {
        slaveRtc();
        return;
      }
      /************************rtc*******************/

      /************************************************* Local Authentication List (SQL DataBase) ************************************/

      if ((strcmp(object, "SendLocalList") == 0))
      {
        // Recevied packet format from Master to Slave
        //{"type":"request","object":"SendLocalList","locallist":{"listVersion":1,"localAuthorizationList":[{"idTag":"8037d79e","idTagInfo":{"status":"Accepted"}},{"idTag":"c079a49f","idTagInfo":{"status":"Accepted"}},{"idTag":"d787007f","idTagInfo":{"status":"Accepted"}}],"updateType":"Full"}}

        uint8_t lu8_locallist_id = 0;
        int listVersion = 0;
        bool empty_locallist_flag = true;
        LL_Local_Authorization_List.clear();
        Local_Authorization_List.listVersion = rx_doc["locallist"]["listVersion"].as<int>();
        Serial.print(F("rx_doc[locallist][listVersion] : "));
        Serial.println(Local_Authorization_List.listVersion);

        Serial.print(F("rx_doc[locallist][updateType] : "));
        Local_Authorization_List.updateType = rx_doc["locallist"]["updateType"].as<char*>();
        Serial.println(Local_Authorization_List.updateType);

        // Get a reference to the array
        JsonArray repos1 = rx_doc["locallist"]["localAuthorizationList"];

        if (Local_Authorization_List.updateType.equals("Full") == true)
        {
          Serial.println(F("Full"));
        }
        else if (Local_Authorization_List.updateType.equals("Differential") == true)
        {
          Serial.println(F("Differential"));
        }

        // Print the values
        for (JsonObject repo : repos1)
        {
          lu8_locallist_id++;
          Local_Authorization_List.id = lu8_locallist_id;
          Serial.print("lu8_locallist_id: ");
          Serial.print(Local_Authorization_List.id);

          Serial.print(F("idTag: "));
          Local_Authorization_List.idTag = repo["idTag"].as<char*>();

          Serial.println(Local_Authorization_List.idTag);
          Serial.print(F("idTagInfo: status "));
          Local_Authorization_List.idTagStatus = repo["idTagInfo"]["status"].as<char*>();
          Serial.println(Local_Authorization_List.idTagStatus);
          Local_Authorization_List.perentidTag = repo["idTagInfo"]["perentidTag"] | "Invalid";
          Serial.println(Local_Authorization_List.perentidTag);
          Local_Authorization_List.expiry_time = repo["idTagInfo"]["expiry_time"] | "Invalid";
          Serial.println(Local_Authorization_List.expiry_time);
          LL_Local_Authorization_List.add(Local_Authorization_List);
          empty_locallist_flag = false;
        }

        if (empty_locallist_flag)
        {
          Serial.print(" send local list is empty ...! ");
          lu8_locallist_id++;
          Local_Authorization_List.id = lu8_locallist_id;
          Serial.print("lu8_locallist_id: ");
          Serial.print(Local_Authorization_List.id);
          Serial.print(F("idTag: "));
          Local_Authorization_List.idTag = "Invalid";
          Serial.println(Local_Authorization_List.idTag);
          Serial.print(F("idTagInfo: status "));
          Local_Authorization_List.idTagStatus = "Invalid";
          Serial.println(Local_Authorization_List.idTagStatus);
          Local_Authorization_List.perentidTag = "Invalid";
          Serial.println(Local_Authorization_List.perentidTag);
          Local_Authorization_List.expiry_time = "Invalid";
          Serial.println(Local_Authorization_List.expiry_time);
          LL_Local_Authorization_List.add(Local_Authorization_List);
        }

        gu8_dBloop_flag = SET_DB_PKT; // 1 for Set the data to Table
        return;
      }

      if ((strcmp(object, "GetLocalListVersion") == 0))
      {
        // Recevied packet format from Master to Slave
        // {"type":"request","object":"GetLocalListVersion"}
        db_getparam = "listVersion";
        gu8_dBloop_flag = GET_DB_PKT;    // 2 for Get the data from Table
        gu8_get_type = GET_LIST_VERSION; // 1 for Get the type of the data from Table
        return;
      }
#if 0
      if ((strcmp(object, "getid_number") == 0)) {
        // Recevied packet format from Master to Slave 
        // {"type":"request","object":"getid_number","id":"1"}
        *db_getparam = "id";
        gu8_dBloop_flag = GET_DB_PKT; // 2 for Get the data from Table
        gu8_get_type = GET_ID_NUM;   // 2 for Get the type of the data from Table
        return;
      }
#endif
      if ((strcmp(object, "GetidTag") == 0))
      {
        // Recevied packet format from Master to Slave
        // {"type":"request","object":"GetidTag","idTag":"8037d79e"}

        db_getparam = rx_doc["idTag"].as<char*>();
        gu8_dBloop_flag = GET_DB_PKT; // 2 for Get the data from Table
        gu8_get_type = GET_ID_TAG;    // 3 for Get the type of the data from Table
        return;
      }

      if ((strcmp(object, "GetidTagStatus") == 0))
      {
        // Recevied packet format from Master to Slave
        // {"type":"request","object":"GetidTagStatus","idTag":"8037d79e"}

        db_getparam = "idTagStatus";
        gu8_dBloop_flag = GET_DB_PKT;     // 2 for Get the data from Table
        gu8_get_type = GET_ID_TAG_STATUS; // 4 for Get the type of the data from Table
        return;
      }
      if ((strcmp(object, "GetupdateType") == 0))
      {
        // Recevied packet format from Master to Slave
        // {"type":"request","object":"GetupdateType"}
        db_getparam = "updateType";
        gu8_dBloop_flag = GET_DB_PKT;   // 2 for Get the data from Table
        gu8_get_type = GET_UPDATE_TYPE; // 5 for Get the type of the data from Table
        return;
      }
      /************************************************* Local Authentication List (SQL DataBase)************************************/
      /************************************************* Local Storage start BLE **********************************************************/
      if ((strcmp(object, "StoreidTag") == 0))
      {
        // Process the request.
        gu8_storage_Loop = REG_RFID;
        gu8_get_type = REG_RFID;
        // slaveStoreRFID();
        return;
      }
      // if ((strcmp(object, "GetidTagList") == 0))
      if ((strcmp(object, "FetchidTag") == 0))
      {
        // Process the request.
        gu8_storage_Loop = GET_RFID_STORED;
        gu8_get_type = GET_RFID_STORED;
        return;
      }

      if ((strcmp(object, "CheckRegidTag") == 0))
      {
        slaveVerifyRFID();
        return;
      }

      if ((strcmp(object, "storestarttxn") == 0))
      {
        // Process the request.
        gu8_storage_Loop = PUT_DATA;
        gu8_get_type = PUT_DATA;

        /*txM_doc["type"] = "request";
  txM_doc["object"] = "storestarttxn";
  txM_doc["idTag"] = rfid_tag;
  txM_doc["start_date"] = start_date;
  txM_doc["stop_date"] = stop_date;
  txM_doc["units"] = units;
  txM_doc["reason_of_stop"] = reason_of_stop; */

  /*
   * @brief : Use preferences to maintain a txn_id counter which rotates from 1 to 100.
   */
        txnid.begin("txn_table", false);
        int tid = txnid.getInt("current_Txn", 0);

        tid = tid + 1; // increment the count

        if (tid > 100)
        {
          tid = 0;
        }
        txnid.putInt("current_Txn", tid);
        txnid.end();

        String tag = rx_doc["idTag"].as<char*>();
        String start_d = rx_doc["start_date"].as<char*>();
        String stop_d = rx_doc["stop_date"].as<char*>();
        String unit = rx_doc["units"].as<char*>();
        String ros = rx_doc["reason_of_stop"].as<char*>();
        slave_store_txn(String(tid), tag, start_d, stop_d, unit, ros);
        return;
      }

      if ((strcmp(object, "storeupdatetxn") == 0))
      {
        // Process the request.
        gu8_storage_Loop = UPDATE_DATA;
        gu8_get_type = UPDATE_DATA;

        /*
         * @brief : Use preferences to maintain a txn_id counter which rotates from 1 to 100.
         */
        txnid.begin("txn_table", false);
        int tid = txnid.getInt("current_Txn", 0);
        txnid.end();

        String tag = rx_doc["idTag"].as<char*>();
        String start_d = rx_doc["start_date"].as<char*>();
        // String stop_d = rx_doc["stop_date"].as<char *>();
        char datestring[6];
        getTime(datestring);
        // String stop_d = rx_doc["stop_date"].as<char *>();
        String unit = rx_doc["units"].as<char*>();
        String ros = rx_doc["reason_of_stop"].as<char*>();
        slave_update_txn(String(tid), tag, start_d, datestring, unit, ros);
        return;
      }

      if ((strcmp(object, "fetchtxn") == 0))
      {
        // Process the request.
        gu8_storage_Loop = GET_DATA;
        gu8_get_type = GET_DATA;
        // String tid = rx_doc["txn_id"].as<char *>();
        String tid = rx_doc["txn_id"];
        slaveFetchTxn(tid);
        return;
      }
      /************************************************* Local Storage end BLE **********************************************************/

      if ((strcmp(object, "connector") == 0))
      {
        slaveConnectorId();
        return;
      }

      if ((strcmp(object, "relay") == 0))
      {

        bool actionR = rx_doc["action"];
        char connectorR = rx_doc["connectorId"];
        if (received_action)
        {

          slaveRelay(actionR, connectorR); // Commented by shiva @23032023

          return;
        }
      }
      if ((strcmp(object, "led") == 0))
      {
        char colourL = rx_doc["colour"];
        bool actionL = rx_doc["action"];
        char connectorL = rx_doc["connectorId"];
        slaveLed(colourL, actionL, connectorL);
        return;
      }

      if ((strcmp(object, "white") == 0))
      {
        offlineWhite();
        return;
      } // end of led

      if (strcmp(object, "emgy") == 0)
      {
        slaveEmgy();
        return;
      } // end of EMGY

      if (strcmp(object, "gfci") == 0)
      {
        slaveGfci();
        return;
      }

      if (strcmp(object, "cpout") == 0)
      {
        bool actionC = rx_doc["action"];

        Serial.println("actionC: " + String(actionC));
        slaveCP_OUT(actionC);
        return;
      }

      if (strcmp(object, "cpin") == 0)
      {

        slaveCP_IN();
        return;
      }
      if (strcmp(object, "TimeSync") == 0)
      {
        String hr = rx_doc["idTag"].as<char*>();
        String min = rx_doc["start_date"].as<char*>();
        String sec = rx_doc["stop_date"].as<char*>();
        String dt = rx_doc["units"].as<char*>();
        String mon = rx_doc["reason_of_stop"].as<char*>();
        String yr = rx_doc["reason_of_stop"].as<char*>();
        syncTime(hr, min, sec, dt, mon, yr);
        return;
      }

      if (strcmp(object, "powersend") == 0)
      {
        received_power = rx_doc["power"];
        Serial.println("Power from master: " + String(received_power));
        // receivePower();
        return;
      }

#if V_charge_lite1_4
      if (strcmp(object, "earth_disc"))
      {
        slaveEarthDisc();
        return;
      }
#endif
    }
    Serial.println("\n*****************Available Heap*******************");
    Serial.println(ESP.getFreeHeap());
  }
}

#if 0
static void example_ledc_init(void)
{
  // Prepare and then apply the LEDC PWM timer configuration
  ledc_timer_config_t ledc_timer = {
      .speed_mode = LEDC_MODE,
      .timer_num = LEDC_TIMER,
      .duty_resolution = LEDC_DUTY_RES,
      .freq_hz = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
      .clk_cfg = LEDC_AUTO_CLK
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // Prepare and then apply the LEDC PWM channel configuration
  ledc_channel_config_t ledc_channel = {
      .speed_mode = LEDC_MODE,
      .channel = LEDC_CHANNEL,
      .timer_sel = LEDC_TIMER,
      .intr_type = LEDC_INTR_DISABLE,
      .gpio_num = LEDC_OUTPUT_IO,
      .duty = 0, // Set duty to 0%
      .hpoint = 0
  };
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}
/*************Control Pilot ************/
#endif

int CalculateDutyCycle(float chargingLimit)
{

  int dutycycle_l = 0;

  if ((chargingLimit <= 51) && (chargingLimit > 5))
  {

    dutycycle_l = ((chargingLimit / 0.6) * 2.55);
    Serial.println("[ControlPilot] Duty Cycle is = " + String(dutycycle_l));
  }
  else if ((chargingLimit < 80) && (chargingLimit > 51))
  {

    dutycycle_l = (((chargingLimit / 2.5) + 64) * 2.55);
    Serial.println("[ControlPilot] Duty Cycle is = " + String(dutycycle_l));
  }
  else
  {

    Serial.println("[ControlPilot] chargingLimit is not in range");
  }

  return dutycycle_l;
}

void slaveVerifyRFID()
{
  // check in DB here
  String rfid_ = rx_doc["idTag"];
  counter = 0;
  uint8_t r = localStoreObject->db_check_rfid_auth(rfid_);
  if (counter > 0)
  {
    slaveVerifyRFID_resp(1);
  }
  else
  {
    slaveVerifyRFID_resp(0);
  }
  return;
}

void slaveVerifyRFID_resp(uint8_t i)
{
  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "CheckRegidTag";
  if (i == 1)
  {
    tx_doc["status"] = "Accepted";
  }
  else
  {
    tx_doc["status"] = "Rejected";
  }
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}

void slaveStoreRFID()
{
  // store to DB here
  /*
   * @brief : Use preferences to maintain a txn_id counter which rotates from 1 to 100.
   */
  rfid_table.begin("rfid_table", false);
  int rid = rfid_table.getInt("current_RFID", 0);

  if (rid == 0)
  {
    rid = 1;
  }
  else
  {
    rid = rid + 1; // increment the count
  }

  if (rid >= 4)
  {
    rid = 1;
  }

  String rfid_ = rx_doc["idTag"];
  uint8_t r = localStoreObject->db_set_rfid(rfid_, String(rid));
  if (r == 1)
  {
    rfid_table.putInt("current_RFID", rid); // update only when it is a success.
  }
  rfid_table.end();

  slaveStoreRFID_resp(r);

  return;
}

void slaveFetchRFIDLIST()
{

  uint8_t r = localStoreObject->db_get_rfid_list();
  slaveFetchRFIDLIST_resp(r);

  return;
}

void slaveStoreRFID_resp(uint8_t i)
{
  tx_doc.clear();

  tx_doc["type"] = "response";
  tx_doc["object"] = "StoreidTag";
  if (i == 1)
  {
    tx_doc["status"] = "Accepted";
  }
  else
  {
    tx_doc["status"] = "Rejected";
  }
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}

void slaveFetchRFIDLIST_resp(uint8_t i)
{
  tx_doc.clear();

  tx_doc["type"] = "response";
  tx_doc["object"] = "FetchidTag";
  if (i == 1)
  {
    tx_doc["status"] = "Accepted";
  }
  else
  {
    tx_doc["status"] = "Rejected";
  }
  for (int i = 0; i < LL_StoredRFIDs.size(); i++)
  {
    String rf = "rfid" + String(i);
    tx_doc[rf] = LL_StoredRFIDs.get(i);
  }
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}

void slave_store_txn(String tid, String rfid, String start_d, String stop_d, String un, String ros)
{
  /*storestarttxn
  txM_doc["type"] = "request";
  txM_doc["object"] = "storestarttxn";
  txM_doc["idTag"] = rfid_tag;
  txM_doc["start_date"] = start_date;
  txM_doc["stop_date"] = stop_date;
  txM_doc["units"] = units;
  txM_doc["reason_of_stop"] = reason_of_stop; */
  uint8_t r = localStoreObject->db_set_txn(tid, rfid, start_d, stop_d, un, ros);

  slave_store_txn_resp(r);
  return;
}

void slave_store_txn_resp(uint8_t i)
{
  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "storestarttxn";
  if (i == 1)
  {
    tx_doc["status"] = "Accepted";
  }
  else
  {
    tx_doc["status"] = "Rejected";
  }
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}

void slave_update_txn(String tid, String rfid, String start_d, String stop_d, String un, String ros)
{
  // Here start_date will be dummy.
  uint8_t r = localStoreObject->db_set_txn(tid, rfid, start_d, stop_d, un, ros);
  slave_update_txn_resp(r);
  return;
}

void slave_update_txn_resp(uint8_t i)
{
  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "storeupdatetxn";
  if (i == 1)
  {
    tx_doc["status"] = "Accepted";
  }
  else
  {
    tx_doc["status"] = "Rejected";
  }
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}

void slaveFetchTxn(String tid)
{
  uint8_t r = localStoreObject->db_get_txn(tid);
  slaveFetchtxn_Resp(r);
  return;
}

void slaveFetchtxn_Resp(uint8_t i)
{
  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "fetchtxn";
  if (i == 1)
  {
    tx_doc["status"] = "Accepted";
  }
  else
  {
    tx_doc["status"] = "Rejected";
  }
  for (int i = 0; i < LL_Local_Stored_List.size(); i++)
  {
    // String rf = "rfid" + String(i);
    // tx_doc[rf] = LL_Local_Stored_List.get(i);
    // Make txn details.
    // Store LL_Local_Stored_List.get(i) in a struct of type stored_sessions
    // Extract individual fields and send the txn.
    /*
    typedef struct StoredSessions
{
    String tid;
    String rfid;
    String start_date;
  String stop_date;
    String units;
  String ros;

}stored_session;
    */
    stored_session ss_t = LL_Local_Stored_List.get(i);
    tx_doc["tid"] = ss_t.tid;
    tx_doc["rfid"] = ss_t.rfid;
    tx_doc["start_date"] = ss_t.start_date;
    tx_doc["stop_date"] = ss_t.stop_date;
    tx_doc["units"] = ss_t.units;
    tx_doc["ros"] = ss_t.ros;
  }
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}

void slaveCP_IN()
{ // Sends ADC value to cloud "Control Pilot Input"
  int stat = 0;
  // StaticJsonDocument tx_doc(200);
  tx_doc.clear();

#if 0
  Serial.println(analogRead(CPIN));
  for (int i = 0; i < 10; i++)
  {
    stat += analogRead(CPIN);
    delay(5);
  }
  stat = stat / 10;
#endif
  stat = ADC_Result;
  Serial.println("Value:-> " + String(stat));
  tx_doc["type"] = "response";
  tx_doc["object"] = "cpin";
  tx_doc["value"] = stat;
  tx_doc["object1"] = "phase";
  tx_doc["value1"] = dis_phase_type;
  tx_doc["object2"] = "LOAD";
  tx_doc["value2"] = device_load;
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}

void slaveCP_OUT(bool action)
{ // Starts PWM on "Control Pilot Output Line",rxL = rx Local
  // StaticJsonDocument tx_doc(200);
  tx_doc.clear();
  // int action = rxL_doc["action"];
  // int connectorR = rxL_doc["connectorId"];

  tx_doc["type"] = "response";
  tx_doc["object"] = "cpout";
  charge_request = action;
  if ((action == 1))
  {
    send_start_charge_status();

#if STM_ENABLE
    digitalWrite(STM_PWM, LOW);
#else
    int dutyC = 0;
#if 0
    int dutyC = CalculateDutyCycle(chargingLimit_7S);

    Serial.println("[ControlPilot]Set maximum Current limit->" + String(chargingLimit_7S));
    Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
#endif

    // if (relay_weld_pwm_stop_flag == 0)
    // {
    //   ledcWrite(pwm_channel, dutyC);
    // }
    // ledcWrite(pwm_channel, dutyC);

#endif
    tx_doc["status"] = 1; // ON
    serializeJson(tx_doc, slaveSerial);
    serializeJson(tx_doc, Serial);
    return;
  }
  else if ((action == 0))
  {

    send_power_flag = 0;
    previous_phase = 0;
    relay_restart_flag = 0;
    send_stop_charge_status();
#if STM_ENABLE
    digitalWrite(STM_PWM, HIGH);
#else
#if 1
    ledcWrite(pwm_channel, 255);
#endif
#endif
    tx_doc["status"] = 0; // OFF
    // tx_doc["connectorId"] = connectorR;
    serializeJson(tx_doc, slaveSerial);
    serializeJson(tx_doc, Serial);
    return;
  }
}

#if 0
void slaveFaultBlink(char colourL, int duration, int connectorL)
{
  startFaultBlink = millis();
  while (millis() - startFaultBlink < duration)
  {
    if (connectorL == 1)
    {
      if (colourL == GREEN)
      {
        digitalWrite(GREEN1, HIGH);
        digitalWrite(RED1, LOW);
        digitalWrite(BLUE1, LOW);
      }
      if (colourL == RED)
      {
        digitalWrite(GREEN1, LOW);
        digitalWrite(RED1, HIGH);
        digitalWrite(BLUE1, LOW);
      }
      if (colourL == BLUE)
      {
        digitalWrite(GREEN1, LOW);
        digitalWrite(RED1, LOW);
        digitalWrite(BLUE1, HIGH);
      }
    }
    if (connectorL == 2)
    {
      if (colourL == GREEN)
      {
        digitalWrite(GREEN2, HIGH);
        digitalWrite(RED2, LOW);
        digitalWrite(BLUE2, LOW);
      }
      else if (colourL == RED)
      {
        digitalWrite(GREEN2, LOW);
        digitalWrite(RED2, HIGH);
        digitalWrite(BLUE2, LOW);
      }
      else if (colourL == BLUE)
      {
        digitalWrite(GREEN2, LOW);
        digitalWrite(RED2, LOW);
        digitalWrite(BLUE3, HIGH);
      }
    }
    if (connectorL == 3)
    {
      if (colourL == GREEN)
      {
        digitalWrite(GREEN3, HIGH);
        digitalWrite(RED3, LOW);
        digitalWrite(BLUE3, LOW);
      }
      else if (colourL == RED)
      {
        digitalWrite(GREEN3, LOW);
        digitalWrite(RED3, HIGH);
        digitalWrite(BLUE3, LOW);
      }
      else if (colourL == BLUE)
      {
        digitalWrite(GREEN3, LOW);
        digitalWrite(RED3, LOW);
        digitalWrite(BLUE3, HIGH);
      }
    }
  }
}
#endif

void offlineWhite()
{
  startOfflineWhite = millis();
  while (millis() - startOfflineWhite < 1000)
  {
    digitalWrite(GREEN3, HIGH);
    digitalWrite(RED3, HIGH);
    digitalWrite(BLUE3, HIGH);
    digitalWrite(GREEN2, HIGH);
    digitalWrite(RED2, HIGH);
    digitalWrite(BLUE2, HIGH);
    digitalWrite(GREEN1, HIGH);
    digitalWrite(RED1, HIGH);
    digitalWrite(BLUE1, HIGH);
  }
}

void slaveConnectorId()
{
  //  StaticJsonDocument tx_doc(200);
  tx_doc.clear();
  int connectorId = 0;
  int startTime = millis();
  state_timer = 0;
  while (millis() - startTime < 20000)
  {
    // Add logic to blink for 1 second.
    if (millis() - startTime < 1000)
    {
      digitalWrite(RED2, LOW);
      digitalWrite(GREEN2, LOW);
      digitalWrite(BLUE2, HIGH);
      digitalWrite(RED1, LOW);
      digitalWrite(GREEN1, LOW);
      digitalWrite(BLUE1, HIGH);
      digitalWrite(RED3, LOW);
      digitalWrite(GREEN3, LOW);
      digitalWrite(BLUE3, HIGH);
    }
    else
    {

      stateTimer();
      stateEINS();
      stateZWEI();
      stateDREI();
    }
    int pb1 = digitalRead(PB1);
    // Serial.print("pb1->"+ String(digitalRead(13)));
    int pb2 = digitalRead(PB2);
    // Serial.print("pb2->"+ String(digitalRead(15)));
    int pb3 = digitalRead(PB3);
    // Serial.print("pb3->"+ String(digitalRead(4)));
    if (pb1 == 0 && pb2 == 1 && pb3 == 1)
    {
      connectorId = 1;
    }
    else if (pb1 == 1 && pb2 == 0 && pb3 == 1)
    {
      connectorId = 2;
    }
    else if (pb1 == 1 && pb2 == 1 && pb3 == 0)
    {
      connectorId = 3;
    }
    if (connectorId > 0)
    {
      tx_doc["type"] = "response";
      tx_doc["object"] = "connector";
      tx_doc["connectorId"] = connectorId;
      serializeJson(tx_doc, slaveSerial);
      serializeJson(tx_doc, Serial);
      delay(100);
      Serial.println("\npush button: " + String(connectorId) + " is pressed");
      return;
    }
  }
  tx_doc["type"] = "response";
  tx_doc["object"] = "connector";
  tx_doc["connectorId"] = 0;
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}

void slaveRelay(bool action, char connectorR)
{
  tx_doc.clear();
  // int action = rxL_doc["action"];
  // int connectorR = rxL_doc["connectorId"];

  tx_doc["type"] = "response";
  tx_doc["object"] = "relay";

#if 0

  if ((action == 1) && (connectorR > 0 && connectorR <= 3))
  {
    if (!slave_emgy_flag)
    {
      if (connectorR == 1)
      {
        digitalWrite(RELAY1, HIGH);
      }
      else if (connectorR == 2)
      {
        digitalWrite(RELAY2, HIGH);
      }
      else if (connectorR == 3)
      {
        digitalWrite(RELAY3, HIGH);
      }

      tx_doc["status"] = 1; // ON
    }
    else
    {
      tx_doc["status"] = 0; // OFF
    }

    tx_doc["connectorId"] = connectorR;
    delay(200);
    serializeJson(tx_doc, slaveSerial);
    serializeJson(tx_doc, Serial);
    return;
  }
#endif
  // else if ((action == 0) && (connectorR > 0 && connectorR <= 3))
#if 0
  if ((action == 0) && (connectorR > 0 && connectorR <= 3))
  {
    if (connectorR == 1)
    {
      digitalWrite(RELAY1, LOW);
      delay(100);
      ledcWrite(pwm_channel, 255);
    }
    else if (connectorR == 2)
    {
      digitalWrite(RELAY2, LOW);
      delay(100);
      ledcWrite(pwm_channel, 255);
    }
    else if (connectorR == 3)
    {
      digitalWrite(RELAY3, LOW);
      delay(100);
      ledcWrite(pwm_channel, 255);
    }
    tx_doc["status"] = 0; // OFF
    tx_doc["connectorId"] = connectorR;
    delay(200);
    serializeJson(tx_doc, slaveSerial);
    serializeJson(tx_doc, Serial);
    return;
  }

#endif
  if (connectorR == 0)
  {

    /*
     *@brief ConnectorR = 0 means all connectors i.e. 1,2,3. relay will be off state
     */
    tx_doc["status"] = 0; // Not available
    tx_doc["connectorId"] = 0;
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
    digitalWrite(RELAY3, LOW);
    // delay(100);
    // ledcWrite(pwm_channel, 255);

    delay(200);
    serializeJson(tx_doc, slaveSerial);
    serializeJson(tx_doc, Serial);
    return;
  }
  // else if (connectorR == 0) {
  //   tx_doc["status"] = "N/A";  //Not available
  //   tx_doc["connectorId"] = 0;
  //   delay(200);
  //   serializeJson(tx_doc, slaveSerial);
  //   serializeJson(tx_doc, Serial);
  //   return;
  // }
}

void slaveEmgy()
{
  //  StaticJsonDocument tx_doc(200);
  tx_doc.clear();
  bool stat = digitalRead(EMGY);
  Serial.println("Status:-> " + String(stat));
  tx_doc["type"] = "response";
  tx_doc["object"] = "emgy";

  if (relay_weld_pwm_stop_flag == 1)
  {
    stat = relay_weld_pwm_stop_flag;
  }
  tx_doc["status"] = stat;
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}

/*
  @brief: STATE_check()
  Feature added by G. Raja Sumant
   04/11/2022
   This function behaves independently for switching off
*/

void STATE_check()
{
  const char* type = "";
  const char* object = "";
  int stat = 0;
  if (phase_assign_load_change)
  {
    phase_assign_load_change = 0;
    // if (previous_load != device_load)
    // {
    if (device_load == 3.30f)
    {
      int dutyC = 0;
      dutyC = CalculateDutyCycle(cp_chargelimit_3S);
      Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      Serial.println("phase_assign_load_change 3.3kw assingned");
      ledcWrite(pwm_channel, dutyC);
      previous_load = device_load;
    }
    else if (device_load == 6.60f)
    {
      int dutyC = 0;
      Serial.println("phase_assign_load_change 6.6kw assingned");
      dutyC = CalculateDutyCycle(chargingLimit_6S);
      Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      ledcWrite(pwm_channel, dutyC);
      previous_load = device_load;
    }
    else if (device_load == 7.40f)
    {
      int dutyC = 0;
      dutyC = CalculateDutyCycle(chargingLimit_7S);
      Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      Serial.println("phase_assign_load_change 7.4kw assingned");
      ledcWrite(pwm_channel, dutyC);
      previous_load = device_load;
    }

    // }
  }



#if 0
  //State B
  /*
  ADC result is between 2.8 and 2.33 volts
  else if ((ADC_Result < 3500) && (ADC_Result > 2800))
  {
    PILOT_reading = V_9;
  }
  */
  if ((stat < 4096) && (stat > 2800))
  {
    digitalWrite(RELAY1, LOW);
    return;
  }

  //if((stat < 2100) && (stat > 1551))
  if ((stat < 2100) && (stat > 1800))
  {
    digitalWrite(RELAY1, LOW);
    return;
  }

  if ((stat < 1550) && (stat > 1150))
  {
    digitalWrite(RELAY1, HIGH);
    return;
  }
#endif




#if 1

  if (device_load == 3.30f)
  {
    for (int i = 0; i < 300; i++)
    {
      stat += analogRead(CPIN);
      delayMicroseconds(10);
    }

    stat = stat / 300;
    ADC_Result = stat;

    /* ADC result is between 3.2 and 2.8 volts */
    if ((ADC_Result < evse_state_a_upper_threshold) && /*(ADC_Result > 935)*/ (ADC_Result > evse_state_a_lower_threshold))
    {
      // PILOT_reading = V_12;
      // flag_state_change_eligible = true;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      EVSE_state = STATE_A;
      if (relay_weld_pwm_stop_flag == 1)
      {
        relay_weld_pwm_stop_flag = 0;
        // flag_red_eins = false;
        slave_emgy_flag = false;
        digitalWrite(RED1, LOW);
      }
    }

    /* ADC result is between 2.8 and 2.33 volts */
    else if ((ADC_Result < evse_state_b_upper_threshold) && (ADC_Result > evse_state_b_lower_threshold)) // defualt 2800
    {
      // PILOT_reading = V_9;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      EVSE_state = STATE_B;
    }

    /* ADC result is between 1.49 and 1.32 volts */ // implemented based on observation
#if V_charge_lite1_4
    else if ((ADC_Result < evse_state_sus_upper_threshold) && (ADC_Result > evse_state_sus_lower_threshold))
    {

      // PILOT_reading = V_SUS;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      int dutyC = 0;
      dutyC = CalculateDutyCycle(cp_chargelimit_3S);
      // Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      // Serial.println("phase_assign_load_change 3.3kw assingned");
      ledcWrite(pwm_channel, dutyC);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      // digitalWrite(RELAY1,HIGH);
      sus_active = 1;
      EVSE_state = STATE_SUS;
    }
#else
    else if ((ADC_Result < 1750) && (ADC_Result > 1550))
    {
      // PILOT_reading = V_SUS;
      EVSE_state = STATE_SUS;
    }
#endif

    /* ADC result is between 1.73 and 1.49 volts */
#if V_charge_lite1_4
    else if ((ADC_Result < evse_state_dis_upper_threshold) && (ADC_Result > evse_state_dis_lower_threshold))
    {
      // PILOT_reading = V_DIS;
      charge_request = 0;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      EVSE_state = STATE_DIS;
    }
#else
    else if ((ADC_Result < 2100) && (ADC_Result > 1750))
    {
      // PILOT_reading = V_DIS;
      EVSE_state = STATE_DIS;
    }
#endif
    /* ADC result is between 1.32 and 1.08 volts */
    else if ((ADC_Result < evse_state_c_upper_threshold) && (ADC_Result > evse_state_c_lower_threshold))
    {
      // PILOT_reading = V_6;
      // digitalWrite(RELAY1, LOW);
      // digitalWrite(RELAY2, LOW);
      // digitalWrite(RELAY3, LOW);

      EVSE_state = STATE_C;

      Serial.println("3.3kw C state");
      // charge_request = 1;
#if LBS
      // if (charge_request == 1)
      // {

      Serial.println("3.3kw charge_request");

      // Serial.println("Charge Requested  by LBS1");

      if (received_action == 1)
      {

        Serial.println("3.3kw received_action");

        if (!slave_emgy_flag)
        {

          send_power_flag = 1;
          switch (phase_Type)
          {
            // Serial.println("phase_Type  by LBS1 3.3kw");
            // Serial.println(phase_Type);

          case 0:
            Serial.println("phase_Type R1 by LBS1 3.3kw");
            Serial.println(phase_Type);
            digitalWrite(RELAY2, LOW);
            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY1, HIGH);




            previous_phase = phase_Type;
            received_action = 0;
            break;

          case 1:
            Serial.println("phase_Type R2  by LBS1 3.3kw");
            Serial.println(phase_Type);
            digitalWrite(RELAY1, LOW);


            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY2, HIGH);
            Serial.println("relay test1");
            previous_phase = phase_Type;
            received_action = 0;
            break;

          case 2:
            Serial.println("phase_Type  R3 by LBS1 3.3kw");
            Serial.println(phase_Type);
            digitalWrite(RELAY1, LOW);
            digitalWrite(RELAY2, LOW);
            digitalWrite(RELAY3, HIGH);

            previous_phase = phase_Type;
            received_action = 0;
            break;

          default:
            break;
          }

          tx_doc["status"] = 1; // ON
        }
        else
        {
          tx_doc["status"] = 0; // OFF
        }
        // tx_doc["connectorId"] = connectorR;
        delay(200);
        serializeJson(tx_doc, slaveSerial);
        serializeJson(tx_doc, Serial);
        return;
      }

      if (!slave_emgy_flag)
      {
        if (phase_change_flag_3)
        {

          // switch (relay_change)
          switch (action_phase_type)
          {
          case 0:

            Serial.println("phase_change  R1 by LBS1 3.3kw");
            digitalWrite(RELAY2, LOW);

            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY1, HIGH);

            phase_change_flag_3 = 0;

            break;
          case 1:

            digitalWrite(RELAY1, LOW);
            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY2, HIGH);
            Serial.println("phase_change  R2 by LBS1 3.3kw");


            phase_change_flag_3 = 0;

            break;

          case 2:

            digitalWrite(RELAY1, LOW);
            digitalWrite(RELAY2, LOW);

            digitalWrite(RELAY3, HIGH);
            Serial.println("phase_change  R3 by LBS1 3.3kw");
            phase_change_flag_3 = 0;

            break;

          default:
            break;
          }

        }
      }
      // else
      // {
      if (sus_active)
      {

        switch (relay_restart_flag)
        {
        case 1:
          digitalWrite(RELAY2, LOW);

          digitalWrite(RELAY3, LOW);
          digitalWrite(RELAY1, HIGH);

          Serial.println("relay1 3.3kw sus to charging");
          break;
        case 2:
          digitalWrite(RELAY1, LOW);
          digitalWrite(RELAY3, LOW);
          digitalWrite(RELAY2, HIGH);
          Serial.println("relay test3");
          Serial.println("relay2 3.3kw sus to charging");
          break;

        case 3:
          digitalWrite(RELAY1, LOW);
          digitalWrite(RELAY2, LOW);
          digitalWrite(RELAY3, HIGH);
          Serial.println("relay3 3.3kw sus to charging");
          break;

        default:
          break;
        }
        sus_active = 0;
      }
      // }
      // }

#endif

      // digitalWrite(RELAY1, HIGH);
    }

    /* ADC result is between 1.08 and 0.60 volts */
    else if ((ADC_Result < evse_state_d_upper_threshold) && (ADC_Result > evse_state_d_lower_threshold)) // testing)
    {
      // PILOT_reading = V_3;
      EVSE_state = STATE_D;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      delay(100);
      Serial.println("pwm stop1");
      ledcWrite(pwm_channel, 255);
    }
    else if ((ADC_Result < evse_state_e_threshold)) // 0.4V
    {
      // PILOT_reading = V_UNKNOWN;
      EVSE_state = STATE_E; // error
      // digitalWrite(RELAY1, LOW);
    }
    else if ((ADC_Result < evse_state_e2_upper_threshold) && (ADC_Result > evse_state_e2_lower_threshold)) // testing)
    {
      // PILOT_reading = V_3;
      EVSE_state = V_UNKNOWN;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
    }
    else
    {
      // PILOT_reading = V_UNKNOWN;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      delay(100);
      // Serial.println("pwm stop2");
      // ledcWrite(pwm_channel, 255);
      // Serial.println(F("ADC values are not in range"));
    }
  }
  else if (device_load == 6.60f)
  {

    for (int i = 0; i < 300; i++)
    {
      stat += analogRead(CPIN);
      delayMicroseconds(10);
    }

    stat = stat / 300;
    ADC_Result = stat;

    /* ADC result is between 3.2 and 2.8 volts */
    if ((ADC_Result < evse_state_a_upper_threshold_6) && /*(ADC_Result > 935)*/ (ADC_Result > evse_state_a_lower_threshold_6))
    {
      // PILOT_reading = V_12;
      // flag_state_change_eligible = true;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      EVSE_state = STATE_A;
      if (relay_weld_pwm_stop_flag == 1)
      {
        relay_weld_pwm_stop_flag = 0;
        // flag_red_eins = false;
        slave_emgy_flag = false;
        digitalWrite(RED1, LOW);
      }
    }

    /* ADC result is between 2.8 and 2.33 volts */
    else if ((ADC_Result < evse_state_b_upper_threshold_6) && (ADC_Result > evse_state_b_lower_threshold_6)) // defualt 2800
    {
      // PILOT_reading = V_9;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      EVSE_state = STATE_B;
    }

    /* ADC result is between 1.49 and 1.32 volts */ // implemented based on observation
#if V_charge_lite1_4
    else if ((ADC_Result < evse_state_sus_upper_threshold_6) && (ADC_Result > evse_state_sus_lower_threshold_6))
    {

      // PILOT_reading = V_SUS;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      int dutyC = 0;
      // Serial.println("phase_assign_load_change 6.6kw assingned");
      dutyC = CalculateDutyCycle(chargingLimit_6S);
      // Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      ledcWrite(pwm_channel, dutyC);
      // digitalWrite(RELAY1,HIGH);
      sus_active = 1;
      EVSE_state = STATE_SUS;
      Serial.println("sus_state");
    }
#else
    else if ((ADC_Result < 1750) && (ADC_Result > 1550))
    {
      // PILOT_reading = V_SUS;
      EVSE_state = STATE_SUS;
    }
#endif

    /* ADC result is between 1.73 and 1.49 volts */
#if V_charge_lite1_4
    else if ((ADC_Result < evse_state_dis_upper_threshold_6) && (ADC_Result > evse_state_dis_lower_threshold_6))
    {
      // PILOT_reading = V_DIS;
      charge_request = 0;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      EVSE_state = STATE_DIS;
    }
#else
    else if ((ADC_Result < 2100) && (ADC_Result > 1750))
    {
      // PILOT_reading = V_DIS;
      EVSE_state = STATE_DIS;
    }
#endif
    /* ADC result is between 1.32 and 1.08 volts */
    else if ((ADC_Result < evse_state_c_upper_threshold_6) && (ADC_Result > evse_state_c_lower_threshold_6))
    {
      // digitalWrite(RELAY1, LOW);
      // digitalWrite(RELAY2, LOW);
      // digitalWrite(RELAY3, LOW);
      // PILOT_reading = V_6;
      EVSE_state = STATE_C;
      // Serial.println("6.6kw c state");

#if LBS
      // if (charge_request == 1)
      // {

        // Serial.println("Charge Requested  by LBS1");

      if (received_action == 1)
      {

        if (!slave_emgy_flag)
        {
          send_power_flag = 1;
          switch (phase_Type)
          {


          case 0:
            digitalWrite(RELAY2, LOW);
            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY1, HIGH);


            Serial.println("phase_Type r1 by LBS1 6.6kw");
            previous_phase = phase_Type;
            received_action = 0;
            break;

          case 1:
            digitalWrite(RELAY1, LOW);


            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY2, HIGH);
            Serial.println("phase_Type r2 by LBS1 6.6kw");
            previous_phase = phase_Type;
            received_action = 0;
            break;

          case 2:
            digitalWrite(RELAY1, LOW);
            digitalWrite(RELAY2, LOW);
            digitalWrite(RELAY3, HIGH);

            Serial.println("phase_Type r3 by LBS1 6.6kw");
            previous_phase = phase_Type;
            received_action = 0;
            break;

          default:
            break;
          }
          tx_doc["status"] = 1; // ON
        }
        else
        {
          tx_doc["status"] = 0; // OFF
        }
        // tx_doc["connectorId"] = connectorR;
        delay(200);
        serializeJson(tx_doc, slaveSerial);
        serializeJson(tx_doc, Serial);
        return;
      }
      if (!slave_emgy_flag)
      {
        // Serial.println("phase_change_flag_1  by LBS3");
        if (phase_change_flag_6)
        {

          // Serial.println("phase_change_flag_1  by LBS4");

          Serial.println("relaY_change " + String(relay_change));
          // switch (relay_change)
          switch (action_phase_type)
          {
          case 0:

            Serial.println("relay1 phase change to charging");

            digitalWrite(RELAY2, LOW);

            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY1, HIGH);
            phase_change_flag_6 = 0;

            break;
          case 1:

            digitalWrite(RELAY1, LOW);

            Serial.println("relay2 phase change to charging");
            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY2, HIGH);
            // Serial.println("relay test5");
            phase_change_flag_6 = 0;

            break;

          case 2:

            digitalWrite(RELAY1, LOW);
            digitalWrite(RELAY2, LOW);
            Serial.println("relay3 phase change to charging");
            digitalWrite(RELAY3, HIGH);
            phase_change_flag_6 = 0;

            break;

          default:
            break;
          }

        }
      }
      // else
      // {
      if (sus_active)
      {
        switch (relay_restart_flag)
        {
        case 1:
          digitalWrite(RELAY2, LOW);
          digitalWrite(RELAY3, LOW);
          digitalWrite(RELAY1, HIGH);
          Serial.println("relay1 sus to charging 6.6kw");
          break;
        case 2:
          digitalWrite(RELAY1, LOW);
          digitalWrite(RELAY3, LOW);
          digitalWrite(RELAY2, HIGH);
          Serial.println("relay test6");
          Serial.println("relay2 sus to charging 6.6kw");
          break;

        case 3:
          digitalWrite(RELAY1, LOW);
          digitalWrite(RELAY2, LOW);
          digitalWrite(RELAY3, HIGH);
          Serial.println("relay3 sus to charging 6.6kw");
          break;

        default:
          break;
        }
        sus_active = 0;
      }
      // }
      // }

#endif

      // digitalWrite(RELAY1, HIGH);
    }

    /* ADC result is between 1.08 and 0.60 volts */
    else if ((ADC_Result < evse_state_d_upper_threshold_6) && (ADC_Result > evse_state_d_lower_threshold_6)) // testing)
    {
      // PILOT_reading = V_3;
      EVSE_state = STATE_D;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      delay(100);
      Serial.println("pwm stop3");
      ledcWrite(pwm_channel, 255);
    }
    else if ((ADC_Result < evse_state_e_threshold_6)) // 0.4V
    {
      // PILOT_reading = V_UNKNOWN;
      EVSE_state = STATE_E; // error
      // digitalWrite(RELAY1, LOW);
    }
    else if ((ADC_Result < evse_state_e2_upper_threshold_6) && (ADC_Result > evse_state_e2_lower_threshold_6)) // testing)
    {
      // PILOT_reading = V_3;
      EVSE_state = V_UNKNOWN;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
    }
    else
    {
      // PILOT_reading = V_UNKNOWN;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      delay(100);
      // Serial.println("pwm stop4");
      // ledcWrite(pwm_channel, 255);
      // Serial.println(F("ADC values are not in range"));
    }



  }

#endif

  else if (device_load == 7.40f)
  {
    for (int i = 0; i < 300; i++)
    {
      stat += analogRead(CPIN);
      delayMicroseconds(10);
    }

    stat = stat / 300;
    ADC_Result = stat;

    // Serial.println("device_load 7.40 kw");
    /* ADC result is between 3.2 and 2.8 volts */
    if ((ADC_Result < evse_state_a_upper_threshold_7) && /*(ADC_Result > 935)*/ (ADC_Result > evse_state_a_lower_threshold_7))
    {
      // PILOT_reading = V_12;
      // flag_state_change_eligible = true;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      EVSE_state = STATE_A;
      if (relay_weld_pwm_stop_flag == 1)
      {
        relay_weld_pwm_stop_flag = 0;
        // flag_red_eins = false;
        slave_emgy_flag = false;
        digitalWrite(RED1, LOW);
      }
    }

    /* ADC result is between 2.8 and 2.33 volts */
    else if ((ADC_Result < evse_state_b_upper_threshold_7) && (ADC_Result > evse_state_b_lower_threshold_7)) // defualt 2800
    {
      // PILOT_reading = V_9;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      EVSE_state = STATE_B;
    }

    /* ADC result is between 1.49 and 1.32 volts */ // implemented based on observation
#if V_charge_lite1_4
    else if ((ADC_Result < evse_state_sus_upper_threshold_7) && (ADC_Result > evse_state_sus_lower_threshold_7))
    {

      // PILOT_reading = V_SUS;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      int dutyC = 0;
      dutyC = CalculateDutyCycle(chargingLimit_7S);
      // Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      // Serial.println("phase_assign_load_change 7.4kw assingned");
      ledcWrite(pwm_channel, dutyC);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      // digitalWrite(RELAY1,HIGH);
      sus_active = 1;
      EVSE_state = STATE_SUS;
    }
#else
    else if ((ADC_Result < 1750) && (ADC_Result > 1550))
    {
      // PILOT_reading = V_SUS;
      EVSE_state = STATE_SUS;
    }
#endif

    /* ADC result is between 1.73 and 1.49 volts */
#if V_charge_lite1_4
    else if ((ADC_Result < evse_state_dis_upper_threshold_7) && (ADC_Result > evse_state_dis_lower_threshold_7))
    {
      // PILOT_reading = V_DIS;
      charge_request = 0;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
      EVSE_state = STATE_DIS;
    }
#else
    else if ((ADC_Result < 2100) && (ADC_Result > 1750))
    {
      // PILOT_reading = V_DIS;
      EVSE_state = STATE_DIS;
    }
#endif
    /* ADC result is between 1.32 and 1.08 volts */
    else if ((ADC_Result < evse_state_c_upper_threshold_7) && (ADC_Result > evse_state_c_lower_threshold_7))
    {

      // PILOT_reading = V_6;
      EVSE_state = STATE_C;
      // Serial.println("State C 7.4 relay start");

#if LBS
      // if (charge_request == 1)
      // {

        // Serial.println("Charge Requested  by LBS1");

      if (received_action == 1)
      {

        if (!slave_emgy_flag)
        {
          send_power_flag = 1;
          switch (phase_Type)
          {
            // Serial.println("phase_Type  by LBS1");
            // Serial.println(phase_Type);

          case 0:


            digitalWrite(RELAY2, LOW);
            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY1, HIGH);
            Serial.println("phase_Type  r1 by LBS1 7.4kw");
            previous_phase = phase_Type;
            received_action = 0;
            break;

          case 1:
            digitalWrite(RELAY1, LOW);


            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY2, HIGH);
            Serial.println("phase_Type  r2 by LBS1 7.4kw");
            previous_phase = phase_Type;
            received_action = 0;
            break;

          case 2:
            digitalWrite(RELAY1, LOW);
            digitalWrite(RELAY2, LOW);
            digitalWrite(RELAY3, HIGH);

            Serial.println("phase_Type  r3 by LBS1 7.4kw");
            previous_phase = phase_Type;
            received_action = 0;
            break;

          default:
            break;
          }
          tx_doc["status"] = 1; // ON
        }
        else
        {
          tx_doc["status"] = 0; // OFF
        }
        // tx_doc["connectorId"] = connectorR;
        delay(200);
        serializeJson(tx_doc, slaveSerial);
        serializeJson(tx_doc, Serial);
        return;
      }

      if (!slave_emgy_flag)
      {
        if (phase_change_flag_7)
        {

          // switch (relay_change)
          switch (action_phase_type)
          {
          case 0:

            Serial.println("phase_change  r1 by LBS1 7.4kw");
            digitalWrite(RELAY2, LOW);

            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY1, HIGH);
            phase_change_flag_7 = 0;

            break;
          case 1:

            digitalWrite(RELAY1, LOW);


            digitalWrite(RELAY3, LOW);
            digitalWrite(RELAY2, HIGH);
            Serial.println("phase_change  r2 by LBS1 7.4kw");
            phase_change_flag_7 = 0;

            break;

          case 2:


            digitalWrite(RELAY1, LOW);
            digitalWrite(RELAY2, LOW);

            digitalWrite(RELAY3, HIGH);
            Serial.println("phase_change  r3 by LBS1 7.4kw");
            phase_change_flag_7 = 0;

            break;

          default:
            break;
          }

        }
      }
      // else
      // {
      if (sus_active)
      {
        switch (relay_restart_flag)
        {
        case 1:
          digitalWrite(RELAY2, LOW);
          digitalWrite(RELAY3, LOW);
          digitalWrite(RELAY1, HIGH);
          Serial.println("relay1 sus to charging 7.4kw");
          break;
        case 2:
          digitalWrite(RELAY1, LOW);
          digitalWrite(RELAY3, LOW);
          digitalWrite(RELAY2, HIGH);
          Serial.println("relay test9");
          Serial.println("relay2 sus to charging 7.4kw");
          break;

        case 3:
          digitalWrite(RELAY1, LOW);
          digitalWrite(RELAY2, LOW);
          digitalWrite(RELAY3, HIGH);
          Serial.println("relay3 sus to charging 7.4kw");
          break;

        default:
          break;
        }
        sus_active = 0;
      }
      // }
    // }

#endif

      // digitalWrite(RELAY1, HIGH);
    }

    /* ADC result is between 1.08 and 0.60 volts */
    else if ((ADC_Result < evse_state_d_upper_threshold_7) && (ADC_Result > evse_state_d_lower_threshold_7)) // testing)
    {
      // PILOT_reading = V_3;
      EVSE_state = STATE_D;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      delay(100);
      Serial.println("pwm stop5");
      ledcWrite(pwm_channel, 255);
    }
    else if ((ADC_Result < evse_state_e_threshold_7)) // 0.4V
    {
      // PILOT_reading = V_UNKNOWN;
      EVSE_state = STATE_E; // error
      // digitalWrite(RELAY1, LOW);
    }
    else if ((ADC_Result < evse_state_e2_upper_threshold_7) && (ADC_Result > evse_state_e2_lower_threshold_7)) // testing)
    {
      // PILOT_reading = V_3;
      EVSE_state = V_UNKNOWN;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      // delay(100);
      // ledcWrite(pwm_channel, 255);
    }
    else
    {
      // PILOT_reading = V_UNKNOWN;
      digitalWrite(RELAY1, LOW);
      digitalWrite(RELAY2, LOW);
      digitalWrite(RELAY3, LOW);
      delay(100);
      // Serial.println("pwm stop6");
      // ledcWrite(pwm_channel, 255);
      // Serial.println(F("ADC values are not in range"));
    }





  }

}

/*
   @brief: EMGY_check()
   Feature added by G. Raja Sumant
   18/05/2022
   This function behaves independently and is more useful during offline phase
*/

void EMGY_check()
{
  if (digitalRead(EMGY))
  {
    slave_emgy_flag = true;
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
    digitalWrite(RELAY3, LOW);
    delay(100);
    Serial.println("pwm stop7");
    ledcWrite(pwm_channel, 255);
    if (!flag_emgy_once)
    {
      /*digitalWrite(RELAY1, LOW);
        delay(50);
        digitalWrite(RELAY2, LOW);
        delay(50);
        digitalWrite(RELAY3, LOW);
        delay(50);*/
        // slaveRelay(0,1);

        // slaveRelay(0,2);
        // slaveRelay(0,3);
        // slaveLed(BLINKYRED, HIGH, 1);
      digitalWrite(RED1, HIGH);
      digitalWrite(GREEN1, LOW);
      digitalWrite(BLUE1, LOW);
      digitalWrite(RED2, HIGH);
      digitalWrite(GREEN2, LOW);
      digitalWrite(BLUE2, LOW);
      digitalWrite(RED3, HIGH);
      digitalWrite(GREEN3, LOW);
      digitalWrite(BLUE3, LOW);
      saveflags();
      flag_blinkygreen_eins = false;
      flag_blinkyblue_eins = false;
      flag_blinkyred_eins = true; // flag for blinking red light
      flag_blinkywhite_eins = false;
      flag_blinkygreen_zwei = false;
      flag_blinkyblue_zwei = false;
      flag_blinkyred_zwei = true; // flag for blinking red light
      flag_blinkywhite_zwei = false;
      flag_blinkygreen_drei = false;
      flag_blinkyblue_drei = false;
      flag_blinkyred_drei = true; // flag for blinking red light
      flag_blinkywhite_drei = false;
      flag_green_eins = false;
      flag_blue_eins = false;
      flag_red_eins = false;
      flag_white_eins = false;
      flag_orange_eins = false;
      flag_violet_eins = false;

      flag_green_zwei = false;
      flag_blue_zwei = false;
      flag_red_zwei = false;
      flag_white_zwei = false;
      flag_orange_zwei = false;
      flag_violet_zwei = false;

      flag_green_drei = false;
      flag_blue_drei = false;
      flag_red_drei = false;
      flag_white_drei = false;
      flag_orange_drei = false;
      flag_violet_drei = false;

      delay(200);
      /*slaveLed(BLINKYRED, HIGH, 2);
        delay(10);
        slaveLed(BLINKYRED, HIGH, 3);
        delay(10);*/
      flag_emgy_once = true;
      Serial.println(F("*****emergency independent****"));
    }
  }
  else
  {
    if (flag_emgy_once)
    {
      restoreflags();
      // Reset all the above flags
      // digitalWrite(RED1, HIGH);
      // digitalWrite(GREEN1, HIGH);
      // digitalWrite(BLUE1, HIGH);
      /*slaveLed(BLINKYWHITE, HIGH, 1);
        delay(10);
        slaveLed(BLINKYWHITE, HIGH, 2);
        delay(10);
        slaveLed(BLINKYWHITE, HIGH, 3);
        delay(10);*/
    }
    flag_emgy_once = false;
    slave_emgy_flag = false;
  }
}

void restoreflags()
{
  flag_blinkygreen_eins = sflag_blinkygreen_eins;
  flag_blinkyblue_eins = sflag_blinkyblue_eins;
  flag_blinkyred_eins = sflag_blinkyred_eins;
  flag_blinkywhite_eins = sflag_blinkywhite_eins;
  flag_blinkygreen_zwei = sflag_blinkygreen_zwei; // flag for blinking white light
  flag_blinkyblue_zwei = sflag_blinkyblue_zwei;
  flag_blinkyred_zwei = sflag_blinkyred_zwei;
  flag_blinkywhite_zwei = sflag_blinkywhite_zwei;
  flag_blinkygreen_drei = sflag_blinkygreen_drei; // flag for blinking white light
  flag_blinkyblue_drei = sflag_blinkyblue_drei;
  flag_blinkyred_drei = sflag_blinkyred_drei;
  flag_blinkywhite_drei = sflag_blinkywhite_drei;
  flag_green_eins = sflag_green_eins;
  flag_blue_eins = sflag_blue_eins;
  flag_red_eins = sflag_red_eins;
  flag_white_eins = sflag_white_eins;
  flag_orange_eins = sflag_orange_eins;
  flag_violet_eins = sflag_violet_eins;

  flag_green_zwei = sflag_green_zwei;
  flag_blue_zwei = sflag_blue_zwei;
  flag_red_zwei = sflag_red_zwei;
  flag_white_zwei = sflag_white_zwei;
  flag_orange_zwei = sflag_orange_zwei;
  flag_violet_zwei = sflag_violet_zwei;

  flag_green_drei = sflag_green_drei;
  flag_blue_drei = sflag_blue_drei;
  flag_red_drei = sflag_red_drei;
  flag_white_drei = sflag_white_drei;
  flag_orange_drei = sflag_orange_drei;
  flag_violet_drei = sflag_violet_drei;
}

void saveflags()
{
  sflag_blinkygreen_eins = flag_blinkygreen_eins;
  sflag_blinkyblue_eins = flag_blinkyblue_eins;
  sflag_blinkyred_eins = flag_blinkyred_eins;
  sflag_blinkywhite_eins = flag_blinkywhite_eins;
  sflag_blinkygreen_zwei = flag_blinkygreen_zwei; // flag for blinking white light
  sflag_blinkyblue_zwei = flag_blinkyblue_zwei;
  sflag_blinkyred_zwei = flag_blinkyred_zwei;
  sflag_blinkywhite_zwei = flag_blinkywhite_zwei;
  sflag_blinkygreen_drei = flag_blinkygreen_drei; // flag for blinking white light
  sflag_blinkyblue_drei = flag_blinkyblue_drei;
  sflag_blinkyred_drei = flag_blinkyred_drei;
  sflag_blinkywhite_drei = flag_blinkywhite_drei;
  sflag_green_eins = flag_green_eins;
  sflag_blue_eins = flag_blue_eins;
  sflag_red_eins = flag_red_eins;
  sflag_white_eins = flag_white_eins;
  sflag_orange_eins = flag_orange_eins;
  sflag_violet_eins = flag_violet_eins;

  sflag_green_zwei = flag_green_zwei;
  sflag_blue_zwei = flag_blue_zwei;
  sflag_red_zwei = flag_red_zwei;
  sflag_white_zwei = flag_white_zwei;
  sflag_orange_zwei = flag_orange_zwei;
  sflag_violet_zwei = flag_violet_zwei;

  sflag_green_drei = flag_green_drei;
  sflag_blue_drei = flag_blue_drei;
  sflag_red_drei = flag_red_drei;
  sflag_white_drei = flag_white_drei;
  sflag_orange_drei = flag_orange_drei;
  sflag_violet_drei = flag_violet_drei;
}

void slaveGfci()
{
  //  StaticJsonDocument tx_doc(200);
  tx_doc.clear();
  bool stat = digitalRead(GFCI);
  Serial.println("Status:-> " + String(stat));
  tx_doc["type"] = "response";
  tx_doc["object"] = "gfci";
  tx_doc["status"] = stat;
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);
  return;
}
void slaveLed(char colourL, bool actionL, int connectorL)
{

  // StaticJsonDocument tx_doc(200);
  tx_doc.clear();
  Serial.println("**LED**");
  // int colourL = rxL_doc["colour"];
  // int actionL =  rxL_doc["action"];
  // int connectorL = rxL_doc["connectorId"];

  tx_doc["type"] = "response";
  tx_doc["object"] = "led";
  if (!slave_emgy_flag)
  {
#if 0
    if (relay_weld_pwm_stop_flag == 1)
    {
      digitalWrite(RED1, HIGH);
      digitalWrite(GREEN1, LOW);
      digitalWrite(BLUE1, LOW);
    }
    else
#endif
      if (actionL == HIGH)
      {
        if (connectorL == 0)
        {
          flag_blinkygreen_eins = false;
          flag_blinkyblue_eins = false;
          flag_blinkyred_eins = true; // flag for blinking red light
          flag_blinkywhite_eins = false;
          flag_blinkygreen_zwei = false;
          flag_blinkyblue_zwei = false;
          flag_blinkyred_zwei = true; // flag for blinking red light
          flag_blinkywhite_zwei = false;
          flag_blinkygreen_drei = false;
          flag_blinkyblue_drei = false;
          flag_blinkyred_drei = true; // flag for blinking red light
          flag_blinkywhite_drei = false;
          flag_green_eins = false;
          flag_blue_eins = false;
          flag_red_eins = false;
          flag_white_eins = false;
          flag_orange_eins = false;
          flag_violet_eins = false;

          flag_green_zwei = false;
          flag_blue_zwei = false;
          flag_red_zwei = false;
          flag_white_zwei = false;
          flag_orange_zwei = false;
          flag_violet_zwei = false;

          flag_green_drei = false;
          flag_blue_drei = false;
          flag_red_drei = false;
          flag_white_drei = false;
          flag_orange_drei = false;
          flag_violet_drei = false;
          tx_doc["colour"] = "BLINKYREDALL";
          // RESPONSE TO MASTER
          tx_doc["status"] = HIGH;
          tx_doc["connectorId"] = connectorL;
          serializeJson(tx_doc, slaveSerial);
          serializeJson(tx_doc, Serial);
          return;
        }
        if (connectorL == 1)
        {
          if (colourL == BLINKYGREEN)
          {
            flag_blinkygreen_eins = true; // flag for blinking green light
            flag_blinkyblue_eins = false;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = false;
            flag_green_eins = false;
            flag_blue_eins = false;
            flag_red_eins = false;
            flag_white_eins = false;
            flag_orange_eins = false;
            flag_violet_eins = false;
            tx_doc["colour"] = "BLINKYGREEN_EINS";
          }
          if (colourL == BLINKYWHITE)
          {
            flag_blinkygreen_eins = false; // flag for blinking white light
            flag_blinkyblue_eins = false;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = true;
            flag_green_eins = false;
            flag_blue_eins = false;
            flag_red_eins = false;
            flag_white_eins = false;
            flag_orange_eins = false;
            flag_violet_eins = false;
            tx_doc["colour"] = "BLINKYWHITE_EINS";
          }
          if (colourL == BLINKYWHITE_ALL)
          {
            rfid = true;
            flag_blinkygreen_eins = false; // flag for blinking white light
            flag_blinkyblue_eins = false;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = true;
            flag_blinkygreen_zwei = false; // flag for blinking white light
            flag_blinkyblue_zwei = false;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = true;
            flag_blinkygreen_drei = false; // flag for blinking white light
            flag_blinkyblue_drei = false;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = true;
            flag_green_eins = false;
            flag_blue_eins = false;
            flag_red_eins = false;
            flag_white_eins = false;
            flag_orange_eins = false;
            flag_violet_eins = false;

            flag_green_zwei = false;
            flag_blue_zwei = false;
            flag_red_zwei = false;
            flag_white_zwei = false;
            flag_orange_zwei = false;
            flag_violet_zwei = false;

            flag_green_drei = false;
            flag_blue_drei = false;
            flag_red_drei = false;
            flag_white_drei = false;
            flag_orange_drei = false;
            flag_violet_drei = false;
            tx_doc["colour"] = "BLINKYWHITE_EINS";
          }
          if (colourL == BLINKYBLUE)
          {
            flag_blinkygreen_eins = false;
            flag_blinkyblue_eins = true;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = false;
            flag_green_eins = false;
            flag_blue_eins = false;
            flag_red_eins = false;
            flag_white_eins = false;
            flag_orange_eins = false;
            flag_violet_eins = false;
            tx_doc["colour"] = "BLINKYBLUE_EINS";
          }
          if (colourL == BLINKYRED)
          {
            flag_blinkygreen_eins = false;
            flag_blinkyred_eins = true;
            flag_blinkyblue_eins = false;
            flag_blinkywhite_eins = false;
            flag_green_eins = false;
            flag_blue_eins = false;
            flag_red_eins = false;
            flag_white_eins = false;
            flag_orange_eins = false;
            flag_violet_eins = false;
            tx_doc["colour"] = "BLINKYRED_EINS";
          }
          if (colourL == RED)
          {
            digitalWrite(RED1, HIGH);
            digitalWrite(GREEN1, LOW);
            digitalWrite(BLUE1, LOW);
            flag_blinkygreen_eins = false; // flag for blinking green light
            flag_blinkyblue_eins = false;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = false;
            flag_red_eins = true;
            flag_green_eins = false;
            flag_blue_eins = false;
            flag_white_eins = false;
            flag_orange_eins = false;
            flag_violet_eins = false;
            tx_doc["colour"] = "RED";
          }
          if (colourL == GREEN)
          {
            digitalWrite(RED1, LOW);
            digitalWrite(GREEN1, HIGH);
            digitalWrite(BLUE1, LOW);
            flag_blinkygreen_eins = false;
            flag_blinkyblue_eins = false;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = false;
            flag_green_eins = true;
            flag_blue_eins = false;
            flag_red_eins = false;
            flag_white_eins = false;
            flag_orange_eins = false;
            flag_violet_eins = false;
            tx_doc["colour"] = "GREEN";
          }
          if (colourL == BLUE)
          {
            digitalWrite(RED1, LOW);
            digitalWrite(GREEN1, LOW);
            digitalWrite(BLUE1, HIGH);
            flag_blinkygreen_eins = false;
            flag_blinkyblue_eins = false;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = false;
            flag_blue_eins = true;
            flag_green_eins = false;
            flag_red_eins = false;
            flag_white_eins = false;
            flag_orange_eins = false;
            flag_violet_eins = false;
            tx_doc["colour"] = "BLUE";
          }
          if (colourL == ORANGE)
          {
            digitalWrite(RED1, HIGH);
            digitalWrite(GREEN1, HIGH);
            digitalWrite(BLUE1, LOW);
            flag_blinkygreen_eins = false;
            flag_blinkyblue_eins = false;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = false;
            flag_orange_eins = true;
            flag_blue_eins = false;
            flag_green_eins = false;
            flag_white_eins = false;
            flag_red_eins = false;
            flag_violet_eins = false;

            tx_doc["colour"] = "ORANGE";
          }
          if (colourL == VOILET)
          {
            digitalWrite(RED1, HIGH);
            digitalWrite(GREEN1, LOW);
            digitalWrite(BLUE1, HIGH);
            flag_blinkygreen_eins = false;
            flag_blinkyblue_eins = false;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = false;
            flag_violet_eins = true;
            flag_green_eins = false;
            flag_blue_eins = false;
            flag_red_eins = false;
            flag_white_eins = false;
            flag_orange_eins = false;
            tx_doc["colour"] = "VOILET";
          }
          if (colourL == WHITE)
          {
            digitalWrite(RED1, HIGH);
            digitalWrite(GREEN1, HIGH);
            digitalWrite(BLUE1, HIGH);
            flag_blinkygreen_eins = false;
            flag_blinkyblue_eins = false;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = false;
            flag_white_eins = true;
            flag_green_eins = false;
            flag_blue_eins = false;
            flag_red_eins = false;
            flag_orange_eins = false;
            flag_violet_eins = false;
            tx_doc["colour"] = "WHITE";
          }
          tx_doc["status"] = HIGH;
          tx_doc["connectorId"] = connectorL;
          serializeJson(tx_doc, slaveSerial);
          serializeJson(tx_doc, Serial);
          return;
        }

        if (connectorL == 4)
        {

          if (colourL == BLINKYBLUE_ALL)
          {
            flag_blinkygreen_eins = false; // flag for blinking blue light
            flag_blinkyblue_eins = true;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = false;
            flag_blinkygreen_zwei = false; // flag for blinking blue light
            flag_blinkyblue_zwei = true;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = false;
            flag_blinkygreen_drei = false; // flag for blinking blue light
            flag_blinkyblue_drei = true;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = false;
            tx_doc["colour"] = "BLINKYBLUE_EINS";
            tx_doc["object"] = "rfid";
          }
          if (colourL == BLINKYWHITE_ALL)
          {
            flag_blinkygreen_eins = false; // flag for blinking white light
            flag_blinkyblue_eins = false;
            flag_blinkyred_eins = false;
            flag_blinkywhite_eins = true;
            flag_blinkygreen_zwei = false; // flag for blinking white light
            flag_blinkyblue_zwei = false;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = true;
            flag_blinkygreen_drei = false; // flag for blinking white light
            flag_blinkyblue_drei = false;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = true;
            tx_doc["colour"] = "BLINKYWHITE_EINS";
          }
          tx_doc["status"] = HIGH;
          tx_doc["connectorId"] = connectorL;
          serializeJson(tx_doc, slaveSerial);
          serializeJson(tx_doc, Serial);
          return;
        }
        if (connectorL == 2)
        {
          if (colourL == BLINKYGREEN)
          {
            flag_blinkygreen_zwei = true; // flag for blinking green light
            flag_blinkyblue_zwei = false;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = false;
            flag_green_zwei = false;
            flag_blue_zwei = false;
            flag_red_zwei = false;
            flag_white_zwei = false;
            flag_orange_zwei = false;
            flag_violet_zwei = false;
            tx_doc["colour"] = "BLINKYGREEN";
          }
          if (colourL == BLINKYWHITE)
          {
            flag_blinkygreen_zwei = false; // flag for blinking white light
            flag_blinkyblue_zwei = false;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = true;
            flag_green_zwei = false;
            flag_blue_zwei = false;
            flag_red_zwei = false;
            flag_white_zwei = false;
            flag_orange_zwei = false;
            flag_violet_zwei = false;
            tx_doc["colour"] = "BLINKYWHITE";
          }
          if (colourL == BLINKYBLUE)
          {
            flag_blinkygreen_zwei = false;
            flag_blinkyblue_zwei = true;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = false;
            flag_green_zwei = false;
            flag_blue_zwei = false;
            flag_red_zwei = false;
            flag_white_zwei = false;
            flag_orange_zwei = false;
            flag_violet_zwei = false;
            tx_doc["colour"] = "BLINKYBLUE";
          }
          if (colourL == BLINKYRED)
          {
            flag_blinkygreen_zwei = false;
            flag_blinkyred_zwei = true;
            flag_blinkyblue_zwei = false;
            flag_blinkywhite_zwei = false;
            flag_green_zwei = false;
            flag_blue_zwei = false;
            flag_red_zwei = false;
            flag_white_zwei = false;
            flag_orange_zwei = false;
            flag_violet_zwei = false;
            tx_doc["colour"] = "BLINKYRED";
          }
          if (colourL == RED)
          {
            digitalWrite(RED2, HIGH);
            digitalWrite(GREEN2, LOW);
            digitalWrite(BLUE2, LOW);
            tx_doc["colour"] = "RED";
            flag_blinkygreen_zwei = false;
            flag_blinkyblue_zwei = false;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = false;
            flag_green_zwei = false;
            flag_blue_zwei = false;
            flag_white_zwei = false;
            flag_orange_zwei = false;
            flag_violet_zwei = false;
            flag_red_zwei = true;
          }
          if (colourL == GREEN)
          {
            digitalWrite(RED2, LOW);
            digitalWrite(GREEN2, HIGH);
            digitalWrite(BLUE2, LOW);
            tx_doc["colour"] = "GREEN";
            flag_blinkygreen_zwei = false;
            flag_blinkyblue_zwei = false;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = false;
            flag_green_zwei = true;
            flag_blue_zwei = false;
            flag_red_zwei = false;
            flag_white_zwei = false;
            flag_orange_zwei = false;
            flag_violet_zwei = false;
          }
          if (colourL == BLUE)
          {
            digitalWrite(RED2, LOW);
            digitalWrite(GREEN2, LOW);
            digitalWrite(BLUE2, HIGH);
            tx_doc["colour"] = "BLUE";
            flag_blinkygreen_zwei = false;
            flag_blinkyblue_zwei = false;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = false;
            flag_blue_zwei = true;
            flag_green_zwei = false;
            flag_red_zwei = false;
            flag_white_zwei = false;
            flag_orange_zwei = false;
            flag_violet_zwei = false;
          }
          if (colourL == ORANGE)
          {
            digitalWrite(RED2, HIGH);
            digitalWrite(GREEN2, HIGH);
            digitalWrite(BLUE2, LOW);
            flag_blinkygreen_zwei = false;
            flag_blinkyblue_zwei = false;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = false;
            flag_orange_zwei = true;
            flag_green_zwei = false;
            flag_blue_zwei = false;
            flag_red_zwei = false;
            flag_white_zwei = false;
            flag_violet_zwei = false;
            tx_doc["colour"] = "ORANGE";
          }
          if (colourL == VOILET)
          {
            digitalWrite(RED2, HIGH);
            digitalWrite(GREEN2, LOW);
            digitalWrite(BLUE2, HIGH);
            flag_blinkygreen_zwei = false;
            flag_blinkyblue_zwei = false;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = false;
            flag_violet_zwei = true;
            flag_green_zwei = false;
            flag_blue_zwei = false;
            flag_red_zwei = false;
            flag_white_zwei = false;
            flag_orange_zwei = false;
            tx_doc["colour"] = "VOILET";
          }

          if (colourL == WHITE)
          {
            digitalWrite(RED2, HIGH);
            digitalWrite(GREEN2, HIGH);
            digitalWrite(BLUE2, HIGH);
            tx_doc["colour"] = "WHITE";
            flag_blinkygreen_zwei = false;
            flag_blinkyblue_zwei = false;
            flag_blinkyred_zwei = false;
            flag_blinkywhite_zwei = false;
            flag_white_zwei = true;
            flag_green_zwei = false;
            flag_blue_zwei = false;
            flag_red_zwei = false;
            flag_orange_zwei = false;
            flag_violet_zwei = false;
          }
          tx_doc["status"] = HIGH;
          tx_doc["connectorId"] = connectorL;
          serializeJson(tx_doc, slaveSerial);
          serializeJson(tx_doc, Serial);
          return;
        }

        if (connectorL == 3)
        {
          if (colourL == BLINKYGREEN)
          {
            flag_blinkygreen_drei = true; // flag for blinking green light
            flag_blinkyblue_drei = false;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = false;
            flag_green_drei = false;
            flag_blue_drei = false;
            flag_red_drei = false;
            flag_white_drei = false;
            flag_orange_drei = false;
            flag_violet_drei = false;
            tx_doc["colour"] = "BLINKYGREEN";
          }
          if (colourL == BLINKYWHITE)
          {
            flag_blinkygreen_drei = false; // flag for blinking white light
            flag_blinkyblue_drei = false;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = true;
            flag_green_drei = false;
            flag_blue_drei = false;
            flag_red_drei = false;
            flag_white_drei = false;
            flag_orange_drei = false;
            flag_violet_drei = false;
            tx_doc["colour"] = "BLINKYWHITE";
          }
          if (colourL == BLINKYBLUE)
          {
            flag_blinkygreen_drei = false;
            flag_blinkyblue_drei = true;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = false;
            flag_green_drei = false;
            flag_blue_drei = false;
            flag_red_drei = false;
            flag_white_drei = false;
            flag_orange_drei = false;
            flag_violet_drei = false;
            tx_doc["colour"] = "BLINKYBLUE";
          }
          if (colourL == BLINKYRED)
          {
            flag_blinkygreen_drei = false;
            flag_blinkyred_drei = true;
            flag_blinkyblue_drei = false;
            flag_blinkywhite_drei = false;
            flag_green_drei = false;
            flag_blue_drei = false;
            flag_red_drei = false;
            flag_white_drei = false;
            flag_orange_drei = false;
            flag_violet_drei = false;
            tx_doc["colour"] = "BLINKYRED";
          }
          if (colourL == RED)
          {
            digitalWrite(RED3, HIGH);
            digitalWrite(GREEN3, LOW);
            digitalWrite(BLUE3, LOW);
            tx_doc["colour"] = "RED";
            flag_blinkygreen_drei = false;
            flag_blinkyblue_drei = false;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = false;
            flag_red_drei = true;
            flag_green_drei = false;
            flag_blue_drei = false;
            flag_white_drei = false;
            flag_orange_drei = false;
            flag_violet_drei = false;
          }
          if (colourL == GREEN)
          {
            digitalWrite(RED3, LOW);
            digitalWrite(GREEN3, HIGH);
            digitalWrite(BLUE3, LOW);
            tx_doc["colour"] = "GREEN";
            flag_blinkygreen_drei = false;
            flag_blinkyblue_drei = false;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = false;
            flag_green_drei = true;
            flag_blue_drei = false;
            flag_red_drei = false;
            flag_white_drei = false;
            flag_orange_drei = false;
            flag_violet_drei = false;
          }
          if (colourL == BLUE)
          {
            digitalWrite(RED3, LOW);
            digitalWrite(GREEN3, LOW);
            digitalWrite(BLUE3, HIGH);
            tx_doc["colour"] = "BLUE";
            flag_blinkygreen_drei = false;
            flag_blinkyblue_drei = false;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = false;
            flag_blue_drei = true;
            flag_green_drei = false;
            flag_red_drei = false;
            flag_white_drei = false;
            flag_orange_drei = false;
            flag_violet_drei = false;
          }
          if (colourL == ORANGE)
          {
            digitalWrite(RED3, HIGH);
            digitalWrite(GREEN3, HIGH);
            digitalWrite(BLUE3, LOW);
            flag_blinkygreen_drei = false;
            flag_blinkyblue_drei = false;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = false;
            flag_orange_drei = true;
            flag_green_drei = false;
            flag_blue_drei = false;
            flag_red_drei = false;
            flag_white_drei = false;
            flag_violet_drei = false;
            tx_doc["colour"] = "ORANGE";
          }
          if (colourL == VOILET)
          {
            digitalWrite(RED3, HIGH);
            digitalWrite(GREEN3, LOW);
            digitalWrite(BLUE3, HIGH);
            flag_blinkygreen_drei = false;
            flag_blinkyblue_drei = false;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = false;
            flag_violet_drei = true;
            flag_green_drei = false;
            flag_blue_drei = false;
            flag_red_drei = false;
            flag_white_drei = false;
            flag_orange_drei = false;
            tx_doc["colour"] = "VOILET";
          }

          if (colourL == WHITE)
          {
            digitalWrite(RED3, HIGH);
            digitalWrite(GREEN3, HIGH);
            digitalWrite(BLUE3, HIGH);
            tx_doc["colour"] = "WHITE";
            flag_blinkygreen_drei = false;
            flag_blinkyblue_drei = false;
            flag_blinkyred_drei = false;
            flag_blinkywhite_drei = false;
            flag_white_drei = true;
            flag_green_drei = false;
            flag_blue_drei = false;
            flag_red_drei = false;
            flag_orange_drei = false;
            flag_violet_drei = false;
          }
          tx_doc["status"] = HIGH;
          tx_doc["connectorId"] = connectorL;
          serializeJson(tx_doc, slaveSerial);
          serializeJson(tx_doc, Serial);
          return;
        }
      }
      else if (actionL == LOW)
      {
        if (connectorL == 1)
        {
          digitalWrite(RED1, LOW);
          digitalWrite(GREEN1, LOW);
          digitalWrite(BLUE1, LOW);
          flag_blinkygreen_eins = false;
          flag_blinkyblue_eins = false;
          flag_blinkyred_eins = false;
          flag_blinkywhite_eins = false;

          flag_blinkygreen_zwei = false;
          flag_blinkyblue_zwei = false;
          flag_blinkyred_zwei = false;
          flag_blinkywhite_zwei = false;

          flag_blinkygreen_drei = false;
          flag_blinkyblue_drei = false;
          flag_blinkyred_drei = false;
          flag_blinkywhite_drei = false;

          flag_green_drei = false;
          flag_blue_drei = false;
          flag_red_drei = false;
          flag_white_drei = false;
          flag_orange_drei = false;
          flag_violet_drei = false;
        }
        if (connectorL == 2)
        {
          digitalWrite(RED2, LOW);
          digitalWrite(GREEN2, LOW);
          digitalWrite(BLUE2, LOW);
          flag_blinkygreen_eins = false;
          flag_blinkyblue_eins = false;
          flag_blinkyred_eins = false;
          flag_blinkywhite_eins = false;

          flag_blinkygreen_zwei = false;
          flag_blinkyblue_zwei = false;
          flag_blinkyred_zwei = false;
          flag_blinkywhite_zwei = false;

          flag_blinkygreen_drei = false;
          flag_blinkyblue_drei = false;
          flag_blinkyred_drei = false;
          flag_blinkywhite_drei = false;

          flag_green_drei = false;
          flag_blue_drei = false;
          flag_red_drei = false;
          flag_white_drei = false;
          flag_orange_drei = false;
          flag_violet_drei = false;
        }
        if (connectorL == 3)
        {
          digitalWrite(RED3, LOW);
          digitalWrite(GREEN3, LOW);
          digitalWrite(BLUE3, LOW);
          flag_blinkygreen_eins = false;
          flag_blinkyblue_eins = false;
          flag_blinkyred_eins = false;
          flag_blinkywhite_eins = false;

          flag_blinkygreen_zwei = false;
          flag_blinkyblue_zwei = false;
          flag_blinkyred_zwei = false;
          flag_blinkywhite_zwei = false;

          flag_blinkygreen_drei = false;
          flag_blinkyblue_drei = false;
          flag_blinkyred_drei = false;
          flag_blinkywhite_drei = false;

          flag_green_drei = false;
          flag_blue_drei = false;
          flag_red_drei = false;
          flag_white_drei = false;
          flag_orange_drei = false;
          flag_violet_drei = false;
        }
        tx_doc["colour"] = "RGB";
        tx_doc["status"] = LOW;
        tx_doc["connectorId"] = connectorL;
        serializeJson(tx_doc, slaveSerial);
        serializeJson(tx_doc, Serial);
        return;
      }
  }
  else
  {
    tx_doc["colour"] = "RED";
    tx_doc["status"] = HIGH;
    tx_doc["connectorId"] = connectorL;
    serializeJson(tx_doc, slaveSerial);
    serializeJson(tx_doc, Serial);
  }
}

unsigned long slaveTime = 0;

void setup()
{
  // deleteFlash();
  // SPIFFS.format();
  //  put your setup code here, to run once:
  /*******Push Buttons**************/
  pinMode(PB1, INPUT);
  pinMode(PB2, INPUT);
  pinMode(PB3, INPUT);

  pinMode(EMGY, INPUT);
  pinMode(GFCI, INPUT);
  /************Relay****************/
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);

  /************Led*****************/
  pinMode(RED1, OUTPUT);
  pinMode(GREEN1, OUTPUT);
  pinMode(BLUE1, OUTPUT);

  pinMode(RED2, OUTPUT);
  pinMode(GREEN2, OUTPUT);
  pinMode(BLUE2, OUTPUT);

  pinMode(RED3, OUTPUT);
  pinMode(GREEN3, OUTPUT);
  pinMode(BLUE3, OUTPUT);

  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);

#if STM_ENABLE
  pinMode(STM_PWM, OUTPUT);
#endif
  // digitalWrite(5,LOW);

  // Setting-up Conrol Pilot
#if CP_ENABLE
  pinMode(CPOUT, OUTPUT);
  ledcSetup(pwm_channel, pwm_freq, pwm_resolution); // configured functionalities

  ledcAttachPin(pwm_pin, pwm_channel);

  analogReadResolution(12);

  ledcWrite(pwm_channel, 255); // State A
#endif
  /*************************/

  Serial.begin(115200);
  Serial.println(F("****NISSAN LEAF SLAVE ****"));

  slaveSerial.begin(9600);

  idTagObject_db->db_init();

  AuthorizationCacheObject_db->AuthorizationCache_db_init();

  // Create a DB for RFID and local storage

  localStoreObject->db_init();

  slaveTime = millis();

  rtc_setup();

#if LBS
  Serial.print(F("Starting Setup FREE HEAP: "));
  Serial.println(ESP.getFreeHeap());

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  delay(5);
  initEspNowSlave();
  Serial.print(F("After Init espnow FREE HEAP: "));
  Serial.println(ESP.getFreeHeap());

  Serial.println("DEVICE->" + String(device_id));
#if 0
  int dutyC = CalculateDutyCycle(chargingLimit_lbs);
  ledcWrite(pwm_channel, dutyC);

  Serial.println("[ControlPilot]Set maximum Current limit->" + String(chargingLimit));
  Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
#endif
#endif
}

void loop()
{

  if (load_change_flag)
  {
    int dutyC = 0;
    load_change_flag = 0;
    if (previous_load != device_load)
    {
      if (device_load == 3.30f)
      {
        Serial.println("load change 3.3kw");
        dutyC = CalculateDutyCycle(cp_chargelimit_3S);
        Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
        ledcWrite(pwm_channel, dutyC);
        delay(3000);
        previous_load = device_load;
      }
      else if (device_load == 6.60f)
      {
        Serial.println("load change 6.6kw");
        dutyC = CalculateDutyCycle(chargingLimit_6S);
        Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
        ledcWrite(pwm_channel, dutyC);
        delay(3000);
        previous_load = device_load;
      }
      else if (device_load == 7.40f)
      {
        Serial.println("load change 7.4kw");
        dutyC = CalculateDutyCycle(chargingLimit_7S);
        Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
        ledcWrite(pwm_channel, dutyC);
        delay(3000);
        previous_load = device_load;
      }
    }


  }

  if (phase_change_flag)
  {
    phase_change_flag = 0;
#if 0
    int dutyC = 0;
    if (device_load == 3.30f)
    {
      Serial.println("load change 3.3kw");
      dutyC = CalculateDutyCycle(cp_chargelimit_3S);
      Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      ledcWrite(pwm_channel, dutyC);
      delay(3000);
      previous_load = device_load;
    }
    else if (device_load == 6.60f)
    {
      Serial.println("load change 6.6kw");
      dutyC = CalculateDutyCycle(chargingLimit_6S);
      Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      ledcWrite(pwm_channel, dutyC);
      delay(3000);
      previous_load = device_load;
    }
    else if (device_load == 7.40f)
    {
      Serial.println("load change 7.4kw");
      dutyC = CalculateDutyCycle(chargingLimit_7S);
      Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      ledcWrite(pwm_channel, dutyC);
      delay(3000);
      previous_load = device_load;
    }
#endif

    if (!slave_emgy_flag)
    {
      ledcWrite(pwm_channel, 255);
      // Serial.println("[ControlPilot]Set maximum Current limit->" + String(chargingLimit));
      Serial.println("previous_phase" + String(previous_phase));
      int dutyC = 0;

      switch (previous_phase)
      {
      case 0:

        Serial.println("case 0");
        digitalWrite(RELAY1, LOW);
        digitalWrite(RELAY2, LOW);
        digitalWrite(RELAY3, LOW);
        delay(3000);


        switch (action_phase_type)
        {
        case 0:
          relay_change = 1;

          if (device_load == 3.30f)
          {
            Serial.println("load change 3.3kw");
            dutyC = CalculateDutyCycle(cp_chargelimit_3S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 6.60f)
          {
            Serial.println("load change 6.6kw");
            dutyC = CalculateDutyCycle(chargingLimit_6S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 7.40f)
          {
            Serial.println("load change 7.4kw");
            dutyC = CalculateDutyCycle(chargingLimit_7S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }


          // digitalWrite(RELAY1, HIGH);
          // digitalWrite(RELAY2, LOW);
          // digitalWrite(RELAY3, LOW);



          break;
        case 1:
          Serial.println("case 1");
          digitalWrite(RELAY1, LOW);
          digitalWrite(RELAY2, LOW);
          digitalWrite(RELAY3, LOW);
          relay_change = 2;
          if (device_load == 3.30f)
          {
            Serial.println("load change 3.3kw");
            dutyC = CalculateDutyCycle(cp_chargelimit_3S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 6.60f)
          {
            Serial.println("load change 6.6kw");
            dutyC = CalculateDutyCycle(chargingLimit_6S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 7.40f)
          {
            Serial.println("load change 7.4kw");
            dutyC = CalculateDutyCycle(chargingLimit_7S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }


          // digitalWrite(RELAY1, LOW);
          // digitalWrite(RELAY2, HIGH);
          // digitalWrite(RELAY3, LOW);

          break;

        case 2:
          Serial.println("case 2");
          digitalWrite(RELAY1, LOW);
          digitalWrite(RELAY2, LOW);
          digitalWrite(RELAY3, LOW);
          relay_change = 3;

          if (device_load == 3.30f)
          {
            Serial.println("load change 3.3kw");
            dutyC = CalculateDutyCycle(cp_chargelimit_3S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 6.60f)
          {
            Serial.println("load change 6.6kw");
            dutyC = CalculateDutyCycle(chargingLimit_6S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 7.40f)
          {
            Serial.println("load change 7.4kw");
            dutyC = CalculateDutyCycle(chargingLimit_7S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }


          // digitalWrite(RELAY3, HIGH);
          // digitalWrite(RELAY2, LOW);
          // digitalWrite(RELAY1, LOW);



          break;

        default:
          break;
        }

        break;

      case 1:

        digitalWrite(RELAY1, LOW);
        digitalWrite(RELAY2, LOW);
        digitalWrite(RELAY3, LOW);
        delay(3000);
        // ledcWrite(pwm_channel, 255);
        switch (action_phase_type)
        {
        case 0:
          relay_change = 1;
          if (device_load == 3.30f)
          {
            Serial.println("load change 3.3kw");
            dutyC = CalculateDutyCycle(cp_chargelimit_3S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 6.60f)
          {
            Serial.println("load change 6.6kw");
            dutyC = CalculateDutyCycle(chargingLimit_6S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 7.40f)
          {
            Serial.println("load change 7.4kw");
            dutyC = CalculateDutyCycle(chargingLimit_7S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }

          // digitalWrite(RELAY1, HIGH);
          // digitalWrite(RELAY2, LOW);
          // digitalWrite(RELAY3, LOW);




          break;
        case 1:
          relay_change = 2;

          if (device_load == 3.30f)
          {
            Serial.println("load change 3.3kw");
            dutyC = CalculateDutyCycle(cp_chargelimit_3S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 6.60f)
          {
            Serial.println("load change 6.6kw");
            dutyC = CalculateDutyCycle(chargingLimit_6S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 7.40f)
          {
            Serial.println("load change 7.4kw");
            dutyC = CalculateDutyCycle(chargingLimit_7S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }

          // digitalWrite(RELAY2, HIGH);
          // digitalWrite(RELAY1, LOW);
          // digitalWrite(RELAY3, LOW);



          break;

        case 2:
          relay_change = 3;
          if (device_load == 3.30f)
          {
            Serial.println("load change 3.3kw");
            dutyC = CalculateDutyCycle(cp_chargelimit_3S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 6.60f)
          {
            Serial.println("load change 6.6kw");
            dutyC = CalculateDutyCycle(chargingLimit_6S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 7.40f)
          {
            Serial.println("load change 7.4kw");
            dutyC = CalculateDutyCycle(chargingLimit_7S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }



          // digitalWrite(RELAY3, HIGH);
          // digitalWrite(RELAY2, LOW);
          // digitalWrite(RELAY1, LOW);


          break;

        default:
          break;
        }

        break;

      case 2:

        digitalWrite(RELAY1, LOW);
        digitalWrite(RELAY2, LOW);
        digitalWrite(RELAY3, LOW);
        delay(3000);
        // ledcWrite(pwm_channel, 255);
        switch (action_phase_type)
        {
        case 0:
          relay_change = 1;

          if (device_load == 3.30f)
          {
            Serial.println("load change 3.3kw");
            dutyC = CalculateDutyCycle(cp_chargelimit_3S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 6.60f)
          {
            Serial.println("load change 6.6kw");
            dutyC = CalculateDutyCycle(chargingLimit_6S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 7.40f)
          {
            Serial.println("load change 7.4kw");
            dutyC = CalculateDutyCycle(chargingLimit_7S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }

          // digitalWrite(RELAY1, HIGH);
          // digitalWrite(RELAY2, LOW);
          // digitalWrite(RELAY3, LOW);


          break;
        case 1:
          relay_change = 2;
          if (device_load == 3.30f)
          {
            Serial.println("load change 3.3kw");
            dutyC = CalculateDutyCycle(cp_chargelimit_3S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 6.60f)
          {
            Serial.println("load change 6.6kw");
            dutyC = CalculateDutyCycle(chargingLimit_6S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 7.40f)
          {
            Serial.println("load change 7.4kw");
            dutyC = CalculateDutyCycle(chargingLimit_7S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }

          // digitalWrite(RELAY2, HIGH);
          // digitalWrite(RELAY1, LOW);
          // digitalWrite(RELAY3, LOW);



          break;

        case 2:
          relay_change = 3;
          if (device_load == 3.30f)
          {
            Serial.println("load change 3.3kw");
            dutyC = CalculateDutyCycle(cp_chargelimit_3S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 6.60f)
          {
            Serial.println("load change 6.6kw");
            dutyC = CalculateDutyCycle(chargingLimit_6S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }
          else if (device_load == 7.40f)
          {
            Serial.println("load change 7.4kw");
            dutyC = CalculateDutyCycle(chargingLimit_7S);
            Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
            ledcWrite(pwm_channel, dutyC);
            // delay(3000);
            previous_load = device_load;
          }


          // digitalWrite(RELAY3, HIGH);
          // digitalWrite(RELAY2, LOW);
          // digitalWrite(RELAY1, LOW);

          break;

        default:
          break;
        }

        break;

      default:
        break;
      }
    }

  }
  // if ((load_value++ % 30) == 0)
  // {
  //   Serial.println("running load" + String(device_load));
  // }
#if 0
  int stat = 0;
  for (int i = 0; i < 300; i++)
  {
    stat += analogRead(CPIN);
    delayMicroseconds(10);
  }

  stat = stat / 300;

  Serial.println("[ControlPilot]Set ADC VALUES 6.6->" + String(stat));
#endif
  // put your main code here, to run repeatedly:
  // digitalWrite(RED1,HIGH);
  slave_loop();
  // receivePower();

  /*AuthCacheLoop();
  dBLoop();
  storageLoop();*/
  // prev_states(); // this will be strored and executed only when we receive rfid or button press
  // delay(100); // why the delay?can it be removed?
  //  digitalWrite(RED1,LOW);
  //   Serial.println("SLAVE FREE HEAP");
  //   Serial.println(ESP.getFreeHeap());
  if (send_power_flag == 1)
  {
    if (((count++) % 2003) == 0)
    {
      // count = 0;
      send_power_status();
    }
  }

  stateTimer();
  stateEINS();
  stateZWEI();
  stateDREI();

  //  if(flag_blinkywhite_drei or flag_blinkywhite_zwei or flag_blinkywhite_drei)
  //  {
  //  endTime = 1500;
  //  }
  //  else
  //  {
  //    endTime = 500;
  //  }

  ///*************************EINS**********************************************/
  //  if(flag_blinkygreen_eins == true){
  //        digitalWrite(RED1, LOW);
  //        digitalWrite(GREEN1, HIGH);
  //        digitalWrite(BLUE1, LOW);
  //
  //        delay(500);
  //
  //        digitalWrite(RED1, LOW);
  //        digitalWrite(GREEN1, LOW);
  //        digitalWrite(BLUE1, LOW);
  //
  //        delay(500);
  //
  //  }
  //
  //  if(flag_blinkywhite_eins == true){
  //       digitalWrite(RED1, HIGH);
  //       digitalWrite(GREEN1, HIGH);
  //       digitalWrite(BLUE1, HIGH);
  //       delay(1500);
  //       digitalWrite(RED1, LOW);
  //       digitalWrite(GREEN1, LOW);
  //       digitalWrite(BLUE1, LOW);
  //       delay(1500);
  // }
  //  if(flag_blinkyblue_eins == true){
  //       digitalWrite(RED1, LOW);
  //       digitalWrite(GREEN1,LOW);
  //       digitalWrite(BLUE1,HIGH);
  //       delay(500);
  //       digitalWrite(RED1, LOW);
  //       digitalWrite(GREEN1,LOW);
  //       digitalWrite(BLUE1,LOW);
  //       delay(500);
  // }
  // if(flag_blinkyred_eins == true){
  //      digitalWrite(RED1, HIGH);
  //      digitalWrite(GREEN1,LOW);
  //      digitalWrite(BLUE1,LOW);
  //      delay(500);
  //      digitalWrite(RED1, LOW);
  //      digitalWrite(GREEN1,LOW);
  //      digitalWrite(BLUE1,LOW);
  //      delay(500);
  //  }
  //
  ///*******************************************************************************/
  //
  //
  ///*************************ZWEI**********************************************/
  //  if(flag_blinkygreen_zwei == true){
  //        digitalWrite(RED2, LOW);
  //        digitalWrite(GREEN2, HIGH);
  //        digitalWrite(BLUE2, LOW);
  //
  //        delay(500);
  //
  //        digitalWrite(RED2, LOW);
  //        digitalWrite(GREEN2, LOW);
  //        digitalWrite(BLUE2, LOW);
  //
  //        delay(500);
  //
  //  }
  //
  //  if(flag_blinkywhite_zwei == true){
  //       digitalWrite(RED2, HIGH);
  //       digitalWrite(GREEN2, HIGH);
  //       digitalWrite(BLUE2, HIGH);
  //       delay(1500);
  //       digitalWrite(RED2, LOW);
  //       digitalWrite(GREEN2, LOW);
  //       digitalWrite(BLUE2, LOW);
  //       delay(1500);
  // }
  //  if(flag_blinkyblue_zwei == true){
  //       digitalWrite(RED2, LOW);
  //       digitalWrite(GREEN2,LOW);
  //       digitalWrite(BLUE2,HIGH);
  //       delay(500);
  //       digitalWrite(RED2, LOW);
  //       digitalWrite(GREEN2,LOW);
  //       digitalWrite(BLUE2,LOW);
  //       delay(500);
  // }
  //
  //  if(flag_blinkyred_zwei == true){
  //      digitalWrite(RED2, HIGH);
  //      digitalWrite(GREEN2,LOW);
  //      digitalWrite(BLUE2,LOW);
  //
  //      delay(500);
  //
  //      digitalWrite(RED2, LOW);
  //      digitalWrite(GREEN2,LOW);
  //      digitalWrite(BLUE2,LOW);
  //
  //      delay(500);
  //  }
  //
  ///*******************************************************************************/
  ///*************************DREI*************************************************/
  //  if(flag_blinkygreen_drei == true){
  //        digitalWrite(RED3, LOW);
  //        digitalWrite  (GREEN3, HIGH);
  //        digitalWrite(BLUE3, LOW);
  //
  //        delay(500);
  //
  //        digitalWrite(RED3, LOW);
  //        digitalWrite(GREEN3, LOW);
  //        digitalWrite(BLUE3, LOW);
  //
  //        delay(500);
  //
  //  }
  //
  //  if(flag_blinkywhite_drei == true){
  //       digitalWrite(RED3, HIGH);
  //       digitalWrite(GREEN3, HIGH);
  //       digitalWrite(BLUE3, HIGH);
  //       delay(1500);
  //       digitalWrite(RED3, LOW);
  //       digitalWrite(GREEN3, LOW);
  //       digitalWrite(BLUE3, LOW);
  //       delay(1500);
  // }
  //  if(flag_blinkyblue_drei == true){
  //       digitalWrite(RED3, LOW);
  //       digitalWrite(GREEN3,LOW);
  //       digitalWrite(BLUE3,HIGH);
  //       delay(500);
  //       digitalWrite(RED3, LOW);
  //       digitalWrite(GREEN3,LOW);
  //       digitalWrite(BLUE3,LOW);
  //       delay(500);
  // }
  //
  //  if(flag_blinkyred_drei == true){
  //      digitalWrite(RED3, HIGH);
  //      digitalWrite(GREEN3,LOW);
  //      digitalWrite(BLUE3,LOW);
  //
  //      delay(500);
  //
  //      digitalWrite(RED3, LOW);
  //      digitalWrite(GREEN3,LOW);
  //      digitalWrite(BLUE3,LOW);
  //
  //      delay(500);
  //  }

  /*******************************************************************************/

  //  if(millis() - slaveTime >3000){
  //    slaveTime = millis();
  //    Serial.println("Slave is Active");
  //
  //  }

  // void buttonCheck();

#if LBS
#if 0
  if (phase_assign_load_change)
  {
    Serial.println("Phase Assign Load Change");
    int dutyC = 0;
    phase_assign_load_change = 0;

    // if (!slave_emgy_flag)
    // {
    if (device_load == 3.30f)
    {
      dutyC = CalculateDutyCycle(cp_chargelimit_3S);
      Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      Serial.println("Load 3.3kw assingned");
      ledcWrite(pwm_channel, dutyC);
    }
    else if (device_load == 6.60f)
    {
      Serial.println("Load 6.6kw assingned");
      dutyC = CalculateDutyCycle(chargingLimit_6S);
      Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      ledcWrite(pwm_channel, dutyC);
    }
    else if (device_load == 7.40f)
    {
      dutyC = CalculateDutyCycle(chargingLimit_7S);
      Serial.println("[ControlPilot]Set Duty Cycle->" + String(dutyC));
      Serial.println("Load 7.4kw assingned");
      ledcWrite(pwm_channel, dutyC);
    }
    // }
  }
#endif




#endif
}

void prev_states()
{
  if (rfid)
  {
    prev_flag_blinkygreen_eins = flag_blinkygreen_eins;
    prev_flag_blinkyblue_eins = flag_blinkyblue_eins;
    prev_flag_blinkyred_eins = flag_blinkyred_eins;
    prev_flag_blinkywhite_eins = flag_blinkywhite_eins;

    prev_flag_blinkygreen_zwei = flag_blinkygreen_zwei;
    prev_flag_blinkyblue_zwei = flag_blinkyblue_zwei;
    prev_flag_blinkyred_zwei = flag_blinkyred_zwei;
    prev_flag_blinkywhite_zwei = flag_blinkywhite_zwei;

    prev_flag_blinkygreen_drei = flag_blinkygreen_drei;
    prev_flag_blinkyblue_drei = flag_blinkyblue_drei;
    prev_flag_blinkyred_drei = flag_blinkyred_drei;
    prev_flag_blinkywhite_drei = flag_blinkywhite_drei;
  }
}

void stateEINS()
{
  if (flag_blinkygreen_eins)
  {
    if (ledOff == true)
    {
      digitalWrite(RED1, LOW);
      digitalWrite(GREEN1, HIGH);
      digitalWrite(BLUE1, LOW);
      //  toggleBGE = false;
    }
    else
    {
      digitalWrite(RED1, LOW);
      digitalWrite(GREEN1, LOW);
      digitalWrite(BLUE1, LOW);
      // toggleBGE = true;
    }
  }

  else if (flag_green_eins)
  {

    digitalWrite(RED1, LOW);
    digitalWrite(GREEN1, HIGH);
    digitalWrite(BLUE1, LOW);
  }

  else if (flag_blinkyred_eins)
  {
    // Serial.println(F("Blinky red eins"));
    if (ledOff == true)
    {
      digitalWrite(RED1, HIGH);
      digitalWrite(GREEN1, LOW);
      digitalWrite(BLUE1, LOW);

      digitalWrite(RED2, HIGH);
      digitalWrite(GREEN2, LOW);
      digitalWrite(BLUE2, LOW);

      digitalWrite(RED3, HIGH);
      digitalWrite(GREEN3, LOW);
      digitalWrite(BLUE3, LOW);
      //  toggleBRE = false;
    }
    else
    {
      digitalWrite(RED1, LOW);
      digitalWrite(GREEN1, LOW);
      digitalWrite(BLUE1, LOW);

      digitalWrite(RED2, LOW);
      digitalWrite(GREEN2, LOW);
      digitalWrite(BLUE2, LOW);

      digitalWrite(RED3, LOW);
      digitalWrite(GREEN3, LOW);
      digitalWrite(BLUE3, LOW);
      // toggleBRE = true;
    }
  }

  else if (flag_red_eins)
  {

    digitalWrite(RED1, HIGH);
    digitalWrite(GREEN1, LOW);
    digitalWrite(BLUE1, LOW);
  }

  else if (flag_blinkyblue_eins)
  {
    if (ledOff == true)
    {
      digitalWrite(RED1, LOW);
      digitalWrite(GREEN1, LOW);
      digitalWrite(BLUE1, HIGH);
      // toggleBBE = false;
    }
    else
    {
      digitalWrite(RED1, LOW);
      digitalWrite(GREEN1, LOW);
      digitalWrite(BLUE1, LOW);
      // toggleBBE = true;
    }
  }

  else if (flag_blue_eins)
  {

    digitalWrite(RED1, LOW);
    digitalWrite(GREEN1, LOW);
    digitalWrite(BLUE1, HIGH);
  }

  else if (flag_blinkywhite_eins)
  {
    if (ledOff == true)
    {
      digitalWrite(RED1, HIGH);
      digitalWrite(GREEN1, HIGH);
      digitalWrite(BLUE1, HIGH);
      // toggleBWE = false;
    }
    else
    {
      digitalWrite(RED1, LOW);
      digitalWrite(GREEN1, LOW);
      digitalWrite(BLUE1, LOW);
      // toggleBWE = true;
    }
  }
  else if (flag_white_eins)
  {

    digitalWrite(RED1, HIGH);
    digitalWrite(GREEN1, HIGH);
    digitalWrite(BLUE1, HIGH);
  }
}

void stateZWEI()
{
  if (flag_blinkygreen_zwei)
  {
    if (ledOff == true)
    {
      digitalWrite(RED2, LOW);
      digitalWrite(GREEN2, HIGH);
      digitalWrite(BLUE2, LOW);
      // toggleBGZ = false;
    }
    else
    {
      digitalWrite(RED2, LOW);
      digitalWrite(GREEN2, LOW);
      digitalWrite(BLUE2, LOW);
      // toggleBGZ = true;
    }
  }

  else if (flag_green_zwei)
  {
    digitalWrite(RED2, LOW);
    digitalWrite(GREEN2, HIGH);
    digitalWrite(BLUE2, LOW);
    // toggleBGZ = false;
  }

  else if (flag_blinkyred_zwei)
  {
    //  Serial.println(F("Blinky red zwei"));
    if (ledOff == true)
    {
      digitalWrite(RED2, HIGH);
      digitalWrite(GREEN2, LOW);
      digitalWrite(BLUE2, LOW);
      // toggleBRZ = false;
    }
    else
    {
      digitalWrite(RED2, LOW);
      digitalWrite(GREEN2, LOW);
      digitalWrite(BLUE2, LOW);
      // toggleBRZ=true;
    }
  }

  else if (flag_red_zwei)
  {

    digitalWrite(RED2, HIGH);
    digitalWrite(GREEN2, LOW);
    digitalWrite(BLUE2, LOW);
  }

  else if (flag_blinkyblue_zwei)
  {
    if (ledOff == true)
    {
      digitalWrite(RED2, LOW);
      digitalWrite(GREEN2, LOW);
      digitalWrite(BLUE2, HIGH);
      // toggleBBZ = false;
    }
    else
    {
      digitalWrite(RED2, LOW);
      digitalWrite(GREEN2, LOW);
      digitalWrite(BLUE2, LOW);
      // toggleBBZ = true;
    }
  }

  else if (flag_blue_zwei)
  {

    digitalWrite(RED2, LOW);
    digitalWrite(GREEN2, LOW);
    digitalWrite(BLUE2, HIGH);
  }

  else if (flag_blinkywhite_zwei)
  {
    if (ledOff == true)
    {
      digitalWrite(RED2, HIGH);
      digitalWrite(GREEN2, HIGH);
      digitalWrite(BLUE2, HIGH);
      // toggleBWZ = false;
    }
    else
    {
      digitalWrite(RED2, LOW);
      digitalWrite(GREEN2, LOW);
      digitalWrite(BLUE2, LOW);
      // toggleBWZ = true;
    }
  }

  else if (flag_white_zwei)
  {

    digitalWrite(RED2, HIGH);
    digitalWrite(GREEN2, HIGH);
    digitalWrite(BLUE2, HIGH);
  }
}

void stateDREI()
{
  if (flag_blinkygreen_drei)
  {
    if (ledOff == true)
    {
      digitalWrite(RED3, LOW);
      digitalWrite(GREEN3, HIGH);
      digitalWrite(BLUE3, LOW);
      // toggleBGD = false;
    }
    else
    {
      digitalWrite(RED3, LOW);
      digitalWrite(GREEN3, LOW);
      digitalWrite(BLUE3, LOW);
      // toggleBGD = true;
    }
  }

  else if (flag_green_drei)
  {

    digitalWrite(RED3, LOW);
    digitalWrite(GREEN3, HIGH);
    digitalWrite(BLUE3, LOW);
  }

  else if (flag_blinkyred_drei)
  {
    //  Serial.println(F("Blinky red"));
    if (ledOff == true)
    {
      digitalWrite(RED3, HIGH);
      digitalWrite(GREEN3, LOW);
      digitalWrite(BLUE3, LOW);
      // toggleBRD = false;
    }
    else
    {
      digitalWrite(RED3, LOW);
      digitalWrite(GREEN3, LOW);
      digitalWrite(BLUE3, LOW);
      // toggleBRD=true;
    }
  }

  else if (flag_red_drei)
  {

    digitalWrite(RED3, HIGH);
    digitalWrite(GREEN3, LOW);
    digitalWrite(BLUE3, LOW);
  }

  else if (flag_blinkyblue_drei)
  {
    if (ledOff == true)
    {
      digitalWrite(RED3, LOW);
      digitalWrite(GREEN3, LOW);
      digitalWrite(BLUE3, HIGH);
      // toggleBBD = false;
    }
    else
    {
      digitalWrite(RED3, LOW);
      digitalWrite(GREEN3, LOW);
      digitalWrite(BLUE3, LOW);
      // toggleBBD = true;
    }
  }

  else if (flag_blue_drei)
  {

    digitalWrite(RED3, LOW);
    digitalWrite(GREEN3, LOW);
    digitalWrite(BLUE3, HIGH);
  }
  else if (flag_blinkywhite_drei)
  {

    if (ledOff == true)
    {
      digitalWrite(RED3, HIGH);
      digitalWrite(GREEN3, HIGH);
      digitalWrite(BLUE3, HIGH);
      // toggleBWD = false;
    }
    else
    {
      digitalWrite(RED3, LOW);
      digitalWrite(GREEN3, LOW);
      digitalWrite(BLUE3, LOW);
      // toggleBWD = true;
    }
  }

  else if (flag_white_drei)
  {

    digitalWrite(RED3, HIGH);
    digitalWrite(GREEN3, HIGH);
    digitalWrite(BLUE3, HIGH);
  }
}

void deleteFlash()
{
  nvs_flash_erase(); // erase the NVS partition and...
  nvs_flash_init();  // initialize the NVS partition.
}

void stateTimer()
{
  switch (state_timer)
  {
  case 0:
    onTime = millis();
    state_timer = 1;
    ledOff = false;
    break;
  case 1:
    if ((millis() - onTime) > 500)
    {
      state_timer = 2;
    }
    break;
  case 2:
    ledOff = true;
    offTime = millis();
    state_timer = 3;
    break;
  case 3:
    if ((millis() - offTime) > 500)
    {
      state_timer = 4;
      // Handle the case for doc being rfid
      /*
         if rfid

         current states = prev states

      */
      /* if (rfid)
        {
         flag_blinkygreen_eins = prev_flag_blinkygreen_eins ;
         flag_blinkyblue_eins = prev_flag_blinkyblue_eins;
         flag_blinkyred_eins = prev_flag_blinkyred_eins;
         flag_blinkywhite_eins = prev_flag_blinkywhite_eins;

         flag_blinkygreen_zwei = prev_flag_blinkygreen_zwei;
         flag_blinkyblue_zwei = prev_flag_blinkyblue_zwei;
         flag_blinkyred_zwei = prev_flag_blinkyred_zwei;
         flag_blinkywhite_zwei = prev_flag_blinkywhite_zwei;

         flag_blinkygreen_drei = prev_flag_blinkygreen_drei;
         flag_blinkyblue_drei = prev_flag_blinkyblue_drei;
         flag_blinkyred_drei = prev_flag_blinkyred_drei;
         flag_blinkywhite_drei = prev_flag_blinkywhite_drei;
         rfid = false;
        }*/
    }

    break;
  case 4:
    state_timer = 0;
    break;
  }
}

/*
  void buttonCheck(){

  bool bt1 = digitalRead(PB1);
  if(bt1 == 0){
    digitalWrite(GREEN1,HIGH);
  }

  bool bt2 = digitalRead(PB2);
if (bt2 == 0) {
digitalWrite(GREEN2, HIGH);
  }

  bool bt3 = digitalRead(PB3);
  if (bt3 == 0) {
    digitalWrite(GREEN3, HIGH);
  }
  }
  **/

  /************************************************* SQL DataBase************************************/

void slavesendlocallist()
{
  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "SendLocalList";
  tx_doc["status"] = "Accepted";
  delay(10);
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);

  return;
}

#if 1
void slavegetlistVersion()
{
  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "GetLocalListVersion";

  Serial.print(F("listVersion"));
  Serial.println(get_Local_Authorization_List.listVersion);

  tx_doc["listVersion"] = String(get_Local_Authorization_List.listVersion);
  tx_doc["status"] = "Accepted";
  delay(10);
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);

  return;
}

void slavegetidtag()
{
  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "GetidTag";

  if (get_Local_Authorization_List.idTag.equals("") == true)
  {
    tx_doc["status"] = "Rejected";
  }
  else
  {
    tx_doc["status"] = "Accepted";
  }

  delay(10);
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);

  return;
}

void slavegetidtagstatus()
{
  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "GetidTagStatus";
  tx_doc["status"] = "Accepted";
  delay(10);
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);

  return;
}

void slavegetupdateType()
{
  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "GetupdateType";
  tx_doc["updateType"] = "Full";

  delay(10);
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);

  return;
}

void slavegetresponses(uint8_t lu8_get_type)
{

  switch (lu8_get_type)
  {
  case GET_LIST_VERSION:
    Serial.println("listVersion");
    slavegetlistVersion();
    break;
#if 0
  case GET_ID_NUM:
    Serial.println("id_number");
    break;
#endif
  case GET_ID_TAG:
    Serial.println("idTag");
    slavegetidtag();
    break;
  case GET_ID_TAG_STATUS:
    Serial.println("idTagStatus");
    break;
  case GET_UPDATE_TYPE:
    Serial.println("updateType");
    slavegetupdateType();
    break;
  default:
    // Serial.println("default");
    break;
  }
}
#endif

void dBLoop()
{
  switch (gu8_dBloop_flag)
  {
  case 1:
    gu8_dBloop_flag = 0;
    idTagObject_db->db_set_table();
    slavesendlocallist();
    break;

  case 2:
    gu8_dBloop_flag = 0;
    idTagObject_db->db_get_table(db_getparam, gu8_get_type);
    slavegetresponses(gu8_get_type);
    break;

  case 3:

    break;

  default:
    gu8_dBloop_flag = 0;
  }
}

void slave_SendAuthCache()
{
  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "SendAuthCache";

  // if (get_Authorization_Cache_List.idTag.equals("") == true)
  if (set_authCache_flag)
  {
    tx_doc["status"] = "Rejected";
  }
  else
  {
    tx_doc["status"] = "Accepted";
  }

  delay(10);
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);

  return;
}

void slave_GetAuthCache()
{
  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "GetAuthCache";

  if (get_authCache_flag)
  {
    tx_doc["status"] = "Accepted";
  }
  else
  {
    tx_doc["status"] = "Rejected";
  }

  delay(10);
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);

  return;
}

void slave_ClearCache()
{

  tx_doc.clear();
  tx_doc["type"] = "response";
  tx_doc["object"] = "ClearCache";

  if (clearcache_flag)
  {
    tx_doc["status"] = "Accepted";
  }
  else
  {
    tx_doc["status"] = "Rejected";
  }

  delay(10);
  serializeJson(tx_doc, slaveSerial);
  serializeJson(tx_doc, Serial);

  return;
}

void AuthCacheLoop()
{
  switch (gu8_authCaheloop_flag)
  {
  case 1:
  {
    gu8_authCaheloop_flag = 0;
    AuthorizationCacheObject_db->AuthorizationCache_db_set_table();
    slave_SendAuthCache();
  }
  break;

  case 2:
  {
    gu8_authCaheloop_flag = 0;
    AuthorizationCacheObject_db->AuthorizationCache_db_get_table(authCache_getparam, gu8_get_auth_type);
    slave_GetAuthCache();
  }
  break;

  case 3:
  {
    gu8_authCaheloop_flag = 0;
    AuthorizationCacheObject_db->AuthorizationCache_db_clear_table();
    slave_ClearCache();
  }

  break;

  default:
    gu8_authCaheloop_flag = 0;
  }
}

/************************************************* SQL DataBase************************************/

/************************************************* local storage***********************************/

void storageLoop()
{
  /*
   * @brief : What do you want to do?
   */
  switch (gu8_storage_Loop)
  {
  case REG_RFID:
    gu8_storage_Loop = 100;
    slaveStoreRFID();
    break;
  case GET_RFID_STORED:
    gu8_storage_Loop = 100;
    slaveFetchRFIDLIST();
    break;
  case PUT_DATA:
    gu8_storage_Loop = 100;
    break;
  case UPDATE_DATA:
    gu8_storage_Loop = 100;
    break;
  case GET_DATA:
    gu8_storage_Loop = 100;
    // slaveFetchTxn();
    break;
  }
}

void checkRelay_Weld()
{

  volatile uint8_t rp;
  volatile uint8_t rn;

  rp = digitalRead(PB1);
  rn = digitalRead(PB2);

  Serial.printf("EVSE_state: %d\r\n", EVSE_state);
  Serial.printf("PRV_EVSE_state: %d\r\n", PRV_EVSE_state);

  if (EVSE_state == STATE_C)
  {
    rp = 1;
    rn = 1;
    gu8state_ctob = 1;
  }
  else if ((EVSE_state == STATE_SUS) && (PRV_EVSE_state == STATE_C))
    // else if (EVSE_state == STATE_SUS)
  {
    /* if (gu8state_ctob == 1)
    {
      gu8state_ctob = 0;
      rp = 1;
      rn = 1;
    } */
    rp = 1;
    rn = 1;
  }
  else if ((EVSE_state == STATE_E) && (PRV_EVSE_state == STATE_C))
    // else if(EVSE_state == STATE_E)
  {
    /* if (gu8state_ctob == 1)
    {
      gu8state_ctob = 0;
      rp = 1;
      rn = 1;
    } */
    rp = 1;
    rn = 1;
  }
  else if ((EVSE_state == STATE_SUS) && (PRV_EVSE_state == STATE_DIS))
    // else if(EVSE_state == STATE_E)
  {
    /* if (gu8state_ctob == 1)
    {
      gu8state_ctob = 0;
      rp = 1;
      rn = 1;
    } */
    rp = 1;
    rn = 1;
  }
  else if ((EVSE_state == STATE_DIS) && (PRV_EVSE_state == STATE_DIS))
  {
    rp = 1;
    rn = 1;
    Serial.printf("STATE_DIS*****");
    // delay(500);
  }

  PRV_EVSE_state = EVSE_state;
  Serial.printf("relay weld phase status: %d\n", rp);
  Serial.printf("relay weld neutral status: %d\n", rn);

  // if (PRV_EVSE_state == EVSE_state)
  // {

  //  if(((rp == 0) && ((getChargePointStatusService_A()->getEvDrawsEnergy() == false)||(EVSE_state == STATE_A )|| (EVSE_state == STATE_B) || (EVSE_state == STATE_D))))
  if (rp == 0) //&& ((getChargePointStatusService_A()->getEvDrawsEnergy() == false)||(EVSE_state == STATE_A )|| (EVSE_state == STATE_B) || (EVSE_state == STATE_D)))
  {
    // STATE_DIS //12
    if ((EVSE_state == STATE_A) || (EVSE_state == STATE_B) || (EVSE_state == STATE_D) || (EVSE_state == STATE_E) || (EVSE_state == STATE_SUS))
      // if((getChargePointStatusService_A()->getEvDrawsEnergy() == false)||(EVSE_state == STATE_A )|| (EVSE_state == STATE_B) || (EVSE_state == STATE_D) || (EVSE_state == STATE_E)||(EVSE_state == STATE_SUS ))
    {
      relay_weld_fault_occ = true;
      relay_weld_pwm_stop_flag = true;
      // slave_emgy_flag = true;
      //  flag_red_eins = true;
      /* digitalWrite(GREEN1, LOW);
      digitalWrite(RED1, HIGH);
      digitalWrite(BLUE1, LOW); */
      // requestforCP_OUT(STOP); // stop pwm
      // EVSE_A_StopSession(); // stop_session new add
#if STM_ENABLE
      digitalWrite(STM_PWM, HIGH);
#else
      Serial.println("pwm stop9");
      ledcWrite(pwm_channel, 255); // stop pwm
#endif
      Serial.println(F("Relay weld phase fault"));
    }
  }
  else if (rn == 0) //&& ((getChargePointStatusService_A()->getEvDrawsEnergy() == false)||(EVSE_state == STATE_A )|| (EVSE_state == STATE_B) || (EVSE_state == STATE_D)))
  {

    if ((EVSE_state == STATE_A) || (EVSE_state == STATE_B) || (EVSE_state == STATE_D) || (EVSE_state == STATE_E) || (EVSE_state == STATE_SUS))
      // if((getChargePointStatusService_A()->getEvDrawsEnergy() == false)||(EVSE_state == STATE_A )|| (EVSE_state == STATE_B) || (EVSE_state == STATE_D)|| (EVSE_state == STATE_E) || (EVSE_state == STATE_SUS ))
    {

      relay_weld_fault_occ = 1;
      relay_weld_pwm_stop_flag = 1;
      // slave_emgy_flag = true;
      /* digitalWrite(GREEN1, LOW);
      digitalWrite(RED1, HIGH);
      digitalWrite(BLUE1, LOW); */
      // requestforCP_OUT(STOP); // stop pwm
      // EVSE_A_StopSession(); // stop_session new add
#if STM_ENABLE
      digitalWrite(STM_PWM, HIGH);
#else
      Serial.println("pwm stop10");
      ledcWrite(pwm_channel, 255); // stop pwm
#endif
      Serial.println(F("Relay weld neutral fault"));
    }
  }
  else
  {

    relay_weld_fault_occ = 0;
  }
  // }
}

#if 0
void checkRelay_Weld() {

  volatile uint8_t rp;
  volatile uint8_t rn;




  if ((EVSE_state != STATE_C) && (EVSE_state != STATE_SUS) && (EVSE_state != STATE_E))
  {
    rp = digitalRead(PB1);
    rn = digitalRead(PB2);

    Serial.printf("EVSE_state: %d\r\n", EVSE_state);
    Serial.printf("PRV_EVSE_state: %d\r\n", PRV_EVSE_state);

#if 0
    if ((PRV_EVSE_state == STATE_C) && ((EVSE_state == STATE_B) || (EVSE_state == STATE_E)))
      // if ((PRV_EVSE_state == STATE_C) == (EVSE_state == STATE_B))
    {
      rp = 1;
      rn = 1;
      gu8state_atob = 1;
      Serial.printf("gu8state_atob: %d\r\n", gu8state_atob);
    }
#endif

  }
  else
  {
    rp = 1;
    rn = 1;
  }

#if 0
  if (gu8state_atob == 1)
  {
    gu16state_atob_count++;
    if (gu16state_atob_count >= 5)
    {
      gu16state_atob_count = 0;
      gu8state_atob = 0;
      rp = 1;
      rn = 1;
      Serial.printf("relay weld phase status: %d\r\n", gu16state_atob_count);
    }

  }
#endif
  /* else{
     rp = digitalRead(PB1);
     rn = digitalRead(PB2);

    // rp=rp;
    // rn=rn;
  } */
  PRV_EVSE_state = EVSE_state;
  Serial.printf("relay weld phase status: %d\n", rp);
  Serial.printf("relay weld neutral status: %d\n", rn);

  //  if(((rp == 0) && ((getChargePointStatusService_A()->getEvDrawsEnergy() == false)||(EVSE_state == STATE_A )|| (EVSE_state == STATE_B) || (EVSE_state == STATE_D))))
  if (rp == 0) //&& ((getChargePointStatusService_A()->getEvDrawsEnergy() == false)||(EVSE_state == STATE_A )|| (EVSE_state == STATE_B) || (EVSE_state == STATE_D)))
  {
    // STATE_DIS //12
    if ((EVSE_state == STATE_A) || (EVSE_state == STATE_B) || (EVSE_state == STATE_D) || (EVSE_state == STATE_E) || (EVSE_state == STATE_SUS))
      //if((getChargePointStatusService_A()->getEvDrawsEnergy() == false)||(EVSE_state == STATE_A )|| (EVSE_state == STATE_B) || (EVSE_state == STATE_D) || (EVSE_state == STATE_E)||(EVSE_state == STATE_SUS ))
    {
      relay_weld_fault_occ = true;
      relay_weld_pwm_stop_flag = true;
      // requestforCP_OUT(STOP); // stop pwm
      //EVSE_A_StopSession(); // stop_session new add
#if STM_ENABLE
      digitalWrite(STM_PWM, HIGH);
#else
      ledcWrite(pwm_channel, 255); // stop pwm
#endif
      Serial.println(F("Relay weld phase fault"));
    }
  }
  else if (rn == 0) //&& ((getChargePointStatusService_A()->getEvDrawsEnergy() == false)||(EVSE_state == STATE_A )|| (EVSE_state == STATE_B) || (EVSE_state == STATE_D)))
  {

    if ((EVSE_state == STATE_A) || (EVSE_state == STATE_B) || (EVSE_state == STATE_D) || (EVSE_state == STATE_E) || (EVSE_state == STATE_SUS))
      // if((getChargePointStatusService_A()->getEvDrawsEnergy() == false)||(EVSE_state == STATE_A )|| (EVSE_state == STATE_B) || (EVSE_state == STATE_D)|| (EVSE_state == STATE_E) || (EVSE_state == STATE_SUS ))
    {

      relay_weld_fault_occ = 1;
      relay_weld_pwm_stop_flag = 1;
      // requestforCP_OUT(STOP); // stop pwm
      //EVSE_A_StopSession(); // stop_session new add
#if STM_ENABLE
      digitalWrite(STM_PWM, HIGH);
#else
      ledcWrite(pwm_channel, 255);  // stop pwm
#endif
      Serial.println(F("Relay weld neutral fault"));
    }
  }
  else
  {

    /* if(relay_weld_pwm_stop_flag==1){

      //relay_weld_fault_occ = false;
      // relay_weld_pwm_stop_flag = false;
       if((EVSE_state == STATE_B) || (EVSE_state == STATE_C)|| (EVSE_state == STATE_D)|| (EVSE_state == STATE_E) || (EVSE_state == STATE_SUS )){
        //requestforCP_OUT(STOP);
  #if STM_ENABLE
        digitalWrite(STM_PWM, HIGH);
  #else
        ledcWrite(pwm_channel, 255);
  #endif

        Serial.println("EVSE_state" + String(STATE_B));
       }
       else
       {
        if(EVSE_state == STATE_A)
        {
          relay_weld_pwm_stop_flag = 0;
        }
       }
   } */
    relay_weld_fault_occ = 0;
  }
}
#endif

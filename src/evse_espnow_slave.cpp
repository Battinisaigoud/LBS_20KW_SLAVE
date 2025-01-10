//// @file evse_espnow_slave.cpp
//// @Author: Gopikrishna S
//// @date 07/11/2024
//// @brief ESP-NOW Slave Code
//// @All Rights Reserved EVRE

#include "evse_espnow_slave.h"

#pragma pack(1)
struct RECIEVEDDATA
{
    char header1[2];
    uint8_t deviceID;
    uint8_t phase;
    uint8_t action;
    float load;
    uint16_t checksum;
    char footer[2];
};
RECIEVEDDATA recieved_data;

// Structure RECIEVEDDATA getting data from another node
#pragma pack(1)
struct SENDDATA
{
    char header1[2] = "P";
    uint8_t deviceID;
    uint8_t phase;
    float power;
    uint8_t status;
    uint16_t checksum;
    char footer[2] = "#";
};
SENDDATA send_data;

enum PhaseType
{
    phase_R,
    phase_Y,
    phase_B
};
enum ChargingStatus
{
    not_charging,
    charging,
    power
};
enum DeviceIdentity
{
    DEVICE1 = 1,
    DEVICE2,
    DEVICE3,
    DEVICE4,
    DEVICE5,
    DEVICE6,
    DEVICE7,
    DEVICE8,
    DEVICE9,
    DEVICE10
};

// REPLACE WITH THE MAC Address of your Receiver/Master
uint8_t MasterMACAdd[6] = { 0xFC, 0xE8, 0xC0, 0x74, 0xA0, 0x18 };
esp_now_peer_info_t master;

enum ActionType
{
    PHASE_ASSIGN,
    PHASE_CHANGE,
    LOAD_CHANGE
};

// Variable to store if sending data was successful
uint8_t sent_status = 11;
uint8_t data_sent = 0;
uint8_t received_action = 0;
volatile  uint8_t phase_Type = 0;
volatile  uint8_t stop_phase = 0;
volatile  uint8_t action_phase_type = 0;

uint8_t dis_phase_type = 0;

volatile float device_load = 7.40f;



uint8_t stop_phase_flag = 0;

uint8_t phase_change_flag = 0;

uint8_t phase_change_flag_3 = 0;
uint8_t phase_change_flag_6 = 0;
uint8_t phase_change_flag_7 = 0;
uint8_t load_change_flag = 0;
uint8_t  test_flag = 0;
uint8_t  test_flag1 = 0;
uint8_t  test_flag2 = 0;

uint8_t phase_assign_load_change = 0;

float PowerRating = 0;
uint8_t device_id = DEVICE1;
extern float received_power;

extern volatile uint8_t relay_restart_flag;

void printMAC(const uint8_t* mac_addr)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.println(macStr);

}

// Callback when data is sent
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status)
{
    memset(&send_data, '\0', sizeof(send_data));
    Serial.print(F("Last Packet Send Status:\t"));
    Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success to " : "Delivery Fail to ");
    printMAC(mac_addr);
    if (status != 0)
    {
        // pairingStatus = NOT_PAIRED;
    }
    if (data_sent == 1)
    {
        sent_status = status;
        data_sent = 0;
    }
}

void OnDataRecv(const uint8_t* mac_addr, const uint8_t* incomingData, int len)
{
    Serial.print("Packet received from LBS: ");
    printMAC(mac_addr);
    Serial.println();

    memcpy(&recieved_data, incomingData, sizeof(recieved_data));

    Serial.println("header : " + String(recieved_data.header1));
    Serial.println("DeviceID : " + String(recieved_data.deviceID));
    Serial.println("Phase : " + String(recieved_data.phase));
    Serial.println("Action : " + String(recieved_data.action));
    // Serial.println("Load : " + String(recieved_data.load));

    switch (recieved_data.action)
    {
    case PHASE_ASSIGN:
        relay_restart_flag = 0;
        received_action = 1;
        phase_assign_load_change = 1;
        stop_phase_flag = 1;
        Serial.print("Phase Assigned : ");
        Serial.println(recieved_data.phase);
        phase_Type = recieved_data.phase;
        stop_phase = recieved_data.phase;
        dis_phase_type = recieved_data.phase;
        device_load = recieved_data.load;
        test_flag = (uint8_t)device_load;
        switch (test_flag)
        {
        case 3:
            phase_change_flag_6 = 0;
            phase_change_flag_7 = 0;
            phase_change_flag_3 = 1;

            break;
        case 6:
            phase_change_flag_3 = 0;
            phase_change_flag_7 = 0;
            phase_change_flag_6 = 1;

            break;
        case 7:
            phase_change_flag_3 = 0;
            phase_change_flag_6 = 0;
            phase_change_flag_7 = 1;

            break;

        default:
            break;
        }
        switch (phase_Type)
        {
        case 0:
            relay_restart_flag = 1;

            break;
        case 1:
            relay_restart_flag = 2;
            break;
        case 2:
            relay_restart_flag = 3;
            break;
        default:
            break;
        }
        break;

    case PHASE_CHANGE:
        relay_restart_flag = 0;
        phase_change_flag = 1;
        // phase_change_flag_3 = 1;
        // phase_change_flag_6 = 1;
        // phase_change_flag_7 = 1;
        stop_phase_flag = 2;
        Serial.print("Phase Changed : ");
        Serial.println(recieved_data.phase);
        action_phase_type = recieved_data.phase;
        stop_phase = recieved_data.phase;
        dis_phase_type = recieved_data.phase;
        device_load = recieved_data.load;
        test_flag2 = (uint8_t)device_load;
        switch (test_flag2)
        {
        case 3:
            phase_change_flag_6 = 0;
            phase_change_flag_7 = 0;
            phase_change_flag_3 = 1;

            break;
        case 6:
            phase_change_flag_3 = 0;
            phase_change_flag_7 = 0;
            phase_change_flag_6 = 1;

            break;
        case 7:
            phase_change_flag_3 = 0;
            phase_change_flag_6 = 0;
            phase_change_flag_7 = 1;

            break;

        default:
            break;
        }
        Serial.print("Phase change device load : ");
        Serial.println(device_load);
        Serial.print("Action Phase_type : ");
        Serial.println(action_phase_type);
        switch (action_phase_type)
        {
        case 0:
            relay_restart_flag = 1;
            break;
        case 1:
            relay_restart_flag = 2;
            break;
        case 2:
            relay_restart_flag = 3;
            break;
        default:
            break;
        }
        break;

    case LOAD_CHANGE:
        load_change_flag = 1;
        Serial.print("load Change : " + String(recieved_data.load));
        // stop_phase_flag = 3;

        device_load = recieved_data.load;


        break;

    default:
        break;
    }

#if PACK_DEBUG_EN
    Serial.print("Header : ");
    Serial.println(recieved_data.header1);
    Serial.print("DeviceID : ");
    Serial.println(recieved_data.deviceID);
    Serial.print("Phase : ");
    Serial.println(recieved_data.phase);
    Serial.print("Checksum : ");
    Serial.println(recieved_data.checksum);
    Serial.print("Footer : ");
    Serial.println(recieved_data.footer);
    Serial.println();
#endif
}

void initEspNowSlave(void)
{
    // WiFi.disconnect();
    // delay(100);
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register peer
    Serial.print("EspNowOperating WiFi Channel:  ");
    Serial.println(WiFi.channel());
    master.channel = 0;
    master.encrypt = false;
    // master.ifidx = ESP_IF_WIFI_STA;

    // WiFi.softAPmacAddress();
    // Add peer
    memcpy(master.peer_addr, MasterMACAdd, 6);
    if (esp_now_add_peer(&master) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);

    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);
}

/**
 * @brief Send a start charge status packet to the master
 * @details This function sends a packet to the master with the device ID, phase, power and status set to charging.
 * It also calculates the checksum of the packet and includes it in the packet.
 * The packet is sent using the esp_now_send function.
 */
void send_start_charge_status(void)
{
    uint8_t sen_cnt = 0;
    Serial.println(F("send_start_charge_status..."));
    send_data.deviceID = device_id;
    send_data.phase = phase_Type;
    send_data.power = PowerRating;
    send_data.status = charging;
    send_data.checksum = send_data.deviceID + send_data.phase + (uint8_t)send_data.power + send_data.status;

#if PACK_DEBUG_EN
    Serial.println("Packet send info:");
    Serial.println("Header : " + send_data.header1);
    Serial.println("DeviceID : " + send_data.deviceID);
    Serial.println("Phase : " + send_data.phase);
    Serial.println("status : " + send_data.status);
    Serial.println("Checksum : " + send_data.checksum);
    Serial.println("Footer : " + send_data.footer);
#endif
#if 0
    while (sen_cnt++ < 14)
    {
        // Serial.print(F("\r\nRetry count: "));
        // Serial.print(sen_cnt);
        Serial.print(F("* "));
        send_data.deviceID = device_id;
        send_data.phase = phase_Type;
        send_data.power = PowerRating;
        send_data.status = charging;
        send_data.checksum = send_data.deviceID + send_data.phase + (uint8_t)send_data.power + send_data.status;
        esp_now_send(master.peer_addr, (uint8_t*)&send_data, sizeof(send_data));
        delay(1000);
        if (sent_status == 0)
        {
            memset(&send_data, '\0', sizeof(send_data));
            sent_status = 11;
            break;
        }
    }
#endif

    esp_now_send(master.peer_addr, (uint8_t*)&send_data, sizeof(send_data));
}

/**
 * @brief Send a stop charge status packet to the master
 * @details This function sends a packet to the master with the device ID, phase, power and status set to not charging.
 * It also calculates the checksum of the packet and includes it in the packet.
 * The packet is sent using the esp_now_send function.
 */
void send_stop_charge_status(void)
{
    uint8_t sen_cnt = 0;
    Serial.println(F("send_stop_charge_status..."));
    send_data.deviceID = device_id;
    if (stop_phase_flag == 1)
    {
        stop_phase_flag = 0;
        // send_data.phase = phase_Type;
        send_data.phase = stop_phase;
    }
    else if (stop_phase_flag == 2)
    {
        stop_phase_flag = 0;
        // send_data.phase = action_phase_type;
        send_data.phase = stop_phase;
    }
    // else if (stop_phase_flag == 3)
    // {
    //     stop_phase_flag = 0;
    //     //     // send_data.phase = device_load;
    // }
    send_data.phase = stop_phase;
    send_data.power = PowerRating;
    send_data.status = not_charging;
    send_data.checksum = send_data.deviceID + send_data.phase + (uint8_t)send_data.power + send_data.status;

#if PACK_DEBUG_EN
    Serial.println("Packet send info:");
    Serial.println("Header : " + send_data.header1);
    Serial.println("DeviceID : " + send_data.deviceID);
    Serial.println("Phase : " + send_data.phase);
    Serial.println("status : " + send_data.status);
    Serial.println("Checksum : " + send_data.checksum);
    Serial.println("Footer : " + send_data.footer);
#endif
#if 0
    while (sen_cnt++ < 14)
    {
        // Serial.print(F("\r\nRetry count: "));
        // Serial.print(sen_cnt);
        Serial.print(F("* "));
        send_data.deviceID = device_id;
        send_data.phase = phase_Type;
        send_data.power = PowerRating;
        send_data.status = not_charging;
        send_data.checksum = send_data.deviceID + send_data.phase + (uint8_t)send_data.power + send_data.status;
        esp_now_send(master.peer_addr, (uint8_t*)&send_data, sizeof(send_data));
        delay(1000);
        if (sent_status == 0)
        {
            memset(&send_data, '\0', sizeof(send_data));
            sent_status = 11;
            break;
        }
    }
#endif

    esp_now_send(master.peer_addr, (uint8_t*)&send_data, sizeof(send_data));
}

/**
 * @brief Send a power status packet to the master
 * @details This function sends a packet to the master with the device ID, phase, power and status set to power.
 * It also calculates the checksum of the packet and includes it in the packet.
 * The packet is sent using the esp_now_send function.
 */
void send_power_status(void)
{
    uint8_t sen_cnt = 0;
    Serial.println(F("Sending Power status..."));
    send_data.deviceID = device_id;
    send_data.phase = phase_Type;
    send_data.power = received_power;
    send_data.status = power;
    send_data.checksum = send_data.deviceID + send_data.phase + (uint8_t)send_data.power + send_data.status;

#if PACK_DEBUG_EN
    Serial.println("Packet send info:");
    Serial.println("Header : " + send_data.header1);
    Serial.println("DeviceID : " + send_data.deviceID);
    Serial.println("Phase : " + send_data.phase);
    Serial.println("status : " + send_data.status);
    Serial.println("Checksum : " + send_data.checksum);
    Serial.println("Footer : " + send_data.footer);
#endif
#if 0
    while (sen_cnt++ < 14)
    {
        // Serial.print(F("\r\nRetry count: "));
        // Serial.print(sen_cnt);
        Serial.print(F("* "));
        send_data.deviceID = device_id;
        send_data.phase = phase_Type;
        send_data.power = PowerRating;
        send_data.status = power;
        send_data.checksum = send_data.deviceID + send_data.phase + (uint8_t)send_data.power + send_data.status;
        esp_now_send(master.peer_addr, (uint8_t*)&send_data, sizeof(send_data));
        delay(1000);

        if (sent_status == 0)
        {
            memset(&send_data, '\0', sizeof(send_data));
            sent_status = 11;
            break;
        }
    }
#endif

    esp_now_send(master.peer_addr, (uint8_t*)&send_data, sizeof(send_data));
}
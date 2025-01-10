// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License


#ifndef LOCALSTORAGE_H
#define LOCALSTORAGE_H

#include <ArduinoJson.h>
#include <Arduino.h>
#include <LinkedList.h>

#include <sqlite3.h>
#include <SPI.h>
#include <FS.h>
#include "SPIFFS.h"
#include "FFat.h"



#define FORMAT_SPIFFS_IF_FAILED true
#define SPI_FFS_USED            (1) 

#define DB_NO_ERROR			    (0x0)
#define DB_FS_MOUNT_ERROR		(0x1)  
#define DB_FS_OPEN_ERROR		(0x2)
#define DB_FS_IS_DIR_ERROR		(0x3)
#define DB_OPEN_ERROR			(0x4)
#define DB_EXEC_ERROR			(0x5)

//#define GET_ID_NUM        	(1)
#define GET_RFID_LIST         (1)
#define GET_RFID              (2)
#define GET_NO_OF_LOGS        (3)
#define GET_LOG_NO            (4)

#define SET_DB_PKT				(1)
#define GET_DB_PKT				(2)

#define LOCAL_STORAGE_DB                (1)

typedef struct StoredSessions
{
    String tid;
    String rfid;
    String start_date;
	String stop_date;
    String units; 
	String ros;

}stored_session;

typedef enum LocalStoreFunctions
{
  REG_RFID,
  GET_RFID_STORED,
  PUT_DATA,
  UPDATE_DATA,
  GET_DATA,
};

typedef enum operation_status
{
	STORE_FAIL,
	STORE_OK
};


typedef enum operations
{
    RFID_EXISTS,
    FETCH_RFID_LIST,
    FETCH_TXN,
    COUNT_TXNS,
    COUNT_RFIDS,
    PRINT_LIST,
    OTHER_OP
};
/*
typedef struct rfid_list
{
    String idTag;
    String idTagStatus;
	String perentidTag;
    String expiry_time; 
	String updateType;

}ids_list;
*/

class LocalStorageClass
{
private:


public:

	/* You only need to format FFAT/SPIFFS the first time you run a
   test or else use the FFAT/SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */

	int db_init(void);
	// static int callback(void *data, int argc, char **argv, char **azColName);
	int db_open(const char *filename, sqlite3 **db);
	int db_exec(sqlite3 *db, const char *sql);
	operation_status db_set_rfid(String,String);
	operation_status db_get_rfid_list();
	operation_status db_set_txn(String txnid, String tag,String start_dt, String stop_dt,String un,String ros);
	operation_status db_get_txn(String);
    operation_status db_check_rfid_auth(String);
	
};

#endif
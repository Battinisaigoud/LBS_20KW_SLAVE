// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License


#ifndef LOCALLISTHANDLER_H
#define LOCALLISTHANDLER_H

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

#define IDTAG_INFO_DB 			(1)

//#define GET_ID_NUM        	(1)
#define GET_LIST_VERSION        (1)
#define GET_ID_TAG              (2)
#define GET_ID_TAG_STATUS       (3)
#define GET_UPDATE_TYPE         (4)

#define SET_DB_PKT				(1)
#define GET_DB_PKT				(2)

#if IDTAG_INFO_DB

typedef struct Local_Authorization_List_tag
{
    int listVersion;
    uint8_t id;
    String idTag;
    String idTagStatus;
	String perentidTag;
    String expiry_time; 
	String updateType;

}Local_Authorization_List_t;

extern Local_Authorization_List_t Local_Authorization_List;

#endif


class IdTagInfoClass
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
	int db_set_table(void);
	int db_get_table(String param, uint8_t get_type);
};

#endif
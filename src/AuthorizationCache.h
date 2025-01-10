// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License


#ifndef AUTHCACHE_HANDLER_H
#define AUTHCACHE_HANDLER_H

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

#define AUTH_CACHE_NO_ERROR			    (0x0)
#define AUTH_CACHE_FS_MOUNT_ERROR		(0x1)  
#define AUTH_CACHE_FS_OPEN_ERROR		(0x2)
#define AUTH_CACHE_FS_IS_DIR_ERROR		(0x3)
#define AUTH_CACHE_OPEN_ERROR			(0x4)
#define AUTH_CACHE_EXEC_ERROR			(0x5)

#define AUTHORIZATION_CACHE_DB 			(1)

#define GET_AUTH_CACHE_INDEX 		    (0)
#define GET_AUTH_CACHE_VERSION 		    (1)
#define GET_AUTH_CACHE_ID_TAG  		    (2)
#define GET_AUTH_CACHE_ID_TAG_STATUS    (3)
#define GET_AUTH_CACHE_ID_PARENT_TAG    (4)
#define GET_AUTH_CACHE_ID_EXPIRY_DATE   (5)
#define GET_AUTH_CACHE_UPDATE           (6)

#define SET_AUTH_CACHE_DB_PKT			(1)
#define GET_AUTH_CACHE_DB_PKT			(2)
#define CLEAR_AUTH_CACHE_DB_PKT			(3)

#if AUTHORIZATION_CACHE_DB

typedef struct Authorization_Cache_List_tag
{
    int listVersion;
    uint8_t id;
    String idTag;
    String idTagStatus;
	String perentidTag;
    String expiry_time; 
	String updateType;

}Authorization_Cache_List_t;

extern Authorization_Cache_List_t Authorization_Cache_List;

#endif


class AuthorizationCacheInfoClass
{
private:


public:

	/* You only need to format FFAT/SPIFFS the first time you run a
   test or else use the FFAT/SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */

	int AuthorizationCache_db_init(void);
	// static int callback(void *data, int argc, char **argv, char **azColName);
	int AuthorizationCache_db_open(const char *filename, sqlite3 **db);
	int AuthorizationCache_db_exec(sqlite3 *db, const char *sql);
	int AuthorizationCache_db_set_table(void);
	int AuthorizationCache_db_get_table(String param, uint8_t get_type);
	int AuthorizationCache_db_clear_table(void);
	int AuthorizationCache_db_set_get_exec(sqlite3 *db, const char *sql);
};

#endif
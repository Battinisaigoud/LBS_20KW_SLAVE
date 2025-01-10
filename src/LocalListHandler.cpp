// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "LocalListHandler.h"
//#include "SendLocalList.h"
#include <FS.h>
#include "SPIFFS.h"
#include "FFat.h"
#include <sstream>


IdTagInfoClass db_idTagObject;//no of Id Tags

uint8_t gu8_get_type_db = 0;
//uint8_t gu8_set_type_db = 0;
//int16_t gs16_set_LL_index = -1;


#if IDTAG_INFO_DB

Local_Authorization_List_t Local_Authorization_List;

// Create a Linklist for 
LinkedList<Local_Authorization_List_t> LL_Local_Authorization_List = LinkedList<Local_Authorization_List_t>();

Local_Authorization_List_t get_Local_Authorization_List;

#endif

/*
 *@brief Sqlite3 database, sqlite3 file name and its Descriptor 
 */
sqlite3 *local_list_db;

File root;
File file;


 #if SPI_FFS_USED
char *db_filename = "/spiffs/local_list.db";
#else
char *db_filename = "/ffat/ll.db";
#endif

uint8_t gu8_sqlLite_Initilsed_flag = 0;


int get_db_callback(void *data, int argc, char **argv, char **azColName);


/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */

/*
 *@brief Sqlite3 database Initialise using SPIFFS file systems
 */

int IdTagInfoClass::db_init(void)
{
     
		uint8_t mount_count = 5;
		LL_Local_Authorization_List.clear();
#if SPI_FFS_USED
		while (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED) && mount_count)
#else
		while ((!FFat.begin(true)) && mount_count)
#endif
		{
			mount_count--;
			Serial.println(F("Failed to mount file system"));
		}
		if (!mount_count)
		{
			return DB_FS_MOUNT_ERROR;
		}

#if SPI_FFS_USED
		// list SPIFFS contents
		root = SPIFFS.open("/");
#else
		// list FFat contents
		root = FFat.open("/");
#endif
		if (!root)
		{
			Serial.println(F("- failed to open directory"));
			return DB_FS_OPEN_ERROR;
		}
		if (!root.isDirectory())
		{
			Serial.println(F(" - not a directory"));
			return DB_FS_IS_DIR_ERROR;
		}
		file = root.openNextFile();
		while (file)
		{
			if (file.isDirectory())
			{
				Serial.print(F("  DIR : "));
				Serial.println(file.name());
			}
			else
			{
				Serial.print(F("  FILE: "));
				Serial.print(file.name());
				Serial.print(F("\tSIZE: "));
				Serial.println(file.size());
			}
			file = root.openNextFile();
		}

		/*
		 *@breif below conditional compliation is enabled to remove the existing file 
		 */
        #if 0
	    	// remove existing file
			SPIFFS.remove("/local_list.db");
	   	#endif

		sqlite3_initialize();
		if (db_open(db_filename, &local_list_db))
			return DB_OPEN_ERROR;

        //int rc = db_exec(local_list_db, "CREATE TABLE IF NOT EXISTS LOCALLIST (id INTEGER NOT NULL , listVersion INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL , idTagInfo VARCHAR(32) NOT NULL, updateType VARCHAR(32) NOT NULL);");
		int rc = db_exec(local_list_db, "CREATE TABLE IF NOT EXISTS LOCALLIST (id INTEGER NOT NULL , listVersion INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL , idTagInfo VARCHAR(32) NOT NULL, parentIdTag VARCHAR(32) NOT NULL , expiryDate VARCHAR(32) NOT NULL,updateType VARCHAR(32) NOT NULL);");  // Need to add the these columns
		
		if (rc != SQLITE_OK)
		{
			sqlite3_close(local_list_db);
			return DB_EXEC_ERROR;
		}

		rc = db_exec(local_list_db, "SELECT * FROM LOCALLIST");
		if (rc != SQLITE_OK)
		{
			sqlite3_close(local_list_db);
			return DB_EXEC_ERROR;
		}
        gu8_sqlLite_Initilsed_flag  = 1;
	return DB_NO_ERROR;
}

/*
 *@brief Sqlite3 database file open using sqlite3 file name and its Descriptor 
 */
int IdTagInfoClass::db_open(const char *filename, sqlite3 **db) {
  int rc = sqlite3_open(filename, db);
  if (rc) {
    Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
  } else {
    Serial.print(F("Opened database successfully\n"));
  }
  return rc;
}

/*
 *@brief Sqlite3 database command execution using sqlite3 file Descriptor and SQL Query
 */
int IdTagInfoClass::db_exec(sqlite3 *db, const char *sql) {
  char *zErrMsg = 0;
  const char *data = "Callback function called";
  Serial.println(sql);
  long start = micros();
  int rc = sqlite3_exec(db, sql, get_db_callback, (void*)data, &zErrMsg);
  if (rc != SQLITE_OK) {
    Serial.printf("SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  } else {
    Serial.print(F("Operation done successfully\n"));
  }
  Serial.print(F("Time taken:"));
  Serial.println(micros() - start);
  return rc;
}


 int get_db_callback(void *data, int argc, char **argv, char **azColName)
	{
		int i;
		Serial.printf("%s: ", (const char *)data);
		for (i = 0; i < argc; i++)
		{
			Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		    
			// Serial.print("get_type");
			// Serial.println(gu8_get_type_db);
			/*
			*  @brief whenever get data from Table 
			*/
			if (gu8_get_type_db == i)
			{
				switch (gu8_get_type_db)
				{
#if 0
				case 0:
					{
					Serial.print("id : ");
					Serial.println(gu8_get_type_db);
					}
					break;
#endif
				case GET_LIST_VERSION:
				{
					int val = 0;
					//Serial.print("listVersion : ");
					//Serial.println(gu8_get_type_db);
					std::string str(argv[i]);
					std::istringstream myStrm(str);
					myStrm >> val;
					Serial.print("listVersion int : ");
					Serial.println(val);
					get_Local_Authorization_List.listVersion = val;
					Serial.print("listVersion inString : ");
					Serial.println(get_Local_Authorization_List.listVersion);
					gu8_get_type_db = 0;
					
				}
				break;

				case GET_ID_TAG:
				{
					Serial.print("idTag : ");
					Serial.println(gu8_get_type_db);
					get_Local_Authorization_List.idTag = argv[i];
				}
				break;
				case GET_ID_TAG_STATUS:
				{
					Serial.print("id : ");
					Serial.println(gu8_get_type_db);
					get_Local_Authorization_List.idTagStatus = argv[i];
					gu8_get_type_db = 0;
					
				}
				break;
				case GET_UPDATE_TYPE:
				{
					Serial.print("id : ");
					Serial.println(gu8_get_type_db);
					get_Local_Authorization_List.updateType = argv[i];
					gu8_get_type_db = 0;
					
				}
				break;
				default:

					// Serial.println("default");
					break;
				}
			}
		}
		Serial.printf("\n");
        
		return 0;
	}

#if 1
int IdTagInfoClass::db_set_table(void)
{
	int ret = 0;
	/*  -- Check the mysql Database exist or not, if not then create Database file.
		*	CREATE LOCAL LIST DATABASE file and locallist dB
		*
		*/
	if (local_list_db == NULL)
	{
		if (db_idTagObject.db_open(db_filename, &local_list_db))
		{
			Serial.print(db_filename);
			Serial.print(F(" file not Opened\n"));
			ret = 1;
		}
	}

    /*  -- Create Table
		*	DELETE TABLE IF NOT EXISTS LOCALLIST ;
		*
		*/
	int rc = db_exec(local_list_db, (const char*)"DELETE FROM LOCALLIST");
	if (rc != SQLITE_OK) 
	{
		sqlite3_close(local_list_db);
		ret = 1;
	}

	/*  -- Create Table
		*	CREATE TABLE IF NOT EXISTS LOCALLIST (	id INTEGER NOT NULL , 
		*											listVersion INTEGER NOT NULL , 
		*											idTag VARCHAR(32) NOT NULL , 
		*											idTagInfo VARCHAR(32) NOT NULL, 
		*											parentIdTag VARCHAR(32) NOT NULL , 
		*											expiryDate VARCHAR(32) NOT NULL ,
		*											updateType VARCHAR(32) NOT NULL);
		*
		*/
	rc = db_exec(local_list_db, "CREATE TABLE IF NOT EXISTS LOCALLIST (id INTEGER NOT NULL , listVersion INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL , idTagInfo VARCHAR(32) NOT NULL, parentIdTag VARCHAR(32) NOT NULL , expiryDate VARCHAR(32) NOT NULL,updateType VARCHAR(32) NOT NULL);");  // Need to add the these columns
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_list_db);
		if (db_idTagObject.db_open(db_filename, &local_list_db))
		{
			Serial.print(db_filename);
			Serial.print(F(" file not Opened\n"));
			ret = 2;
		}
	}

	for (int i = 0; i < LL_Local_Authorization_List.size(); i++)
	{
		// Get local list from linked list
		Local_Authorization_List = LL_Local_Authorization_List.get(i);
		String sql_query;
		Serial.println("\n\nAppending to a sql_query String:");
		sql_query = "INSERT INTO LOCALLIST VALUES (" + String(Local_Authorization_List.id) + "," + String(Local_Authorization_List.listVersion) + "," + "'" + Local_Authorization_List.idTag + "'" + "," + "'" + Local_Authorization_List.idTagStatus + "'" + "," + "'" +Local_Authorization_List.perentidTag +"'" + "," +"'"+ Local_Authorization_List.expiry_time+"'" + "," +"'" + Local_Authorization_List.updateType + "'" + ");";
		Serial.println(sql_query);
        
		/*
			* -- Insert to Table
			*		INSERT INTO LOCALLIST VALUES (index, idTag, idTagInfo);
			*		e.g. INSERT INTO LOCALLIST VALUES (0001, '8037d79e', 'Accepted');
			*/
		int rc = db_idTagObject.db_exec(local_list_db, sql_query.c_str());
		if (rc != SQLITE_OK)
		{
			sqlite3_close(local_list_db);
			ret = 3;
		}
		
	}

	return ret;
}
#endif

int IdTagInfoClass::db_get_table(String param, uint8_t get_type)
{
	/*  -- Check the mysql Database exist or not, if not then create Database file.
	 *	CREATE LOCAL LIST DATABASE file and locallist dB
	 *
	 */
	if (local_list_db == NULL)
	{
		if (db_idTagObject.db_open(db_filename, &local_list_db))
		{
			Serial.print(db_filename);
			Serial.print(F(" file not Opened\n"));
		}
	}

	/*
	 * -- fetch id tag from  Table
	 * SELECT FROM LOCALLIST WHERE idTag
	 * e.g. SELECT * FROM LOCALLIST WHERE idTag = 'd787007f';
	 */
	String get_sql_query = "SELECT * FROM LOCALLIST ";

	/*
	 *  @brief whenever get data from Table
	 */
	gu8_get_type_db = get_type;
	Serial.print("get_type");
	Serial.println(get_type);
	switch (get_type)
	{

#if 0
		case GET_ID_NUM:
				{
				// get_sql_query = "id";
				// get_sql_query += "'" + String(param) + "';";
				Serial.println("id_number");
				}
				break;
#endif
	case GET_LIST_VERSION:
	{
		get_sql_query += "WHERE id = 1";
		Serial.println("listVersion");
	}
	break;
	case GET_ID_TAG:
	{
		get_sql_query += "WHERE idTag =";
		get_sql_query += "'" + String(param) + "';";
		Serial.println("idTag");
	}
	break;
	case GET_ID_TAG_STATUS:
	{
		// get_sql_query += "idTagStatus =";
		// get_sql_query += "'" + String(param) + "';";
		Serial.println("idTagStatus");
	}
	break;
	case GET_UPDATE_TYPE:
	{
		// get_sql_query += "updateType =";
		// get_sql_query += "'" + String(param) + "';";
		Serial.println("updateType");
	}
	break;
	default:

		// Serial.println("default");
		break;
	}

	int rc = db_exec(local_list_db, get_sql_query.c_str());
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_list_db);
		if (db_idTagObject.db_open(db_filename, &local_list_db))
		{
			Serial.print(db_filename);
			Serial.print(F(" file not Opened\n"));
		}
	}
}

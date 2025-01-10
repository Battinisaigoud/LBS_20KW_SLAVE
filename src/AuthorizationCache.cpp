// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "AuthorizationCache.h"

#include <FS.h>
#include "SPIFFS.h"
#include "FFat.h"
#include <sstream>

#include "TimeHelper.h"          

AuthorizationCacheInfoClass AuthorizationCache_Object; // no of Id Tags

uint8_t gu8_get_auth_cb_flag = 0;
bool clearcache_flag = false;
bool check_get_authCache_flag =false;
bool set_authCache_flag =false;
bool get_authCache_flag =false;
static uint16_t auth_cache_table_slno = 0;
bool db_init_flag = false;
uint8_t gu8_get_auth_db_type = 0;

extern String authCache_getparam;

/*
 * @brief below variable used to check whether sqlite is already initialised or not
 *
 */

extern uint8_t gu8_sqlLite_Initilsed_flag;

#if AUTHORIZATION_CACHE_DB

Authorization_Cache_List_t Authorization_Cache_List;

// Create a Linklist for
LinkedList<Authorization_Cache_List_t> LL_Authorization_Cache_List = LinkedList<Authorization_Cache_List_t>();

Authorization_Cache_List_t get_Authorization_Cache_List;

#endif

/*
 *@brief Sqlite3 database, sqlite3 file name and its Descriptor
 */

sqlite3 *Auth_cache_db;
File auth_cache_root;
File auth_cache_file;

#if SPI_FFS_USED
char *auth_cache_db_filename = "/spiffs/auth_cache.db";
#else
char *auth_cache_db_filename = "/ffat/auth_cache.db";
#endif

int get_auth_cache_db_callback(void *data, int argc, char **argv, char **azColName);
int set_get_auth_cache_db_callback(void *data, int argc, char **argv, char **azColName);

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */

/*
 *@brief Sqlite3 database Initialise using SPIFFS file systems
 */

int AuthorizationCacheInfoClass::AuthorizationCache_db_init(void)
{

	uint8_t mount_count = 5;
	LL_Authorization_Cache_List.clear();

	if (!gu8_sqlLite_Initilsed_flag)
	{
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
			return AUTH_CACHE_FS_MOUNT_ERROR;
		}

#if SPI_FFS_USED
		// list SPIFFS contents
		auth_cache_root = SPIFFS.open("/");
#else
		// list FFat contents
		auth_cache_root = FFat.open("/");
#endif
		if (!auth_cache_root)
		{
			Serial.println(F("- failed to open directory"));
			return AUTH_CACHE_FS_OPEN_ERROR;
		}
		if (!auth_cache_root.isDirectory())
		{
			Serial.println(F(" - not a directory"));
			return AUTH_CACHE_FS_IS_DIR_ERROR;
		}
		auth_cache_file = auth_cache_root.openNextFile();
		while (auth_cache_file)
		{
			if (auth_cache_file.isDirectory())
			{
				Serial.print(F("  DIR : "));
				Serial.println(auth_cache_file.name());
			}
			else
			{
				Serial.print(F("  FILE: "));
				Serial.print(auth_cache_file.name());
				Serial.print(F("\tSIZE: "));
				Serial.println(auth_cache_file.size());
			}
			auth_cache_file = auth_cache_root.openNextFile();
		}

/*
 *@breif below conditional compliation is enabled to remove the existing file
 */
#if 0
	    	// remove existing file
			SPIFFS.remove("/local_list.db");
#endif

		sqlite3_initialize();
	}
	if (AuthorizationCache_db_open(auth_cache_db_filename, &Auth_cache_db))
		return AUTH_CACHE_FS_OPEN_ERROR;

	int rc = AuthorizationCache_db_exec(Auth_cache_db, "CREATE TABLE IF NOT EXISTS AUTH_CACHE_LIST (id INTEGER NOT NULL , listVersion INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL , idTagInfo VARCHAR(32) NOT NULL, parentIdTag VARCHAR(32) NOT NULL , expiryDate VARCHAR(32) NOT NULL,updateType VARCHAR(32) NOT NULL);"); // Need to add the these columns

	if (rc != SQLITE_OK)
	{
		sqlite3_close(Auth_cache_db);
		return AUTH_CACHE_EXEC_ERROR;
	}

	db_init_flag = true;

	rc = AuthorizationCache_db_exec(Auth_cache_db, "SELECT * FROM AUTH_CACHE_LIST");
	if (rc != SQLITE_OK)
	{
		sqlite3_close(Auth_cache_db);
		return AUTH_CACHE_EXEC_ERROR;
	}
    db_init_flag = false;
	return AUTH_CACHE_NO_ERROR;
}

/*
 *@brief Sqlite3 database file open using sqlite3 file name and its Descriptor
 */
int AuthorizationCacheInfoClass::AuthorizationCache_db_open(const char *filename, sqlite3 **db)
{
	int rc = sqlite3_open(filename, db);
	if (rc)
	{
		Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
	}
	else
	{
		Serial.print(F("Opened database successfully\n"));
	}
	return rc;
}

/*
 *@brief Sqlite3 database command execution using sqlite3 file Descriptor and SQL Query
 */
int AuthorizationCacheInfoClass::AuthorizationCache_db_exec(sqlite3 *db, const char *sql)
{
	char *zErrMsg = 0;
	const char *data = "Auth_cache Callback function called";
	Serial.println(sql);
	long start = micros();
	int rc = sqlite3_exec(db, sql, get_auth_cache_db_callback, (void *)data, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		Serial.printf("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		
		Serial.print(F("Operation done successfully\n"));
	}
	Serial.print(F("Time taken:"));
	Serial.println(micros() - start);
	return rc;
}

/*
 *@brief Sqlite3 database command execution using sqlite3 file Descriptor and SQL Query
 */
int AuthorizationCacheInfoClass::AuthorizationCache_db_set_get_exec(sqlite3 *db, const char *sql)
{
	char *zErrMsg = 0;
	const char *set_get_data = "Auth_cache set get Callback function called";
	Serial.println(sql);
	long start = micros();
	int rc = sqlite3_exec(db, sql, set_get_auth_cache_db_callback, (void *)set_get_data, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		Serial.printf("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
	{
		if(gu8_get_auth_cb_flag)
		{
			gu8_get_auth_cb_flag = false;
			
			set_authCache_flag = false;
			
			Serial.println("idTag is not Avaliable...!");

		} 
		Serial.print(F("Operation done successfully\n"));
	}
	Serial.print(F("Time taken:"));
	Serial.println(micros() - start);
	return rc;
}

int set_get_auth_cache_db_callback(void *data, int argc, char **argv, char **azColName)
{
	int i;
	Serial.printf("%s: ", (const char *)data);
	for (i = 0; i < argc; i++)
	{
		Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

		
		/*
		 *  @brief whenever get data from Table
		 */
		if (gu8_get_auth_cb_flag )
		{
			switch (i)
			{

#if 1			
            case GET_AUTH_CACHE_INDEX:
			{
				int val = 0;
				Serial.print("SL NO : ");
				std::string str(argv[i]);
				std::istringstream myStrm(str);
				myStrm >> val;
				Serial.println(val);
				//auth_cache_table_slno = val;
				//Serial.println(val);
			}
			break;

			case GET_AUTH_CACHE_VERSION:
			{
				int val = 0;
				Serial.print("listVersion : ");
				std::string str(argv[i]);
				std::istringstream myStrm(str);
				myStrm >> val;
				Serial.println(val);
				
			}
			break;
#endif

#if 1
			case GET_AUTH_CACHE_ID_TAG:
			{
				 
				Serial.print("idTag : ");
				Serial.println(get_Authorization_Cache_List.idTag);
				
				 if (get_Authorization_Cache_List.idTag.equals(argv[i]) == true)
				 {
					set_authCache_flag = true;
					//get_authCache_flag = true;
					Serial.println(argv[i]);
					if(check_get_authCache_flag)
					{
						get_authCache_flag = true;
						check_get_authCache_flag = false;
						return 1;
					}
				 }
				 else
				 {
					set_authCache_flag = false;
					//get_authCache_flag = false;
					Serial.println(argv[i]);
					//return 1;
				 }
			}
			break;

			case GET_AUTH_CACHE_ID_TAG_STATUS:
			{
				Serial.print("id : ");
				Serial.println(argv[i]);
			}
			break;
			case GET_AUTH_CACHE_ID_PARENT_TAG:
			{
				Serial.print("id : ");
				Serial.println(argv[i]);
			}
			break;
			case GET_AUTH_CACHE_ID_EXPIRY_DATE:
			{
				time_t expiry_time = 0;
				Serial.print("id expiry_time : ");
				std::string str(argv[i]);
				std::istringstream myStrm(str);
				myStrm >> expiry_time;
				if ((expiry_time - now()) > 0 )
				 {
					set_authCache_flag = true;
					
					Serial.println("authCache_flag : true");
					Serial.println(expiry_time);
					if(check_get_authCache_flag)
					{
						get_authCache_flag = true;
						check_get_authCache_flag = false;
						return 1;
					}
				 }
				 else
				 {
					set_authCache_flag = false;
					//get_authCache_flag = false;
					Serial.println("authCache_flag : false");
					Serial.println(expiry_time);
				 }
			}
			break;
			case GET_AUTH_CACHE_UPDATE:
			{
				Serial.print("Update type : ");
				Serial.println(argv[i]);
				
				gu8_get_auth_cb_flag = false;
			}
			break;
#endif
			default:
				  //if(DEBUG) Serial.println("default");
				break;
			}
		}
		
		
	}
	if(db_init_flag)
	{
		
		auth_cache_table_slno++;   // Count the Authencation Cache Table seril number or index number
		Serial.printf("StartUp auth cache Table Sl no : %d\n",auth_cache_table_slno);
	}
	Serial.printf("auth cache Table Sl no : %d\n",auth_cache_table_slno);

	return 0;
}

int get_auth_cache_db_callback(void *data, int argc, char **argv, char **azColName)
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
		if (gu8_get_auth_db_type == i)
		{
			switch (gu8_get_auth_db_type)
			{
#if 1

				case GET_AUTH_CACHE_INDEX:
					{
					Serial.print("id : ");
					Serial.println(gu8_get_auth_db_type);
					}
					break;

			case GET_AUTH_CACHE_VERSION:
			{
				int val = 0;
				Serial.print("listVersion : ");
				Serial.println(gu8_get_auth_db_type);
				std::string str(argv[i]);
				std::istringstream myStrm(str);
				myStrm >> val;
				Serial.print("listVersion int : ");
				Serial.println(val);
				get_Authorization_Cache_List.listVersion = val;
				Serial.print("listVersion inString : ");
				Serial.println(get_Authorization_Cache_List.listVersion);
				//gu8_get_auth_db_type = 0;
			}
			break;

			case GET_AUTH_CACHE_ID_TAG:
			{
				Serial.print("idTag : ");
				Serial.println(argv[i]);
				//get_Authorization_Cache_List.idTag = argv[i];
				if ( authCache_getparam.equals(argv[i]) == true	)
				{
					/* code */
					get_authCache_flag = true;
					return 0;
				}
				
				

			}
			break;
			case GET_AUTH_CACHE_ID_TAG_STATUS:
			{
				Serial.print("id : ");
				Serial.println(gu8_get_auth_db_type);
				get_Authorization_Cache_List.idTagStatus = argv[i];
				gu8_get_auth_db_type = 0;
			}
			break;
			case GET_AUTH_CACHE_ID_PARENT_TAG:
			{
				Serial.print("id : ");
				Serial.println(argv[i]);
			}
			break;
			case GET_AUTH_CACHE_ID_EXPIRY_DATE:
			{
				time_t expiry_time = 0;
				Serial.print("id expiry_time : ");
				std::string str(argv[i]);
				std::istringstream myStrm(str);
				myStrm >> expiry_time;
				if ((expiry_time - now()) > 0 )
				 {
									
					Serial.println("authCache_flag : true");
					Serial.println(expiry_time);
					if(check_get_authCache_flag)
					{
						get_authCache_flag = true;
						check_get_authCache_flag = false;
						return 1;
					}
				 }
				 else
				 {
					
					Serial.println("authCache_flag : false");
					Serial.println(expiry_time);
				 }
			}
			break;
			case GET_AUTH_CACHE_UPDATE:
			{
				Serial.print("Update type : ");
				Serial.println(argv[i]);
				
				gu8_get_auth_cb_flag = false;
			}
			break;
#endif
			default:

				// Serial.println("default");
				break;
			}
		}
	}
	Serial.printf("\n");

	return 0;
}
#if 0
int get_auth_cache_db_callback(void *data, int argc, char **argv, char **azColName)
{
	int i;
	Serial.printf("%s: ", (const char *)data);
	for (i = 0; i < argc; i++)
	{
		Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

		
		/*
		 *  @brief whenever get data from Table
		 */
		if (gu8_get_auth_cb_flag )
		{
			switch (i)
			{

#if 1			
            case GET_AUTH_CACHE_INDEX:
			{
				int val = 0;
				Serial.print("SL NO : ");
				std::string str(argv[i]);
				std::istringstream myStrm(str);
				myStrm >> val;
				Serial.println(val);
				//auth_cache_table_slno = val;
				//Serial.println(val);
			}
			break;

			case GET_AUTH_CACHE_VERSION:
			{
				int val = 0;
				Serial.print("listVersion : ");
				std::string str(argv[i]);
				std::istringstream myStrm(str);
				myStrm >> val;
				Serial.println(val);
				
			}
			break;
#endif

#if 1
			case GET_AUTH_CACHE_ID_TAG:
			{
				 
				Serial.print("idTag : ");
				Serial.println(get_Authorization_Cache_List.idTag);
				
				 if (get_Authorization_Cache_List.idTag.equals(argv[i]) == true)
				 {
					set_authCache_flag = true;
					//get_authCache_flag = true;
					Serial.println(argv[i]);
					if(check_get_authCache_flag)
					{
						get_authCache_flag = true;
						check_get_authCache_flag = false;
						return 1;
					}
				 }
				 else
				 {
					set_authCache_flag = false;
					//get_authCache_flag = false;
					Serial.println(argv[i]);
					//return 1;
				 }
			}
			break;

			case GET_AUTH_CACHE_ID_TAG_STATUS:
			{
				Serial.print("id : ");
				Serial.println(argv[i]);
			}
			break;
			case GET_AUTH_CACHE_ID_PARENT_TAG:
			{
				Serial.print("id : ");
				Serial.println(argv[i]);
			}
			break;
			case GET_AUTH_CACHE_ID_EXPIRY_DATE:
			{
				time_t expiry_time = 0;
				Serial.print("id expiry_time : ");
				std::string str(argv[i]);
				std::istringstream myStrm(str);
				myStrm >> expiry_time;
				if ((expiry_time - now()) > 0 )
				 {
					set_authCache_flag = true;
					
					Serial.println("authCache_flag : true");
					Serial.println(expiry_time);
					if(check_get_authCache_flag)
					{
						get_authCache_flag = true;
						check_get_authCache_flag = false;
						return 1;
					}
				 }
				 else
				 {
					set_authCache_flag = false;
					//get_authCache_flag = false;
					Serial.println("authCache_flag : false");
					Serial.println(expiry_time);
				 }
			}
			break;
			case GET_AUTH_CACHE_UPDATE:
			{
				Serial.print("Update type : ");
				Serial.println(argv[i]);
				
				gu8_get_auth_cb_flag = false;
			}
			break;
#endif
			default:
				  //if(DEBUG) Serial.println("default");
				break;
			}
		}
		
		
	}
	if(db_init_flag)
	{
		
		auth_cache_table_slno++;   // Count the Authencation Cache Table seril number or index number
		Serial.printf("StartUp auth cache Table Sl no : %d\n",auth_cache_table_slno);
	}
	Serial.printf("auth cache Table Sl no : %d\n",auth_cache_table_slno);

	return 0;
}
#endif

#if 1
int AuthorizationCacheInfoClass::AuthorizationCache_db_set_table(void)
{
	int ret = 0;
	/*  -- Check the mysql Database exist or not, if not then create Database file.
	 *	CREATE LOCAL LIST DATABASE file and locallist dB
	 *
	 */
	if (Auth_cache_db == NULL)
	{
		if (AuthorizationCache_Object.AuthorizationCache_db_open(auth_cache_db_filename, &Auth_cache_db))
		{
			Serial.print(auth_cache_db_filename);
			Serial.print(F(" file not Opened\n"));
			ret = 1;
		}
	}

	
	/*  -- Create Table
	 *	CREATE TABLE IF NOT EXISTS AUTH_CACHE_LIST (	id INTEGER NOT NULL ,
	 *											listVersion INTEGER NOT NULL ,
	 *											idTag VARCHAR(32) NOT NULL ,
	 *											idTagInfo VARCHAR(32) NOT NULL,
	 *											parentIdTag VARCHAR(32) NOT NULL ,
	 *											expiryDate VARCHAR(32) NOT NULL ,
	 *											updateType VARCHAR(32) NOT NULL);
	 *
	 */
	int rc = AuthorizationCache_db_exec(Auth_cache_db, "CREATE TABLE IF NOT EXISTS AUTH_CACHE_LIST (id INTEGER NOT NULL , listVersion INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL , idTagInfo VARCHAR(32) NOT NULL, parentIdTag VARCHAR(32) NOT NULL , expiryDate VARCHAR(32) NOT NULL,updateType VARCHAR(32) NOT NULL);"); // Need to add the these columns
	if (rc != SQLITE_OK)
	{
		sqlite3_close(Auth_cache_db);
		if (AuthorizationCache_Object.AuthorizationCache_db_open(auth_cache_db_filename, &Auth_cache_db))
		{
			Serial.print(auth_cache_db_filename);
			Serial.print(F(" file not Opened\n"));
			ret = 2;
		}
	}

	for (int i = 0; i < LL_Authorization_Cache_List.size(); i++)
	{
		// Get local list from linked list
		Authorization_Cache_List = LL_Authorization_Cache_List.get(i);

		/*
		 * -- fetch id tag from  Table
		 * SELECT FROM AUTH_CACHE_LIST WHERE idTag
		 * e.g. SELECT * FROM AUTH_CACHE_LIST WHERE idTag = 'd787007f';
		 */
		String get_sql_query = "SELECT * FROM AUTH_CACHE_LIST ";
		get_sql_query += "WHERE idTag =";
		get_sql_query += "'" + String(Authorization_Cache_List.idTag) + "';";
        
		get_Authorization_Cache_List.idTag  = Authorization_Cache_List.idTag;
		
		gu8_get_auth_cb_flag = true;
		
		
		int rc = AuthorizationCache_db_set_get_exec(Auth_cache_db, get_sql_query.c_str());
		if (rc != SQLITE_OK)
		{
			sqlite3_close(Auth_cache_db);
			if (AuthorizationCache_Object.AuthorizationCache_db_open(auth_cache_db_filename, &Auth_cache_db))
			{
				Serial.print(auth_cache_db_filename);
				Serial.print(F(" file not Opened\n"));
			}
		}

		if (!set_authCache_flag)
		{
			String sql_query;
			Serial.println("\n\nAppending to a sql_query String:");
			
			Authorization_Cache_List.id = (++auth_cache_table_slno);
			sql_query = "INSERT INTO AUTH_CACHE_LIST VALUES (" + String(Authorization_Cache_List.id) + "," + String(Authorization_Cache_List.listVersion) + "," + "'" + Authorization_Cache_List.idTag + "'" + "," + "'" + Authorization_Cache_List.idTagStatus + "'" + "," + "'" + Authorization_Cache_List.perentidTag + "'" + "," + "'" + Authorization_Cache_List.expiry_time + "'" + "," + "'" + Authorization_Cache_List.updateType + "'" + ");";
			Serial.println(sql_query);

			/*
			 * -- Insert to Table
			 *		INSERT INTO AUTH_CACHE_LIST VALUES (index, idTag, idTagInfo);
			 *		e.g. INSERT INTO AUTH_CACHE_LIST VALUES (0001, '8037d79e', 'Accepted');
			 */
			int rc = AuthorizationCache_Object.AuthorizationCache_db_exec(Auth_cache_db, sql_query.c_str());
			if (rc != SQLITE_OK)
			{
				sqlite3_close(Auth_cache_db);
				ret = 2;
			}
		}
	}

	return ret;
}
#endif

int AuthorizationCacheInfoClass::AuthorizationCache_db_get_table(String param, uint8_t get_type)
{
	/*  -- Check the mysql Database exist or not, if not then create Database file.
	 *	CREATE LOCAL LIST DATABASE file and Auth_cache  dB
	 *
	 */
	if (Auth_cache_db == NULL)
	{
		if (AuthorizationCache_Object.AuthorizationCache_db_open(auth_cache_db_filename, &Auth_cache_db))
		{
			Serial.print(auth_cache_db_filename);
			Serial.print(F(" file not Opened\n"));
		}
	}

	/*
	 * -- fetch id tag from  Table
	 * SELECT FROM AUTH_CACHE_LIST WHERE idTag
	 * e.g. SELECT * FROM AUTH_CACHE_LIST WHERE idTag = 'd787007f';
	 */
	String get_sql_query = "SELECT * FROM AUTH_CACHE_LIST ";
		   get_sql_query += "WHERE idTag =";
		   get_sql_query += "'" + String(param) + "';";

    gu8_get_auth_cb_flag = true; 
	get_authCache_flag = false;
	check_get_authCache_flag = false;
	gu8_get_auth_db_type = 2;
	get_Authorization_Cache_List.idTag = param;
	int rc = AuthorizationCache_db_exec(Auth_cache_db, get_sql_query.c_str());
	if (rc != SQLITE_OK)
	{
		sqlite3_close(Auth_cache_db);
		if (AuthorizationCache_Object.AuthorizationCache_db_open(auth_cache_db_filename, &Auth_cache_db))
		{
			Serial.print(auth_cache_db_filename);
			Serial.print(F(" file not Opened\n"));
		}
	}

}


int AuthorizationCacheInfoClass::AuthorizationCache_db_clear_table(void)
{
	int ret = 0;
	clearcache_flag = true;
	/*  -- Check the mysql Database exist or not, if not then create Database file.
	 *	CREATE LOCAL LIST DATABASE file and Auth_cache  dB
	 *
	 */
	if (Auth_cache_db == NULL)
	{
		if (AuthorizationCache_Object.AuthorizationCache_db_open(auth_cache_db_filename, &Auth_cache_db))
		{
			Serial.print(auth_cache_db_filename);
			Serial.print(F(" file not Opened\n"));
		}
	}

	/*  -- Create Table
	 *	DELETE TABLE IF NOT EXISTS AUTH_CACHE_LIST ;
	 *
	 */
	int rc = AuthorizationCache_db_exec(Auth_cache_db, (const char *)"DELETE FROM AUTH_CACHE_LIST");
	if (rc != SQLITE_OK)
	{
		sqlite3_close(Auth_cache_db);
		ret = 1;
		clearcache_flag = false;
	}

    // clear the Cache table Serial number to zero.
    auth_cache_table_slno = 0;
	

   return ret;
}
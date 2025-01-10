// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "LocalStorage.h"
//#include "SendLocalList.h"
#include <FS.h>
#include "SPIFFS.h"
#include "FFat.h"
#include <sstream>

LocalStorageClass db_localStoreObject; // no of Id Tags
extern uint8_t gu8_get_type_db;

/*
 *@brief Sqlite3 database, sqlite3 file name and its Descriptor
 */
sqlite3 *local_storage_db;

File local_store_root;
File local_file;

int op_type = 100;

bool found = false;

int counter = 0;

#if SPI_FFS_USED
char *local_storage_db_filename = "/spiffs/local_storage.db";
#else
char *local_storage_db_filename = "/ffat/ll.db";
#endif

extern uint8_t gu8_sqlLite_Initilsed_flag;

// We need to create a linkedlist of type strings to return RFID tags
//  Create a Linklist for
LinkedList<String> LL_StoredRFIDs = LinkedList<String>();

stored_session ss_t;

// Create a Linklist for 
LinkedList<stored_session> LL_Local_Stored_List = LinkedList<stored_session>();


static int get_db_callback_local(void *data, int argc, char **argv, char **azColName);
static int db_get_list_callback(void *data, int argc, char **argv, char **azColName);
static int db_get_count_callback(void *data, int argc, char **argv, char **azColName);
static int db_get_txn_callback(void *data, int argc, char **argv, char **azColName);

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */

/*
 *@brief Sqlite3 database Initialise using SPIFFS file systems
 */

int LocalStorageClass::db_init(void)
{

	if(!gu8_sqlLite_Initilsed_flag)
	{
	uint8_t mount_count = 5;

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
	local_store_root = SPIFFS.open("/");
#else
	// list FFat contents
	local_store_root = FFat.open("/");
#endif
	if (!local_store_root)
	{
		Serial.println(F("- failed to open directory"));
		return DB_FS_OPEN_ERROR;
	}
	if (!local_store_root.isDirectory())
	{
		Serial.println(F(" - not a directory"));
		return DB_FS_IS_DIR_ERROR;
	}
	local_file = local_store_root.openNextFile();
	while (local_file)
	{
		if (local_file.isDirectory())
		{
			Serial.print(F("  DIR : "));
			Serial.println(local_file.name());
		}
		else
		{
			Serial.print(F("  FILE: "));
			Serial.print(local_file.name());
			Serial.print(F("\tSIZE: "));
			Serial.println(local_file.size());
		}
		local_file = local_store_root.openNextFile();
	}

/*
 *@breif below conditional compliation is enabled to remove the existing file
 */
#if 0
	    	// remove existing file
			SPIFFS.remove("/local_list.db");
#endif

	sqlite3_initialize();
	gu8_sqlLite_Initilsed_flag = 1;
	}
	if (db_open(local_storage_db_filename, &local_storage_db))
		return DB_OPEN_ERROR;

	// int rc = db_exec(local_storage_db, "CREATE TABLE IF NOT EXISTS LOCALSTORE (id INTEGER NOT NULL , listVersion INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL , idTagInfo VARCHAR(32) NOT NULL, updateType VARCHAR(32) NOT NULL);");
	 Serial.println(F("\n*******************************************S****************************************************"));
  Serial.println(F("FREE HEAP"));
  Serial.println(ESP.getFreeHeap());
  Serial.println(F("\n*******************************************E****************************************************"));

	op_type = OTHER_OP;
	String query = "CREATE TABLE IF NOT EXISTS LOCALSTORE (tid INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL , startDate VARCHAR(32) NOT NULL , stopDate VARCHAR(32) NOT NULL, units INTEGER NOT NULL , reason VARCHAR(32) NOT NULL);";
	int rc = db_exec(local_storage_db, query.c_str()); // Need to add the these columns
	op_type = OTHER_OP;
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		return DB_EXEC_ERROR;
	}
	op_type = OTHER_OP;
	rc = db_exec(local_storage_db, "SELECT * FROM LOCALSTORE");
	op_type = OTHER_OP;
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		return DB_EXEC_ERROR;
	}
	gu8_sqlLite_Initilsed_flag = 1;
	return DB_NO_ERROR;
}

/*
 *@brief Sqlite3 database file open using sqlite3 file name and its Descriptor
 */
int LocalStorageClass::db_open(const char *filename, sqlite3 **db)
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
int LocalStorageClass::db_exec(sqlite3 *db, const char *sql)
{
	char *zErrMsg = 0;
	const char *data = "Callback function called";
	Serial.println(sql);
	long start = micros();
	int rc = -1;
	switch (op_type)
	{
	case RFID_EXISTS:
		rc = sqlite3_exec(db, sql, get_db_callback_local, (void *)data, &zErrMsg);
		break;
	case FETCH_RFID_LIST:
		rc = sqlite3_exec(db, sql, db_get_list_callback, (void *)data, &zErrMsg);
		break;
	case COUNT_TXNS:
		rc = sqlite3_exec(db, sql, db_get_count_callback, (void *)data, &zErrMsg);
		break;
	case OTHER_OP:
        rc = sqlite3_exec(db, sql, get_db_callback_local, (void *)data, &zErrMsg);
        break;
	case COUNT_RFIDS:
		rc = sqlite3_exec(db, sql, db_get_count_callback, (void *)data, &zErrMsg);
		break;
	case FETCH_TXN: 
        rc = sqlite3_exec(db, sql, db_get_txn_callback, (void *)data, &zErrMsg);
        break;
	default:
        rc = sqlite3_exec(db, sql, get_db_callback_local, (void *)data, &zErrMsg);
        break;
	}
	if (rc != SQLITE_OK)
	{
		//Serial.printf("SQL error: %s\n", zErrMsg);
		Serial.print(F("SQL error:"));
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

static int db_get_list_callback(void *data, int argc, char **argv, char **azColName)
{
	int i;
	// Serial.printf("%s: ", (const char*)data);
	for (i = 0; i < argc; i++) // Since there are max 4 rfid ?
	{
		Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");

		if (strcmp(azColName[i], "idTag") == 0)
		{
			// Add to LL
			LL_StoredRFIDs.add(argv[i]);
		}
	}

	return 0;
}

static int get_db_callback_local(void *data, int argc, char **argv, char **azColName)
{
	int i;
	// Serial.printf("%s: ", (const char *)data);
	for (i = 0; i < argc; i++)
	{
		 Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		 uint8_t s;
            s = argv[i] ? 1 : 0;
            switch (s)
            {
            case 0: // Serial.println(F("not found!"));
                found = false;
                break;
            case 1: // Serial.println(F("found!"));
                found = true;
                break;
            }
	}
	 Serial.println(F("\n"));
	return 0;
}



/*
* @brief : These call back functions get called each time a query matches, i.e. if 3 results are hit, then it gets called thrice!
*/
static int db_get_count_callback(void *data, int argc, char **argv, char **azColName)
{
    counter++;
	return 0;
}

/*
 * @brief : These call back functions get called each time a query matches, i.e. if 3 results are hit, then it gets called thrice!
 */
static int db_get_txn_callback(void *data, int argc, char **argv, char **azColName)
{
        /*
        mysql> describe LOCALSTORE;
+-----------+-------------+------+-----+---------+-------+
| Field     | Type        | Null | Key | Default | Extra |
+-----------+-------------+------+-----+---------+-------+
| tid       | int         | NO   |     | NULL    |       |
| idTag     | varchar(32) | NO   |     | NULL    |       |
| startDate | varchar(32) | NO   |     | NULL    |       |
| stopDate  | varchar(32) | NO   |     | NULL    |       |
| units     | int         | NO   |     | NULL    |       |
| reason    | varchar(32) | NO   |     | NULL    |       |
+-----------+-------------+------+-----+---------+-------+
6 rows in set (0.16 sec)

mysql> INSERT INTO LOCALSTORE VALUES (1 , 'd787007e' , '280922034012', '280922034012' , 32 , 'local');
Query OK, 1 row affected (0.25 sec)
*/

     int i;
    // Serial.printf("%s: ", (const char*)data);
    for (i = 0; i < argc; i++)
    {
        if (strcmp(azColName[i], "tid") == 0)
        {
            // Add to LL
            ss_t.tid = argv[i];
        }
        else if (strcmp(azColName[i], "idTag") == 0)
        {
            // Add to LL
            ss_t.rfid = (argv[i]);
        }
        else if (strcmp(azColName[i], "startDate") == 0)
        {
            // Add to LL
            ss_t.start_date = (argv[i]);
        }
        else if (strcmp(azColName[i], "stopDate") == 0)
        {
            // Add to LL
            ss_t.stop_date =(argv[i]);
        }
        else if (strcmp(azColName[i], "units") == 0)
        {
            // Add to LL
            ss_t.units =(argv[i]);
        }
        else if (strcmp(azColName[i], "reason") == 0)
        {
            // Add to LL
            ss_t.ros =(argv[i]);
        }
    }

    return 0;
}

operation_status LocalStorageClass::db_set_rfid(String tag,String position)
{

	/*  -- Check the mysql Database exist or not, if not then create Database file.
	 *	CREATE LOCAL LIST DATABASE file and locallist dB
	 *
	 */
	if (local_storage_db == NULL)
	{
		if (db_localStoreObject.db_open(local_storage_db_filename, &local_storage_db))
		{
			Serial.print(local_storage_db_filename);
			Serial.print(F(" file not Opened\n"));
			return STORE_FAIL;
		}
	}

	/*  -- Create Table
	 *	CREATE TABLE IF NOT EXISTS RFID_LIST (	id INTEGER NOT NULL ,
	 *											idTag VARCHAR(32) NOT NULL);
	 *
	 */
	op_type = OTHER_OP;
	int rc = db_exec(local_storage_db, "CREATE TABLE IF NOT EXISTS RFID_LIST (id INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL);"); // Need to add the these columns
	op_type = OTHER_OP;
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		if (db_localStoreObject.db_open(local_storage_db_filename, &local_storage_db))
		{
			Serial.print(local_storage_db_filename);
			Serial.print(F(" file not Opened\n"));
			return STORE_FAIL;
		}
	}

	/*
	 * Query if the rfid exists or not!
	 */
	found = false;
	String query = "SELECT idTag FROM RFID_LIST where idTag='" + tag + "';";
	op_type = RFID_EXISTS;
	rc = db_exec(local_storage_db, query.c_str());
	op_type = OTHER_OP;
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		return STORE_FAIL;
	}

	// Fetch the rfid count. Limit it to 4
	counter = 0;
	query = "SELECT * FROM RFID_LIST;";
	//query = "SELECT idTag FROM RFID_LIST where idTag='" + tag + "';";
	op_type = COUNT_RFIDS;
	rc = db_exec(local_storage_db, query.c_str());
	op_type = OTHER_OP;
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		return STORE_FAIL;
	}
	
	if (!found)
	{
		/*
		mysql> UPDATE RFID_LIST SET idTag = 'd787007e' WHERE id = 1;
		Query OK, 1 row affected (0.22 sec)
		Rows matched: 1  Changed: 1  Warnings: 0
		*/
		#if 0
		counter = counter + 1; // such that the tag starts from 1.
		if (counter >= 4)
		{
			counter = 4;
			query = "UPDATE RFID_LIST SET idTag = '"+tag+"' WHERE id = "+String(counter)+";";
		}
		else
		{
			query = "INSERT INTO RFID_LIST VALUES (" + String(counter) + ", '" + tag + "');";
		}
		#endif
		
		if(counter > 1)
		{
			query = "UPDATE RFID_LIST SET idTag = '"+tag+"' WHERE id = "+String(position)+";";
		}
		else
		{
			query = "INSERT INTO RFID_LIST VALUES (" + String(position) + ", '" + tag + "');";
		}

		rc = db_exec(local_storage_db, query.c_str());
		op_type = OTHER_OP;
		if (rc != SQLITE_OK)
		{
			sqlite3_close(local_storage_db);
			return STORE_FAIL;
		}
	}
	else
	{
		//It has been found. Do not insert again.
		return STORE_FAIL;
	}
	return STORE_OK;
}

operation_status LocalStorageClass::db_get_rfid_list()
{

	/*  -- Check the mysql Database exist or not, if not then create Database file.
	 *	CREATE LOCAL LIST DATABASE file and locallist dB
	 *
	 */

	/*
	* @brief : Consider clearing the linked list before performing the operation.
	*/

	LL_StoredRFIDs.clear();
	if (local_storage_db == NULL)
	{
		if (db_localStoreObject.db_open(local_storage_db_filename, &local_storage_db))
		{
			Serial.print(local_storage_db_filename);
			Serial.print(F(" file not Opened\n"));
			return STORE_FAIL;
		}
	}

	/*  -- Create Table
	 *	CREATE TABLE IF NOT EXISTS RFID_LIST (	id INTEGER NOT NULL ,
	 *											idTag VARCHAR(32) NOT NULL);
	 *
	 */
	int rc = db_exec(local_storage_db, "CREATE TABLE IF NOT EXISTS RFID_LIST (id INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL);"); // Need to add the these columns
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		if (db_localStoreObject.db_open(local_storage_db_filename, &local_storage_db))
		{
			Serial.print(local_storage_db_filename);
			Serial.print(F(" file not Opened\n"));
			return STORE_FAIL;
		}
	}

	op_type = FETCH_RFID_LIST;
	rc = db_exec(local_storage_db, "SELECT idTag FROM RFID_LIST;");
	op_type = OTHER_OP;
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		return STORE_FAIL;
	}

	return STORE_OK;
}

operation_status LocalStorageClass::db_check_rfid_auth(String tag)
{

	/*  -- Check the mysql Database exist or not, if not then create Database file.
	 *	CREATE LOCAL LIST DATABASE file and locallist dB
	 *
	 */

	/*
	* @brief : Consider clearing the linked list before performing the operation.
	*/

	LL_StoredRFIDs.clear();
	if (local_storage_db == NULL)
	{
		if (db_localStoreObject.db_open(local_storage_db_filename, &local_storage_db))
		{
			Serial.print(local_storage_db_filename);
			Serial.print(F(" file not Opened\n"));
			return STORE_FAIL;
		}
	}

	/*  -- Create Table
	 *	CREATE TABLE IF NOT EXISTS RFID_LIST (	id INTEGER NOT NULL ,
	 *											idTag VARCHAR(32) NOT NULL);
	 *
	 */
	int rc = db_exec(local_storage_db, "CREATE TABLE IF NOT EXISTS RFID_LIST (id INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL);"); // Need to add the these columns
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		if (db_localStoreObject.db_open(local_storage_db_filename, &local_storage_db))
		{
			Serial.print(local_storage_db_filename);
			Serial.print(F(" file not Opened\n"));
			return STORE_FAIL;
		}
	}
	String query = "SELECT idTag FROM RFID_LIST WHERE idTag = '"+tag+"';";
	op_type = COUNT_RFIDS;
	rc = db_exec(local_storage_db, query.c_str());
	op_type = OTHER_OP;
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		return STORE_FAIL;
	}

	return STORE_OK;
}

/*
@brief: In order to set a txn, use the below.
  txM_doc["type"] = "request";
  txM_doc["object"] = "storestarttxn";
  txM_doc["idTag"] = rfid_tag;
  txM_doc["start_date"] = start_date;
  txM_doc["stop_date"] = stop_date;
  txM_doc["units"] = units;
  txM_doc["reason_of_stop"] = reason_of_stop; */
operation_status LocalStorageClass::db_set_txn(String tid, String tag,String start_dt, String stop_dt,String un,String ros)
{

	/*  -- Check the mysql Database exist or not, if not then create Database file.
	 *	CREATE LOCAL LIST DATABASE file and locallist dB
	 *
	 */
	if (local_storage_db == NULL)
	{
		if (db_localStoreObject.db_open(local_storage_db_filename, &local_storage_db))
		{
			Serial.print(local_storage_db_filename);
			Serial.print(F(" file not Opened\n"));
			return STORE_FAIL;
		}
	}

	/*  -- Create Table
	 *	CREATE TABLE IF NOT EXISTS LOCALSTORE (tid INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL , startDate VARCHAR(32) NOT NULL , stopDate VARCHAR(32) NOT NULL, units INTEGER NOT NULL , reason VARCHAR(32) NOT NULL);
	 * mysql> describe LOCALSTORE;
+-----------+-------------+------+-----+---------+-------+
| Field     | Type        | Null | Key | Default | Extra |
+-----------+-------------+------+-----+---------+-------+
| tid       | int         | NO   |     | NULL    |       |
| idTag     | varchar(32) | NO   |     | NULL    |       |
| startDate | varchar(32) | NO   |     | NULL    |       |
| stopDate  | varchar(32) | NO   |     | NULL    |       |
| units     | int         | NO   |     | NULL    |       |
| reason    | varchar(32) | NO   |     | NULL    |       |
+-----------+-------------+------+-----+---------+-------+
6 rows in set (0.32 sec)
	 */ 
	int rc = db_exec(local_storage_db, "CREATE TABLE IF NOT EXISTS LOCALSTORE (tid INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL , startDate VARCHAR(32) NOT NULL , stopDate VARCHAR(32) NOT NULL, units INTEGER NOT NULL , reason VARCHAR(32) NOT NULL);"); // Need to add the these columns
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		if (db_localStoreObject.db_open(local_storage_db_filename, &local_storage_db))
		{
			Serial.print(local_storage_db_filename);
			Serial.print(F(" file not Opened\n"));
			return STORE_FAIL;
		}
	}

	/*
	 * Query to push the request : Update request
	 */
	String query = "SELECT tid FROM LOCALSTORE WHERE tid='"+tid+"';"; // Fetch the existing TID
	counter = 0;
	op_type = COUNT_TXNS;
	rc = db_exec(local_storage_db, query.c_str());
	op_type = OTHER_OP;
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		return STORE_FAIL;
	}
 //If tid exists, then the query should be to insert if not then the query will be update!

    Serial.printf("The total no. of search results are: %d\n",counter);
	String query2;
    if(counter > 0)
    {
        /*mysql> UPDATE LOCALSTORE SET stopDate = '280922034012' , units = 33 WHERE tid = 1;
Query OK, 1 row affected (0.13 sec)
Rows matched: 1  Changed: 1  Warnings: 0*/
        
		//if start_dt = empty / stop_dt = empty then do not consider them in the update query.
		if(start_dt=="")
		{
		Serial.println(F("Update query with start_dt empty!"));
		if(stop_dt!="")
		{
			Serial.println(F("Update query NO issue with stop_dt empty!"));
		query2 = "UPDATE LOCALSTORE SET stopDate = '"+stop_dt+"' , units = "+un+" , reason = '"+ros+"' WHERE tid = "+tid+";";
		}
		else
		{
		query2 = "UPDATE LOCALSTORE SET units = "+un+" , reason = '"+ros+"' WHERE tid = "+tid+";";
		}
		}
		else
		{
		Serial.println(F("Update query with start_dt proper!"));
        query2 = "UPDATE LOCALSTORE SET startDate = '"+start_dt+"' stopDate = '"+stop_dt+"' , units = "+un+" , reason = '"+ros+"' WHERE tid = "+tid+";";
		}
    }
    else
    {
        /*
        mysql> describe LOCALSTORE;
+-----------+-------------+------+-----+---------+-------+
| Field     | Type        | Null | Key | Default | Extra |
+-----------+-------------+------+-----+---------+-------+
| tid       | int         | NO   |     | NULL    |       |
| idTag     | varchar(32) | NO   |     | NULL    |       |
| startDate | varchar(32) | NO   |     | NULL    |       |
| stopDate  | varchar(32) | NO   |     | NULL    |       |
| units     | int         | NO   |     | NULL    |       |
| reason    | varchar(32) | NO   |     | NULL    |       |
+-----------+-------------+------+-----+---------+-------+
6 rows in set (0.16 sec)

mysql> INSERT INTO LOCALSTORE VALUES (1 , 'd787007e' , '280922034012', '280922034012' , 32 , 'local');
Query OK, 1 row affected (0.25 sec)
*/
        Serial.println(F("Insert query."));
        query2 = "INSERT INTO LOCALSTORE VALUES ("+tid+" , '"+tag+"' , '"+start_dt+"', '"+stop_dt+"' , "+un+" , '"+ros+"');";
    }
	counter = 0; // reset the counter.
    op_type = OTHER_OP;
    rc = db_exec(local_storage_db, query2.c_str());
    op_type = OTHER_OP;
    if (rc != SQLITE_OK)
    {
        sqlite3_close(local_storage_db);
        return STORE_FAIL;
    }

	return STORE_OK;
}

/*
* @brief : Get a txn one after the other!
*/

operation_status LocalStorageClass::db_get_txn(String txnid)
{

	/*  -- Check the mysql Database exist or not, if not then create Database file.
	 *	CREATE LOCAL LIST DATABASE file and locallist dB
	 *
	 */
	LL_Local_Stored_List.clear();
	//Serial.printf("[LocalStorage] The received txn id is: %s",txnid);
	if (local_storage_db == NULL)
	{
		if (db_localStoreObject.db_open(local_storage_db_filename, &local_storage_db))
		{
			Serial.print(local_storage_db_filename);
			Serial.print(F(" file not Opened\n"));
			return STORE_FAIL;
		}
	}

	/*  -- Create Table
	 *	CREATE TABLE IF NOT EXISTS RFID_LIST (	id INTEGER NOT NULL ,
	 *											idTag VARCHAR(32) NOT NULL);
	 *
	 */
	int rc = db_exec(local_storage_db, "CREATE TABLE IF NOT EXISTS LOCALSTORE (tid INTEGER NOT NULL , idTag VARCHAR(32) NOT NULL , startDate VARCHAR(32) NOT NULL , stopDate VARCHAR(32) NOT NULL, units INTEGER NOT NULL , reason VARCHAR(32) NOT NULL);"); // Need to add the these columns
	
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		if (db_localStoreObject.db_open(local_storage_db_filename, &local_storage_db))
		{
			Serial.print(local_storage_db_filename);
			Serial.print(F(" file not Opened\n"));
			return STORE_FAIL;
		}
	}

	op_type = FETCH_TXN;
	String query = "SELECT * FROM LOCALSTORE WHERE tid = "+txnid+";";
	rc = db_exec(local_storage_db, query.c_str());
	op_type = OTHER_OP;
	if (rc != SQLITE_OK)
	{
		sqlite3_close(local_storage_db);
		return STORE_FAIL;
	}

	// Update it in a linked list

	LL_Local_Stored_List.add(ss_t);

	return STORE_OK;
}

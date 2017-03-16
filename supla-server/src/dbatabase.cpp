/*
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <my_global.h>
#include <mysql.h>
#include <string.h>


#include "database.h"
#include "svrcfg.h"
#include "log.h"
#include "tools.h"

char *database::cfg_get_host(void) {

	return scfg_string(CFG_MYSQL_HOST);
}

char *database::cfg_get_user(void) {

	return scfg_string(CFG_MYSQL_USER);
}
char *database::cfg_get_password(void) {

	return scfg_string(CFG_MYSQL_PASSWORD);
}
char *database::cfg_get_database(void) {

	return scfg_string(CFG_MYSQL_DB);
}
int database::cfg_get_port(void) {

	return scfg_int(CFG_MYSQL_PORT);
}

bool database::auth(const char *query, int ID, char *_PWD, int _PWD_HEXSIZE, int *UserID, bool *is_enabled, int *limit) {

	*is_enabled = true;

	if ( _mysql == NULL
			|| ID == 0
			|| strlen(_PWD) < 1 )
		return false;

	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	char PWD[_PWD_HEXSIZE];
	st_str2hex(PWD, _PWD);

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ID;

	pbind[1].buffer_type= MYSQL_TYPE_STRING;
	pbind[1].buffer= (char *)PWD;
	pbind[1].buffer_length = strlen(PWD);

	int __ID = 0;
	int __is_enabled = 1;
	MYSQL_STMT *stmt;
	stmt_get_int((void**)&stmt, &__ID, UserID, &__is_enabled, limit, query, pbind, 2);

	*is_enabled = __is_enabled == 1;

	return __ID != 0 && __is_enabled == 1;

}

bool database::location_auth(int LocationID, char *LocationPWD, int *UserID, bool *is_enabled, int *limit_iodev) {

	return auth("SELECT id, user_id, `enabled`, `limit_iodev` FROM `supla_v_device_location` WHERE id = ? AND password = unhex(?)",
			    LocationID,
			    LocationPWD,
			    SUPLA_LOCATION_PWDHEX_MAXSIZE,
			    UserID,
			    is_enabled,
			    limit_iodev);
}

bool database::accessid_auth(int AccessID, char *AccessIDpwd, int *UserID, bool *is_enabled, int *limit_client) {

	return auth("SELECT id, user_id, `enabled`, `limit_client` FROM `supla_v_device_accessid` WHERE id = ? AND password = unhex(?)",
				AccessID,
				AccessIDpwd,
				SUPLA_ACCESSID_PWDHEX_MAXSIZE,
			    UserID,
			    is_enabled,
			    limit_client);

}

int database::get_device_id(const char GUID[SUPLA_GUID_SIZE], int *location_id, int *oryginal_location_id, bool *is_enabled) {

	if ( _mysql == NULL )
		return false;

	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	char GUIDHEX[SUPLA_GUID_HEXSIZE];
	st_guid2hex(GUIDHEX, GUID);

	pbind[0].buffer_type= MYSQL_TYPE_STRING;
	pbind[0].buffer= (char *)GUIDHEX;
	pbind[0].buffer_length= SUPLA_GUID_SIZE*2;

	int _is_enabled = 0;

	MYSQL_STMT *stmt;
	int dev_id;
	if ( stmt_get_int((void**)&stmt, &dev_id, location_id, oryginal_location_id, &_is_enabled, "SELECT id, location_id, oryginal_location_id, CAST(`enabled` AS unsigned integer) `enabled` FROM supla_iodevice WHERE guid = unhex(?)", pbind, 1) ) {
		*is_enabled = _is_enabled == 1;
        return dev_id;
	}

	*is_enabled = _is_enabled == 1;
	return 0;

}

bool database::on_newdevice(int DeviceID) {

	char sql[51];
	snprintf(sql, 50, "CALL `supla_on_newdevice`(%i)", DeviceID);


	return query(sql) != 0;

}

bool database::on_channeladded(int DeviceID, int ChannelID) {

	char sql[51];
	snprintf(sql, 50, "CALL `supla_on_channeladded`(%i, %i)", DeviceID, ChannelID);


	return query(sql) != 0;
}


int database::get_device_count(int UserID) {
    return get_count(UserID, "SELECT COUNT(*) FROM supla_iodevice WHERE user_id = ?");
}

int database::add_device(int *LocationID, const char GUID[SUPLA_GUID_SIZE], const char Name[SUPLA_DEVICE_NAME_MAXSIZE],
		                   unsigned int ipv4, char softver[SUPLA_SOFTVER_MAXSIZE], int proto_version,
		                   int UserID, bool *new_device, bool *is_enabled, int *Limit) {

	int _LocationID = 0, _OryginalLocationID = 0;
	bool _is_enabled;

	if ( is_enabled == NULL )
		is_enabled = & _is_enabled;

	*is_enabled = true;

	int device_id = get_device_id(GUID, &_LocationID, &_OryginalLocationID, is_enabled);

	if ( device_id != 0
			&& is_enabled
			&& *is_enabled == false )
		return 0;


	char NameHEX[SUPLA_DEVICE_NAMEHEX_MAXSIZE];

	st_str2hex(NameHEX, Name);

	if ( device_id == 0 ) {


		//In this case (without db locks) it's possible to over the limit by 1 but it's not the problem
		(*Limit) -= get_device_count(UserID);


		if ( *Limit <= 0 )
			return 0;

		MYSQL_BIND pbind[8];
		memset(pbind, 0, sizeof(pbind));

		char GUIDHEX[SUPLA_GUID_HEXSIZE];
		st_guid2hex(GUIDHEX, GUID);

		pbind[0].buffer_type= MYSQL_TYPE_LONG;
		pbind[0].buffer= (char *)LocationID;

		pbind[1].buffer_type= MYSQL_TYPE_STRING;
		pbind[1].buffer= (char *)NameHEX;
		pbind[1].buffer_length = strlen(NameHEX);

		pbind[2].buffer_type= MYSQL_TYPE_LONG;
		pbind[2].buffer= (char *)&UserID;

		pbind[3].buffer_type= MYSQL_TYPE_LONG;
		pbind[3].buffer= (char *)&ipv4;

		pbind[4].buffer_type= MYSQL_TYPE_LONG;
		pbind[4].buffer= (char *)&ipv4;

		pbind[5].buffer_type= MYSQL_TYPE_STRING;
		pbind[5].buffer= (char *)GUIDHEX;
		pbind[5].buffer_length= (SUPLA_GUID_SIZE*2);

		pbind[6].buffer_type= MYSQL_TYPE_STRING;
		pbind[6].buffer= (char *)softver;
		pbind[6].buffer_length = strlen(softver);

		pbind[7].buffer_type= MYSQL_TYPE_LONG;
		pbind[7].buffer= (char *)&proto_version;

		const char sql[] = "INSERT INTO `supla_iodevice`(`location_id`, `oryginal_location_id`, `name`, `enabled`, `reg_date`, `last_connected`, `user_id`, `reg_ipv4`, `last_ipv4`, `guid`, `software_version`, `protocol_version`) VALUES (?,NULL,unhex(?),1,NOW(),NOW(),?,?,?,unhex(?),?, ?)";

		MYSQL_STMT *stmt;
		stmt_execute((void**)&stmt, sql, pbind, 8, false);

		if ( stmt != NULL )
			  mysql_stmt_close(stmt);

		device_id = get_device_id(GUID, &_LocationID, &_OryginalLocationID, is_enabled);

		if ( device_id != 0
			 && _LocationID != *LocationID
			 && _OryginalLocationID != *LocationID
			 && ( is_enabled == NULL || *is_enabled == true ) ) {

				 *LocationID = 0;
				 device_id = 0;
		} else if ( new_device != NULL ) {

			*new_device = true;

		}


	} else if ( _LocationID != *LocationID
			    && _OryginalLocationID != *LocationID ) {

		*LocationID = 0;
		device_id = 0;

	} else {

		if ( *LocationID == _LocationID ) {
			_OryginalLocationID = 0;
		}

		MYSQL_BIND pbind[7];
		memset(pbind, 0, sizeof(pbind));

		pbind[0].buffer_type= MYSQL_TYPE_STRING;
		pbind[0].buffer= (char *)NameHEX;
		pbind[0].buffer_length = strlen(NameHEX);

		pbind[1].buffer_type= MYSQL_TYPE_LONG;
		pbind[1].buffer= (char *)&ipv4;

		pbind[2].buffer_type= MYSQL_TYPE_STRING;
		pbind[2].buffer= (char *)softver;
		pbind[2].buffer_length = strlen(softver);

		pbind[3].buffer_type= MYSQL_TYPE_LONG;
		pbind[3].buffer= (char *)&proto_version;

		pbind[4].buffer_type= MYSQL_TYPE_LONG;
		pbind[4].buffer= (char *)&device_id;

		pbind[5].buffer_type= MYSQL_TYPE_LONG;
		pbind[5].buffer= (char *)&_LocationID;

		pbind[6].buffer_type= MYSQL_TYPE_LONG;
		pbind[6].buffer= (char *)&_OryginalLocationID;

		if ( _OryginalLocationID == 0 )
			pbind[6].is_null_value = true;

		const char sql[] = "UPDATE `supla_iodevice` SET `name` = unhex(?), `last_connected` = NOW(), `last_ipv4` = ?, `software_version` = ?, `protocol_version` = ?, location_id = ?, oryginal_location_id = ? WHERE id = ?";

		MYSQL_STMT *stmt;
		if ( !stmt_execute((void**)&stmt, sql, pbind, 7) ) {
			device_id = 0;
		}

		if ( stmt != NULL )
			  mysql_stmt_close(stmt);
	}

	return device_id;
}

int database::get_device_channel_count(int DeviceID) {

	if ( _mysql == NULL )
		return 0;


	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&DeviceID;

	MYSQL_STMT *stmt;
	int count = 0;

	stmt_get_int((void**)&stmt, &count, NULL, NULL, NULL, "SELECT COUNT(*) FROM `supla_dev_channel` WHERE `iodevice_id` = ?", pbind, 1);

	return count;
}

int database::get_device_channel_id(int DeviceID, int ChannelNumber, int *Type) {

	if ( _mysql == NULL )
		return 0;

	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&DeviceID;

	pbind[1].buffer_type= MYSQL_TYPE_LONG;
	pbind[1].buffer= (char *)&ChannelNumber;

	MYSQL_STMT *stmt;
	int id = 0;
	int _type;

	if ( Type == NULL )
		Type = &_type;

	stmt_get_int((void**)&stmt, &id, Type, NULL, NULL, "SELECT `id`, `type` FROM `supla_dev_channel` WHERE `iodevice_id` = ? AND `channel_number` = ?", pbind, 2);

	return id;
}


int database::add_device_channel(int DeviceID, int ChannelNumber, int Type, int Func, int FList, int UserID, bool *new_channel) {

	MYSQL_BIND pbind[6];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&Type;

	pbind[1].buffer_type= MYSQL_TYPE_LONG;
	pbind[1].buffer= (char *)&Func;

	pbind[2].buffer_type= MYSQL_TYPE_LONG;
	pbind[2].buffer= (char *)&UserID;

	pbind[3].buffer_type= MYSQL_TYPE_LONG;
	pbind[3].buffer= (char *)&ChannelNumber;

	pbind[4].buffer_type= MYSQL_TYPE_LONG;
	pbind[4].buffer= (char *)&DeviceID;

	pbind[5].buffer_type= MYSQL_TYPE_LONG;
	pbind[5].buffer= (char *)&FList;

	{
		const char sql[] = "INSERT INTO `supla_dev_channel` (`type`, `func`, `param1`, `param2`, `param3`, `user_id`, `channel_number`, `iodevice_id`, `flist`) VALUES (?,?,0,0,0,?,?,?,?)";

		MYSQL_STMT *stmt;
		if ( !stmt_execute((void**)&stmt, sql, pbind, 6, false) ) {
			mysql_stmt_close(stmt);
			return 0;
		} else if ( new_channel ) {
			*new_channel = true;
		}

		if ( stmt != NULL )
			  mysql_stmt_close(stmt);
	}

	return get_device_channel_id(DeviceID, ChannelNumber, NULL);

}

void database::get_device_channels(int DeviceID, supla_device_channels *channels) {


	MYSQL_STMT *stmt;
	const char sql[] = "SELECT `type`, `func`, `param1`, `param2`, `param3`, `channel_number`, `id` FROM `supla_dev_channel` WHERE `iodevice_id` = ? ORDER BY `channel_number`";

	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&DeviceID;

	if ( stmt_execute((void**)&stmt, sql, pbind, 1, true) ) {

		my_bool       is_null[5];

		MYSQL_BIND rbind[7];
		memset(rbind, 0, sizeof(rbind));

		int type, func, param1, param2, param3, number, id;

		rbind[0].buffer_type= MYSQL_TYPE_LONG;
		rbind[0].buffer= (char *)&type;
		rbind[0].is_null= &is_null[0];

		rbind[1].buffer_type= MYSQL_TYPE_LONG;
		rbind[1].buffer= (char *)&func;
		rbind[1].is_null= &is_null[1];

		rbind[2].buffer_type= MYSQL_TYPE_LONG;
		rbind[2].buffer= (char *)&param1;
		rbind[2].is_null= &is_null[2];

		rbind[3].buffer_type= MYSQL_TYPE_LONG;
		rbind[3].buffer= (char *)&param2;
		rbind[3].is_null= &is_null[3];

		rbind[4].buffer_type= MYSQL_TYPE_LONG;
		rbind[4].buffer= (char *)&param3;
		rbind[4].is_null= &is_null[4];

		rbind[5].buffer_type= MYSQL_TYPE_LONG;
		rbind[5].buffer= (char *)&number;

		rbind[6].buffer_type= MYSQL_TYPE_LONG;
		rbind[6].buffer= (char *)&id;



		if ( mysql_stmt_bind_result(stmt, rbind) ) {
			supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));
		} else {

			mysql_stmt_store_result(stmt);

			if ( mysql_stmt_num_rows(stmt) > 0 ) {

				while (!mysql_stmt_fetch(stmt)) {


					if ( is_null[0] == true ) type = 0;
					if ( is_null[1] == true ) func = 0;
					if ( is_null[2] == true ) param1 = 0;
					if ( is_null[3] == true ) param2 = 0;
					if ( is_null[4] == true ) param3 = 0;

					channels->add_channel(id, number, type, func, param1, param2, param3);
				}

			}

		}


		mysql_stmt_close(stmt);
	}

}

int database::get_client_id(const char GUID[SUPLA_GUID_SIZE], int access_id, bool *is_enabled) {

	if ( _mysql == NULL )
		return false;

	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	char GUIDHEX[SUPLA_GUID_HEXSIZE];
	st_guid2hex(GUIDHEX, GUID);

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&access_id;

	pbind[1].buffer_type= MYSQL_TYPE_STRING;
	pbind[1].buffer= (char *)GUIDHEX;
	pbind[1].buffer_length= SUPLA_GUID_SIZE*2;

	int _is_enabled = 0;

	MYSQL_STMT *stmt;
	int client_id;
	if ( stmt_get_int((void**)&stmt, &client_id, &_is_enabled, NULL, NULL, "SELECT id, CAST(`enabled` AS unsigned integer) `enabled` FROM supla_client WHERE access_id = ? AND guid = unhex(?)", pbind, 2) ) {
		*is_enabled = _is_enabled == 1;
        return client_id;
	}

	*is_enabled = _is_enabled == 1;
	return 0;

}

int database::get_client_count(int UserID) {
    return get_count(UserID, "SELECT COUNT(*) FROM supla_v_client WHERE user_id = ?");
}

int database::add_client(int AccessID, const char GUID[SUPLA_GUID_SIZE], const char Name[SUPLA_DEVICE_NAME_MAXSIZE],
		                   unsigned int ipv4, char softver[SUPLA_SOFTVER_MAXSIZE], int proto_version, int UserID, bool *is_enabled, int *Limit) {


	bool _is_enabled;

	if ( is_enabled == NULL )
		is_enabled = & _is_enabled;

	*is_enabled = true;

	int client_id = get_client_id(GUID, AccessID, is_enabled);

	if ( client_id != 0
			&& is_enabled
			&& *is_enabled == false )
		return 0;


	char NameHEX[SUPLA_DEVICE_NAMEHEX_MAXSIZE];

	st_str2hex(NameHEX, Name);


	if ( client_id == 0 ) {


		//In this case (without db locks) it's possible to over the limit by 1 but it's not the problem
		(*Limit) -= get_client_count(UserID);

		if ( *Limit <= 0 )
			return 0;

		MYSQL_BIND pbind[7];
		memset(pbind, 0, sizeof(pbind));

		char GUIDHEX[SUPLA_GUID_HEXSIZE];
		st_guid2hex(GUIDHEX, GUID);

		pbind[0].buffer_type= MYSQL_TYPE_LONG;
		pbind[0].buffer= (char *)&AccessID;

		pbind[1].buffer_type= MYSQL_TYPE_STRING;
		pbind[1].buffer= (char *)GUIDHEX;
		pbind[1].buffer_length= (SUPLA_GUID_SIZE*2);

		pbind[2].buffer_type= MYSQL_TYPE_STRING;
		pbind[2].buffer= (char *)NameHEX;
		pbind[2].buffer_length = strlen(NameHEX);

		pbind[3].buffer_type= MYSQL_TYPE_LONG;
		pbind[3].buffer= (char *)&ipv4;

		pbind[4].buffer_type= MYSQL_TYPE_LONG;
		pbind[4].buffer= (char *)&ipv4;

		pbind[5].buffer_type= MYSQL_TYPE_STRING;
		pbind[5].buffer= (char *)softver;
		pbind[5].buffer_length = strlen(softver);

		pbind[6].buffer_type= MYSQL_TYPE_LONG;
		pbind[6].buffer= (char *)&proto_version;

		const char sql[] = "INSERT INTO `supla_client`(`access_id`, `guid`, `name`, `enabled`, `reg_ipv4`, `reg_date`, `last_access_ipv4`, `last_access_date`, `software_version`, `protocol_version`) VALUES (?,unhex(?),unhex(?),1,?,NOW(),?,NOW(),?,?)";


		MYSQL_STMT *stmt;
		stmt_execute((void**)&stmt, sql, pbind, 7, false);

		if ( stmt != NULL )
			  mysql_stmt_close(stmt);

		client_id = get_client_id(GUID, AccessID, is_enabled);

	} else {

		MYSQL_BIND pbind[5];
		memset(pbind, 0, sizeof(pbind));

		pbind[0].buffer_type= MYSQL_TYPE_STRING;
		pbind[0].buffer= (char *)NameHEX;
		pbind[0].buffer_length = strlen(NameHEX);

		pbind[1].buffer_type= MYSQL_TYPE_LONG;
		pbind[1].buffer= (char *)&ipv4;

		pbind[2].buffer_type= MYSQL_TYPE_STRING;
		pbind[2].buffer= (char *)softver;
		pbind[2].buffer_length = strlen(softver);

		pbind[3].buffer_type= MYSQL_TYPE_LONG;
		pbind[3].buffer= (char *)&proto_version;

		pbind[4].buffer_type= MYSQL_TYPE_LONG;
		pbind[4].buffer= (char *)&client_id;


		const char sql[] = "UPDATE `supla_client` SET `name` = unhex(?), `last_access_date` = NOW(), `last_access_ipv4` = ?, `software_version` = ?, `protocol_version` = ? WHERE id = ?";

		MYSQL_STMT *stmt;
		if ( !stmt_execute((void**)&stmt, sql, pbind, 5) ) {
			client_id = 0;
		}

		if ( stmt != NULL )
			  mysql_stmt_close(stmt);

	}

	return client_id;
}

void database::get_client_locations(int ClientID, supla_client_locations *locs) {


	MYSQL_STMT *stmt;

	const char sql[] = "SELECT `id`, `caption` FROM `supla_v_client_location`  WHERE `client_id` = ?";

	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ClientID;

	if ( stmt_execute((void**)&stmt, sql, pbind, 1, true) ) {

		my_bool       is_null[2];

		MYSQL_BIND rbind[2];
		memset(rbind, 0, sizeof(rbind));

		int id;
		unsigned long size;
		char caption[401]; // utf8

		rbind[0].buffer_type= MYSQL_TYPE_LONG;
		rbind[0].buffer= (char *)&id;
		rbind[0].is_null= &is_null[0];

		rbind[1].buffer_type= MYSQL_TYPE_STRING;
		rbind[1].buffer= caption;
		rbind[1].buffer_length =  401;
		rbind[1].length = &size;
		rbind[1].is_null= &is_null[1];


		if ( mysql_stmt_bind_result(stmt, rbind) ) {
			supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));
		} else {

			mysql_stmt_store_result(stmt);

			if ( mysql_stmt_num_rows(stmt) > 0 ) {


				while (!mysql_stmt_fetch(stmt)) {

					caption[size] = 0;
					locs->add_location(id, caption);

				}

			}

		}

		mysql_stmt_close(stmt);
	}

}

void database::get_client_channels(int ClientID, int *DeviceID, supla_client_channels *channels) {

	MYSQL_STMT *stmt;
	const char sql1[] = "SELECT `id`, `func`, `param1`, `iodevice_id`, `location_id`, `caption` FROM `supla_v_client_channel` WHERE `client_id` = ? ORDER BY `iodevice_id`, `channel_number`";
	const char sql2[] = "SELECT `id`, `func`, `param1`, `iodevice_id`, `location_id`, `caption` FROM `supla_v_client_channel` WHERE `client_id` = ? AND `iodevice_id` = ? ORDER BY `channel_number`";


	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ClientID;

	pbind[1].buffer_type= MYSQL_TYPE_LONG;
	pbind[1].buffer= (char *)DeviceID;

	if ( stmt_execute((void**)&stmt, DeviceID ? sql2 : sql1, pbind, DeviceID ? 2 : 1, true) ) {

		my_bool       is_null;

		MYSQL_BIND rbind[6];
		memset(rbind, 0, sizeof(rbind));

		int id, func, param1, iodevice_id, location_id;
		unsigned long size;
		char caption[401];

		rbind[0].buffer_type= MYSQL_TYPE_LONG;
		rbind[0].buffer= (char *)&id;

		rbind[1].buffer_type= MYSQL_TYPE_LONG;
		rbind[1].buffer= (char *)&func;

		rbind[2].buffer_type= MYSQL_TYPE_LONG;
		rbind[2].buffer= (char *)&param1;

		rbind[3].buffer_type= MYSQL_TYPE_LONG;
		rbind[3].buffer= (char *)&iodevice_id;

		rbind[4].buffer_type= MYSQL_TYPE_LONG;
		rbind[4].buffer= (char *)&location_id;

		rbind[5].buffer_type= MYSQL_TYPE_STRING;
		rbind[5].buffer= caption;
		rbind[5].is_null= &is_null;
		rbind[5].buffer_length = 401;
		rbind[5].length = &size;

		if ( mysql_stmt_bind_result(stmt, rbind) ) {
			supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));
		} else {

			mysql_stmt_store_result(stmt);

			if ( mysql_stmt_num_rows(stmt) > 0 ) {

				while (!mysql_stmt_fetch(stmt)) {

					if ( is_null == false )
						caption[size] = 0;

					channels->update_channel(id, iodevice_id, location_id, func, param1, is_null ? NULL : caption);
				}

			}

		}


		mysql_stmt_close(stmt);
	}

}

void database::add_temperature(int ChannelID, double temperature) {

	char buff[20];
	memset(buff, 0, sizeof(buff));

	MYSQL_BIND pbind[2];
	memset(pbind, 0, sizeof(pbind));

	snprintf(buff, 20, "%04.4f", temperature);

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ChannelID;

	pbind[1].buffer_type= MYSQL_TYPE_DECIMAL;
	pbind[1].buffer= (char *)buff;
	pbind[1].buffer_length = strlen(buff);


	const char sql[] = "INSERT INTO `supla_temperature_log`(`channel_id`, `date`, `temperature`) VALUES (?,NOW(),?)";

	MYSQL_STMT *stmt;
	stmt_execute((void**)&stmt, sql, pbind, 2, false);

	if ( stmt != NULL )
		  mysql_stmt_close(stmt);

}

void database::add_temperature_and_humidity(int ChannelID, double temperature, double humidity) {

	char buff1[20];
	memset(buff1, 0, sizeof(buff1));

	char buff2[20];
	memset(buff2, 0, sizeof(buff2));

	MYSQL_BIND pbind[3];
	memset(pbind, 0, sizeof(pbind));

	snprintf(buff1, 20, "%04.3f", temperature);
	snprintf(buff2, 20, "%04.3f", humidity);

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&ChannelID;

	pbind[1].buffer_type= MYSQL_TYPE_DECIMAL;
	pbind[1].buffer= (char *)buff1;
	pbind[1].buffer_length = strlen(buff1);

	pbind[2].buffer_type= MYSQL_TYPE_DECIMAL;
	pbind[2].buffer= (char *)buff2;
	pbind[2].buffer_length = strlen(buff2);

	const char sql[] = "INSERT INTO `supla_temphumidity_log`(`channel_id`, `date`, `temperature`, `humidity`) VALUES (?,NOW(),?,?)";

	MYSQL_STMT *stmt;
	stmt_execute((void**)&stmt, sql, pbind, 3, false);

	if ( stmt != NULL )
		  mysql_stmt_close(stmt);

}

bool database::get_oauth_user(char *access_token, int *OAuthUserID, int *UserID, int *expires_at) {

	MYSQL_STMT *stmt;
	MYSQL_BIND pbind[1];
	memset(pbind, 0, sizeof(pbind));

	bool result = false;

	pbind[0].buffer_type= MYSQL_TYPE_STRING;
	pbind[0].buffer= (char *)access_token;
	pbind[0].buffer_length = strlen(access_token);

	const char sql[] = "SELECT  t.user_id, c.parent_id, t.expires_at FROM `supla_oauth_access_tokens` AS t, `supla_oauth_clients` AS c WHERE c.id = t.client_id AND c.parent_id != 0 AND t.expires_at > UNIX_TIMESTAMP(NOW()) AND t.scope = 'restapi' AND token = ? LIMIT 1";

	if ( stmt_execute((void**)&stmt, sql, pbind, 1, true) ) {

		mysql_stmt_store_result(stmt);

		if ( mysql_stmt_num_rows(stmt) > 0 ) {

			MYSQL_BIND rbind[3];
			memset(rbind, 0, sizeof(rbind));

			int _OAuthUserID, _UserID, _expires_at;

			rbind[0].buffer_type= MYSQL_TYPE_LONG;
			rbind[0].buffer= (char *)&_OAuthUserID;

			rbind[1].buffer_type= MYSQL_TYPE_LONG;
			rbind[1].buffer= (char *)&_UserID;

			rbind[2].buffer_type= MYSQL_TYPE_LONG;
			rbind[2].buffer= (char *)&_expires_at;

			if ( mysql_stmt_bind_result(stmt, rbind) ) {
				supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));

			} else if ( mysql_stmt_fetch(stmt) == 0 ) {

				if ( OAuthUserID != NULL )
					*OAuthUserID = _OAuthUserID;

				if ( UserID != NULL )
					*UserID = _UserID;

				if ( expires_at != NULL ) {
					*expires_at = _expires_at;
				}

				result = true;

			}

		}

		mysql_stmt_free_result(stmt);
		mysql_stmt_close(stmt);
	}

	return result;
}

bool database::get_device_firmware_update_url(int DeviceID, TDS_FirmwareUpdateParams *params, TSD_FirmwareUpdate_UrlResult *url) {

	MYSQL_STMT *stmt;
	MYSQL_BIND pbind[6];
	memset(pbind, 0, sizeof(pbind));
	memset(url, 0, sizeof(TSD_FirmwareUpdate_UrlResult));

	bool result = false;

	pbind[0].buffer_type= MYSQL_TYPE_LONG;
	pbind[0].buffer= (char *)&DeviceID;

	pbind[1].buffer_type= MYSQL_TYPE_TINY;
	pbind[1].buffer= (char *)&params->Platform;

	pbind[2].buffer_type= MYSQL_TYPE_LONG;
	pbind[2].buffer= (char *)&params->Param1;

	pbind[3].buffer_type= MYSQL_TYPE_LONG;
	pbind[3].buffer= (char *)&params->Param2;

	pbind[4].buffer_type= MYSQL_TYPE_LONG;
	pbind[4].buffer= (char *)&params->Param3;

	pbind[5].buffer_type= MYSQL_TYPE_LONG;
	pbind[5].buffer= (char *)&params->Param4;

	const char sql1[] = "SET @p0=?, @p1=?, @p2=?, @p3=?, @p4=?, @p5=?";
	const char sql2[] = "CALL `supla_get_device_firmware_url`(@p0, @p1, @p2, @p3, @p4, @p5, @p6, @p7, @p8, @p9)";
	const char sql3[] = "SELECT @p6 AS `protocols`, @p7 AS `host`, @p8 AS `port`, @p9 AS `path`";

	char q_executed = 0;

	if ( stmt_execute((void**)&stmt, sql1, pbind, 6, true) ) {

		 mysql_stmt_close((MYSQL_STMT *)stmt);
		 q_executed++;
	}

	if ( stmt_execute((void**)&stmt, sql2, NULL, 0, true) ) {

		 mysql_stmt_close((MYSQL_STMT *)stmt);
		 q_executed++;
	}

	if ( q_executed == 2
		 && stmt_execute((void**)&stmt, sql3, NULL, 0, true) ) {

		mysql_stmt_store_result(stmt);

		if ( mysql_stmt_num_rows(stmt) > 0 ) {

			MYSQL_BIND rbind[4];
			memset(rbind, 0, sizeof(rbind));

			unsigned long host_size;
			my_bool       host_is_null;

			unsigned long path_size;
			my_bool       path_is_null;

			rbind[0].buffer_type= MYSQL_TYPE_TINY;
			rbind[0].buffer= (char *)&url->url.available_protocols;

			rbind[1].buffer_type= MYSQL_TYPE_STRING;
			rbind[1].buffer= url->url.host;
			rbind[1].is_null= &host_is_null;
			rbind[1].buffer_length = SUPLA_URL_HOST_MAXSIZE;
			rbind[1].length = &host_size;

			rbind[2].buffer_type= MYSQL_TYPE_LONG;
			rbind[2].buffer= (char *)&url->url.port;

			rbind[3].buffer_type= MYSQL_TYPE_STRING;
			rbind[3].buffer= url->url.path;
			rbind[3].is_null= &path_is_null;
			rbind[3].buffer_length = SUPLA_URL_PATH_MAXSIZE;
			rbind[3].length = &path_size;


			if ( mysql_stmt_bind_result(stmt, rbind) ) {
				supla_log(LOG_ERR, "MySQL - stmt bind error - %s", mysql_stmt_error(stmt));

			} else if ( mysql_stmt_fetch(stmt) == 0 ) {

				if ( host_is_null || host_size == 0 || host_size >= SUPLA_URL_HOST_MAXSIZE )
					url->url.host[0] = 0;

				if ( path_is_null || path_size == 0 || path_size >= SUPLA_URL_PATH_MAXSIZE )
					url->url.path[0] = 0;


				if ( url->url.available_protocols > 0
					 && strlen(url->url.host) > 0
					 && strlen(url->url.path) > 0 )
					url->exists = 1;

				result = true;

			}

		}

		mysql_stmt_free_result(stmt);
		mysql_stmt_close(stmt);
	}

	return result;
}


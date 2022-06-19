#pragma once
#include <mysql.h>
#include <string>

namespace MYSQLAPI {
	MYSQL* connectDb(const char*, const char*, const char*, const char*, unsigned int);
	MYSQL* connectTestDb();
	int makeQuery(MYSQL*, std::string);
	MYSQL_RES* queryResult(MYSQL*, std::string);
	MYSQL_RES* getTable(MYSQL*, std::string);
	std::string tableToString(MYSQL_RES*);
	
}
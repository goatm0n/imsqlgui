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
	struct Column {
		std::string name;
		std::string dataType;
	};
	template<typename... Columns> bool createTable(MYSQL*, std::string, Column, Columns...);
	bool dropTable(MYSQL*, std::string);
	template<typename... Columns> bool alterTable(MYSQL*, std::string, MYSQLAPI::Column, Columns...);
	struct Data : Column {
		std::string value;
	};
	template<typename... Datas> bool insertInto(MYSQL*, std::string, Data, Datas...);
	template<typename...Datas> bool updateTable(MYSQL*, std::string, Data, Data, Datas...);
	bool deleteRowFromTable(MYSQL*, std::string, Data);
}
/* INCLUDES */
#include "mysqlapi.h"
#include <iostream>

using namespace MYSQLAPI;

/* HEADERS */
std::string addColumnsToCreateTableQuery(std::string);
template<typename... Columns> std::string addColumnsToCreateTableQuery(std::string, Column, Columns...);
std::string addColumnsToAlterTableQuery(std::string);
template<typename... Columns> std::string addColumnsToAlterTableQuery(std::string, MYSQLAPI::Column, Columns...);
std::string addColumnsToInsertIntoQuery(std::string);
template<typename... Datas> std::string addColumnsToInsertIntoQuery(std::string, Data, Datas...);
std::string addValuesToInsertIntoQuery(std::string query);
template<typename... Datas> std::string addValuesToInsertIntoQuery(std::string, Data, Datas...);
template<typename...Datas> std::string addValuesToUpdateTableQuery(std::string, Data, Datas...);
std::string addValuesToUpdateTableQuery(std::string);


/* DEFINITIONS */
MYSQL* MYSQLAPI::connectDb(const char* host, const char* user, const char* password, const char* db, unsigned int port) {
	MYSQL* conn;
	conn = mysql_init(0);
	conn = mysql_real_connect(conn, host, user, password, db, port, NULL, 0);
	if (conn == NULL) {
		throw "Connection to database has failed!";
	}
	return conn;
}

MYSQL* MYSQLAPI::connectTestDb() {
	MYSQL* conn;
	const char* host = "localhost";
	const char* user = "root";
	const char* password = "password";
	const char* db = "testdb";
	const unsigned int port = 3306;
	try {
		conn = connectDb(host, user, password, db, port);
	}
	catch (const char* e) {
		std::cerr << e << std::endl;
		conn = nullptr;
	}
	return conn;
}

// returns 0 for success
int MYSQLAPI::makeQuery(MYSQL* conn, std::string query) {
	const char* q;
	int qstate;
	q = query.c_str();
	qstate = mysql_query(conn, q);
	if (!qstate) {
		return 0;
	}
	else {
		const char* err = mysql_error(conn);
		throw err;
	}
}

// returns nullptr if fails
MYSQL_RES* MYSQLAPI::queryResult(MYSQL* conn, std::string query) {
	MYSQL_RES* res;
	int qstate;
	try {
		qstate = makeQuery(conn, query);
	}
	catch (const char* e) {
		std::cout << "FAILED TO RESOLVE QUERY" << std::endl;
		std::cerr << e << std::endl;
		res = nullptr;
		return res;
	}
	res = mysql_store_result(conn);
	if (res == NULL) {
		const char* err = mysql_error(conn);
		std::cout << "mysql_store_result returned null" << std::endl;
		std::cerr << err << std::endl;
		res = nullptr;
	}
	return res;

}

MYSQL_RES* MYSQLAPI::getTable(MYSQL* conn, std::string table) {
	MYSQL_RES* res;
	std::string query;
	query = "SELECT * FROM " + table;
	res = queryResult(conn, query);
	return res;
}

std::string MYSQLAPI::tableToString(MYSQL_RES* table) {
	std::string tableString = "";
	MYSQL_ROW row;
	MYSQL_FIELD* fields;
	unsigned int num_fields;

	num_fields = mysql_num_fields(table);
	fields = mysql_fetch_fields(table);
	for (unsigned int i = 0; i < num_fields; i++) {
		tableString += fields[i].name;
		tableString += +", ";
	}
	tableString += "\n";
	while (row = mysql_fetch_row(table)) {
		tableString += row[0];
		tableString += ", ";
		tableString += row[1];
		tableString += ", ";
		tableString += row[2];
		tableString += ", ";
		tableString += "\n";
	}
	return tableString;
}

std::string addColumnsToCreateTableQuery(std::string query) {
	query.pop_back();
	return query;
}

template<typename... Columns>
std::string addColumnsToCreateTableQuery(std::string query, Column c1, Columns... cols) {
	query += c1.name + " " + c1.dataType + ",";
	query = addColumnsToCreateTableQuery(query, cols...);
	return query;
}


template<typename... Columns>
bool MYSQLAPI::createTable(MYSQL* conn, std::string table, Column c1, Columns... cols) {
	std::string query;
	int qstate;

	query = "CREATE TABLE " + table + " (";
	query = addColumnsToCreateTableQuery(query, c1, cols...);
	query += ");";
	try {
		qstate = makeQuery(conn, query);
	}
	catch (const char* e) {
		std::cout << "Failed to create table" << std::endl;
		std::cerr << e << std::endl;
	}
	if (qstate == 0) {
		return true;
	}
	else {
		return false;
	}
}

bool MYSQLAPI::dropTable(MYSQL* conn, std::string table) {
	std::string query = "DROP TABLE " + table + ";";
	int qstate;
	try {
		qstate = MYSQLAPI::makeQuery(conn, query);
	}
	catch (const char* e) {
		std::cout << "Failed to drop table" << std::endl;
		std::cerr << e << std::endl;
	}
	if (qstate == 0) {
		return true;
	}
	else {
		return false;
	}

}

std::string addColumnsToAlterTableQuery(std::string query) {
	return query;
}

template<typename... Columns>
std::string addColumnsToAlterTableQuery(std::string query, MYSQLAPI::Column c1, Columns... cols) {
	query += "ADD " + c1.name + " " + c1.dataType + ";";
	query = addColumnsToAlterTableQuery(query, cols...);
	return query;
}


template<typename... Columns>
bool MYSQLAPI::alterTable(MYSQL* conn, std::string table, MYSQLAPI::Column c1, Columns... cols) {
	std::string query = "ALTER TABLE " + table + "\n";
	query = addColumnsToAlterTableQuery(query, c1, cols...);
	std::cout << query;


}

std::string addValuesToInsertIntoQuery(std::string query) {
	query.pop_back();
	query.pop_back();
	query += ");";
	return query;
}
template<typename... Datas> std::string addValuesToInsertIntoQuery(std::string query, Data d1, Datas... datas) {
	if (d1.dataType == "varchar(255)") {
		query += "\"" + d1.value + "\", ";
	}
	else {
		query += d1.value + ", ";
	}
	query = addValuesToInsertIntoQuery(query, datas...);
	return query;
}
std::string addColumnsToInsertIntoQuery(std::string query) {
	query.pop_back();
	query.pop_back();
	query += ")";
	return query;
}
template<typename... Datas> std::string addColumnsToInsertIntoQuery(std::string query, Data d1, Datas... datas) {
	query += d1.name + ", ";
	query = addColumnsToInsertIntoQuery(query, datas...);
	return query;
}
template<typename... Datas> bool MYSQLAPI::insertInto(MYSQL* conn, std::string table, Data d1, Datas... datas) {
	int qstate;
	std::string query = "INSERT INTO " + table + " (";
	query = addColumnsToInsertIntoQuery(query, d1, datas...);
	query += " VALUES (";
	query = addValuesToInsertIntoQuery(query, d1, datas...);
	try {
		qstate = MYSQLAPI::makeQuery(conn, query);
	}
	catch (const char* err) {
		std::cout << "insertInto Failed" << "\n";
		std::cerr << err << "\n";
	}
	if (qstate) {
		return true;
	}
	else {
		return false;
	}
}

std::string addValuesToUpdateTableQuery(std::string query) {
	query.pop_back();
	query.pop_back();
	return query;
}
template<typename... Datas> std::string addValuesToUpdateTableQuery(std::string query, Data d1, Datas... datas) {
	if (d1.dataType == "varchar(255)") {
		query += d1.name + " = " + "\"" + d1.value + "\"" + ", ";
	}
	else {
		query += d1.name + " = " + d1.value + ", ";
	}

	query = addValuesToUpdateTableQuery(query, datas...);
	return query;
}
template<typename... Datas> bool MYSQLAPI::updateTable(MYSQL* conn, std::string table, Data condition, Data d1, Datas... datas) {
	int qstate;
	std::string query = "UPDATE " + table + " SET ";
	query = addValuesToUpdateTableQuery(query, d1, datas...);
	if (condition.dataType == "varchar(255)") {
		query += " WHERE " + condition.name + " = " + "\"" + condition.value + "\"" + ";";
	}
	else {
		query += " WHERE " + condition.name + " = " + condition.value + ";";
	}
	try {
		qstate = MYSQLAPI::makeQuery(conn, query);
	}
	catch (const char* err) {
		std::cout << "updateTable Failed" << std::endl;
		std::cerr << err << std::endl;
		return false;
	}
	if (qstate == 0) {
		return true;
	}
	else {
		return false;
	}
}


bool deleteRowFromTable(MYSQL* conn, std::string table, Data d1) {
	int qstate;
	std::string query = "DELETE FROM " + table + " WHERE ";
	if (d1.dataType == "varchar(255)") {
		query += d1.name + " = " + +"\"" + d1.value + "\"" + ";";
	}
	else {
		query += d1.name + " = " + d1.value + ";";
	}
	try {
		qstate = MYSQLAPI::makeQuery(conn, query);
	}
	catch (const char* err) {
		std::cout << "deleteFromTable Failure" << "\n";
		std::cerr << err << "\n";
	}
	if (qstate == 0) {
		return true;
	}
	else {
		return false;
	}
}

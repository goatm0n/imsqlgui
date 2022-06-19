#include "mysqlapi.h"
#include <iostream>

using namespace MYSQLAPI;

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



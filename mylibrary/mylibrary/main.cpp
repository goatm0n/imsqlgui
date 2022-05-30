#include <iostream>
#include <mysql.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h> 

MYSQL* connectDb(const char*, const char*, const char*, const char*, unsigned int);
MYSQL* connectTestDb();
int makeQuery(MYSQL*, std::string);
MYSQL_RES* queryResult(MYSQL*, std::string);
MYSQL_RES* getTable(MYSQL*, std::string);
std::string tableToString(MYSQL_RES*);
std::string readTable(MYSQL*, std::string);
struct Column {
	std::string name;
	std::string dataType;
};
std::string addColumnsToCreateTableQuery(std::string);
template<typename... Columns> std::string addColumnsToCreateTableQuery(std::string, Column, Columns...);
template<typename... Columns> bool createTable(MYSQL*, std::string, Column, Columns...);
bool dropTable(MYSQL*, std::string);
std::string addColumnsToAlterTableQuery(std::string);
template<typename... Columns> std::string addColumnsToAlterTableQuery(std::string, Column, Columns...);
template<typename... Columns> bool alterTable(MYSQL*, std::string, Column, Columns...);
struct Data : Column {
	std::string value;
};
std::string addColumnsToInsertIntoQuery(std::string);
template<typename... Datas> std::string addColumnsToInsertIntoQuery(std::string, Data, Datas...);
template<typename... Datas> std::string addValuesToInsertIntoQuery(std::string, Data, Datas...);
template<typename... Datas> bool insertInto(MYSQL*, std::string, Data, Datas...);
template<typename...Datas> bool updateTable(MYSQL*, std::string, Data, Data, Datas...);
template<typename...Datas> std::string addValuesToUpdateTableQuery(std::string, Data, Datas...);
std::string addValuesToUpdateTableQuery(std::string);
bool deleteRowFromTable(MYSQL*, std::string, Data);

int main() {
	MYSQL* conn;
	std::string table;
	bool createdTable;
	bool droppedTable;
	bool updatedTable;
	bool deletedTable;

	conn = connectTestDb();

	Column c1, c2, c3;
	c1.name = "PersonId";
	c1.dataType = "int";
	c2.name = "username";
	c2.dataType = "varchar(255)";
	c3.name = "age";
	c3.dataType = "int";
	createdTable = createTable(conn, "testTable", c1, c2, c3);
	if (createdTable) {
		std::cout << "createTable Success" << "\n";
	} else {
		return 1;
	}

	Data d1, d2, d3;
	d1.name = c1.name;
	d1.dataType = c1.dataType;
	d1.value = "123";
	d2.name = c2.name;
	d2.dataType = c2.dataType;
	d2.value = "ethan";
	d3.name = c3.name;
	d3.dataType = c3.dataType;
	d3.value = "22";

	insertInto(conn, "testTable", d1, d2, d3);

	table = readTable(conn, "testTable");
	std::cout << table << std::endl;

	Data condition, d4, d5;
	condition.name = "username";
	condition.value = "ethan";
	condition.dataType = "varchar(255)";
	d4.name = "age";
	d4.value = "23";
	d4.dataType = "int";
	d5.name = "username";
	d5.value = "goatman";
	d5.dataType = "varchar(255)";
	updatedTable = updateTable(conn, "testTable", condition, d4, d5);
	if (updatedTable) {
		std::cout << "updateTable Success" << "\n";
	}

	std::cout << readTable(conn, "testTable") << std::endl;

	deletedTable = deleteRowFromTable(conn, "testTable", d5);
	if (deletedTable) {
		std::cout << "deleteFromTable Sucess" << "\n";
	}

	std::cout << readTable(conn, "testTable") << std::endl;

	droppedTable = dropTable(conn, "testTable");
	if (droppedTable) {
		std::cout << "dropTable Success" << "\n";
	}

	delete conn;

	return 0;
}

bool deleteRowFromTable(MYSQL* conn, std::string table , Data d1) {
	int qstate;
	std::string query = "DELETE FROM " + table + " WHERE ";
	if (d1.dataType == "varchar(255)") {
		query += d1.name + " = " + + "\"" + d1.value + "\"" + ";";
	}
	else {
		query += d1.name + " = " + d1.value + ";";
	}
	try {
		qstate = makeQuery(conn, query);
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
template<typename... Datas> bool updateTable(MYSQL* conn, std::string table, Data condition, Data d1, Datas... datas) {
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
		qstate = makeQuery(conn, query);
	}
	catch (const char* err) {
		std::cout << "updateTable Failed" << std::endl;
		std::cerr << err << std::endl;
	}
	if (qstate == 0) {
		return true;
	}
	else {
		return false;
	}
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
template<typename... Datas> bool insertInto(MYSQL* conn, std::string table, Data d1, Datas... datas) {
	int qstate;
	std::string query = "INSERT INTO " + table + " (";
	query = addColumnsToInsertIntoQuery(query, d1, datas...);
	query += " VALUES (";
	query = addValuesToInsertIntoQuery(query, d1, datas...);
	try {
		qstate = makeQuery(conn, query);
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

std::string addColumnsToAlterTableQuery(std::string query) {
	return query;
}

template<typename... Columns>
std::string addColumnsToAlterTableQuery(std::string query, Column c1, Columns... cols) {
	query += "ADD " + c1.name + " " + c1.dataType + ";";
	query = addColumnsToAlterTableQuery(query, cols...);
	return query;
}


template<typename... Columns>
bool alterTable(MYSQL* conn, std::string table, Column c1, Columns... cols) {
	std::string query = "ALTER TABLE " + table + "\n";
	query = addColumnsToAlterTableQuery(query, c1, cols...);
	std::cout << query;


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
bool createTable(MYSQL* conn, std::string table, Column c1, Columns... cols) {
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

bool dropTable(MYSQL* conn, std::string table) {
	std::string query = "DROP TABLE " + table + ";";
	int qstate;
	try {
		qstate = makeQuery(conn, query);
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


MYSQL* connectDb(const char* host, const char* user, const char* password, const char* db, unsigned int port) {
	MYSQL* conn;
	conn = mysql_init(0);
	conn = mysql_real_connect(conn, host, user, password, db, port, NULL, 0);
	if (conn==NULL) {
		throw "Connection to database has failed!";
	}
	return conn;
}

MYSQL* connectTestDb() {
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
int makeQuery(MYSQL* conn, std::string query) {
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
MYSQL_RES* queryResult(MYSQL* conn, std::string query) {
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

MYSQL_RES* getTable(MYSQL* conn, std::string table) {
	MYSQL_RES* res;
	std::string query;
	query = "SELECT * FROM " + table;
	res = queryResult(conn, query);
	return res;
}

std::string tableToString(MYSQL_RES* table) {
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

std::string readTable(MYSQL* conn, std::string table) {
	std::string str = "";
	MYSQL_RES* res;

	res = getTable(conn, table);
	str = tableToString(res);
	
	return str;
}












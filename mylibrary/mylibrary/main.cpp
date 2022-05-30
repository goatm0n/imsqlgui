/* INCLUDES */
#include <iostream>
#include <mysql.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h> 

/* HEADERS */
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
static void glfw_error_callback(int, const char*);

int main() {
	GLFWwindow* window;

	/* SET UP WINDOW */
	glfwSetErrorCallback(glfw_error_callback); //set up glfw error handling
	if (!glfwInit()) { //initialize the library
		return 1;
	}
	const char* glsl_version = "#version 130"; // GL 3.0 + GLSL 130
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // dk what these 2 lines do but doesnt work without them
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL); // create window and its opengl context
	if (!window) { // terminate glfw library if error creating window
		glfwTerminate();
		return -1;
	} 
	glfwMakeContextCurrent(window); // make window's context current
	glfwSwapInterval(1); // enable vsync


	/* SET UP DEAR IMGUI CONTEXT */
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;						// create variable to deal with inputs/ouputs
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	/* SET UP DEAR IMGUI STYLE */
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {    // When viewports are enabled we tweak WindowRounding/WindowBg 
		style.WindowRounding = 0.0f;							// so platform windows can look identical to regular ones.
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	/* SETUP PLATFORM / RENDERER BACKENDS */ 
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);            

	/* STATE */
	bool show_demo_window = true;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool showWindow = true;

	/* -----MAIN LOOP----- */
	while (!glfwWindowShouldClose(window))
	{
		// Poll and handle events (inputs, window resize, etc.)
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// stuff
		if (showWindow) {
			ImGui::Begin("Hello World!", &showWindow);
			ImGui::Text("This is some useful text.");
			ImGui::End();
		}

		/* RENDERING */
		ImGui::Render(); // ends the Dear ImGui frame, finalize the draw data.
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h); // stores framebuffer width and height 
		glViewport(0, 0, display_w, display_h); // sets viewport ??
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w); // Specifies rbga values for glClear.
		glClear(GL_COLOR_BUFFER_BIT); // Sets colour writing buffers to values previously selected by glClearColor.
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // opengl3 render function

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(window);




	}

	/* CLEAN UP */
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
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












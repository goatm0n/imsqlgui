/* INCLUDES */
#include <iostream>
#include <mysql.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h> 
#include "mysqlapi.h"

/* HEADERS */
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
void showMySqlTableString(MYSQL*, std::string, bool*);	// display table as formatted string in imgui window
void showMySqlTable(MYSQL*, std::string, bool*); // display table as dear imgui table

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
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool show_window = true;
	bool show_table_string = false;
	bool show_table = false;
	bool show_demo = false;
	MYSQL* conn = MYSQLAPI::connectTestDb();

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
		if (show_window) {
			ImGui::Begin("Hello World!", &show_window);
			ImGui::Text("This is some useful text.");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Checkbox("Show Demo", &show_demo);
			ImGui::Checkbox("Show Table String ", &show_table_string);
			ImGui::Checkbox("Show Table ", &show_table);
			ImGui::End();
		}

		if (show_table) {
			showMySqlTable(conn, "testTable", &show_table);
		}

		if (show_demo) {
			ImGui::ShowDemoWindow();
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

		glfwSwapBuffers(window); // swap virtual and rendered windows
	}

	/* CLEAN UP */
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	delete conn;

	return 0;
}


void showMySqlTable(MYSQL* conn, std::string table, bool* show_table) {
	MYSQL_RES* res = MYSQLAPI::getTable(conn, table);
	MYSQL_FIELD* fields = mysql_fetch_fields(res);
	unsigned int num_fields = mysql_num_fields(res);
	MYSQL_ROW row;

	ImGui::Begin(table.c_str(), show_table);
	if (ImGui::BeginTable(table.c_str(), num_fields)) {
		ImGui::TableNextRow();
		for (unsigned int column = 0; column < num_fields; column++) {
			ImGui::TableSetColumnIndex(column);
			ImGui::Text(fields[column].name);
		}
		while (row = mysql_fetch_row(res)) {
			ImGui::TableNextRow();
			for (unsigned int column = 0; column < num_fields; column++) {
				ImGui::TableSetColumnIndex(column);
				ImGui::Text(row[column]);
			}
		}
		ImGui::EndTable();
	}
	ImGui::End();
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
		qstate = MYSQLAPI::makeQuery(conn, query);
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

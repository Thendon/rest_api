//******************************************************************
//DBManager
//Florian Herrmann
//Steffen Adreessen
//******************************************************************

#pragma once

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <iostream>
#include <json.hpp>

using namespace std;
using json = nlohmann::json;

class DatabaseManager
{
public:
	static DatabaseManager* instance();
	bool connect(std::string address, std::string username, std::string password, std::string database);
	bool tableCheck(std::string tableName);
	bool execute(std::string queryString);
	bool executeQuery(std::string queryString, json* jRes);
	void save(std::string table, json values);
	~DatabaseManager();
private:
	DatabaseManager() {};
	bool connectDB(std::string address, std::string username, std::string password);
	bool connectShema(std::string database);
	void exceptionHandler(sql::SQLException &e);
	void insert(std::string table, json values); //TODO
	void update(std::string table, json values, std::string pkValue); //TODO
	string getJSONValueAsString(const json* j);
	void handleQueryResult(sql::ResultSet* table, json* jRes);

	static DatabaseManager* databaseManager;
	sql::mysql::MySQL_Driver *mysqlDriver;
	sql::Connection *mysqlCon;
	sql::Statement *mysqlStatement;
};


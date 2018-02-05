#include "stdafx.h"
#include "DatabaseManager.h"

DatabaseManager* DatabaseManager::databaseManager;

DatabaseManager::~DatabaseManager()
{
	if (mysqlCon)
		delete mysqlCon;
	if (mysqlStatement)
		delete mysqlStatement;
}

DatabaseManager* DatabaseManager::instance()
{
	if (!DatabaseManager::databaseManager)
		DatabaseManager::databaseManager = new DatabaseManager();

	return DatabaseManager::databaseManager;
}

bool DatabaseManager::connectDB(std::string address, std::string username, std::string password)
{
	this->mysqlDriver = sql::mysql::get_mysql_driver_instance();
	try
	{
		this->mysqlCon = mysqlDriver->connect(address, username, password);
		this->mysqlStatement = this->mysqlCon->createStatement();
	}
	catch (sql::SQLException &e)
	{
		std::cout << "[FATAL] could not connect to database!\n";
		this->exceptionHandler(e);
		return false;
	}
	return true;
}

bool DatabaseManager::connectShema(std::string database)
{
	try
	{
		this->mysqlStatement->execute("USE " + database);
	}
	catch (sql::SQLException &e)
	{
		std::cout << "[FATAL] schema invalid!\n";
		this->exceptionHandler(e);
		return false;
	}
	return true;
}

void DatabaseManager::exceptionHandler(sql::SQLException &e)
{
	std::cout << "[DBERROR] " << e.what() << " code: " << e.getErrorCode() << "\n";
}

void DatabaseManager::insert(std::string table, json values)
{
	string tq1, tq2;
	for (json::iterator value = values.begin(); value != values.end(); ++value)
	{
		if (value != values.begin())
		{
			tq1 += ", ";
			tq2 += ", ";
		}
		tq1 += value.key();
		tq2 += "'" + this->getJSONValueAsString( &value.value() ) + "'";
	}
	this->execute("INSERT INTO " + table + " ( " + tq1 + " ) VALUES ( " + tq2 + " )");
}

void DatabaseManager::update(std::string table, json values, std::string pkValue)
{
	string queryString;
	for (json::iterator value = values.begin(); value != values.end(); ++value)
	{
		if (value != values.begin())
			queryString += ", ";

		queryString += value.key() + " = '" + this->getJSONValueAsString(&value.value()) +"'";
	}
	this->execute("UPDATE " + table + " SET " + queryString + " WHERE id = " + pkValue);
}

string DatabaseManager::getJSONValueAsString(const json* j)
{
	json::value_t type = j->type();

	switch (type) {
	case json::value_t::number_integer:
		return to_string(j->get<int>());
	case json::value_t::number_unsigned:
		return to_string(j->get<unsigned int>());
	default:
		return j->get<string>();
	}
}

bool DatabaseManager::connect(std::string address, std::string username, std::string password, std::string database)
{
	if (!this->connectDB(address, username, password))
		return false;
	if (!this->connectShema(database))
		return false;

	return true;
}

void DatabaseManager::handleQueryResult(sql::ResultSet* table, json* jRes)
{
	sql::ResultSetMetaData* meta = table->getMetaData();
	int columnCount = meta->getColumnCount();

	while (table->next()) {
		json jRow = json();

		for (int i = 1; i <= columnCount; i++)
		{
			string name = meta->getColumnName(i);
			int datatype = meta->getColumnType(i);
			switch (datatype)
			{
			case sql::DataType::INTEGER:
				jRow[name] = table->getInt(i);
				break;
			case sql::DataType::VARCHAR:
				jRow[name] = table->getString(i);
				break;
			}
		}

		jRes->push_back(jRow);
	}
}

bool DatabaseManager::executeQuery(std::string queryString, json* jRes)
{
	try 
	{
		sql::ResultSet* table = this->mysqlStatement->executeQuery(queryString);
		this->handleQueryResult(table, jRes);
		delete table;
	}
	catch (sql::SQLException &e)
	{
		this->exceptionHandler(e);
		return false;
	}
	return true;
}

void DatabaseManager::save(std::string table, json values)
{
	json res = json();
	string pkValue = this->getJSONValueAsString( &values.at("id") );

	this->executeQuery("SELECT * FROM " + table + " WHERE id = " + pkValue, &res);

	if (res.empty())
	{
		cout << "[" << table << "] DB Object not found, inserting new one ... \n";
		
		this->insert(table, values);

		return;
	}

	this->update(table, values, pkValue);
}

bool DatabaseManager::tableCheck(std::string tableName)
{
	try
	{
		this->mysqlStatement->executeQuery("SELECT 1 FROM " + tableName + " LIMIT 1");
	}
	catch (sql::SQLException &e)
	{
		this->exceptionHandler(e);
		return false;
	}
	return true;
}

bool DatabaseManager::execute(std::string queryString)
{
	try 
	{
		this->mysqlStatement->execute(queryString);
	}
	catch (sql::SQLException &e)
	{
		this->exceptionHandler(e);
		return false;
	}
	return true;
}

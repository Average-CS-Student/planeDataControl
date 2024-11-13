#include <iostream>
#include <map>
#include "mysql/jdbc.h"
#include "DataBaseConnection.h"
#include "Arinc429.h"

//#define HOST_NAME ""
//#define USER_NAME ""
//#define PASSWORD ""
//#define SCHEMA ""

DataBaseConnection::DataBaseConnection() {
	try {
		driver = sql::mysql::get_driver_instance();

		con = driver->connect(HOST_NAME, USER_NAME, PASSWORD);

		con->setSchema(SCHEMA);

		statement = con->createStatement();
	}
	catch (sql::SQLException &e) {
		std::cerr << "MySQL error: " << e.what() << std::endl;
	}
}

DataBaseConnection::~DataBaseConnection() {
	statement->close();
	con->close();
	statement->~Statement();
	con->~Connection();
	driver->~MySQL_Driver();
	delete statement;
	delete con;
	delete driver;
}

int DataBaseConnection::InsertArinc429Data(std::map<std::string, Arinc429>& data) {
	if (data.size() == 0) {
		return -1;
	}

	std::string query = "insert into arinc429_data (time_stamp, label, parameter, value, status, extra) values ";
	
	try {
		for (const auto& word : data) {
			query += "(NOW(), '" + word.second.label + "', '" + word.second.parameter + "', " + std::to_string(word.second.value) + ", '" + word.second.status + "', '" + word.second.extra + "'), ";
		}
		query.pop_back();
		query.pop_back();
		query += ";";

		return statement->executeUpdate(query);
	}
	catch (sql::SQLException &e) {
		std::cerr << "MySQL error: " << e.what() << std::endl;
		return -1;
	}
}
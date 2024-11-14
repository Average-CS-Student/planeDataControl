#include <iostream>
#include <map>
#include <chrono>
#include <thread>
#include <future>
#include <functional>
#include "Arinc429.h"
#include "COMCon.h"
#include "DataBaseConnection.h"

#define COM "\\\\.\\COM3"
#define DB_UPDATE_INTERVAL_SECONDS 60

std::future<int> future;

std::chrono::steady_clock::time_point timeNew;
std::chrono::steady_clock::time_point timeLast;
std::chrono::duration<float> timer;


int main() {
	timeNew = std::chrono::high_resolution_clock::now();
	timeLast = timeNew;
	timer = std::chrono::seconds(0);

	DataBaseConnection dbCon;

	COMCon com(COM);
	if (com.error == -1) {
		std::cout << "COM port error: unable to open port." << std::endl;
		return -1;
	}

	uint32_t comData;
	Arinc429 arinc429;
	std::map<std::string, Arinc429> upToDateData;

	std::cout << "Reciving data on " << COM << std::endl;
	std::cout << "Seconds between database updates: " << DB_UPDATE_INTERVAL_SECONDS << std::endl;

	// main loop
	while (true) {
		if (com.ReadCOM(comData)) {
			arinc429 = Arinc429(comData);
			upToDateData[arinc429.label + arinc429.extra] = arinc429;
		}
		

		timeNew = std::chrono::high_resolution_clock::now();
		timer += timeNew - timeLast;
		timeLast = timeNew;
		if (timer >= std::chrono::seconds(DB_UPDATE_INTERVAL_SECONDS)) {
			timer -= std::chrono::seconds(DB_UPDATE_INTERVAL_SECONDS);
			std::cout << "Updating database..." << std::endl;

			
			future = std::async(std::launch::async, [&dbCon, &upToDateData] {
				return dbCon.InsertArinc429Data(upToDateData);
			});

			std::cout << "Rows inserted: " << future._Get_value() << std::endl;
		}
	}

	return 0;
}

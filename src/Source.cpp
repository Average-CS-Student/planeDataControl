#include <iostream>
#include <map>
#include <chrono>
#include <thread>
#include <future>
#include <functional>
#include "Arinc429.h"
#include "COMCon.h"
#include "DataBaseConnection.h"
#include "NeuralNetwork.h"

#define COM "\\\\.\\COM3"
#define DB_UPDATE_INTERVAL_SECONDS 15
#define NN_MODEL_PATH ""
#define TEST_DATA_PATH ""
#define TEST_TARGETS_PATH ""

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

	
	std::vector<std::string> labels = { "052", "053", "054", "205", "206", "210", "217", "256", "312", "316", "317", "324", "325", "326", "327", "330", "340", "342", "345", "347", "365" };

	NeuralNetwork* ann;
	ann = new FFN();
	// load neural network
	ann->LoadModel(NN_MODEL_PATH);
	// test error on training set
	std::cout << ann->TestClassificationError(TEST_DATA_PATH, TEST_TARGETS_PATH) << std::endl;

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

			std::vector<Arinc429> toNN = std::vector<Arinc429>();
			for (const auto& word : upToDateData) {
				if (std::any_of(labels.begin(), labels.end(), [&word](std::string x) {return x == word.second.label; })) {
					toNN.push_back(word.second);
				}
			}

			int predictedClass = ann->Predict(&toNN);
			switch (predictedClass)
			{
			case 0:
				std::cout << "All systems fine\n";
				break;
			case 1:
				std::cout << "Left Engine Fire!\n";
				break;
			default:
				break;
			}

			future = std::async([&dbCon, &upToDateData] { return dbCon.InsertArinc429Data(upToDateData); });
		}
	}

	return 0;
}
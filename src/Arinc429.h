#pragma once

class Arinc429 {
public:
	Arinc429();

	Arinc429(uint32_t arinc429Word);

	std::string label;
	uint8_t labelBNR;
	std::string parameter;
	float value;
	std::string status;
	std::string extra;

private:
	static std::string getLabel(uint8_t label);

	void setArincFields(uint32_t arinc429Word, std::string& parameter, float& value, std::string& status, std::string& extra);

	static float decodeBNR(uint32_t arinc429Word, float maxValue, uint8_t sigBits);
};
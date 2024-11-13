#include <iostream>
#include <string>
#include <bit>
#include <bitset>
#include "Arinc429.h"

Arinc429::Arinc429() {
	label = "000";
	parameter = "Not Used";
	value = 0;
	status = "";
	labelBNR = 0;
	extra = "";
}

Arinc429::Arinc429(uint32_t arinc429Word) {
	if (~std::_Popcount(arinc429Word) % 2) {
		status = "Error detected";
		label = "";
		parameter = "";
		value = 0;
		labelBNR = 0;
		extra = "";
	}
	else {
		labelBNR = arinc429Word;
		label = getLabel(arinc429Word);
		setArincFields(arinc429Word, parameter, value, status, extra);
	}
}

std::string Arinc429::getLabel(uint8_t label) {
		uint8_t hundreds = (label & 0b1) << 1 | (label & 0b10) >> 1;
		uint8_t tens = (label & 0b100) | (label & 0b1000) >> 2 | (label & 0b10000) >> 4;
		uint8_t ones = (label & 0b100000) >> 3 | (label & 0b1000000) >> 5 | (label & 0b10000000) >> 7;;
		return std::to_string(hundreds) +std::to_string(tens) + std::to_string(ones);
	}

void Arinc429::setArincFields(uint32_t arinc429Word, std::string& parameter, float& value, std::string& status, std::string& extra) {
	std::string BNRStatus[] = { "Failure Warning", "No Computed Data", "Functional Test", "Normal Operation" };

	extra = "";
	uint8_t label = arinc429Word;

	switch (label) {
	case 0: // 000
		parameter = "Not Used";
		value = 0;
		status = "";
		break;
	case 0b01100001: // 206
		parameter = "Computed Airspeed";
		value = decodeBNR(arinc429Word, 1024, 14);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b01110101: // 256 
		parameter = "Equivalent Airspeed";
		value = decodeBNR(arinc429Word, 1024, 14);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b00010001: // 210 
		parameter = "True Airspeed";
		value = decodeBNR(arinc429Word, 2048, 15);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b01010011: // 312 
		parameter = "Ground Speed";
		value = decodeBNR(arinc429Word, 4096, 15);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b10100001: // 205 
		parameter = "Mach";
		value = decodeBNR(arinc429Word, 4.096f, 16);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b10101111: // 365 
		parameter = "Inertial Vertical Velocity (EFI)";
		value = decodeBNR(arinc429Word, 32768, 15);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b10001001: // 221 
		parameter = "Indicated Angle of Attack (Average)";
		value = decodeBNR(arinc429Word, 180, 12);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b00010101: // 250 
		parameter = "Indicated Side Slip Angle";
		value = decodeBNR(arinc429Word, 180, 12);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b00101011: // 324 
		parameter = "Pitch Angle";
		value = decodeBNR(arinc429Word, 180, 14);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b10101011: // 325 
		parameter = "Roll Angle";
		value = decodeBNR(arinc429Word, 180, 14);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b00110011: // 314 
		parameter = "True Heading";
		value = decodeBNR(arinc429Word, 180, 15);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b00001011: // 320 
		parameter = "Magnetic Heading";
		value = decodeBNR(arinc429Word, 180, 15);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b00010011: // 310 
		parameter = "Present Position - Latitude";
		{
			float angle = decodeBNR(arinc429Word, 180, 20);
			if (angle < 0) {
				extra = "South";
			}
			else {
				extra = "North";
			}
			value = abs(angle);
		}
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b10010011: // 311 
		parameter = "Present Position - Longitude";
		{
			float angle = decodeBNR(arinc429Word, 180, 20);
			if (angle < 0) {
				extra = "West";
			}
			else {
				extra = "East";
			}
			value = abs(angle);
		}
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b11000001: // 203 
		parameter = "Altitude (1013.25 mb)";
		value = decodeBNR(arinc429Word, 131072, 17);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b01010100: // 052 
		parameter = "Body Pitch Acceleration";
		value = decodeBNR(arinc429Word, 64, 15);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b11010100: // 053 
		parameter = "Body Roll Acceleration";
		value = decodeBNR(arinc429Word, 64, 15);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b00110100: // 054 
		parameter = "Body Yaw Acceleration";
		value = decodeBNR(arinc429Word, 64, 15);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b01101011: // 326 
		parameter = "Body Pitch Rate";
		value = decodeBNR(arinc429Word, 128, 13);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b11101011: // 327 
		parameter = "Body Roll Rate";
		value = decodeBNR(arinc429Word, 128, 13);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b00011011: // 330 
		parameter = "Body Yaw Rate";
		value = decodeBNR(arinc429Word, 128, 13);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b10010001: // 211 
		parameter = "Total Air Temperature (TAT)";
		value = decodeBNR(arinc429Word, 512, 12);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b11010001: // 213 
		parameter = "Static Air Temperature (SAT)";
		value = decodeBNR(arinc429Word, 512, 11);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b11110001: // 217 
		parameter = "Static Pressure Corrected (In. Hg.)";
		value = decodeBNR(arinc429Word, 64, 16);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b01000111: // 342 
		parameter = "Air Density Ratio";
		value = decodeBNR(arinc429Word, 4, 12);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		break;
	case 0b00000111: // 340 
		parameter = "EPR Actual (Engine Direct)";
		value = decodeBNR(arinc429Word, 4, 12);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		if (((arinc429Word >> 8) & 0b11) == 0b01) {
			extra = "Left Engine";
		}
		else if (((arinc429Word >> 8) & 0b11) == 0b10) {
			extra = "Right Engine";
		}
		break;
	case 0b11100111: // 347 
		parameter = "Fuel Flow (Engine)";
		value = decodeBNR(arinc429Word, 32768 / 8, 12) * 8;
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		if (((arinc429Word >> 8) & 0b11) == 0b01) {
			extra = "Left Engine";
		}
		else if (((arinc429Word >> 8) & 0b11) == 0b10) {
			extra = "Right Engine";
		}
		break;
	case 0b10100111: // 345 
		parameter = "Exhaust Gas Temperature";
		value = decodeBNR(arinc429Word, 2048, 12);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		if (((arinc429Word >> 8) & 0b11) == 0b01) {
			extra = "Left Engine";
		}
		else if (((arinc429Word >> 8) & 0b11) == 0b10) {
			extra = "Right Engine";
		}
		break;
	case 0b11110011: // 317 
		parameter = "Oil Pressure (Engine)";
		value = decodeBNR(arinc429Word, 4096, 12);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		if (((arinc429Word >> 8) & 0b11) == 0b01) {
			extra = "Left Engine";
		}
		else if (((arinc429Word >> 8) & 0b11) == 0b10) {
			extra = "Right Engine";
		}
		break;
	case 0b01110011: // 316 
		parameter = "Oil Temperature (Engine)";
		value = decodeBNR(arinc429Word, 2048, 12);
		status = BNRStatus[(arinc429Word >> 29) & 0b11];
		if (((arinc429Word >> 8) & 0b11) == 0b01) {
			extra = "Left Engine";
		}
		else if (((arinc429Word >> 8) & 0b11) == 0b10) {
			extra = "Right Engine";
		}
		break;
	default:
		parameter = "Not Used";
		value = 0;
		status = "";
		extra = "";
		break;
	}
}

float Arinc429::decodeBNR(uint32_t arinc429Word, float maxValue, uint8_t sigBits) {
	// get value from word
	uint32_t value = arinc429Word << 3; // remove parity bit and SSM
	value = value >> 32 - sigBits - 1; // remove padding, SDI, label
	
	// if negative convert from 2's complement
	bool isNegative = false;
	if (value & 1 << sigBits) {
		isNegative = true;
		uint32_t mask = (1 << sigBits + 1) - 1;
		value--;
		value = ~value & mask;
	}

	// convert from integer representation
	float result = value * maxValue / 2 / (1 << sigBits - 1);
	if (isNegative) {
		result = -result;
	}

	return result;
}

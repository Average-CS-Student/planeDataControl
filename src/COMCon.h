#pragma once
#include "Windows.h"

class COMCon {
public:
	COMCon();

	COMCon(std::string com);

	~COMCon();

	bool ReadCOM(uint32_t& word);

	int error;
	
private:
	const char* COM;

	HANDLE hComm;
};
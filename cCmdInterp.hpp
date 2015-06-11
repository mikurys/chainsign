#ifndef CCMDINTERP_HPP
#define CCMDINTERP_HPP

#include <fstream>
#include <string>
#include <csignal>
#include "ckeysstorage.h"

class cCmdInterp // TODO singletone
{
public:
	cCmdInterp() = default;
	cCmdInterp(std::string pFifoName);
	void cmdReadLoop();
	unsigned int verify(std::string firstKey);
	void setOutDir(std::string outDir);
	unsigned int verifyOneFile(std::string fileName);
	
	cKeysStorage keyStorage;
private:
	std::ifstream inputFIFO;
	std::string mOutDir;
	bool mStop = false;
	static void signalHandler(int signum);
	static cKeysStorage *sKeyStoragePtr;
};

#endif

#ifndef CCMDINTERP_HPP
#define CCMDINTERP_HPP

#include <fstream>
#include <string>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
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
	static std::atomic<bool> mStop;
	std::unique_ptr<std::thread> mFifoReadThread;
	std::string mFifoLine; // line form fifo, USE mFifoLineMutex !!!
	std::mutex mFifoLineMutex;
	static void signalHandler(int signum);
	static cKeysStorage *sKeyStoragePtr;
};

#endif

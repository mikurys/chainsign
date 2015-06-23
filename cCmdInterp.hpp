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
#include <boost/interprocess/ipc/message_queue.hpp>
#include <sys/types.h>
#include <pwd.h>
#include "ckeysstorage.h"

#define MAX_MESSAGE_SIZE 256
#define MAX_MESSAGE_NUMBER 100

// TODO singletone ? not sure.

class cCmdInterp {
	public:
		cCmdInterp();
		cCmdInterp(std::string pFifoName); ///< ready to read commands from the file
		~cCmdInterp();

		void cmdReadLoop(); ///< main loop - wait/read/parse commands

		/***
 		* verify entire chain of keys. returns number of the last good key, or -1 if error/verifiacation failed. Warns user (cout)
		* @param firstKey - name of the key - entire file name to open this file in format: BASEPATH/key_1.pub" e.g. "data1/key_1.pub"
		*/
		unsigned int verify(std::string firstKey); 

		void setOutDir(std::string outDir); ///< (not used?) move files there after verification

		unsigned int verifyOneFile(std::string fileName); ///< verifies one signature.
		
		cKeysStorage keyStorage;

	private:
		std::ifstream inputFIFO; ///< get commands from here

		std::string mOutDir;
		std::string mKeyDir;
		static std::atomic<bool> mStop;

		//std::unique_ptr<std::thread> mFifoReadThread;
		std::string mFifoLine; // line form fifo, USE mFifoLineMutex !!!
		std::mutex mFifoLineMutex;
		boost::interprocess::message_queue mMsgQueue;
		std::string mMsgQueueName;
		std::string getCmdFromMsgQueue(); ///< receive one message form mMsgQueue, calling thread is blocked if mMsgQueue is empty

		static void signalHandler(int signum); ///< react to event like ctrl-C key, sets flag to exit
		
};

#endif

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
#include <ctime>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/filesystem.hpp>
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
		unsigned int verify(const std::string &sigFile, std::string &keyPath); ///< sigFile for generate path, key path == out ref

		void setOutDir(std::string outDir); ///< (not used?) move files there after verification

		unsigned int verifyOneFile(std::string fileName); ///< verifies one signature.
		/**
		 * @param file_type i.e. png, mp3, txt, ...
		 * @param dir dir name
		 */
		unsigned int verifyFilesInDir(const std::string &file_type, std::string dir);
		
		cKeysStorage keyStorage;

	private:
		std::ifstream inputFIFO; ///< get commands from here

		std::string mOutDir;
		std::string mKeyDir;
		static std::atomic<bool> mStop;

		std::unique_ptr<std::thread> mStopThread;
		boost::interprocess::message_queue mMsgQueue;
		std::string mMsgQueueName;
		std::string getCmdFromMsgQueue(); ///< receive one message form mMsgQueue, calling thread is blocked if mMsgQueue is empty

		static void signalHandler(int signum); ///< react to event like ctrl-C key, sets flag to exit
		static bool mCrtlC;
		static std::string getHomeDir(); ///< /home/user/
		static std::string getPathFromFile(std::string fullFilePath); ///< /home/user/dir/file.txt => /home/user/dir/
};

#endif

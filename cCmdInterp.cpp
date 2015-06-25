#include "cCmdInterp.hpp"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <boost/filesystem.hpp>

#define KEY_SIZE 2048

cCmdInterp::cCmdInterp() :
mMsgQueue(boost::interprocess::open_or_create,
	"chainsign",
	MAX_MESSAGE_NUMBER,
	MAX_MESSAGE_SIZE * sizeof(char)
)
{
}

cCmdInterp::cCmdInterp(std::string pFifoName) :
mMsgQueueName(pFifoName),
mMsgQueue(boost::interprocess::open_or_create,
	pFifoName.c_str(),
	MAX_MESSAGE_NUMBER,
	MAX_MESSAGE_SIZE * sizeof(char)
)
{
	struct passwd *pw = getpwuid(getuid());
	mKeyDir = std::string(pw->pw_dir) + std::string("/keys/");
	std::cout << "create dir for keys" << std::endl;
	std::cout << "key dir = " << mKeyDir << std::endl;
	boost::filesystem::create_directory(mKeyDir);
	//inputFIFO.open(pFifoName);
	std::cout << "Installing signal handler" << std::endl;
	signal(SIGINT, cCmdInterp::signalHandler);
	mStopThread.reset(new std::thread([this]() {
		while (!mStop) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		std::string quitMsg("QUIT");
		mMsgQueue.send(quitMsg.c_str(), quitMsg.size(), 1);
	}));
}

cCmdInterp::~cCmdInterp()
{
	boost::interprocess::message_queue::remove(mMsgQueueName.c_str());
}


void cCmdInterp::cmdReadLoop()
{
	std::string line;
	std::cout << "Starting main loop cmdReadLoop" << std::endl;
	//std::cout << "mOutDir = " << mOutDir << std::endl;
	mOutDir.clear(); // TODO rm mOutDir
	if (keyStorage.getCurrentKey() == 1) {
		keyStorage.GenerateECDSAKey(mKeyDir + "key_" + std::to_string(keyStorage.getCurrentKey()) + ".pub");
	}
	
	while (!mStop) {
		std::cout << "load cmd from mas queue" << std::endl;
		line = getCmdFromMsgQueue();
		
		if (line == "QUIT") {
			mStop = true;
			break;
		}
		
		else if(line == "SIGN-NEXTKEY") {
			std::cout << "SIGN-NEXTKEY" << std::endl;
			std::cout << "current key: " << keyStorage.getCurrentKey() << std::endl;
			std::cout << std::endl;
			std::string pubFileName = mKeyDir + "key_" + std::to_string(keyStorage.getCurrentKey()) + ".pub";
			//std::string nextPubFileName = inst + "-key" + std::to_string(keyStorage.getCurrentKey() + 1) + ".pub";
			std::string path; // input dir
			//system(std::string("touch " + pubFileName).c_str());
			std::cout << "pubFileName " << pubFileName << std::endl;

			// get filename			
			std::cout << "load filename" << std::endl;
			auto target_filename = getCmdFromMsgQueue();
			std::cout << "filename = " << target_filename << std::endl;
			if (!boost::filesystem::exists(target_filename)) {
				std::cout << "File not found " << target_filename << std::endl;
				//system("rm *.pub");
				continue;
			}

			std::cout << "sign file " << target_filename << std::endl;
			//std::cout << "cp " << target_filename << " to ." << std::endl;
			//system(std::string("cp " + target_filename + " .").c_str());
			std::string fullPath(target_filename); // full path to file (/home/user/file.txt)
			if (target_filename.find('/') != std::string::npos) {
				auto it = target_filename.end();
				while (*it != '/')
					it--;
				auto it2 = target_filename.begin();
				while (it2 != it + 1) {
					path += *it2;
					it2++;
				}
				target_filename.erase(target_filename.begin(), it + 1);
			}
			else {
				std::cout << "ERROR filename = " << target_filename << std::endl;
				std::cout << "use full path" << std::endl;
				continue;
			}
			std::cout << "path: " << path << std::endl;
			std::cout << "fullPath " << fullPath << std::endl;
			//std::string outDir = target_filename;
			std::string file(target_filename); // <-- the more finall version of the file to sign
			std::cout << "FILE: " << file << std::endl;
			//outDir.erase(outDir.end() - 4, outDir.end());
			//std::cout << "outDir " << outDir << std::endl;
			//setOutDir(outDir);
			std::cout << "current file: " << target_filename << std::endl;
			//system(std::string("cp " + target_filename + " " + outDir).c_str()); // cp file to out dir(archive)
			//std::cout << "out path" << mOutDir + "-" + target_filename + ".sig" << std::endl;
			std::cout << "current key: " << keyStorage.getCurrentKey() << std::endl;
			//system(std::string("cp " + target_filename + " .").c_str()); // cp file co current dir
			
			std::cout << "Sign last key" << std::endl;
			std::cout << "Last key name: " << pubFileName << std::endl;
			std::cout << "current key: " << keyStorage.getCurrentKey() << std::endl;
			
			keyStorage.GenerateECDSAKey(pubFileName);
			std::cout << "Generated the key" << std::endl;

			//keyStorage.RSASignFile(file, mOutDir + "-" + file + ".sig", false); // sign file
			//keyStorage.RSASignFile(mOutDir + pubFileName, mOutDir + pubFileName + ".sig", true);	// sign key
			std::cout << "start sign file" << fullPath << std::endl;
			keyStorage.ECDSASignNormalFile(fullPath, fullPath + ".sig", false); // sign file
// 			keyStorage.RSASignNormalFile(file, file + ".sig", false); // sign file
			std::cout << "start sign key " << pubFileName << std::endl;
			keyStorage.ECDSASignNormalFile(pubFileName, pubFileName + ".sig", true); // sign key
			
			//keyStorage.GenerateRSAKey(KEY_SIZE, mOutDir + pubFileName); // XXX
			std::cout << "removing the previous key" << std::endl;
			keyStorage.RemoveECDSAKey();
			//system(std::string("mv *.sig2 " + mOutDir).c_str());
			//system(std::string("cp *.sig2 " + mOutDir).c_str());
			//system(std::string("cp *.sig " + path).c_str());
			//system(std::string("cp *.pub " + path).c_str());
			//system("rm *.pub");
			
			//std::cout << "outDir " << outDir << std::endl;
			//std::cout << "create archive" << std::endl;
			//std::cout << std::string("tar zcvf " + path +  outDir + ".tar.gz " + outDir) << std::endl;
			//system(std::string("tar zcvf " + path + outDir + ".tar.gz " + outDir).c_str());
			std::cout << "done the command to sign next key/file" << std::endl;
		}
/*		else if(line == "VERIFY-FILE")
		{
			bool ok;
			std::cout << "VERIFY-FILE" << std::endl;
			std::cout << std::endl;
			inputFIFO.close();
			inputFIFO.open("fifo");
			std::getline(inputFIFO, line);

			std::cout << "line " << line << std::endl;
			std::string tmp;
			unsigned int key; // file was signed this key
			std::ifstream inFile(line);
			inFile >> tmp;
			inFile >> tmp;
			std::cout << "tmp before stoi" << tmp << std::endl;
			tmp.erase(0, 4); // 1.prv
			tmp.erase(tmp.size() - 4); // 1
			key = std::stoi(tmp);
			
			inFile.close();
			if (key >= verify(std::string("key_1.pub"))) // XXX key1.pub.sig
			//if (key < verify(std::string(instance + "-key1.pub"))) // XXX 
			{
				std::cout << "                                               Keys OK" << std::endl;
				ok = true;
			}
			else
			{
				ok = false;
				std::cout << "                                               Keys verification error" << std::endl;
			}
			
			//bool fileOK = keyStorage.RSAVerifyFile(line);
			bool fileOK = keyStorage.RSAVerifyNormalFile(line, line + ".sig");
			inputFIFO.close();
			
			std::cout << "keys " << ok << std::endl;
			std::cout << "file " << fileOK << std::endl;
			if (ok && fileOK)
				std::cout << "File OK" << std::endl;
		}*/
		/*else if(line == "SIGN-NEXTKEY-WAV-FILES")
		{
			std::cout << "SIGN-NEXTKEY-WAV-FILES" << std::endl;
			boost::filesystem::directory_iterator dirIterator(".");
			boost::filesystem::directory_iterator endIterator;
			//generate new key
			std::string pubFileName =  "key_" + std::to_string(keyStorage.getCurrentKey()) + ".pub";
			system(std::string("touch " + pubFileName).c_str());
			std::cout << "pubFileName " << pubFileName << std::endl;
			while (dirIterator != endIterator)
			{
				if (boost::filesystem::is_regular_file(dirIterator->status()))
				{
					std::stringstream ss;
					std::string fileName;
					ss << *dirIterator;
					ss >> fileName;
					fileName.erase(fileName.begin(), fileName.begin() + 3);
					fileName.pop_back();
					//if (fileName.find("txt") == (fileName.size() - 3)) // XXX wav
					if (fileName.find("wav") == (fileName.size() - 3))
					{
						std::cout << fileName << std::endl;
						// sign
						keyStorage.RSASignFile(fileName, mOutDir + "-" + fileName + ".sig", false);
					}
				}
				
				dirIterator++;
			}
			std::cout << "generate new key" << std::endl;
			keyStorage.GenerateRSAKey(KEY_SIZE, mOutDir + pubFileName);
			std::cout << "rm old key" << std::endl;
			keyStorage.RemoveRSAKey();
			keyStorage.RSASignFile(pubFileName, mOutDir + pubFileName + ".sig", true);	// sign key
			
			//std::cout << "tar cf wav_files.tar " + inst + "*" << std::endl;
			system(std::string("mv *.pub " + mOutDir).c_str());
			system(std::string("mv *.sig2 " + mOutDir).c_str());
			system(std::string("mv *.wav " + mOutDir).c_str());
			//system(std::string("mv *.txt " + mOutDir).c_str());
			//system(std::string("tar czf " + inst + ".tar.gz " + mOutDir).c_str());
			//system(std::string().c_str());
		}*/
	}
	std::cout << "Ended the main loop cmdReadLoop" << std::endl;

	keyStorage.saveECDSAPrivKey(mKeyDir);
	std::cout << "Join thread" << std::endl;
	mStopThread->join();
	std::cout << "Exiting the main loop cmdReadLoop" << std::endl;
}


unsigned int cCmdInterp::verify(std::string firstKey) // verify keys, get name of 1st pub file, return last good key
{
	std::cout << "Starting verification of key chain." << std::endl;
	//std::ifstream pubFile;
	//system(std::string("cp " + firstKey + " " + mOutDir + firstKey).c_str()); // copy 1st key to out dir
/*	
	std::string fileName = instance;
	bool good = true;
	int keyNumber = 2;
	while (firstKey.front() != '-')
	{
		instance += firstKey.front();
		firstKey.erase(firstKey.begin());
	}
*/

	// parse the file name. Make sure it matches this function descr.
	const std::string prefixKeyName("key_"); // key_1.pub, key_2.pub ...
	const std::string suffixKeyName(".pub");
	bool good = true;
	int keyNumber = 2;
	std::string fileName;
	
	std::cout << "Starting verification loop" << std::endl; 
	unsigned int lastGoodKey = 1;
	while (good) {
		fileName = prefixKeyName + std::to_string(keyNumber) + suffixKeyName;
		std::cout << "file name " << fileName << std::endl;
		std::ifstream pubFile;
		pubFile.open(fileName);
		if(!pubFile.good()) // no file
		{
			std::cout << "File not found: " << fileName << " so the chain ended at keyNumber="<<keyNumber<<" and last good was lastGoodKey="<<lastGoodKey<<  std::endl;
			break;
		}
		
		std::cout << "Verifying: " << fileName << std::endl;
		//good = keyStorage.RSAVerifyFile(fileName);
		good = keyStorage.ECDSAVerifyNormalFile(fileName, fileName + ".sig");
		//RSAVerifyNormalFile(const std::string& inputFilename, const std::string& signatureFilename);
		if (good) {
			lastGoodKey = keyNumber;
			std::cout << "Verifying: " << fileName << " was correctly signed" << std::endl;
			//std::cout << "mv cmd " << "mv " + fileName + " " + mOutDir + fileName << std::endl;
			//fileName.erase(fileName.end() - 4, fileName.end()); // rm ".sig"
			//system(std::string("cp " + fileName + " " + mOutDir + fileName).c_str());
		}
		else {
			
		}
		keyNumber++;
	}
	std::cout << "Done verification loop" << std::endl; 
	
	std::cout << "Last good key: " << lastGoodKey << std::endl;
	if (lastGoodKey > 1)
	{
		std::cout << "OK" << std::endl;
		return lastGoodKey;
	}
	else {
		std::cout << "\n\n" << "***************************************************************" << std::endl;
		std::cout << "Chain verification totally failed: there was no good signature of next key" << std::endl;
		std::cout << "\n\n" << "***************************************************************" << std::endl;
		return -1;
	}
}

void cCmdInterp::setOutDir(std::string outDir)
{
	mOutDir = outDir;
	system(std::string("mkdir " + mOutDir).c_str());
	if (outDir.back() != '/')
		mOutDir.push_back('/');
	//std::cout << "out dir: " << mOutDir << std::endl;
}

unsigned int cCmdInterp::verifyOneFile(std::string fileName) //fileName = file
{
	
	//std::cout << "RSAVerifyFile " << keyStorage.RSAVerifyFile(fileName, instance) << std::endl;
	
	std::string firstPubKey = "key_1.pub";
	
	std::cout << "Start keys verification" << std::endl;
	std::cout << "first pub key " << firstPubKey << std::endl;
	

	// TODO improve signature... N*N checks?  this re-checks from start
	
	unsigned int ret = verify(firstPubKey); // verify keys
	if (ret == -1) {
		std::cout << "***keys verification error***" << std::endl;
		return 2;
	}
	
	std::cout << "file name " << fileName << " using " << fileName + ".sig" << std::endl;
	ret = keyStorage.ECDSAVerifyNormalFile(fileName, fileName + ".sig");
	//std::cout << ret << std::endl;
	if (ret == 0) {
		std::cout << "***file verification error***" << std::endl;
		return 3;
	}
	
	std::cout << "OK" << std::endl;
	return 1;
}

std::string cCmdInterp::getCmdFromMsgQueue() {
	char receiveMsg[MAX_MESSAGE_SIZE];
	boost::interprocess::message_queue::size_type recv_size;
	unsigned int priority;
	mMsgQueue.receive(static_cast<void *>(receiveMsg), MAX_MESSAGE_SIZE, recv_size, priority);
	std::string receiveMsgStr(receiveMsg, recv_size);
	std::cout << "receiveMsgStr: " << receiveMsgStr << std::endl;
	std::cout << "recv_size " << recv_size << std::endl;
	std::cout << "priority " << priority << std::endl;
	return receiveMsgStr;
}


void cCmdInterp::signalHandler(int signum) {
	//std::cout << "signalHandler" << std::endl;
	mStop = true;
	//exit(signum); // XXX
}

std::string cCmdInterp::getHomeDir() {
	struct passwd *pw = getpwuid(getuid());
	std::string home = pw->pw_dir;
	home += "/";
	return home;
}

std::string cCmdInterp::getPathFromFile(std::string fullFilePath) {
	if (fullFilePath.find('/') == std::string::npos) {
		throw std::runtime_error("path error");
	}
	std::string::iterator it = fullFilePath.end() - 1;
	while (*it != '/') {
		it--;
	}
	fullFilePath.erase(fullFilePath.begin(), it);
	return fullFilePath;
}


std::atomic<bool> cCmdInterp::mStop(false);



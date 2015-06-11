#include <iostream>
#include <chrono>
#include <fstream>
#include <thread>
#include <string>
#include <vector>
#include <cassert>
#include <fstream>
#include <sys/types.h>
#include <signal.h>

#include "sys/stat.h"

#include <boost/program_options.hpp>
#include "ckeysstorage.h"
//#include "cCmdInterp.hpp"

// XXX
#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>
#include <crypto++/hex.h>
using namespace CryptoPP;

#define KEY_SIZE 2048

using namespace std;

enum parser {
	UNKNOWN_COMMAND = 4,
	SUCCESS = 0,
	ERROR = 5
};

void printHelp() {
	cout << "examples: " << endl;
	cout << "./chainsign --daemon name_of_instance out_dir" << endl;
	cout << "./chainsign --verify-chain 1st_pub_key --move out_dir" << endl;
	cout << "./chainsign --verify sig_file" << endl;

	cout << "./chainsign --daemon my_instance ." << endl;
	cout << "./chainsign --verify-chain my_instance-key1.pub --move good_keys" << endl;
	cout << "./chainsign --verify-file sig_file" << endl;

}

bool isDaemonRunning() {


	return true; // TODO
}

void clientCmd(const string &fifo, const string &cmd) {
	fstream file;
	cout << cmd << endl;
	try {
		file.open(fifo.c_str(), ios::app | ios::out);
		file << cmd;
		file.close();
	} catch(fstream::failure &e) {
		std::cerr << "Exception opening/reading/closing file\n";
	}
}

int main(int argc, char* argv[]) {

	cKeysStorage keyStorage;
	keyStorage.GenerateRSAKey(KEY_SIZE, "key_1.pub");
	keyStorage.RSASignFile("test.txt", "test.txt.sig", false);
	//keyStorage.RSAVerifyFile("test.txt.sig", "");
	std::string signedTxt;
	FileSource("test.txt.sig2", true, new StringSink(signedTxt)); 
	std::cout << "text from test.txt.sig2" << std::endl;
	std::cout << signedTxt << std::endl;
	std::cout << "signedTxt length (data from test.txt.sig2) " << signedTxt.size() << std::endl;
	
	std::string signedTxtFromIfstream;
	std::ifstream input("test.txt.sig2");
	input >> signedTxtFromIfstream;
	std::cout << "signedTxtFromIfstream " << signedTxtFromIfstream << std::endl;
	std::cout << "size of signedTxtFromIfstream " << signedTxtFromIfstream.size() << std::endl;
	
	return 0;
}

// return 0 - OK
// return 1 - other error (exception)
// return 2 - keys verification error
// return 3 - file verification error
// return 4 - unknown command
// return 5 - parser error


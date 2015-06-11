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

	const string fifo = "fifo";

	try {
		string opt;
		namespace po = boost::program_options;
		po::options_description desc("Options");
		desc.add_options()
		("help", "print help messages")
		("daemon", "[name of instance] [outdir name] run as daemon mode")
		("verify-chain", "[key.pub] [good_keys] verify keys and move them to good-key")
		("verify-file", "[sig_file]")
		("client", "[command]");

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		try {
			std::cout << "start main" << std::endl;
			if (vm.count("help") || argc == 1) {
				cout << desc << endl;
				printHelp();
				return 0;
			}

			if (vm.count("daemon")) {
				assert(argc == 4);
//				if (isDaemonRunning()) return 1;

				auto fifoFile = mkfifo(fifo.c_str(), 0666);
				if (fifoFile == -1) cout << "problem with creating fifo" << endl;

				cCmdInterp cmdInterp(fifo, argv[2]);
				cmdInterp.setOutDir(std::string(argv[3]));
				std::cout << "start loop" << std::endl;
				cmdInterp.cmdReadLoop();
			}

			if (vm.count("verify-chain")) {
				assert(argc == 5);

				cCmdInterp cmdInterp;
				cmdInterp.setOutDir(std::string(argv[3]));
				unsigned int ret = cmdInterp.verify(std::string(argv[2]));
				if (ret == -1) return 2; // keys verification error
				std::cout << "OK" << std::endl;
			}

			if (vm.count("verify-file")) {
				assert(argc == 3);
				cCmdInterp cmdInterp;
				return cmdInterp.verifyOneFile(std::string(argv[2]));
			}

			if (vm.count("client")) {
				if (!isDaemonRunning()) return 1;
				string cmd = argv[2]; cmd += "\n";
				clientCmd(fifo,cmd);

				return 0;

			}

			/*
			 std::cout << KEY_SIZE << std::endl;
			 cKeysStorage keyStorage = cKeysStorage();
			 keyStorage.RSAVerifyFile("test.txt.sig", "my_instance");
			 for (int i = 0; i < 1; ++i)
			 keyStorage.generate(std::to_string(i));
			 keyStorage.sign("test.txt", 0);
			 keyStorage.GenerateRSAKey(KEY_SIZE);
			 keyStorage.RSASignFile("test.txt", "test.sig", 0);
			 */

		}

		catch (po::error& e) {
			std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
			std::cerr << desc << std::endl;
			return parser::UNKNOWN_COMMAND;
		}
	}

	catch (std::exception& e)
	{
		std::cerr << "Unhandled Exception reached the top of main: "
				<< e.what() << ", application will now exit" << std::endl;
		return parser::ERROR;

	}
	
	return 0;
}

// return 0 - OK
// return 1 - other error (exception)
// return 2 - keys verification error
// return 3 - file verification error
// return 4 - unknown command
// return 5 - parser error


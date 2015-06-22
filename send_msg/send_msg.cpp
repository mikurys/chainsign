#include <iostream>
#include <string>
#include <cstring>
#include <boost/interprocess/ipc/message_queue.hpp>

#define MAX_MESSAGE_SIZE 256
#define MAX_MESSAGE_NUMBER 100

// ./send_msg queue_name message_string
int main(int argc, char *argv[]) {
	assert(argc == 3);
	boost::interprocess::message_queue msg_queue(
		boost::interprocess::open_or_create,
		argv[1],	// queue name
		MAX_MESSAGE_NUMBER,	// max message number
		MAX_MESSAGE_SIZE * sizeof(char)	//max message size
	);
	
	msg_queue.send(argv[2], std::strlen(argv[2]), 1); // !!! send without \0
	
	/***********************/
	/*char receiveMsg[MAX_MESSAGE_SIZE];
	boost::interprocess::message_queue::size_type recv_size;
	unsigned int priority;
	msg_queue.try_receive(static_cast<void *>(receiveMsg), MAX_MESSAGE_SIZE, recv_size, priority);
	std::string receiveMsgStr(receiveMsg, recv_size);
	std::cout << "receiveMsgStr: " << receiveMsgStr << std::endl;
	std::cout << "recv_size " << recv_size << std::endl;
	std::cout << "priority " << priority << std::endl;*/
	return 0;
}
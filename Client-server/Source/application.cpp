#include <iostream>
#include <unistd.h>
#include "zmq.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	//const char * messageString = "Test message";
	string messageToSend = "Test message to send\n";
	string messageToReceive = "";

  	cout << "Ну здарова, отец..." << endl;

	void* zmqContext = zmq_ctx_new();
	//void* zmqContext1 = zmq_ctx_new();

	void* zmqSocket = zmq_socket(zmqContext, ZMQ_REP);
	//void* zmqSocket1 = zmq_socket(zmqContext1, ZMQ_REP);

	zmq_bind(zmqSocket, "tcp://*:4040");
	//zmq_bind(zmqSocket1, "tcp://*:4040");

	zmq_msg_t sendMessage;
	zmq_msg_init(&sendMessage);

	zmq_msg_t receiveMessage;
	zmq_msg_init(&receiveMessage);





	/* Create a new message, allocating 6 bytes for message content */
	zmq_msg_t msg;
	int rc = zmq_msg_init_size (&msg, 6);
	assert (rc == 0);
	/* Fill in message content with 'AAAAAA' */
	memset (zmq_msg_data (&msg), 'A', 6);
	/* Send the message to the socket */
	rc = zmq_msg_send (&msg, zmqSocket, 0);

	cout << "rc = " << rc << endl;

	cout << "Errno = " << zmq_errno() << endl;



	




	// zmq_msg_init_size(&sendMessage, messageToSend.size());
	// memcpy(zmq_msg_data(&sendMessage), messageToSend.c_str(), messageToSend.size());

	// zmq_msg_send(&sendMessage, zmqSocket, 0);
	// zmq_msg_close(&sendMessage);

	cout << "Waiting..." << endl;

	const char * s;

  	while(1)
	{
		zmq_msg_recv(&receiveMessage, zmqSocket, 0);
		

		s = zmq_msg_gets(&receiveMessage, "Message");

		cout << s << endl;

		if(messageToReceive != "") 
		{
			zmq_msg_close(&receiveMessage);
			break;
		}

		//sleep(1); // sleep one second

		// zmq_msg_t reply;
		// zmq_msg_init_size(&reply, strlen("world"));
		// memcpy(zmq_msg_data(&reply), "world", 5);
		// zmq_msg_send(&reply, respond, 0);
		// zmq_msg_close(&reply);
	}

	zmq_close(zmqSocket);
	zmq_ctx_destroy(zmqContext);

	cout << "End of program!" << endl;

  	return 0;
}

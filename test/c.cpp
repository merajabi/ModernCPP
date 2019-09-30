#include <iostream>
#include <string>
#include "socket.h"

int main(int argc, char **argv) {
	std::string peerName="localhost";
	std::string peerPort="8080";
	std::string peerProtocol="tcp";

    printf("usage: %s [server [port [protocol] ] ]   \n", argv[0]);

	if( argc > 1 ){
		peerName = argv[1];
	}
	if( argc > 2 ){
		peerPort = argv[2];
	}
	if( argc > 3 ){
		peerProtocol = argv[3];
	}
	
   /*
   ** Determine the program name (w/o directory prefix).
   */
	Socket::verbose = true;

	{
		Socket c(peerName, peerPort, peerProtocol);
		c.OpenClient();

		c.SetTimeout(5*1000);
		std::string str(" \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. ");
		
		//std::string str("this is a test.");

		std::cout << str.size() << std::endl;
		c.SendTCP(str);

		std::string res;
		c.RecvTCP(res,9); //9
		//c.Close();
		std::cout << res.size() << std::endl;
	}
	return 0;
};


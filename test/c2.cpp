#include <iostream>
#include <ctime>
#include <string>
#include "socket.h"

int main(int argc, char **argv) {
	std::string peerName="localhost";
	std::string peerPort="8080";
	std::string peerProtocol="tcp";

    std::time_t result;

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
		size_t l1=200000;
		size_t l2=100000;
		std::string str(l1,'x');
		std::string res;

		result = std::time(nullptr);
		std::cout << std::asctime(std::localtime(&result));
		Socket c(peerName, peerPort, peerProtocol);
		sleep(2);

		result = std::time(nullptr);
		std::cout << std::asctime(std::localtime(&result));
		c.Open();
		//c.SetTimeout(5*1000);
		sleep(2);
		
		//result = std::time(nullptr);
		//std::cout << std::asctime(std::localtime(&result));
		//c.ShutDown(SHUT_RDWR); //SHUT_RD SHUT_WR SHUT_RDWR
		//sleep(2);


		result = std::time(nullptr);
		std::cout << std::asctime(std::localtime(&result));
		c.Send(str);
		std::cout << str.size() << std::endl;
		sleep(2);

		//result = std::time(nullptr);
		//std::cout << std::asctime(std::localtime(&result));
		//c.ShutDown(SHUT_RD); //SHUT_RD SHUT_WR SHUT_RDWR
		//sleep(2);


		result = std::time(nullptr);
		std::cout << std::asctime(std::localtime(&result));
		c.Recv(res,l2); //9
		std::cout << res.size() << std::endl;
		sleep(2);

		result = std::time(nullptr);
		std::cout << std::asctime(std::localtime(&result));
		c.Close();
		sleep(2);
	}
	return 0;
};


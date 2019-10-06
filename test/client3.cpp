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
	
	Socket::verbose = true;

	{/*
		std::string sendStr(50,'x');
		std::string recvStr;

		Socket c(peerName, peerPort, peerProtocol);
		c.Open();
		
		c.Send(sendStr);
		std::cout << sendStr.size() << std::endl;
		
		c.Recv(recvStr,50);
		std::cout << recvStr.size() << std::endl;

		c.ShutDown(SHUT_WR); // // peer recv POLLIN | POLLOUT | POLLRDHUP

		c.Recv(recvStr,50);
		std::cout << recvStr.size() << std::endl;
		*/
	}
	{
		/*
		std::string sendStr(50,'x');
		std::string recvStr;

		Socket c(peerName, peerPort, peerProtocol);
		c.Open();
		
		c.Send(sendStr);
		std::cout << sendStr.size() << std::endl;
		
		c.Recv(recvStr,50);
		std::cout << recvStr.size() << std::endl;

		c.ShutDown(SHUT_WR); // // peer recv POLLIN | POLLOUT | POLLRDHUP

		c.Send(sendStr);		// System call failed ('sendto') - Broken pipe.
								// peer read 0 bytes
		std::cout << sendStr.size() << std::endl;
		*/
	}

	{ /*
		std::string sendStr(50,'x');
		std::string recvStr;

		Socket c(peerName, peerPort, peerProtocol);
		c.Open();
		
		//c.SetTimeout(5*1000);
		c.Send(sendStr);
		std::cout << sendStr.size() << std::endl;
		
		c.Recv(recvStr,50);
		std::cout << recvStr.size() << std::endl;

		c.ShutDown(SHUT_RD); // // peer recv POLLIN | POLLOUT | POLLRDHUP
		//sleep(2);

		c.Send(sendStr);
		std::cout << sendStr.size() << std::endl;
		*/
	}

	{ 
		std::string sendStr(50,'x');
		std::string recvStr;

		Socket c(peerName, peerPort, peerProtocol);
		c.Open();
		
		//c.SetTimeout(5*1000);
		c.Send(sendStr);
		std::cout << sendStr.size() << std::endl;
		
		c.Recv(recvStr,50);
		std::cout << recvStr.size() << std::endl;

		c.ShutDown(SHUT_RD); // // peer recv POLLIN | POLLOUT | POLLRDHUP
		//sleep(2);

		c.Recv(recvStr,50);
		std::cout << recvStr.size() << std::endl;
		sleep(5);
	}

	return 0;
};


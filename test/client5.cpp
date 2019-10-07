#include <iostream>
#include <string>
#include <ctime>
#include "socket.h"

bool Recv(Socket& c){
	bool result;
	std::string recvStr;
	result = c.Recv(recvStr,50);
	std::cout << GetDateTimeStr() << " Recv: " << recvStr.size() << ((result)?" OK":" NOK") << std::endl;
	return result;
}
bool Send(Socket& c){
	bool result;
	std::string sendStr(50,'x');
	result = c.Send(sendStr);
	std::cout << GetDateTimeStr() << " Send: "  << sendStr.size() << ((result)?" OK":" NOK") << std::endl;
	return result;
}
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

	{ 
		uint64_t start;
		std::string sendStr(50,'x');
		std::string recvStr;

		Socket c(peerName, peerPort, peerProtocol);
		c.Open();

		//c.SetTimeout(5*1000);
		start=get_ss();
		while( get_ss()-start < 200);
		Send(c);

		start=get_ss();
		while( get_ss()-start < 200);
		Recv(c);
		c.ShutDown(SHUT_RD); // // peer recv POLLIN | POLLOUT | POLLRDHUP

		start=get_ss();
		while( get_ss()-start < 200);
		Send(c);

		start=get_ss();
		while( get_ss()-start < 200);
		Recv(c);

		start=get_ss();
		while( get_ss()-start < 200);
		c.Close();
	}

	return 0;
};


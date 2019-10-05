#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

#include "socket.h"
int connections=0;

void HandelPtr(std::unique_ptr<Socket> sp){
	sp->SetTimeout(2*1000);
	std::cout << "\n thread: "<<connections<< std::endl;
	std::string res;
	sp->Recv(res,959); //959
	std::cout << res.size() << std::endl;

	std::string str="Hi Client";
	std::cout << str.size() << std::endl;
	sp->Send(str);
}

void Handel(Socket& sp){
	sp.SetTimeout(2*1000);
	std::cerr << "\n connections: "<<connections<< std::endl;
	std::string res;
	sp.Recv(res,959); //959
	std::cerr << res.size() << std::endl;

	std::string str="Hi Client";
	std::cerr << str.size() << std::endl;
	sp.Send(str);
}

int main(int argc, char **argv) {
	std::string hostName="localhost";
	std::string hostPort="8080";
	std::string hostProtocol="tcp";
	std::string hostFamily="ipv4";

    printf("usage: %s [port [protocol [family] ] ] ]   \n", argv[0]);

	if( argc > 1 ){
		hostPort = argv[1];
	}
	if( argc > 2 ){
		hostProtocol = argv[2];
	}
	if( argc > 3 ){
		hostFamily = argv[3];
	}

	Socket::verbose = true;

	{
		Socket s(DFLT_HOST,hostPort,hostProtocol,hostFamily,true);

		if(s.Open()){
			while(connections<1){
				std::unique_ptr<Socket> sp(new Socket(s.Listen()));
				if(*sp){
					std::thread t( HandelPtr,std::move(sp) );
					t.detach(); //join detach
				}else{
					Handel(s);
				}			
				connections++;
			};
			sleep(5);
		}
		//s.Close();
	}
	return 0;
};

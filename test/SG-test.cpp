#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

#include "socket.h"
#include "select.h"
int i=0;
void HandleSocket(SocketGuard sg){
	sg.get().SetTimeout(2*1000);
	std::cout << "\n thread: "<<i<< std::endl;
	std::string res;
	sg.get().Recv(res,959); //959
	std::cout << res.size() << std::endl;

	std::string str="Hi Client";
	std::cout << str.size() << std::endl;
	sg.get().Send(str);
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
		std::vector<SocketGuard> sglist;
		{
			SocketGuard sg1(Socket(DFLT_HOST,hostPort,hostProtocol,hostFamily,true));
			sglist.push_back(sg1);
		}
		if(sglist[0].get().Open()){
			while(i<1){
				SocketGuard sg2(Socket(sglist[0].get().Listen()));
				if(sg2.get()){
					HandleSocket(sg2);
				}
				else{
					HandleSocket(sglist[0]);
				}			
				i++;
			};
			//sleep(5);
		}

	}
	return 0;
};


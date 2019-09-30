#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

#include "socket.h"
int i=0;
void Handel(std::unique_ptr<Socket> sp){
	sp->SetTimeout(2*1000);
	std::cout << "\n thread: "<<i<< std::endl;
	std::string res;
	sp->RecvTCP(res,959); //959
	std::cout << res.size() << std::endl;

	std::string str="Hi Client";
	std::cout << str.size() << std::endl;
	sp->SendTCP(str);
	sleep(5);
	//sp->Close();
}

int main(int argc, char **argv) {

	Socket::verbose = true;

	{
		Socket s(DFLT_HOST,"8080");

		if(s.OpenServer("8080","tcp") && s.OpenServer("8080","udp")){
			while(i<1){
				std::unique_ptr<Socket> sp(new Socket(s.Accept()));
				if(*sp){
					std::thread t( Handel,std::move(sp) );
					t.detach();
				}else{
					s.SetTimeout(2*1000);
					std::cout << "\n No thread: "<<i<< std::endl;
					std::string res;
					s.RecvUDP(res,959); //959
					std::cout << res.size() << std::endl;

					std::string str="Hi Client";
					std::cout << str.size() << std::endl;
					s.SendUDP(str);
					//break;
				}			
				i++;
			};
			sleep(5);
		}
	}
	return 0;
};

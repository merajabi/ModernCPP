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
	sp->RecvTC(res,959); //959
	std::cout << res.size() << std::endl;

	std::string str="Hi Client";
	std::cout << str.size() << std::endl;
	sp->SendTC(str);
	sleep(5);
	//sp->Close();
}

int main(int argc, char **argv) {

	Socket::pgmName = (const char*) strrchr( argv[ 0 ], '/' );
	Socket::pgmName = Socket::pgmName == NULL  ?  argv[ 0 ]  :  Socket::pgmName+1;
	Socket::verbose = true;

	{
		Socket s(DFLT_HOST,"8080");
		if(s.OpenServer()){
			while(i<1){
				std::unique_ptr<Socket> sp(new Socket(s.Accept()));
				if(*sp){
					std::thread t( Handel,std::move(sp) );
					t.detach();
				}else{
					s.SetTimeout(2*1000);
					std::cout << "\n No thread: "<<i<< std::endl;
					std::string res;
					s.RecvUC(res,959); //959
					std::cout << res.size() << std::endl;

					std::string str="Hi Client";
					std::cout << str.size() << std::endl;
					s.SendUC(str);
					//break;
				}			
				i++;
			};
			sleep(5);
		}
	}
	return 0;
};

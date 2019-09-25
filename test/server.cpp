#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

#include "socket.h"

void Handel(std::unique_ptr<Socket> sp){
	sp->SetTimeout(2*1000);
	std::cout << "thread: "<< std::endl;
	std::string res;
	sp->Recv(res,1000);
	//std::cout << res << std::endl;
	std::string str=res;
	sp->Send(str);
	Sleep(10);
	sp->Close();
}

int main(void) {
	{
		Socket s("","27015","tcp");
		int i=0;
		while(i<10){
			std::unique_ptr<Socket> sp(new Socket(s.Accept()));
			if(sp){
				std::thread t( Handel,std::move(sp) );
				t.detach();
			}			
			i++;
		};
		Sleep(20*1000);
	}
	return 0;
};

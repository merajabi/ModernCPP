#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <vector>
#include <signal.h>
#include <chrono>

#include "socket.h"
#include "select.h"

bool Recv(Socket& c){
	bool result;
	std::string recvStr;
	result = c.Recv(recvStr,50);
	std::cout << GetDateTimeStr() << " Recv: " << recvStr.size() << ((result)?" OK":" NOK") << std::endl;
	//sleep(2);
	return result;
}
bool Send(Socket& c){
	bool result;
	std::string sendStr(50,'x');
	result = c.Send(sendStr);
	std::cout << GetDateTimeStr() << " Send: "  << sendStr.size() << ((result)?" OK":" NOK") << std::endl;
	//sleep(2);
	return result;
}
int main(int argc, char **argv) {

	Socket::verbose = true;
	Select pool;

	std::string recvStr;
	std::string sendStr(50,'x');

	std::time_t start;
	std::time_t now;

	{
		Socket s1(DFLT_HOST,"9999","tcp","ipv4",true);
		s1.Open();
		pool.Add(s1);
	}
	{
		int state=0;
		std::vector<Socket> selected;
		while( pool.Listen(selected) ){//connections<4 && 
			now=std::time(nullptr);
			std::cerr << "\nNew network activity: " << now << std::endl;

			for(int i=0; i < selected.size(); i++ ){
				std::cerr << "Processing Socket: " << selected[i].GetFD() << std::endl << std::endl;
				if( selected[i].Listening() && selected[i].Protocol()=="tcp"){
					Socket sp( selected[i].Accept() );
					if(sp){
						//sp.SetTimeout(2*1000);
						pool.Add(sp);
						start=now;
						state=0;
					}
				}else{
					if( !selected[i].Listening() && selected[i].Protocol()=="tcp"){
						std::cerr << "state: " << state << std::endl;
						if( (selected[i].GetEvent() & POLLIN) && (now-start>1) && (state == 0) ){
							Recv(selected[i]);
							state++;
						}
						else if( (selected[i].GetEvent() & POLLOUT) && (now-start>2) && (state == 1) ){
							Send(selected[i]);
							state++;
						}
						else if( (selected[i].GetEvent() & POLLOUT) && (now-start>3) && (state == 2) ){
							Send(selected[i]);
							state++;
						}
						else if((now-start>4) && (state == 3) ){
							pool.Remove(selected[i]);
							state++;
						}
					}
				}			
			};
			selected.clear();
			std::cerr << " ******** End Of Listen *********" << std::endl;
		}
		sleep(5);
	}
	return 0;
};

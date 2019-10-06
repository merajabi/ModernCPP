#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <vector>
#include <signal.h>

#include "socket.h"
#include "select.h"

int connections=0;

int main(int argc, char **argv) {

	Socket::verbose = true;
	Select pool;

	std::string recvStr;
	std::string sendStr(50,'x');

	{
		Socket s1(DFLT_HOST,"8888","tcp","ipv4",true);
		s1.Open();
		pool.Add(s1);
	}
	{
		std::vector<Socket> selected;
		while( pool.Listen(selected) ){//connections<4 && 
			std::cerr << "\nNew network activity.\n" << std::endl;
			for(int i=0; i < selected.size(); i++ ){
				std::cerr << "Processing Socket: " << selected[i].GetFD() << std::endl << std::endl;
				if( selected[i].Listening() && selected[i].Protocol()=="tcp"){
					Socket sp( selected[i].Accept() );
					if(sp){
						//sp.SetTimeout(2*1000);
						pool.Add(sp);
						connections=0;
					}
				}else{
					if( !selected[i].Listening() && selected[i].Protocol()=="tcp"){
						std::cerr << "connections: " << connections << std::endl;
						if(connections==0){
							selected[i].Recv(recvStr,50);
							std::cout << "Recv: " << recvStr.size() << std::endl;
						}
						else if(connections==1){
							selected[i].Send(sendStr);
							std::cout << "Send: " << sendStr.size() << std::endl;
						}
						else if(connections==3){
							selected[i].Send(sendStr);
							std::cout << "Send: " << sendStr.size() << std::endl;
						}
						else if(connections==5){
							pool.Remove(selected[i]);
						}
						connections++;
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

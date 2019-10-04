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
void Handel(Socket& sp){
	sp.SetTimeout(2*1000);
	std::cerr << "\n connections: "<<connections<< std::endl;
	std::string res;
	std::string str="Hi Client";
	if(sp.Recv(res,959)){ //959
		std::cerr << res.size() << std::endl;
		if(res.size()){
			sleep(2);
			if(sp.Send(str)){
				std::cerr << str.size() << std::endl;
			}
		}
	}
}

int main(int argc, char **argv) {

	signal(SIGPIPE, SIG_IGN);

	Socket::verbose = true;
	Select pool;
	{
		Socket s1(DFLT_HOST,"8080","tcp","ipv4",true);
		s1.Open();
		pool.Add(s1);

		Socket s2(DFLT_HOST,"8080","tcp","ipv6",true);
		s2.Open();
		pool.Add(s2);

		Socket s3(DFLT_HOST,"8080","udp","ipv4",true);
		s3.Open();
		pool.Add(s3);

		Socket s4(DFLT_HOST,"8080","udp","ipv6",true);
		s4.Open();
		pool.Add(s4);
	}
	{
		std::vector<Socket> selected;
		while( pool.Listen(selected) ){//connections<4 && 
			std::cerr << "\nNew network activity.\n" << std::endl;
			for(int i=0; i < selected.size(); i++ ){
				if( selected[i].Listening() && selected[i].Protocol()=="tcp"){
					Socket sp( selected[i].Accept() );
					if(sp){
						pool.Add(sp);
						//std::thread t( Handel, std::ref(sp) );
						//t.join();
					}
				}else{
					Handel( selected[i] );
					if( !selected[i].Listening() && selected[i].Protocol()=="tcp"){
						pool.Remove(selected[i]);
					}
				}			
			};
			connections++;
			selected.clear();
		}
		sleep(5);
	}
	return 0;
};

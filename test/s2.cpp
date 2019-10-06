#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <vector>
#include <signal.h>

#include "socket.h"
#include "select.h"

//std::shared_ptr<void(int)> handler( signal(SIGFPE, [](int signum) {throw std::logic_error("FPE"); }),[](__sighandler_t f) { signal(SIGFPE, f); });
//std::shared_ptr<void(int)> handler( signal(SIGPIPE, [](int signum) {throw std::logic_error("SIGPIPE"); }),[](__sighandler_t f) { signal(SIGPIPE, f); });

int connections=0;
std::time_t result;

void Handel(Socket& sp){
	size_t l1=3000;
	size_t l2=3000;
	std::string res;
	std::string str(l2,'y');

	sp.SetTimeout(5*1000);
	std::cerr << "\n connections: "<<connections<< std::endl;

	result = std::time(nullptr);
	std::cerr << std::asctime(std::localtime(&result));
	sp.Recv(res,l1);
	std::cerr << res.size() << std::endl;
	sleep(2);

	result = std::time(nullptr);
	std::cerr << std::asctime(std::localtime(&result));
	sp.Send(str);
	std::cerr << str.size() << std::endl;
	sleep(2);
}

int main(int argc, char **argv) {

	//signal(SIGPIPE, SIG_IGN);

	Socket::verbose = true;
	Select pool;
	{
		Socket s1(DFLT_HOST,"9999","tcp","ipv4",true);
		s1.Open();
		pool.Add(s1);

		Socket s2(DFLT_HOST,"9999","tcp","ipv6",true);
		s2.Open();
		pool.Add(s2);

		Socket s3(DFLT_HOST,"9999","udp","ipv4",true);
		s3.Open();
		pool.Add(s3);

		Socket s4(DFLT_HOST,"9999","udp","ipv6",true);
		s4.Open();
		pool.Add(s4);
	}
	{
		std::vector<Socket> selected;
		while( pool.Listen(selected) ){//connections<4 && 
			std::cerr << "\nNew network activity.\n" << std::endl;
			result = std::time(nullptr);
			std::cerr << std::asctime(std::localtime(&result));
			for(int i=0; i < selected.size(); i++ ){
				if( selected[i].Listening() && selected[i].Protocol()=="tcp"){
					result = std::time(nullptr);
					std::cerr << std::asctime(std::localtime(&result));
					Socket sp( selected[i].Accept() );
					sleep(2);
					if(sp){
						Handel( sp );
						result = std::time(nullptr);
						std::cerr << std::asctime(std::localtime(&result));
						sp.Close();

						//pool.Add(sp);
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

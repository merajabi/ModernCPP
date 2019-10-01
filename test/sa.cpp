#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

#include "socket.h"
#include "select.h"

int i=0;
void Handel(std::unique_ptr<Socket> sp){
	sp->SetTimeout(2*1000);
	std::cerr << "\n thread: "<<i<< std::endl;
	std::string res;
	sp->RecvTCP(res,959); //959
	std::cerr << res.size() << std::endl;

	std::string str="Hi Client";
	std::cerr << str.size() << std::endl;
	sp->SendTCP(str);
	sp->Close();
	//sleep(5);
}

int main(int argc, char **argv) {

	Socket::verbose = true;

	{
		Select pool;
		std::vector<Socket> selected;

		Socket s1(DFLT_HOST,"8080","tcp","ipv4");
		s1.OpenServer();
		pool.Add(s1);

		Socket s2(DFLT_HOST,"8080","tcp","ipv6");
		s2.OpenServer();
		pool.Add(s2);

		Socket s3(DFLT_HOST,"8080","udp","ipv4");
		s3.OpenServer();
		pool.Add(s3);

		Socket s4(DFLT_HOST,"8080","udp","ipv6");
		s4.OpenServer();
		pool.Add(s4);

		while(pool.Listen(selected)){
			std::cerr << "New network activity.\n" << std::endl;
			for(i=0; i < selected.size(); i++ ){
				Socket s = selected[i];
				std::unique_ptr<Socket> sp( new Socket(s.AcceptIncomming()) );
				if(*sp){
					std::thread t( Handel,std::move(sp) );
					t.detach();
				}else{
					s.SetTimeout(2*1000);
					std::cerr << "\n No thread: "<<i<< std::endl;
					std::string res;
					s.RecvUDP(res,959); //959
					std::cerr << res.size() << std::endl;

					std::string str="Hi Client";
					std::cerr << str.size() << std::endl;
					s.SendUDP(str);
					//break;
				}			
			};
		}
		s1.Close();
		s2.Close();
		s3.Close();
		s4.Close();
		sleep(5);
	}
	return 0;
};

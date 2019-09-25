#include <iostream>
#include <string>
#include "socket.h"

int main(int argc, char **argv) {
	
	if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }
	{
		Socket s(argv[1],"27015","tcp",5*1000);
		std::string str(" \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. this is a test. this is a test. \
		this is a test. this is a test. ");
		s.Send(str);
		std::string res;
		s.Recv(res,1000);
		std::cout << res << std::endl;
		s.Close();
	}
	return 0;
};

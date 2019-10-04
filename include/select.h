#ifndef _POSIX_SOCKET_SELECT_H_
#define _POSIX_SOCKET_SELECT_H_
#include <map>
#include "socket.h"

class Select {
	std::map<int,Socket>		sockMap;	// Map of Socket Objects
	std::vector<struct pollfd>	descVec;	// Array of socket descriptors. 

	public:
	bool Add(const Socket& s);
	bool Remove(const Socket& s);
	bool Listen(std::vector<Socket>& selected);
};

#endif // _POSIX_SOCKET_SELECT_H_


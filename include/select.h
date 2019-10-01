#ifndef _POSIX_SOCKET_SELECT_H_
#define _POSIX_SOCKET_SELECT_H_

#include "socket.h"

class Select {
	std::vector<Socket>        scktList;	// Array of TCP socket descriptors. MAXTCPSCKTS

	public:
	bool Add(const Socket& s);
	bool Remove(const Socket& s);
	bool Listen(std::vector<Socket>& selected);
};

#endif // _POSIX_SOCKET_SELECT_H_



#include "select.h"

bool Select::Add(const Socket& s){
	/*
	** Bump index to next descriptor array element.
	*/
	if ( s.protocol == "tcp" || s.protocol == "udp") {      /* TCP protocol.  UDP protocol.   */
		scktList.push_back(s);
		return true;
	}
	return false;
}

bool Select::Remove(const Socket& s){
}

bool Select::Listen(std::vector<Socket>& selected){
	std::vector<struct pollfd> desc;

	/*
	** Initialize the poll(2) array.
	*/
	for ( int idx = 0;     idx < scktList.size();     idx++ ) {
		struct pollfd pfd;
		pfd.fd      = scktList[ idx ].GetFD();
		pfd.events  = POLLIN;
		pfd.revents = 0;
		desc.push_back(pfd);
	}

	/*
	** Main server loop.  Handles both TCP & UDP requests.
	*/
	while ( true ) {  /* Do forever. */
		selected.clear();

		/*
		** Wait for activity on one of the sockets.  The DO..WHILE construct is
		** used to restart the system call in the event the process is
		** interrupted by a signal.
		*/
		int status;
		do {
			status = poll( &desc[0], desc.size(), -1 ); /* Wait indefinitely for input. */
		} while ( ( status < 0 ) && ( errno == EINTR ) );

		if(!SYSCALL("poll", __LINE__,  status )){   /* Check for a bona fide system call error. */
			return false;
		}

		/*
		** Process sockets with input available.
		*/
		for ( int idx = 0;     idx < desc.size();     idx++ ) {
			switch ( desc[ idx ].revents ) {
				case 0:        /* No activity on this socket; try the next. */
					continue;
				case POLLIN:   /* Network activity.  Go process it.         */
					desc[ idx ].revents = 0;   /* Clear the returned poll events. */
					selected.push_back( scktList[ idx ] );
					break;
				default:       /* Invalid poll events.                      */
					{
						fprintf( stderr,
							"%s (line %d): ERROR - Invalid poll event (0x%02X).\n",
							"Select",
							__LINE__,
							desc[ idx ].revents );
						return false;
					}
			}  /* End SWITCH on returned poll events. */
		}  /* End FOR each socket descriptor. */
		if(selected.size()){
			return true;
		}
	}  /* End WHILE forever. */
}  /* End Listen() */



#include "select.h"

/*
** Initialize the poll(2) array.
*/
bool Select::Add(const Socket& s){
	if( sockMap.insert(std::pair<int,Socket>(s.GetFD(),s)).second ){
		{
			struct pollfd pfd;
			pfd.fd      = s.GetFD();
			pfd.events  = POLLIN;
			pfd.revents = 0;
			descVec.push_back(pfd);
		}
		return true;
	}
	return false;
}

bool Select::Remove(const Socket& s){
	if( sockMap.erase(s.GetFD()) ){
		std::vector<struct pollfd>::iterator it;
		for(it = descVec.end(); it-- != descVec.begin() ;){
			if( it->fd == s.GetFD() ){
				descVec.erase(it);
				return true;
			}
		}
	}
	return false;
}

bool Select::Listen(std::vector<Socket>& selected){
	if(!descVec.size()){
		return false;
	}
	/*
	** Main server loop.  Handles both TCP & UDP requests.
	*/
	bool activity = false;
	do {  /* Do forever. */
		/*
		** Wait for activity on one of the sockets.  The DO..WHILE construct is
		** used to restart the system call in the event the process is
		** interrupted by a signal.
		*/
		int status;
		do {
			status = poll( &descVec[0], descVec.size(), -1 ); /* Wait indefinitely for input. */
		} while ( ( status < 0 ) && ( errno == EINTR ) );
		if(!SYSCALL("poll", __LINE__,  status )){   /* Check for a bona fide system call error. */
			return false;
		}

		/*
		** Process sockets with input available.
		*/
		for ( int idx = 0;     idx < descVec.size();     idx++ ) {
			switch ( descVec[ idx ].revents ) {
				case 0:        /* No activity on this socket; try the next. */
					continue;
				case POLLIN:   /* Network activity.  Go process it.         */
					descVec[ idx ].revents = 0;   /* Clear the returned poll events. */
					selected.push_back( sockMap[ descVec[ idx ].fd ] );
					activity = true;
					break;
				default:       /* Invalid poll events.                      */
					{
						fprintf( stderr,
							"%s (line %d): ERROR - Invalid poll event (0x%02X).\n",
							"Select",
							__LINE__,
							descVec[ idx ].revents );
						return false;
					}
			}  /* End SWITCH on returned poll events. */
		}  /* End FOR each socket descriptor. */
	}while(!activity);  /* End WHILE forever. */
	return activity;
}  /* End Listen() */


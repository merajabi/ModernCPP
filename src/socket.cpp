#include <string>
#include "socket.h"

std::atomic<unsigned long> Socket::sockCount(0);
//char        Socket::hostBfr[ NI_MAXHOST ];   /* For use w/getnameinfo(3).    */
//char        Socket::servBfr[ NI_MAXSERV ];   /* For use w/getnameinfo(3).    */
//const char *Socket::pgmName=NULL;            /* Program name w/o dir prefix. */
boolean     Socket::verbose = false;         /* Verbose mode indication.     */

boolean SYSCALL( const char *syscallName, int lineNbr, int status ) {
   if ( ( status == -1 ) && Socket::verbose ) {
      fprintf( stderr,
               "%s (line %d): System call failed ('%s') - %s.\n",
               "Socket",
               lineNbr,
               syscallName,
               strerror( errno ) );
   }
   return (status != -1);   /* True if the system call was successful. */
}  /* End SYSCALL() */


Socket::Socket(const int& s):sock(s),timeout(0){
	Initialize();
//	tScktSize = 0;
//	uScktSize = 0;
	cSckt=INVALID_SOCKET;
}

Socket::Socket(const std::string& pH, const std::string& pP, const std::string& proto, unsigned long tout): sock(INVALID_SOCKET),host(pH),service(pP),protocol(proto),scope(DFLT_SCOPE_ID),timeout(tout) {
	Initialize();
//	tScktSize = 0;
//	uScktSize = 0;
	cSckt=INVALID_SOCKET;
/*
	if(host.size()!=0){
		Open();
	}
	if(timeout){
		SetTimeout(tout);
	}
*/
}

Socket::~Socket(){ 
	fprintf(stderr, "~Socket called.\n");
	Close();
	for(int i=0;i<tSckt.size();i++){
		fprintf(stderr, "Close called on socket id: %d \n",tSckt[i]);
		SYSCALL( "close",
                   __LINE__,
                   close( tSckt[i] ) );
	}
	for(int i=0;i<uSckt.size();i++){
		fprintf(stderr, "Close called on socket id: %d \n",uSckt[i]);
		SYSCALL( "close",
                   __LINE__,
                   close( uSckt[i] ) );
	}
	Finalize();
}

bool Socket::Close(){
	bool result = false;
	if( cSckt >=0 ){
		if( SYSCALL( "close", __LINE__, close( cSckt ) ) ) {
			result=true;
		}
	}
	cSckt=INVALID_SOCKET;
	sock=INVALID_SOCKET;
	return result;
}

bool Socket::SetTimeout(unsigned long tout){
	if( !SYSCALL("SetTimeout", __LINE__,  sock )){
		return false;
	}

	timeout=tout;
	struct timeval tv;
	tv.tv_sec = static_cast<int>(timeout/1000);
	tv.tv_usec = static_cast<int>(0);

	if( !SYSCALL("setsockopt", __LINE__,  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)) )){
		return false;
	}
	return true;
}

bool Socket::SendTCP(const std::string& buffer){
	if( !SYSCALL("SendTC", __LINE__,  sock )){
		return false;
	}
    ssize_t wBytes = buffer.size();
    while ( wBytes > 0 ) {
		ssize_t count;
		do {
			count = write( sock,
				         buffer.c_str(),
				         wBytes );
		} while ( ( count < 0 ) && ( errno == EINTR ) );
		if(!SYSCALL("write", __LINE__,  count )) {   /* Check for a bona fide error. */
			return false;
		}
		wBytes -= count;
	}  /* End WHILE there is data to send. */
	return true;
}

bool Socket::SendUDP(const std::string& buffer){
	if( !SYSCALL("SendTC", __LINE__,  sock )){
		return false;
	}
	struct sockaddr         *sadr;
	socklen_t                sadrLen;

	sadrLen = sizeof( udpSockStor );
	sadr    = (struct sockaddr*) &udpSockStor;

    ssize_t wBytes = buffer.size();
	ssize_t count;
	while ( wBytes > 0 ) {
		do {
			count = sendto( sock,
						  buffer.c_str(),
						  wBytes,
						  0,
						  sadr,        /* Address & address length   */
						  sadrLen );   /*    received in recvfrom(). */
		} while ( ( count < 0 ) && ( errno == EINTR ) );
		if(!SYSCALL("write", __LINE__,  count )) {   /* Check for a bona fide error. */
			return false;
		}
		wBytes -= count;
	}  /* End WHILE there is data to send. */

}

// Receive until the peer closes the connection or read recvbuflen bytes
bool Socket::RecvTCP(std::string& buffer, int recvbuflen){
	if( !SYSCALL("RecvTC", __LINE__,  sock )){
		return false;
	}
	ssize_t inBytes;
	do {
		char recvbuf[DEFAULT_BUFLEN];
		inBytes = read(sock, recvbuf, (recvbuflen<DEFAULT_BUFLEN)?recvbuflen:DEFAULT_BUFLEN);
		if(inBytes > 0){
			buffer+=std::string(recvbuf,recvbuf+inBytes);
			recvbuflen-=inBytes;
		}
		else if( (inBytes < 0) && (errno != EAGAIN ) ){
			SYSCALL("read", __LINE__,  inBytes );
			return false;
		}
	} while( inBytes > 0 && recvbuflen > 0 );
	return true;
}

/*
** This is a UDP socket, and a datagram is available.  The funny
** thing about UDP requests is that this server doesn't require any
** client input; but it can't send the TOD unless it knows a client
** wants the data, and the only way that can occur with UDP is if
** the server receives a datagram from the client.  Thus, the
** server must receive _something_, but the content of the datagram
** is irrelevant.  Read in the datagram.  Again note the use of
** sockaddr_storage to receive the address.
*/
bool Socket::RecvUDP(std::string& buffer, int recvbuflen) {
	if( !SYSCALL("RecvUC", __LINE__,  sock )){
		return false;
	}
	bool result = false;
	memset (&udpSockStor,0,sizeof(udpSockStor));
	struct sockaddr *sadr = (struct sockaddr*) &udpSockStor;
	socklen_t       sadrLen = sizeof( udpSockStor );

	ssize_t inBytes;
	//do {
		std::string recvbuf;
		recvbuf.resize(recvbuflen);
		inBytes = recvfrom( sock, &recvbuf[0], recvbuflen, 0, sadr, &sadrLen );
		if(inBytes > 0){
			buffer+=std::string(recvbuf.begin(),recvbuf.begin()+inBytes);
			recvbuflen-=inBytes;
			result = true;
		}
		else if( (inBytes < 0) && (errno != EAGAIN ) ){
			SYSCALL("recvfrom", __LINE__,  inBytes );
		}
	//} while( inBytes > 0 && recvbuflen > 0 );
	{
		struct addrinfo saddri={0,sadr->sa_family,0,0,sadrLen,sadr,0,0};
		PrintAddrInfo(&saddri);
	}
	return result;
}

/*
** Open a connection to the indicated host/service.
**
** Note that if all three of the following conditions are met, then the
** scope identifier remains unresolved at this point.
**    1) The default network interface is unknown for some reason.
**    2) The -s option was not used on the command line.
**    3) An IPv6 "scoped address" was not specified for the hostname on the
**       command line.
** If the above three conditions are met, then only an IPv4 socket can be
** opened (connect(2) fails without the scope ID properly set for IPv6
** sockets).
*/
bool Socket::OpenClient(){
	cSckt = openClientSckt( );

	if ( !SYSCALL( "OpenClient", __LINE__, cSckt )  ) {
		return false;
	}
	sock = cSckt;
	return true;

}


/******************************************************************************
* Function: openSckt
*
* Description:
*    Open passive (server) sockets for the indicated inet service & protocol.
*    Notice in the last sentence that "sockets" is plural.  During the interim
*    transition period while everyone is switching over to IPv6, the server
*    application has to open two sockets on which to listen for connections...
*    one for IPv4 traffic and one for IPv6 traffic.
*
* Parameters:
*    service  - Pointer to a character string representing the well-known port
*               on which to listen (can be a service name or a decimal number).
*    protocol - Pointer to a character string representing the transport layer
*               protocol (only "tcp" or "udp" are valid).
*    desc     - Pointer to an array into which the socket descriptors are
*               placed when opened.
*    descSize - This is a value-result parameter.  On input, it contains the
*               max number of descriptors that can be put into 'desc' (i.e. the
*               number of elements in the array).  Upon return, it will contain
*               the number of descriptors actually opened.  Any unused slots in
*               'desc' are set to INVALID_DESC.
*
* Return Value:
*    0 on success, -1 on error.
******************************************************************************/
int Socket::openServerSckt( const std::string& service, const std::string& protocol ) {
   struct addrinfo *ai;
   struct addrinfo *aiHead;
   struct addrinfo  hints    = { .ai_flags  = AI_PASSIVE,    /* Server mode. */
                                 .ai_family = PF_UNSPEC };   /* IPv4 or IPv6. */

   /*
   ** Check which protocol is selected (only TCP and UDP are valid).
   */
   if ( protocol == "tcp" )        /* TCP protocol.     */
   {
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
   }
   else if ( protocol == "udp" )   /* UDP protocol.     */
   {
      hints.ai_socktype = SOCK_DGRAM;
      hints.ai_protocol = IPPROTO_UDP;
   }
   else                                         /* Invalid protocol. */
   {
      fprintf( stderr,
               "%s (line %d): ERROR - Unknown transport "
               "layer protocol \"%s\".\n",
               "Socket",
               __LINE__,
               protocol.c_str() );
      return false;
   }
   /*
   ** Look up the service's well-known port number.  Notice that NULL is being
   ** passed for the 'node' parameter, and that the AI_PASSIVE flag is set in
   ** 'hints'.  Thus, the program is requesting passive address information.
   ** The network address is initialized to :: (all zeros) for IPv6 records, or
   ** 0.0.0.0 for IPv4 records.
   */
   int aiErr = getaddrinfo( NULL, service.c_str(), &hints, &aiHead );
   if ( !SYSCALL("getaddrinfo", __LINE__, aiErr ) ) { //gai_strerror( aiErr )
      return false;
   }

   /*
   ** For each of the address records returned, attempt to set up a passive
   ** socket.
   */
   for ( ai = aiHead; ai != NULL; ai = ai->ai_next ) {

		if(!PrintAddrInfo(ai)){
			freeaddrinfo( aiHead );
			return false;
		}

		/*
		** Create a socket using the info in the addrinfo structure.
		*/
		socket_guard sg( socket( ai->ai_family, ai->ai_socktype, ai->ai_protocol ) );

		if( !SYSCALL("socket", __LINE__,  sg.get() ) ) {
			freeaddrinfo( aiHead );
			return false;
		}

      /*
      ** Here is the code that prevents "IPv4 mapped addresses", as discussed
      ** in Section 22.1.3.1.  If an IPv6 socket was just created, then set the
      ** IPV6_V6ONLY socket option.
      */
      if ( ai->ai_family == PF_INET6 )
      {
#if defined( IPV6_V6ONLY )
         /*
         ** Disable IPv4 mapped addresses.
         */
         int v6Only = 1;
         if(!SYSCALL("setsockopt", __LINE__,  setsockopt( sg.get(),
														  IPPROTO_IPV6,
														  IPV6_V6ONLY,
														  &v6Only,
														  sizeof( v6Only ) ) ) ){
			//SYSCALL("close", __LINE__,  close( tempSock ) );
			freeaddrinfo( aiHead );
			return false;
		}
#else
         /*
         ** IPV6_V6ONLY is not defined, so the socket option can't be set and
         ** thus IPv4 mapped addresses can't be disabled.  Print a warning
         ** message and close the socket.  Design note: If the
         ** #if...#else...#endif construct were removed, then this program
         ** would not compile (because IPV6_V6ONLY isn't defined).  That's an
         ** acceptable approach; IPv4 mapped addresses are certainly disabled
         ** if the program can't build!  However, since this program is also
         ** designed to work for IPv4 sockets as well as IPv6, I decided to
         ** allow the program to compile when IPV6_V6ONLY is not defined, and
         ** turn it into a run-time warning rather than a compile-time error.
         ** IPv4 mapped addresses are still disabled because _all_ IPv6 traffic
         ** is disabled (all IPv6 sockets are closed here), but at least this
         ** way the server can still service IPv4 network traffic.
         */
         fprintf( stderr,
                  "%s (line %d): WARNING - Cannot set IPV6_V6ONLY socket "
                  "option.  Closing IPv6 %s socket.\n",
                  "Socket",
                  __LINE__,
                  ai->ai_protocol == IPPROTO_TCP  ?  "TCP"  :  "UDP" );
         //SYSCALL("close", __LINE__,  close( tempSock ) );
         continue;   /* Go to top of FOR loop w/o updating *descSize! */
#endif /* IPV6_V6ONLY */
      }  /* End IF this is an IPv6 socket. */
      /*
      ** Bind the socket.  Again, the info from the addrinfo structure is used.
      */
		if(!SYSCALL("bind", __LINE__,  bind( sg.get(), ai->ai_addr, ai->ai_addrlen ) ) ) {
			//SYSCALL("close", __LINE__,  close( tempSock ) );
			freeaddrinfo( aiHead );
			return false;
		}
      /*
      ** If this is a TCP socket, put the socket into passive listening mode
      ** (listen is only valid on connection-oriented sockets).
      */
		if ( ai->ai_socktype == SOCK_STREAM ) {
			if( !SYSCALL("listen", __LINE__,  listen( sg.get(), MAXCONNQLEN ) ) ){
				//SYSCALL("close", __LINE__,  close( tempSock ) );
				freeaddrinfo( aiHead );
				return false;
			}
		}
      /*
      ** Socket set up okay.  Bump index to next descriptor array element.
      */
		if ( protocol == "tcp" ) {      /* TCP protocol.     */
			tSckt.push_back(sg.release());
		}
		else if ( protocol == "udp" ) {  /* UDP protocol.     */
			uSckt.push_back(sg.release());
		}
   }  /* End FOR each address info structure returned. */
   /*
   ** Clean up.
   */
   freeaddrinfo( aiHead );
   return true;
}  /* End openSckt() */


/******************************************************************************
* Function: Listen
*
* Description:
*    Listen on a set of sockets and send the current time-of-day to any
*    clients.  This function never returns.
*
* Parameters:
*    tSckt     - Array of TCP socket descriptors on which to listen.
*    tScktSize - Size of the tSckt array (nbr of elements).
*    uSckt     - Array of UDP socket descriptors on which to listen.
*    uScktSize - Size of the uSckt array (nbr of elements).
*
* Return Value: Socket descriptors which has new activity
******************************************************************************/
int Socket::Listen() {
	std::vector<struct pollfd> desc;
	//size_t                   descSize = tSckt.size() + uSckt.size();
	int                      newSckt;

	sock = INVALID_SOCKET;

	/*
	** Allocate memory for the poll(2) array.
	*/
	/*
	desc = (pollfd*)malloc( descSize * sizeof( struct pollfd ) );
	if ( desc == NULL )	{
		fprintf( stderr,
			"%s (line %d): ERROR - %s.\n",
			"Socket",
			__LINE__,
			strerror( ENOMEM ) );
		return INVALID_SOCKET;
	}
	*/

	/*
	** Initialize the poll(2) array.
	*/
	/*
	for ( idx = 0;     idx < descSize;     idx++ ) {
		desc[ idx ].fd      = idx < tScktSize  ?  tSckt[ idx ]
				   								  :  uSckt[ idx - tScktSize ];
		desc[ idx ].events  = POLLIN;
		desc[ idx ].revents = 0;
	}
	*/

	for ( int idx = 0;     idx < tSckt.size();     idx++ ) {
		struct pollfd pfd;
		pfd.fd      = tSckt[ idx ];
		pfd.events  = POLLIN;
		pfd.revents = 0;
		desc.push_back(pfd);
	}
	for ( int idx = 0;     idx < uSckt.size();     idx++ ) {
		struct pollfd pfd;
		pfd.fd      = uSckt[ idx ];
		pfd.events  = POLLIN;
		pfd.revents = 0;
		desc.push_back(pfd);
	}

	/*
	** Main time-of-day server loop.  Handles both TCP & UDP requests.  This is
	** an interative server, and all requests are handled directly within the
	** main loop.
	*/
	while ( true ) {  /* Do forever. */
		/*
		** Wait for activity on one of the sockets.  The DO..WHILE construct is
		** used to restart the system call in the event the process is
		** interrupted by a signal.
		*/
		int status;
		do {
			status = poll( &desc[0], tSckt.size() + uSckt.size(), -1 ); /* Wait indefinitely for input. */
		} while ( ( status < 0 ) && ( errno == EINTR ) );

		if(!SYSCALL("poll", __LINE__,  status )){   /* Check for a bona fide system call error. */
			return INVALID_SOCKET;
		}

		/*
		** Indicate that there is new network activity.
		*/
		if ( verbose ) {
			fprintf( stderr, "%s: New network activity.\n", "Socket");
		}  /* End IF verbose. */

		/*
		** Process sockets with input available.
		*/
		for ( int idx = 0;     idx < tSckt.size() + uSckt.size();     idx++ ) {
			switch ( desc[ idx ].revents ) {
				case 0:        /* No activity on this socket; try the next. */
					continue;
				case POLLIN:   /* Network activity.  Go process it.         */
					break;
				default:       /* Invalid poll events.                      */
					{
						fprintf( stderr,
							"%s (line %d): ERROR - Invalid poll event (0x%02X).\n",
							"Socket",
							__LINE__,
							desc[ idx ].revents );
						return INVALID_SOCKET;
					}
			}  /* End SWITCH on returned poll events. */
			/*
			** Determine if this is a TCP request or UDP request.
			*/
			if ( idx < tSckt.size() ) {
				/*
				** TCP connection requested.  Accept it.  Notice the use of
				** the sockaddr_storage data type.
				*/
				struct sockaddr_storage  sockStor;
				struct sockaddr         *sadr = (struct sockaddr*) &sockStor;
				socklen_t                sadrLen = sizeof( sockStor );

				socket_guard newSG( accept( desc[ idx ].fd, sadr, &sadrLen ) );

				if(!SYSCALL("accept", __LINE__, newSG.get() ) ) {
					return INVALID_SOCKET;
				}
				/*	********************  NOT REQUIRED ********************
				SYSCALL("shutdown", __LINE__,  shutdown( newSckt, SHUT_RD ) );      // Server never recv's anything.
				
				*/ // ********************  NOT REQUIRED ********************

				{
					struct addrinfo saddri={0,sadr->sa_family,0,0,sadrLen,sadr,0,0};
					PrintAddrInfo(&saddri);
				}


				/*	********************  NOT REQUIRED ********************
				SYSCALL("close", __LINE__,  close( newSckt ) );
				*/ // ********************  NOT REQUIRED ********************

				desc[ idx ].revents = 0;   /* Clear the returned poll events. */
				return newSG.release();

			}  /* End IF this was a TCP connection request. */
			//	********************  Added for Socket Class ********************
			else {
				desc[ idx ].revents = 0;   /* Clear the returned poll events. */
				sock = desc[ idx ].fd;
				return INVALID_SOCKET;

			}/* End ELSE a UDP datagram is available. */
			//	********************  Added for Socket Class ********************

		}  /* End FOR each socket descriptor. */
	}  /* End WHILE forever. */
}  /* End tod() */

/******************************************************************************
* Function: openSckt
*
* Description:
*    Sets up a UDP/TCP socket to a remote server.  Getaddrinfo(3) is used to
*    perform lookup functions and can return multiple address records (i.e. a
*    list of 'struct addrinfo' records).  This function traverses the list and
*    tries to establish a connection to the remote server.  The function ends
*    when either a connection has been established or all records in the list
*    have been processed.
*
* Parameters:
*    host    - A pointer to a character string representing the hostname or IP
*              address (IPv4 or IPv6) of the remote server.
*    service - A pointer to a character string representing the service name or
*              well-known port number.
*    scopeId - For IPv6 sockets only.  This is the index corresponding to the
*              network interface on which to exchange datagrams/streams.  This
*              parameter is ignored for IPv4 sockets or when an IPv6 "scoped
*              address" is specified in 'host' (i.e. where the colon-hex
*              network address is augmented with the scope ID).
*
* Return Value:
*    Returns the socket descriptor for the connection, or INVALID_DESC if all
*    address records have been processed and a socket could not be initialized.
******************************************************************************/
int Socket::openClientSckt() {
	struct addrinfo *ai;
	struct addrinfo *aiHead;
	struct addrinfo  hints;

	/*
	** Initialize the 'hints' structure for getaddrinfo(3).
	**
	** Notice that the 'ai_family' field is set to PF_UNSPEC, indicating to
	** return both IPv4 and IPv6 address records for the host/service.  Most of
	** the time, the user isn't going to care whether an IPv4 connection or an
	** IPv6 connection is established; the user simply wants to exchange data
	** with the remote host and doesn't care how it's done.  Sometimes, however,
	** the user might want to explicitly specify the type of underlying socket.
	** It is left as an exercise for the motivated reader to add a command line
	** option allowing the user to specify the IP protocol, and then process the
	** list of addresses accordingly (it's not that difficult).
	*/
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family   = PF_UNSPEC;     /* IPv4 or IPv6 records (don't care). */
	if(protocol=="tcp") {
		hints.ai_socktype = SOCK_STREAM;   /* Connection-oriented byte stream.   */
		hints.ai_protocol = IPPROTO_TCP;   /* TCP transport layer protocol only. */
	}
	else if(protocol=="udp"){
		hints.ai_socktype = SOCK_DGRAM;   /* Connectionless communication.   */
		hints.ai_protocol = IPPROTO_UDP;   /* UDP transport layer protocol only. */
	}
	else{
		return INVALID_DESC;
	}

	/*
	** Look up the host/service information.
	*/
	int aiErr = getaddrinfo( host.c_str(), service.c_str(), &hints, &aiHead );
	if ( !SYSCALL("getaddrinfo", __LINE__, aiErr ) ) { //gai_strerror( aiErr )
		return INVALID_DESC;
	}

	/*
	** Go through the list and try to open a connection.  Continue until either
	** a connection is established or the entire list is exhausted.
	*/
	for ( ai = aiHead; ai != NULL; ai = ai->ai_next ) {
		/*
		** IPv6 kluge.  Make sure the scope ID is set.
		*/
		if ( ai->ai_family == PF_INET6 ) {
			sockaddr_in6_t  *pSadrIn6 = (sockaddr_in6_t*) ai->ai_addr;
			if ( pSadrIn6->sin6_scope_id == 0 ) {
				pSadrIn6->sin6_scope_id = if_nametoindex( scope.c_str() );
			}  /* End IF the scope ID wasn't set. */
		}  /* End IPv6 kluge. */

		PrintAddrInfo(ai); //PrintRemoteInfo(ai);

		/*
		** Create a socket.
		*/
		socket_guard sg( socket( ai->ai_family, ai->ai_socktype, ai->ai_protocol ) );
		if ( !SYSCALL( "socket", __LINE__, sg.get() ) ) {
			//sckt = INVALID_DESC;
			continue;   /* Try the next address record in the list. */
		}

		/*
		** Set the target destination for the remote host on this socket.
		** That is, this socket only communicates with the specified host.
		*/
		if ( SYSCALL( "connect",
						__LINE__,
						connect( sg.get(), ai->ai_addr, ai->ai_addrlen ) ) )
		{
			//(void) close( sckt );   /* Could use SYSCALL() again here, but why? */
			//sckt = INVALID_DESC;
			//continue;   /* Try the next address record in the list. */
			freeaddrinfo( aiHead );
			return sg.release();
		}
	}  /* End FOR each address record returned by getaddrinfo(3). */

	/*
	** Clean up & return.
	*/
	freeaddrinfo( aiHead );
	return INVALID_DESC;
}  /* End openSckt() */

/*
** Display the address info.
*/
bool Socket::PrintAddrInfo( struct addrinfo *sadr ){

    if ( verbose )
    {

		/*
		** Display the current address info.   Start with the protocol-
		** independent fields first.
		*/

		char hostBfr[ NI_MAXHOST ];
		char servBfr[ NI_MAXSERV ];

		/*
		** Display the socket address of the remote client.  Address-
		** independent fields first.
		*/

		fprintf( stderr,
			"Address info (PrintAddrInfo):\n"
			"   ai_flags     = 0x%02X\n"
			"   ai_family    = %d (PF_INET = %d, PF_INET6 = %d)\n"
			"   ai_socktype  = %d (SOCK_STREAM = %d, SOCK_DGRAM = %d)\n"
			"   ai_protocol  = %d (IPPROTO_TCP = %d, IPPROTO_UDP = %d)\n"
			"   ai_addrlen   = %d (sockaddr_in = %lu, sockaddr_in6 = %lu)\n",
			sadr->ai_flags,
			sadr->ai_family,
			PF_INET,
			PF_INET6,
			sadr->ai_socktype,
			SOCK_STREAM,
			SOCK_DGRAM,
			sadr->ai_protocol,
			IPPROTO_TCP,
			IPPROTO_UDP,
			sadr->ai_addrlen,
			sizeof( struct sockaddr_in ),
			sizeof( struct sockaddr_in6 ) );

		/*
		** Now display the protocol-specific formatted socket address.  Note
		** that the program is requesting that getnameinfo(3) convert the
		** host & service into numeric strings.
		*/
		getnameinfo( sadr->ai_addr,
			sadr->ai_addrlen,
			hostBfr,
			sizeof( hostBfr ),
			servBfr,
			sizeof( servBfr ),
			NI_NUMERICHOST | NI_NUMERICSERV );

		/*
		** Notice that we're switching on an address family now, not a
		** protocol family.
		*/

		switch ( sadr->ai_family )
		{
			case PF_INET:   /* IPv4 address. */
			{
				struct sockaddr_in *pSadrIn = (struct sockaddr_in*) sadr->ai_addr;
				fprintf( stderr,
					"   sin_addr  = sin_family: %d\n"
					"               sin_addr:   %s\n"
					"               sin_port:   %s\n",
					pSadrIn->sin_family,
					hostBfr,
					servBfr );
				break;
			}  /* End CASE of IPv4 address. */
			case PF_INET6:   /* IPv6 address. */
			{
				struct sockaddr_in6 *pSadrIn6 = (struct sockaddr_in6*) sadr->ai_addr;
				fprintf( stderr,
					"   sin6_addr = sin6_family:   %d\n"
					"               sin6_addr:     %s\n"
					"               sin6_port:     %s\n"
					"               sin6_flowinfo: %d\n"
					"               sin6_scope_id: %d\n",
					pSadrIn6->sin6_family,
					hostBfr,
					servBfr,
					pSadrIn6->sin6_flowinfo,
					pSadrIn6->sin6_scope_id );
				break;
			}  /* End CASE of IPv6 address. */
			default:   /* Can never get here, but just for completeness. */
			{
				fprintf( stderr,
					"%s (line %d): ERROR - Unknown protocol family (%d).\n",
					"Socket",
					__LINE__,
					sadr->ai_family );
				//freeaddrinfo( aiHead );
				return false;
			}  /* End DEFAULT case (unknown protocol family). */
		}  /* End SWITCH on protocol family. */
	}  /* End IF verbose mode. */
	return true;
}



#include <string>
#include <assert.h>

#include "socket.h"

std::atomic<unsigned long> Socket::sockCount(0);
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


Socket::Socket(int fd,const std::string& protocol):sockGuard(new socket_guard(fd)), host(DFLT_HOST), service(DFLT_SERVICE), protocol(protocol), family(DFLT_FAMILY), scope(DFLT_SCOPE_ID), listening(false), timeout(0){
	Initialize();
}

Socket::Socket(const std::string& pH, const std::string& pP, const std::string& proto, const std::string& family,bool listening, unsigned long tout): sockGuard(new socket_guard()), host(pH), service(pP), protocol(proto), family(family), scope(DFLT_SCOPE_ID), listening(listening), timeout(tout) {
	Initialize();
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
//	Close();
	Finalize();
}

bool Socket::Close(){
	fprintf( stderr,"Socket: %d Closed.\n",sockGuard->get());
	bool result = false;
	if( sockGuard->get() >=0 ){
		if( SYSCALL( "close", __LINE__, close( sockGuard->get() ) ) ) {
			result=true;
		}
	}
	sockGuard->reset();
	return result;
}

bool Socket::SetTimeout(unsigned long tout){
	if( !SYSCALL("SetTimeout", __LINE__,  sockGuard->get() )){
		return false;
	}

	timeout=tout;
	struct timeval tv;
	tv.tv_sec = static_cast<int>(timeout/1000);
	tv.tv_usec = static_cast<int>(0);

	if( !SYSCALL("setsockopt", __LINE__,  setsockopt(sockGuard->get(), SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)) )){
		return false;
	}
	return true;
}

bool Socket::SendTo(const std::string& buffer){
	if( !SYSCALL("SendTo", __LINE__,  sockGuard->get() )){
		return false;
	}

	struct sockaddr         *sadr	 = NULL;
	socklen_t                sadrLen = 0;
	if(listening){
		sadrLen = sizeof( udpSockStor );
		sadr    = (struct sockaddr*) &udpSockStor;
	}

	ssize_t wBytes = buffer.size();
	while ( wBytes > 0 ) {
		ssize_t count;
		do {
			count = sendto( sockGuard->get(),
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
	return true;
}
bool Socket::RecvFrom(std::string& buffer, int recvbuflen){
	if( !SYSCALL("RecvFrom", __LINE__,  sockGuard->get() )){
		return false;
	}

	struct sockaddr         *sadr	 = NULL;
	socklen_t                sadrLen = 0;
	if(listening && protocol=="udp"){
		sadrLen = sizeof( udpSockStor );
		sadr    = (struct sockaddr*) &udpSockStor;
	}

	ssize_t inBytes;
	do {
		std::string recvbuf;
		recvbuf.resize(recvbuflen+1);
		inBytes = recvfrom( sockGuard->get(), &recvbuf[0], recvbuflen, 0, sadr, &sadrLen );
		if(inBytes > 0){
			buffer+=std::string(recvbuf.begin(),recvbuf.begin()+inBytes);
			recvbuflen-=inBytes;
		}
		else if( (inBytes < 0) && (errno != EAGAIN ) ){
			SYSCALL("recvfrom", __LINE__,  inBytes );
			return false;
		}
	} while( inBytes > 0 && recvbuflen > 0 && protocol=="tcp" );
	if(listening && protocol=="udp"){
		struct addrinfo saddri={0,sadr->sa_family,0,0,sadrLen,sadr,0,0};
		PrintAddrInfo(&saddri);
	}
	return true;
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

int Socket::Accept () {
	/*
	** Determine if this is a TCP request or UDP request.
	*/
	if ( protocol == "tcp") {
		/*
		** TCP connection requested.  Accept it.  Notice the use of
		** the sockaddr_storage data type.
		*/
		struct sockaddr_storage  sockStor;
		struct sockaddr         *sadr = (struct sockaddr*) &sockStor;
		socklen_t                sadrLen = sizeof( sockStor );

		socket_guard newSG( accept( sockGuard->get(), sadr, &sadrLen ) );

		if(!SYSCALL("accept", __LINE__, newSG.get() ) ) {
			return INVALID_SOCKET;
		}

		{
			struct addrinfo saddri={0,sadr->sa_family,0,0,sadrLen,sadr,0,0};
			PrintAddrInfo(&saddri);
		}
		fprintf( stderr,"Socket: %d Opened.\n",newSG.get());
		return newSG.release();

	}  /* End IF this was a TCP connection request. */
	//	********************  Added for Socket Class ********************
	else {
		return INVALID_SOCKET;

	}/* End ELSE a UDP datagram is available. */
	//	********************  Added for Socket Class ********************
}
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
	int                      newSckt;
	
	if( sockGuard->get() < 0 ) { // Socket not open
         fprintf( stderr,
                  "%s (line %d): ERROR - Socket: Socket not open ",
                  "Socket",
                  __LINE__);

		return INVALID_SOCKET;
	}

	/*
	** Initialize the poll(2) array.
	*/
	{
		struct pollfd pfd;
		pfd.fd      = sockGuard->get();
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
			status = poll( &desc[0], desc.size(), -1 ); /* Wait indefinitely for input. */
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
		int idx = 0;
			switch ( desc[ idx ].revents ) {
				case 0:        /* No activity on this socket; try the next. */
					break;
				case POLLIN:   /* Network activity.  Go process it.         */
					desc[ idx ].revents = 0;   /* Clear the returned poll events. */
					return Accept(); //desc[ idx ].fd
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
	}  /* End WHILE forever. */
}  /* End tod() */



/******************************************************************************
* Function: OpenServer
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
bool Socket::OpenServer( ) {
	fprintf(stderr, "OpenServer called.\n");
   struct addrinfo *ai;
   struct addrinfo *aiHead;
   struct addrinfo  hints    = { .ai_flags  = AI_PASSIVE };   // Server mode.
//                                 .ai_family = PF_UNSPEC };   // IPv4 or IPv6.

	if( sockGuard->get() != INVALID_SOCKET ) { // Socket already open
         fprintf( stderr,
                  "%s (line %d): ERROR - Socket: %d already open ",
                  "Socket",
                  __LINE__,
                  sockGuard->get() );

		return false;
	}

   /*
   ** Check which network family is selected (only ipv4 and ipv6 are valid).
   */
	if ( family == "ipv4" ){ 
		hints.ai_family = PF_INET;
	}
	else if( family == "ipv6" ){
		hints.ai_family = PF_INET6;
	}
	else {
      fprintf( stderr,
               "%s (line %d): ERROR - Unknown family "
               "network family \"%s\".\n",
               "Socket",
               __LINE__,
               family.c_str() );
      return false;
	}

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
               "%s (line %d): ERROR - Unknown protocol "
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

		*sockGuard = sg;
   }  /* End FOR each address info structure returned. */
   /*
   ** Clean up.
   */
   freeaddrinfo( aiHead );
	fprintf( stderr,"Socket: %d Opened.\n",sockGuard->get());
   return true;
}  /* End openSckt() */


/******************************************************************************
* Function: OpenClient
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
bool Socket::OpenClient() {
	fprintf(stderr, "OpenClient called.\n");
	struct addrinfo *ai;
	struct addrinfo *aiHead;
	struct addrinfo  hints;

	if( sockGuard->get() != INVALID_SOCKET ) { // Socket already open
         fprintf( stderr,
                  "%s (line %d): ERROR - Socket: %d already open ",
                  "Socket",
                  __LINE__,
                  sockGuard->get() );

		return false;
	}

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
		return false;
	}

	/*
	** Look up the host/service information.
	*/
	int aiErr = getaddrinfo( host.c_str(), service.c_str(), &hints, &aiHead );
	if ( !SYSCALL("getaddrinfo", __LINE__, aiErr ) ) { //gai_strerror( aiErr )
		return false;
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
			freeaddrinfo( aiHead );
			*sockGuard = sg;
			fprintf( stderr,"Socket: %d Opened.\n",sockGuard->get());
			return true;
		}
	}  /* End FOR each address record returned by getaddrinfo(3). */

	/*
	** Clean up & return.
	*/
	freeaddrinfo( aiHead );
	return false;
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


bool Socket::SendTCP(const std::string& buffer){
	fprintf(stderr, "SendTCP called.\n");
	if( !SYSCALL("SendTC", __LINE__,  sockGuard->get() )){
		return false;
	}
    ssize_t wBytes = buffer.size();
    while ( wBytes > 0 ) {
		ssize_t count;
		do {
			count = write( sockGuard->get(),
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

// Receive until the peer closes the connection or read recvbuflen bytes
bool Socket::RecvTCP(std::string& buffer, int recvbuflen){
	fprintf(stderr, "RecvTCP called.\n");
	if( !SYSCALL("RecvTC", __LINE__,  sockGuard->get() )){
		return false;
	}
	ssize_t inBytes;
	do {
		char recvbuf[DEFAULT_BUFLEN];
		inBytes = read(sockGuard->get(), recvbuf, (recvbuflen<DEFAULT_BUFLEN)?recvbuflen:DEFAULT_BUFLEN);
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

bool Socket::SendUDP(const std::string& buffer){
	fprintf(stderr, "SendUDP called.\n");
	if( !SYSCALL("SendTC", __LINE__,  sockGuard->get() )){
		return false;
	}

	struct sockaddr         *sadr;
	socklen_t                sadrLen;
	if(listening){
		sadrLen = sizeof( udpSockStor );
		sadr    = (struct sockaddr*) &udpSockStor;
	}
	else{
		sadrLen = 0;
		sadr    = NULL;
	}

    ssize_t wBytes = buffer.size();
	ssize_t count;
	while ( wBytes > 0 ) {
		do {
			count = sendto( sockGuard->get(),
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
	return true;
}

bool Socket::RecvUDP(std::string& buffer, int recvbuflen) {
	fprintf(stderr, "RecvUDP called.\n");
	if( !SYSCALL("RecvUC", __LINE__,  sockGuard->get() )){
		return false;
	}
	bool result = false;

	struct sockaddr         *sadr;
	socklen_t                sadrLen;
	if(listening){
		fprintf(stderr, "RecvUDP listening.\n");
		memset (&udpSockStor,0,sizeof(udpSockStor));
		sadrLen = sizeof( udpSockStor );
		sadr    = (struct sockaddr*) &udpSockStor;
	}
	else{
		sadrLen = 0;
		sadr    = NULL;
	}
	ssize_t inBytes;
	//do {
		std::string recvbuf;
		recvbuf.resize(recvbuflen+1);
		inBytes = recvfrom( sockGuard->get(), &recvbuf[0], recvbuflen, 0, sadr, &sadrLen );
		if(inBytes > 0){
			buffer+=std::string(recvbuf.begin(),recvbuf.begin()+inBytes);
			recvbuflen-=inBytes;
			result = true;
		}
		else if( (inBytes < 0) && (errno != EAGAIN ) ){
			SYSCALL("recvfrom", __LINE__,  inBytes );
		}
	//} while( inBytes > 0 && recvbuflen > 0 );
	if(listening){
		struct addrinfo saddri={0,sadr->sa_family,0,0,sadrLen,sadr,0,0};
		PrintAddrInfo(&saddri);
	}
	return result;
}


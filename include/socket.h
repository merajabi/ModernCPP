#ifndef _POSIX_SOCKET_H_
#define _POSIX_SOCKET_H_

#include <iostream>
#include <atomic>
#include <string>

/*
** System header files.
*/
#include <errno.h>        /* errno declaration & error codes.            */
#include <netdb.h>        /* getaddrinfo(3) et al.                       */
#include <net/if.h>       /* if_nametoindex(3).                          */
#include <netinet/in.h>   /* sockaddr_in & sockaddr_in6 definition.      */
#include <stdio.h>        /* printf(3) et al.                            */
#include <stdlib.h>       /* exit(2).                                    */
#include <string.h>       /* String manipulation & memory functions.     */
#include <sys/poll.h>     /* poll(2) and related definitions.            */
#include <sys/socket.h>   /* Socket functions (socket(2), bind(2), etc). */
#include <time.h>         /* time(2) & ctime(3).                         */
#include <unistd.h>       /* getopt(3), read(2), etc.                    */

/*
** Constants.
*/
#define DFLT_SERVICE "daytime"   /* Default service name.                    */
#define INVALID_DESC -1          /* Invalid file descriptor.                 */
#define MAXCONNQLEN  3           /* Max nbr of connection requests to queue. */
#define MAXTCPSCKTS  2           /* One TCP socket for IPv4 & one for IPv6.  */
#define MAXUDPSCKTS  2           /* One UDP socket for IPv4 & one for IPv6.  */
#define VALIDOPTS    "v"         /* Valid command options.                   */

/*
** Constants & macros.
*/
#define DFLT_HOST      "localhost"   /* Default server name.              */
#define DFLT_SCOPE_ID  "eth0"        /* Default scope identifier.         */
//#define DFLT_SERVICE   "daytime"     /* Default service name.             */
//#define INVALID_DESC   -1            /* Invalid file (socket) descriptor. */
#define MAXBFRSIZE     256           /* Max bfr sz to read remote TOD.    */
//#define VALIDOPTS      "s:v"         /* Valid command options.            */


#define DEFAULT_BUFLEN 512
#define INVALID_SOCKET -1

/*
** Simple boolean type definition.
*/
// typedef enum { false = 0, true } boolean;
typedef bool boolean;

/*
** Type definitions (for convenience).
*/
//typedef enum { false = 0, true } boolean;
typedef struct sockaddr_in       sockaddr_in_t;
typedef struct sockaddr_in6      sockaddr_in6_t;


/*
** Usage macro for command syntax violations.
*/
#define USAGE                                       \
        {                                           \
           fprintf( stderr,                         \
                    "Usage: %s [-v] [service]\n",   \
                    pgmName );                      \
           exit( 127 );                             \
        }  /* End USAGE macro. */

/*
** Macro to terminate the program if a system call error occurs.  The system
** call must be one of the usual type that returns -1 on error.  This macro is
** a modified version of a macro authored by Dr. V. Vinge, SDSU Dept. of
** Computer Science (retired)... best professor I ever had.  I hear he writes
** great science fiction in addition to robust code, too.
*/
#define CHK(expr)                                                   \
        do                                                          \
        {                                                           \
           if ( (expr) == -1 )                                      \
           {                                                        \
              fprintf( stderr,                                      \
                       "%s (line %d): System call ERROR - %s.\n",   \
                       pgmName,                                     \
                       __LINE__,                                    \
                       strerror( errno ) );                         \
              exit( 1 );                                            \
           }   /* End IF system call failed. */                     \
        } while ( false )

/*
** This "macro" (even though it's really a function) is loosely based on the
** CHK() macro by Dr. V. Vinge (see server code).  The status parameter is
** a boolean expression indicating the return code from one of the usual system
** calls that returns -1 on error.  If a system call error occurred, an alert
** is written to stderr.  It returns a boolean value indicating success/failure
** of the system call.
**
** Example: if ( !SYSCALL( "write",
**                         count = write( fd, bfr, size ) ) )
**          {
**             // Error processing... but SYSCALL() will have already taken
**             // care of dumping an error alert to stderr.
**          }
*/
class Socket;
static __inline boolean SYSCALL( const char *syscallName, int lineNbr, int status );
/*
{
   if ( ( status == -1 ) && verbose ) {
      fprintf( stderr,
               "%s (line %d): System call failed ('%s') - %s.\n",
               Socket::pgmName,
               lineNbr,
               syscallName,
               strerror( errno ) );
   }
   return status != -1;   // True if the system call was successful. /
}  // End SYSCALL() 
*/
typedef int SOCKET;

class Socket {
	static std::atomic<unsigned long> sockCount;

    int sock;
	std::string host;
	std::string service; // service OR port
	std::string protocol;
	unsigned long timeout;

	//const char *service   = DFLT_SERVICE;
	int         cSckt;					// Client Socket
	int         tSckt[ MAXTCPSCKTS ];	/* Array of TCP socket descriptors. */
	size_t      tScktSize;				/* Size of uSckt (# of elements).   */
	int         uSckt[ MAXUDPSCKTS ];	/* Array of UDP socket descriptors. */
	size_t      uScktSize;				/* Size of uSckt (# of elements).   */

	struct sockaddr_storage  sockStor;


	int openSckt( const char *service, const char *protocol, int desc[ ], size_t maxDescs, size_t *descSize );
	int openScktTC( const char *host, const char *service, unsigned int scopeId );
	int openScktUC( const char *host, const char *service, unsigned int scopeId );

	int tod( int tSckt[ ], size_t tScktSize, int uSckt[ ], size_t uScktSize );

	public:
	/*
	** Global (within this file only) data objects.
	*/
	static char        hostBfr[ NI_MAXHOST ];   /* For use w/getnameinfo(3).    */
	static char        servBfr[ NI_MAXSERV ];   /* For use w/getnameinfo(3).    */
	static const char *pgmName;                 /* Program name w/o dir prefix. */
	static boolean     verbose;         /* Verbose mode indication.     */

	public:
		Socket(const int& s);
		Socket(const std::string& pH=DFLT_HOST, const std::string& pP=DFLT_SERVICE, const std::string& proto="tcp", unsigned long tout=0);
		~Socket();

		bool SendUC(const std::string& buffer);
		bool SendTC(const std::string& buffer);

		bool RecvUC(std::string& buffer, int recvbuflen);
		bool RecvTC(std::string& buffer, int recvbuflen);

		bool SetTimeout(unsigned long tout);
		bool PrintDebugInfo();

		bool Close();

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
		bool OpenTcpClient(){
		   unsigned int  scopeId  = if_nametoindex( DFLT_SCOPE_ID );
		  fprintf( stderr, " scopeName: %s scopeId: %d \n",DFLT_SCOPE_ID,scopeId);

		   if ( ( cSckt = openScktTC( host.c_str(),
					               service.c_str(),
					               scopeId ) ) == INVALID_DESC )
		   {
			  fprintf( stderr,
					   "%s: Sorry... a connection could not be established.\n",
					   pgmName );
			  exit( 1 );
		   }
			sock = cSckt;
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
		bool OpenUdpClient(){
		   unsigned int  scopeId  = if_nametoindex( DFLT_SCOPE_ID );
		  fprintf( stderr, " scopeName: %s scopeId: %d \n",DFLT_SCOPE_ID,scopeId);

		   if ( ( cSckt = openScktUC( host.c_str(),
				                   service.c_str(),
				                   scopeId ) ) == INVALID_DESC )
		   {
			  fprintf( stderr,
				       "%s: Sorry... a connectionless socket could "
				       "not be set up.\n",
				       pgmName );
			  exit( 1 );
		   }
			sock = cSckt;
		}
		bool OpenServer(){
			return ( openSckt( service.c_str(), "tcp", tSckt, MAXTCPSCKTS, &tScktSize ) && 
					openSckt( service.c_str(), "udp", uSckt, MAXUDPSCKTS, &uScktSize ) );
		}
		int Accept(){
			/*
			** Run the time-of-day server.
			*/
			if ( ( tScktSize > 0 ) || ( uScktSize > 0 ) ) {         
				return tod( tSckt, tScktSize, uSckt, uScktSize ); /* tod() never returns. */
			}
		}
		operator bool () {return ( sock >= 0 );}

/*
		bool Open(){
			int iResult;
			//struct sockaddr_in serv_addr; 
			struct addrinfo *result = NULL;
			struct addrinfo *rp = NULL;
			struct addrinfo hints;


			memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_family = AF_INET6;    // Allow IPv4 or IPv6:AF_UNSPEC IPv4:AF_INET IPv6:AF_INET6
			hints.ai_socktype = SOCK_STREAM; // Datagram socket SOCK_DGRAM SOCK_STREAM
			hints.ai_flags = AI_PASSIVE;    // For wildcard IP address
			hints.ai_protocol = 0;          // Any protocol:0 tcp:IPPROTO_TCP
			hints.ai_canonname = NULL;
			hints.ai_addr = NULL;
			hints.ai_next = NULL;

			// Resolve the server address and port
			iResult = getaddrinfo((host.size())?host.c_str():NULL, port.c_str(), &hints, &result);
			if ( iResult != 0 ) {
		        fprintf(stderr, "getaddrinfo failed with error: %s\n", gai_strerror(iResult));
				return false;
			}
			//  getaddrinfo() returns a list of address structures.
			//   Try each address until we successfully bind(2).
			//   If socket(2) (or bind(2)) fails, we (close the socket
			//   and) try the next address. 


			// Attempt to connect to an address until one succeeds
		   for (rp = result; rp != NULL; rp = rp->ai_next) {

				// Create a SOCKET for connecting to server
				sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
				//sock = socket(AF_INET, SOCK_STREAM, 0);
				if (sock < 0 ) {
					fprintf(stderr, "\n Socket creation failed with error %d: %s\n",errno,strerror(errno)); 
					continue;
				}

//				serv_addr.sin_family = rp->ai_family; 
//				serv_addr.sin_port = htons(atoi(port.c_str())); 

				// Convert IPv4 and IPv6 addresses from text to binary form 
//				iResult = inet_pton(rp->ai_family, host.c_str(), &serv_addr.sin_addr);
//				if(iResult == 0) { 
//					fprintf(stderr, "\n Invalid address/ Address not supported %d: %s\n",errno,strerror(errno)); 
//					Close();
//					continue;
//				} else if(iResult <0){
//					fprintf(stderr, "\n Invalid address family %d: %s\n",errno,strerror(errno)); 
//					Close();
//					continue;
//				}

				//iResult = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
				iResult = connect(sock, (struct sockaddr *)rp->ai_addr, sizeof(*(rp->ai_addr)));
				if ( iResult < 0) { 
					fprintf(stderr, "\n connect failed with error %d: %s\n",errno,strerror(errno)); 
					Close();
					continue;
				} 

				break;
			}
			freeaddrinfo(result); // No longer needed 

			if (sock < 0 ) {
				fprintf(stderr, "\n Unable to connect to server! \n"); 
				return false;
			}

			return true;
		}
*/

/*
		int Accept(){

			int iResult;
			int opt = 1; 
			struct sockaddr_in6 address; 
			int addrlen = sizeof(address); 

			address.sin6_family = AF_INET6; 					// Allow IPv4 or IPv6:AF_UNSPEC IPv4:AF_INET
			//address.sin6_addr.s6_addr = in6addr_any; 
			address.sin6_addr = in6addr_any; 
			address.sin6_port = htons( atoi(port.c_str()) ); 

			//inet_pton(AF_INET6, "::", &address.sin6_addr);

			// Creating socket file descriptor 
			sock = socket(AF_INET6, SOCK_STREAM, 0);
			if ( sock < 0 ) 
			{ 
				fprintf(stderr, "\n Creating socket failed with error %d: %s\n",errno,strerror(errno)); 
				sock=INVALID_SOCKET;
				return INVALID_SOCKET;
			} 


			// Forcefully attaching socket to the port 8080 
			if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
			{ 
				fprintf(stderr, "\n setsockopt failed with error %d: %s\n",errno,strerror(errno)); 
				Close();
				return INVALID_SOCKET;
			} 

			// Forcefully attaching socket to the port 8080 
			if (bind(sock, (struct sockaddr *)&address, sizeof(address) )<0) 
			{ 
				fprintf(stderr, "\n bind failed with error %d: %s\n",errno,strerror(errno)); 
				Close();
				return INVALID_SOCKET;
			} 

			iResult = listen(sock, SOMAXCONN); //SOMAXCONN
			if (iResult < 0) {
				fprintf(stderr, "\n listen failed with error %d: %s\n",errno,strerror(errno)); 
				Close();
				return INVALID_SOCKET;
			}

			// Accept a client socket
			int clientSocket = accept(sock, (struct sockaddr *)&address, (socklen_t*)&addrlen);
			if (clientSocket < 0 ) {
				fprintf(stderr, "\n accept failed with error %d: %s\n",errno,strerror(errno)); 
				Close();
				return INVALID_SOCKET;
			}

			// No longer need server socket
			Close();
			return clientSocket;

		}
*/

	private:
		static bool Initialize(){
			if( 0 == sockCount++ ){
				int iResult;
/*
				// Initialize Winsock
				iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
				if (iResult != 0) {
					printf("WSAStartup failed with error: %d\n", iResult);
					return false;
				}
*/
			}
			return true;
		}
		static void Finalize(){
			if( --sockCount == 0 ){
				//WSACleanup();
			}
		}

};

#endif //_WIN_SOCKET_H_

#ifndef _POSIX_SOCKET_H_
#define _POSIX_SOCKET_H_
/*
typedef uint16_t in_port_t;         
typedef uint32_t in_addr_t;         
 
struct in_addr {
     in_addr_t s_addr;           
 };
 
 struct in6_addr {
     uint8_t s6_addr[16];        
 };
 
 struct sockaddr_in {
     sa_family_t     sin_family; //    short            sin_family;   // e.g. AF_INET
     in_port_t       sin_port;   //    unsigned short   sin_port;     // e.g. htons(3490)
     struct in_addr  sin_addr;   //    struct in_addr   sin_addr;     // see struct in_addr, below
 };								//    char             sin_zero[8];  // zero this if you want to

 struct sockaddr_in6 {
     sa_family_t     sin6_family;    
     in_port_t       sin6_port;      
     uint32_t        sin6_flowinfo;  
     struct in6_addr sin6_addr;      
     uint32_t        sin6_scope_id;  
 };

struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
}; 

struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname

    struct addrinfo *ai_next;      // linked list, next node
};

*/
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
** Constants & macros.
*/
#define DFLT_HOST			"localhost" /* Default server name.              */
#define DFLT_SCOPE_ID		"eth0"      /* Default scope identifier.         */
#define DFLT_SERVICE		"daytime"   /* Default service name.             */
#define INVALID_DESC		-1          /* Invalid file descriptor.          */
#define INVALID_SOCKET		-1
#define MAXBFRSIZE			256         /* Max bfr sz to read remote TOD.    */
#define DEFAULT_BUFLEN		256
#define MAXCONNQLEN			3           /* Max nbr of connection requests to queue. */
#define MAXTCPSCKTS			2           /* One TCP socket for IPv4 & one for IPv6.  */
#define MAXUDPSCKTS			2           /* One UDP socket for IPv4 & one for IPv6.  */
#define VALIDOPTS			"v"         /* Valid command options.                   */
//#define VALIDOPTS			"s:v"       /* Valid command options.            */


/*
** Type definitions (for convenience).
*/
// typedef enum { false = 0, true } boolean;
typedef bool boolean;
typedef int SOCKET;
typedef struct sockaddr_in       sockaddr_in_t;
typedef struct sockaddr_in6      sockaddr_in6_t;

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

static __inline boolean SYSCALL( const char *syscallName, int lineNbr, int status );

class Socket {
	static std::atomic<unsigned long> sockCount;

    int sock;						// Active Socket
	std::string host;
	std::string service; // service OR port / DFLT_SERVICE
	std::string protocol;
	unsigned long timeout;

	int         cSckt;					// Client Socket

	int         tSckt[ MAXTCPSCKTS ];	/* Array of TCP socket descriptors. */
	size_t      tScktSize;				/* Size of uSckt (# of elements).   */
	int         uSckt[ MAXUDPSCKTS ];	/* Array of UDP socket descriptors. */
	size_t      uScktSize;				/* Size of uSckt (# of elements).   */

	struct sockaddr_storage  sockStor;


	int openSckt( const char *service, const char *protocol, int desc[ ], size_t maxDescs, size_t *descSize );
	int openSckt( const char *host, const char *service, unsigned int scopeId, const char *transport );

	int Listen( int tSckt[ ], size_t tScktSize, int uSckt[ ], size_t uScktSize );

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

		bool SendUDP(const std::string& buffer);
		bool SendTCP(const std::string& buffer);

		bool RecvUDP(std::string& buffer, int recvbuflen);
		bool RecvTCP(std::string& buffer, int recvbuflen);

		bool SetTimeout(unsigned long tout);
		bool PrintAddrInfo( struct addrinfo *ai );
		bool PrintIncomingInfo( struct addrinfo *sadri );
		bool Close();

		bool OpenClient();

		bool OpenServer(){
			return ( openSckt( service.c_str(), "tcp", tSckt, MAXTCPSCKTS, &tScktSize ) && 
					openSckt( service.c_str(), "udp", uSckt, MAXUDPSCKTS, &uScktSize ) );
		}
		int Accept(){
			if ( ( tScktSize > 0 ) || ( uScktSize > 0 ) ) {         
				return Listen( tSckt, tScktSize, uSckt, uScktSize ); /* tod() never returns. */
			}
		}
		operator bool () {return ( sock >= 0 );}

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

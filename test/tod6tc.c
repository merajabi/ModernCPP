/*
The TCP client code is found in file tod6tc.c (time-of-day IPv6 TCP client). Once built, the TCP client may be started using the following command syntax (assuming tod6tc is the executable file):

tod6tc [-v] [-s scope_id] [host [service]]

ARGUMENTS:

host

    The hostname or IP address (dotted decimal or colon-hex) of the remote host providing the service. Default is "localhost".
service

    The TCP service (or well-known port number) to which a connection attempt is made. Default is "daytime".

OPTIONS:

-s

    This option is only meaningful for IPv6 addresses, and is used to set the scope identifier (i.e. the network interface on which to establish the connection). Default is "eth0". If host is a scoped address, this option is ignored.
-v

    Turn on verbose mode.

The TCP client source code contained in tod6tc.c follows:
*/
/******************************************************************************
* File: tod6tc.c
* Description: Contains source code for an IPv6-capable 'daytime' TCP client.
* Author: John Wenker, Sr. Software Engineer
*         Performance Technologies, San Diego, USA
******************************************************************************/
/*
** System header files.
*/
#include <errno.h>        /* errno declaration and error codes.             */
#include <net/if.h>       /* if_nametoindex(3).                             */
#include <netdb.h>        /* getaddrinfo(3) and associated definitions.     */
#include <netinet/in.h>   /* sockaddr_in and sockaddr_in6 definitions.      */
#include <stdio.h>        /* printf(3) et al.                               */
#include <stdlib.h>       /* exit(2).                                       */
#include <string.h>       /* String manipulation and memory functions.      */
#include <sys/socket.h>   /* Socket functions (socket(2), connect(2), etc). */
#include <unistd.h>       /* getopt(3), read(2), etc.                       */
/*
** Constants & macros.
*/
#define DFLT_HOST      "localhost"   /* Default server name.              */
#define DFLT_SCOPE_ID  "eth0"        /* Default scope identifier.         */
#define DFLT_SERVICE   "daytime"     /* Default service name.             */
#define INVALID_DESC   -1            /* Invalid file (socket) descriptor. */
#define MAXBFRSIZE     256           /* Max bfr sz to read remote TOD.    */
#define VALIDOPTS      "s:v"         /* Valid command options.            */
/*
** Type definitions (for convenience).
*/
typedef enum { false = 0, true } boolean;
typedef struct sockaddr_in       sockaddr_in_t;
typedef struct sockaddr_in6      sockaddr_in6_t;
/*
** Prototypes for internal helper functions.
*/
static int  openSckt( const char   *host,
                      const char   *service,
                      unsigned int  scopeId );
static void tod( int sckt );
/*
** Global (within this file only) data objects.
*/
static const char *pgmName;           /* Program name (w/o directory). */
static boolean     verbose = false;   /* Verbose mode.                 */
/*
** Usage macro.
*/
#define USAGE                                                            \
        {                                                                \
           fprintf( stderr,                                              \
                    "Usage: %s [-v] [-s scope_id] [host [service]]\n",   \
                    pgmName );                                           \
           exit( 127 );                                                  \
        }  /* End USAGE macro. */
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
static __inline boolean SYSCALL( const char *syscallName,
                                 int         lineNbr,
                                 int         status )
{
   if ( ( status == -1 ) && verbose )
   {
      fprintf( stderr,
               "%s (line %d): System call failed ('%s') - %s.\n",
               pgmName,
               lineNbr,
               syscallName,
               strerror( errno ) );
   }
   return status != -1;   /* True if the system call was successful. */
}  /* End SYSCALL() */
/******************************************************************************
* Function: main
*
* Description:
*    Connect to a remote time-of-day service and write the remote host's TOD to
*    stdout.
*
* Parameters:
*    The usual argc & argv parameters to a main() program.
*
* Return Value:
*    This function always returns zero.
******************************************************************************/
int main( int   argc,
          char *argv[ ] )
{
   const char   *host     = DFLT_HOST;
   int           opt;
   int           sckt;
   unsigned int  scopeId  = if_nametoindex( DFLT_SCOPE_ID );
   const char   *service  = DFLT_SERVICE;
   /*
   ** Determine the program name (w/o directory prefix).
   */
   pgmName = (const char*) strrchr( argv[ 0 ], '/' );
   pgmName = pgmName == NULL  ?  argv[ 0 ]  :  pgmName+1;
   /*
   ** Process command line options.
   */
   opterr = 0;   /* Turns off "invalid option" error messages. */
   while ( ( opt = getopt( argc, argv, VALIDOPTS ) ) != -1 )
   {
      switch ( opt )
      {
         case 's':   /* Scope identifier (IPv6 kluge). */
         {
            scopeId = if_nametoindex( optarg );
            if ( scopeId == 0 )
            {
               fprintf( stderr,
                        "%s: Unknown network interface (%s).\n",
                        pgmName,
                        optarg );
               USAGE;
            }
            break;
         }
         case 'v':   /* Verbose mode. */
         {
            verbose = true;
            break;
         }
         default:
         {
            USAGE;
         }
      }  /* End SWITCH on command option. */
   } /* End WHILE processing command options. */
   /*
   ** Process command arguments.  At the end of the above loop, optind is the
   ** index of the first NON-option argv element.
   */
   switch ( argc - optind )
   {
      case 2:   /* Both host & service are specified on the command line. */
      {
          service = argv[ optind + 1 ];
          /***** Fall through *****/
      }
      case 1:   /* Host is specified on the command line. */
      {
          host = argv[ optind ];
          /***** Fall through *****/
      }
      case 0:   /* Use default host & service. */
      {
          break;
      }
      default:
      {
         USAGE;
      }
   }  /* End SWITCH on number of command arguments. */
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
   if ( ( sckt = openSckt( host,
                           service,
                           scopeId ) ) == INVALID_DESC )
   {
      fprintf( stderr,
               "%s: Sorry... a connection could not be established.\n",
               pgmName );
      exit( 1 );
   }
   /*
   ** Get the remote time-of-day.
   */
   tod( sckt );
   /*
   ** Close the connection and terminate.
   */
   (void) SYSCALL( "close",
                   __LINE__,
                   close( sckt ) );
   return 0;
}  /* End main() */
/******************************************************************************
* Function: openSckt
*
* Description:
*    Sets up a TCP connection to a remote server.  Getaddrinfo(3) is used to
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
*              network interface on which to set up the connection.  This
*              parameter is ignored for IPv4 sockets or when an IPv6 "scoped
*              address" is specified in 'host' (i.e. where the colon-hex
*              network address is augmented with the scope ID).
*
* Return Value:
*    Returns the socket descriptor for the connection, or INVALID_DESC if all
*    address records have been processed and a connection could not be
*    established.
******************************************************************************/
static int openSckt( const char   *host,
                     const char   *service,
                     unsigned int  scopeId )
{
   struct addrinfo *ai;
   int              aiErr;
   struct addrinfo *aiHead;
   struct addrinfo  hints;
   sockaddr_in6_t  *pSadrIn6;
   int              sckt;
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
   hints.ai_socktype = SOCK_STREAM;   /* Connection-oriented byte stream.   */
   hints.ai_protocol = IPPROTO_TCP;   /* TCP transport layer protocol only. */
   /*
   ** Look up the host/service information.
   */
   if ( ( aiErr = getaddrinfo( host,
                               service,
                               &hints,
                               &aiHead ) ) != 0 )
   {
      fprintf( stderr,
               "%s (line %d): ERROR - %s.\n",
               pgmName,
               __LINE__,
               gai_strerror( aiErr ) );
      return INVALID_DESC;
   }
   /*
   ** Go through the list and try to open a connection.  Continue until either
   ** a connection is established or the entire list is exhausted.
   */
   for ( ai = aiHead,   sckt = INVALID_DESC;
         ( ai != NULL ) && ( sckt == INVALID_DESC );
         ai = ai->ai_next )
   {
      /*
      ** IPv6 kluge.  Make sure the scope ID is set.
      */
      if ( ai->ai_family == PF_INET6 )
      {
         pSadrIn6 = (sockaddr_in6_t*) ai->ai_addr;
         if ( pSadrIn6->sin6_scope_id == 0 )
         {
            pSadrIn6->sin6_scope_id = scopeId;
         }  /* End IF the scope ID wasn't set. */
      }  /* End IPv6 kluge. */
      /*
      ** Display the address info for the remote host.
      */
      if ( verbose )
      {
         /*
         ** Temporary character string buffers for host & service.
         */
         char hostBfr[ NI_MAXHOST ];
         char servBfr[ NI_MAXSERV ];
         /*
         ** Display the address information just fetched.  Start with the
         ** common (protocol-independent) stuff first.
         */
         fprintf( stderr,
                  "Address info:\n"
                  "   ai_flags     = 0x%02X\n"
                  "   ai_family    = %d (PF_INET = %d, PF_INET6 = %d)\n"
                  "   ai_socktype  = %d (SOCK_STREAM = %d, SOCK_DGRAM = %d)\n"
                  "   ai_protocol  = %d (IPPROTO_TCP = %d, IPPROTO_UDP = %d)\n"
                  "   ai_addrlen   = %d (sockaddr_in = %d, "
                  "sockaddr_in6 = %d)\n",
                  ai->ai_flags,
                  ai->ai_family,
                  PF_INET,
                  PF_INET6,
                  ai->ai_socktype,
                  SOCK_STREAM,
                  SOCK_DGRAM,
                  ai->ai_protocol,
                  IPPROTO_TCP,
                  IPPROTO_UDP,
                  ai->ai_addrlen,
                  sizeof( struct sockaddr_in ),
                  sizeof( struct sockaddr_in6 ) );
         /*
         ** Display the protocol-specific formatted address.
         */
         getnameinfo( ai->ai_addr,
                      ai->ai_addrlen,
                      hostBfr,
                      sizeof( hostBfr ),
                      servBfr,
                      sizeof( servBfr ),
                      NI_NUMERICHOST | NI_NUMERICSERV );
         switch ( ai->ai_family )
         {
            case PF_INET:   /* IPv4 address record. */
            {
               sockaddr_in_t *pSadrIn = (sockaddr_in_t*) ai->ai_addr;
               fprintf( stderr,
                        "   ai_addr      = sin_family: %d (AF_INET = %d, "
                        "AF_INET6 = %d)\n"
                        "                  sin_addr:   %s\n"
                        "                  sin_port:   %s\n",
                        pSadrIn->sin_family,
                        AF_INET,
                        AF_INET6,
                        hostBfr,
                        servBfr );
               break;
            }  /* End CASE of IPv4 record. */
            case PF_INET6:   /* IPv6 address record. */
            {
               pSadrIn6 = (sockaddr_in6_t*) ai->ai_addr;
               fprintf( stderr,
                        "   ai_addr      = sin6_family:   %d (AF_INET = %d, "
                        "AF_INET6 = %d)\n"
                        "                  sin6_addr:     %s\n"
                        "                  sin6_port:     %s\n"
                        "                  sin6_flowinfo: %d\n"
                        "                  sin6_scope_id: %d\n",
                        pSadrIn6->sin6_family,
                        AF_INET,
                        AF_INET6,
                        hostBfr,
                        servBfr,
                        pSadrIn6->sin6_flowinfo,
                        pSadrIn6->sin6_scope_id );
               break;
            }  /* End CASE of IPv6 record. */
            default:   /* Can never get here, but just for completeness. */
            {
               fprintf( stderr,
                        "%s (line %d): ERROR - Unknown protocol family (%d).\n",
                        pgmName,
                        __LINE__,
                        ai->ai_family );
               break;
            }  /* End DEFAULT case (unknown protocol family). */
         }  /* End SWITCH on protocol family. */
      }  /* End IF verbose mode. */
      /*
      ** Create a socket.
      */
      if ( !SYSCALL( "socket",
                     __LINE__,
                     sckt = socket( ai->ai_family,
                                    ai->ai_socktype,
                                    ai->ai_protocol ) ) )
      {
         sckt = INVALID_DESC;
         continue;   /* Try the next address record in the list. */
      }
      /*
      ** Connect to the remote host.
      */
      if ( !SYSCALL( "connect",
                     __LINE__,
                     connect( sckt,
                              ai->ai_addr,
                              ai->ai_addrlen ) ) )
      {
         (void) close( sckt );   /* Could use SYSCALL() again here, but why? */
         sckt = INVALID_DESC;
         continue;   /* Try the next address record in the list. */
      }
   }  /* End FOR each address record returned by getaddrinfo(3). */
   /*
   ** Clean up & return.
   */
   freeaddrinfo( aiHead );
   return sckt;
}  /* End openSckt() */
/******************************************************************************
* Function: tod
*
* Description:
*    Receive the time-of-day from the remote server and write it to stdout.
*
* Parameters:
*    sckt - The socket descriptor for the connection.
*
* Return Value: None.
******************************************************************************/
static void tod( int sckt )
{
   char bfr[ MAXBFRSIZE+1 ];
   int  inBytes;
   /*
   ** The client never sends anything, so shut down the write side of the
   ** connection.
   */
   if ( !SYSCALL( "shutdown",
                  __LINE__,
                  shutdown( sckt, SHUT_WR ) ) )
   {
      return;
   }
   /*
   ** Read the time-of-day from the remote host.
   */
   do
   {
      if ( !SYSCALL( "read",
                     __LINE__,
                     inBytes = read( sckt,
                                     bfr,
                                     MAXBFRSIZE ) ) )
      {
         return;
      }
      bfr[ inBytes ] = '\0';   /* Null-terminate the received string. */
      fputs( bfr, stdout );    /* Null string if EOF (inBytes == 0).  */
   } while ( inBytes > 0 );
   fflush( stdout );
}  /* End tod() */


#ifndef _WIN_SOCKET_H_
#define _WIN_SOCKET_H_

#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <atomic>
#include <string>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")
// #pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512

class Socket {
    static WSADATA wsaData;
	static std::atomic<unsigned long> count;

    SOCKET winsocket;
	std::string host;
	std::string port;
	std::string proto;
	DWORD timeout;
	public:
		Socket(const SOCKET& s):winsocket(s),timeout(0){
			Initialize();
		}
		Socket(const std::string& pH, const std::string& pP, std::string proto="tcp", unsigned long tout=0):winsocket(INVALID_SOCKET),host(pH),port(pP),proto(proto),timeout(tout) {
			Initialize();
			if(host.size()!=0){
				Open();
			}
			if(timeout){
				SetTimeout(tout);
			}
		}
		~Socket(){ 
			Close();
			Finalize();
		}
		bool Open(){
			int iResult;
			struct addrinfo *result = NULL;
			struct addrinfo *ptr = NULL;
			struct addrinfo hints;
			
//			if(!Init()){
//				return false;
//			};

			ZeroMemory( &hints, sizeof(hints) );
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			// Resolve the server address and port
			iResult = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
			if ( iResult != 0 ) {
				printf("getaddrinfo failed with error: %d\n", iResult);
				WSACleanup();
				return false;
			}
			// Attempt to connect to an address until one succeeds
			for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

				// Create a SOCKET for connecting to server
				winsocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if (winsocket == INVALID_SOCKET) {
					printf("socket failed with error: %ld\n", WSAGetLastError());
					WSACleanup();
					return false;
				}

				// Connect to server.
				iResult = connect( winsocket, ptr->ai_addr, (int)ptr->ai_addrlen);
				if (iResult == SOCKET_ERROR) {
					closesocket(winsocket);
					winsocket = INVALID_SOCKET;
					continue;
				}
				break;
			}

			freeaddrinfo(result);

			if (winsocket == INVALID_SOCKET) {
				printf("Unable to connect to server!\n");
				WSACleanup();
				return false;
			}			
			return true;
		}
		SOCKET Accept(){
			int iResult;
			struct addrinfo *result = NULL;
			struct addrinfo hints;
			
//			if(!Init()){
//				return INVALID_SOCKET;
//			};
			
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			hints.ai_flags = AI_PASSIVE;

			// Resolve the server address and port
			iResult = getaddrinfo((host.size())?host.c_str():NULL, port.c_str(), &hints, &result);
			if ( iResult != 0 ) {
				printf("getaddrinfo failed with error: %d\n", iResult);
				WSACleanup();
				return INVALID_SOCKET;
			}

			// Create a SOCKET for connecting to server
			winsocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (winsocket == INVALID_SOCKET) {
				printf("socket failed with error: %ld\n", WSAGetLastError());
				freeaddrinfo(result);
				WSACleanup();
				return INVALID_SOCKET;
			}

			// Setup the TCP listening socket
			iResult = bind( winsocket, result->ai_addr, (int)result->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				printf("bind failed with error: %d\n", WSAGetLastError());
				freeaddrinfo(result);
				closesocket(winsocket);
				WSACleanup();
				return INVALID_SOCKET;
			}

			freeaddrinfo(result);

			iResult = listen(winsocket, SOMAXCONN);
			if (iResult == SOCKET_ERROR) {
				printf("listen failed with error: %d\n", WSAGetLastError());
				closesocket(winsocket);
				WSACleanup();
				return INVALID_SOCKET;
			}

			// Accept a client socket
			SOCKET clientSocket = accept(winsocket, NULL, NULL);
			if (clientSocket == INVALID_SOCKET) {
				printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(winsocket);
				WSACleanup();
				return INVALID_SOCKET;
			}

			// No longer need server socket
			Close();
			return clientSocket;
		}
		bool Close(){
		    closesocket(winsocket);
			winsocket=INVALID_SOCKET;
			return true;
		}
		bool Send(const std::string& buffer){
			int iResult;
			// Send an initial buffer
		    iResult = send( winsocket, buffer.c_str(), (int)buffer.size(), 0 );
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(winsocket);
				WSACleanup();
				return false;
			}

			printf("Bytes Sent: %ld\n", iResult);
			return true;
		}
		bool Recv(std::string& buffer, int recvbuflen){
			int iResult;
		    // Receive until the peer closes the connection or read recvbuflen bytes
			do {
				char recvbuf[DEFAULT_BUFLEN];
				iResult = recv(winsocket, recvbuf, (recvbuflen<DEFAULT_BUFLEN)?recvbuflen:DEFAULT_BUFLEN, 0);
				if ( iResult > 0 ) {
					buffer+=std::string(recvbuf,recvbuf+iResult);
					recvbuflen-=iResult;
					printf("Bytes received: %d\n", iResult);
				} else if ( iResult == 0 ) {
					printf("Connection closed\n");
				} else {
					if(WSAGetLastError() != WSAETIMEDOUT ){
						printf("recv failed with error: %d\n", WSAGetLastError());
						closesocket(winsocket);
						WSACleanup();
						return false;
					}else{
						printf("Connection timed out\n");
					}
				}
			} while( iResult > 0 && recvbuflen > 0 );
			return true;
		}
		operator bool () {return !(winsocket==INVALID_SOCKET);}
	bool SetTimeout(unsigned long tout){
		bool result=false;
		timeout=tout;
		if(winsocket != INVALID_SOCKET){
			int iResult;
			iResult = setsockopt(winsocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
			if ( iResult == 0 ){
				result=true;
			} else {
				printf("setsockopt failed with error: %d\n", WSAGetLastError());
			}
		}
		return result;
	}
	private:
		static bool Initialize(){
			if( 0 == count++ ){
				int iResult;
				// Initialize Winsock
				iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
				if (iResult != 0) {
					printf("WSAStartup failed with error: %d\n", iResult);
					return false;
				}
			}
			return true;
		}
		static void Finalize(){
			if( --count == 0 ){
				WSACleanup();
			}
		}

};

WSADATA Socket::wsaData;
std::atomic<unsigned long> Socket::count(0);

#endif //_WIN_SOCKET_H_

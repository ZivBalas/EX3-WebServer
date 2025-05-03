#pragma once
#include "CommonHeaders.h"
#include "SocketState.h"

class Server
{
	WSAData wsaData;
	SOCKET listenSocket;
	SocketState sockets[MAX_SOCKETS];
	int socketsCount = 0;
	void onlineLoop();
	void shutDownServer();
public:
	bool addSocket(SOCKET id, int what);
	void removeSocket(int index);
	void acceptConnection(int index);
	void receiveMessage(int index);
	int initialize();
	void sendMessage(int index);
	int Run();
};


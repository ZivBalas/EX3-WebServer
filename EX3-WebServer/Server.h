#pragma once
#include "CommonHeaders.h"
#include "SocketState.h"

class Server
{
    WSAData wsaData;                // Holds data for Windows Socket API
    SOCKET listenSocket;           // Listening socket for incoming connections
    SocketState sockets[MAX_SOCKETS]; // Array of socket states
    int socketsCount = 0;          // Number of active sockets
    void onlineLoop();             // Main loop to handle I/O events
    void shutDownServer();         // Clean up resources on shutdown
public:
    bool addSocket(SOCKET id, int what);     // Add new socket to the array
    void removeSocket(int index);            // Remove socket at given index
    void acceptConnection(int index);        // Accept new client connection
    void receiveMessage(int index);          // Handle incoming data
    int initialize();                        // Initialize WinSock and bind socket
    void sendMessage(int index);             // Send data through socket
    void checkForTimeouts();                 // Close idle sockets
    int Run();                               // Launch the server
};


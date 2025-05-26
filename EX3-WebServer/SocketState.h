#pragma once
#include "CommonHeaders.h"

class SocketState
{
    SOCKET id;               // Socket handle
    int recieve_state;       // State for receiving data
    int send_state;          // State for sending data
    char buffer[2000];       // Buffer for received data
    int len;                 // Length of data currently in buffer
    time_t lastActivityTime; // Last activity timestamp for timeout tracking

public:
    SocketState() : id(INVALID_SOCKET), recieve_state(0), send_state(0), len(0) {
        memset(buffer, 0, sizeof(buffer));
    }
    int getRecieveState() { return recieve_state; }  // Getter for receive state
    int getSendState() { return send_state; }        // Getter for send state
    SOCKET getID() { return id; }                    // Getter for socket handle
    void createNewSocket(SOCKET id, int what_to_response);  // Initialize new socket state
    void emptySocket();                              // Reset socket state
    int recieve();                                   // Receive data from socket
    int sendMsg();                                   // Send data to client
    time_t getLastActivity() { return lastActivityTime; } // Getter for last activity time
};

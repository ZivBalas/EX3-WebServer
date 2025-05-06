#pragma once
#include "CommonHeaders.h"

class SocketState
{
    SOCKET id;         // Socket handle
    int recieve_state; // Receiving?
    int send_state;          // Sending?
    int sendSubType;	// Sending sub-type
    char buffer[2000]; // Buffer to hold received data
    int len;
    time_t lastActivityTime; 

public:
    SocketState() : id(INVALID_SOCKET), recieve_state(0), send_state(0), len(0) { memset(buffer, 0, sizeof(buffer)); }
    int getRecieveState() { return recieve_state; }
    int getSendState() { return send_state; }
    SOCKET getID() { return id; }
    void createNewSocket(SOCKET id, int what_to_response);
    void emptySocket();
    int recieve();
    int sendMsg(); //need to work on
    time_t getLastActivity() { return lastActivityTime; }
};


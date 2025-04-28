#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>

//TODO:
// 1. SPLIT TO CLASSES
// 2. CHANGE CHAT-GPT THINGS TO NORMAL
// 3. CLIENT NOT STOP AFTER 1 ASK
// 4. ADD AND DELETE COMMENTS

struct SocketState
{
    SOCKET id;         // Socket handle
    int recv;          // Receiving?
    int send;          // Sending?
    char buffer[4096]; // Buffer to hold received data
    int len;
};

const int TIME_PORT = 27015;
const int MAX_SOCKETS = 60;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;

bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);

struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;

// Adds a new socket to the socket array
bool addSocket(SOCKET id, int what)
{
    for (int i = 0; i < MAX_SOCKETS; i++)
    {
        if (sockets[i].recv == EMPTY)
        {
            sockets[i].id = id;
            sockets[i].recv = what;
            sockets[i].send = IDLE;
            sockets[i].len = 0;
            socketsCount++;
            return true;
        }
    }
    return false;
}

// Removes a socket from the socket array
void removeSocket(int index)
{
    sockets[index].recv = EMPTY;
    sockets[index].send = EMPTY;
    socketsCount--;
}

// Accepts a new incoming connection
void acceptConnection(int index)
{
    SOCKET id = sockets[index].id;
    struct sockaddr_in from;
    int fromLen = sizeof(from);

    SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
    if (INVALID_SOCKET == msgSocket)
    {
        cout << "Error at accept(): " << WSAGetLastError() << endl;
        return;
    }
    cout << "Client connected: " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << endl;

    unsigned long flag = 1;
    if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
    {
        cout << "Error at ioctlsocket(): " << WSAGetLastError() << endl;
    }

    if (!addSocket(msgSocket, RECEIVE))
    {
        cout << "Too many connections, dropped!\n";
        closesocket(msgSocket);
    }
}

// Receives a message from the client
void receiveMessage(int index)
{
    SOCKET msgSocket = sockets[index].id;
    int len = sockets[index].len;
    int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len - 1, 0);

    if (SOCKET_ERROR == bytesRecv)
    {
        cout << "Error at recv(): " << WSAGetLastError() << endl;
        closesocket(msgSocket);
        removeSocket(index);
        return;
    }
    //test delete
    if (bytesRecv == 0)
    {
        closesocket(msgSocket);
        removeSocket(index);
        return;
    }

    sockets[index].buffer[len + bytesRecv] = '\0'; // Null-terminate the received data
    cout << "Received:\n" << sockets[index].buffer << endl;

    sockets[index].len += bytesRecv;
    sockets[index].recv = IDLE;
    sockets[index].send = SEND;
}

// Sends a response to the client based on the request
void sendMessage(int index)
{
    SOCKET msgSocket = sockets[index].id;
    char response[4096] = { 0 };

    string request(sockets[index].buffer);

    if (request.find("GET") == 0)
    {
        string lang = "en"; // Default language
        size_t langPos = request.find("lang=");
        if (langPos != string::npos)
        {
            lang = request.substr(langPos + 5, 2);
        }

        string fileName = "index_" + lang + ".html";
        ifstream file(fileName);
        if (!file.is_open())
        {
            // If file not found, return 404
            string notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(msgSocket, notFound.c_str(), (int)notFound.length(), 0);
        }
        else
        {
            string html((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            string ok = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_string(html.length()) + "\r\n\r\n" + html;
            send(msgSocket, ok.c_str(), (int)ok.length(), 0);
        }
    }
    else if (request.find("POST") == 0)
    {
        size_t bodyPos = request.find("\r\n\r\n");
        if (bodyPos != string::npos)
        {
            string body = request.substr(bodyPos + 4);
            cout << "POST Body: " << body << endl;
        }

        string ok = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        send(msgSocket, ok.c_str(), (int)ok.length(), 0);
    }
    else if (request.find("OPTIONS") == 0)
    {
        string options = "HTTP/1.1 200 OK\r\nAllow: GET, POST, OPTIONS, DELETE, TRACE\r\nContent-Length: 0\r\n\r\n";
        send(msgSocket, options.c_str(), (int)options.length(), 0);
    }
    else if (request.find("DELETE") == 0)
    {
        string ok = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        send(msgSocket, ok.c_str(), (int)ok.length(), 0);
    }
    else if (request.find("TRACE") == 0)
    {
        string echo = "HTTP/1.1 200 OK\r\nContent-Type: message/http\r\nContent-Length: " + to_string(request.length()) + "\r\n\r\n" + request;
        send(msgSocket, echo.c_str(), (int)echo.length(), 0);
    }
    else
    {
        string badRequest = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        send(msgSocket, badRequest.c_str(), (int)badRequest.length(), 0);
    }

    closesocket(msgSocket);
    removeSocket(index);
}

// Main function
int main()
{
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed\n";
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = INADDR_ANY;
    serverService.sin_port = htons(TIME_PORT);

    if (bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)) == SOCKET_ERROR)
    {
        cout << "Bind failed\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, 5) == SOCKET_ERROR)
    {
        cout << "Listen failed\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    addSocket(listenSocket, LISTEN);

    cout << "HTTP Server: Listening on port " << TIME_PORT << endl;

    while (true)
    {
        fd_set waitRecv;
        FD_ZERO(&waitRecv);

        for (int i = 0; i < MAX_SOCKETS; i++)
        {
            if (sockets[i].recv == LISTEN || sockets[i].recv == RECEIVE)
                FD_SET(sockets[i].id, &waitRecv);
        }

        int nfd = select(0, &waitRecv, NULL, NULL, NULL);
        if (nfd == SOCKET_ERROR)
        {
            cout << "Select error\n";
            break;
        }

        for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
        {
            if (FD_ISSET(sockets[i].id, &waitRecv))
            {
                nfd--;
                if (sockets[i].recv == LISTEN)
                    acceptConnection(i);
                else if (sockets[i].recv == RECEIVE)
                    receiveMessage(i);
            }
        }

        for (int i = 0; i < MAX_SOCKETS; i++)
        {
            if (sockets[i].send == SEND)
                sendMessage(i);
        }
    }
    WSACleanup();
    return 0;
}

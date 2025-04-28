#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 27015

// Function to send a full HTTP request
void sendHttpRequest(const string& request)
{
    // Initialize Winsock
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed.\n";
        return;
    }

    // Create socket
    SOCKET connSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }

    // Setup server address
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_port = htons(SERVER_PORT);

    // Connect to server
    if (connect(connSocket, (SOCKADDR*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        cout << "Connection failed: " << WSAGetLastError() << endl;
        closesocket(connSocket);
        WSACleanup();
        return;
    }

    // Send request
    int bytesSent = send(connSocket, request.c_str(), (int)request.length(), 0);
    if (bytesSent == SOCKET_ERROR)
    {
        cout << "Send failed: " << WSAGetLastError() << endl;
        closesocket(connSocket);
        WSACleanup();
        return;
    }

    // Receive response
    char recvBuff[4096];
    int bytesRecv = recv(connSocket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytesRecv > 0)
    {
        recvBuff[bytesRecv] = '\0';
        cout << "Server Response:\n" << recvBuff << endl;
    }
    else
    {
        cout << "No response from server.\n";
    }

    // Clean up
    closesocket(connSocket);
    WSACleanup();
}

int main()
{
    cout << "--- HTTP Client ---\n";
    cout << "Choose request type:\n";
    cout << "1. GET (default page)\n";
    cout << "2. GET with language (he/en/fr)\n";
    cout << "3. POST (send data)\n";
    cout << "4. OPTIONS\n";
    cout << "5. DELETE\n";
    cout << "6. TRACE\n";
    cout << "0. Exit\n";

    int choice;
    cin >> choice;
    cin.ignore(); // clean newline

    string request;
    if (choice == 1)
    {
        request = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    }
    else if (choice == 2)
    {
        cout << "Enter language (he/en/fr): ";
        string lang;
        cin >> lang;
        request = "GET /?lang=" + lang + " HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    }
    else if (choice == 3)
    {
        cout << "Enter data to send: ";
        string data;
        getline(cin, data);
        request = "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: " + to_string(data.length()) + "\r\nConnection: close\r\n\r\n" + data;
    }
    else if (choice == 4)
    {
        request = "OPTIONS / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    }
    else if (choice == 5)
    {
        request = "DELETE / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    }
    else if (choice == 6)
    {
        request = "TRACE / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    }
    else
    {
        cout << "Goodbye.\n";
        return 0;
    }

    sendHttpRequest(request);

    return 0;
}

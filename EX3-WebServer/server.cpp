#include "Server.h"

bool Server::addSocket(SOCKET id, int what)
{
    for (int i = 0; i < MAX_SOCKETS; i++)
    {
        if (sockets[i].getRecieveState() == EMPTY)
        {
            sockets[i].createNewSocket(id, what);
            socketsCount++;
            return true;
        }
        return false;
    }
}

void Server::removeSocket(int index)
{
    sockets[index].emptySocket();
    socketsCount--;
}

void Server::acceptConnection(int index)
{
    SOCKET id = sockets[index].getID();
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

void Server::receiveMessage(int index)
{
    int bytesRecv = sockets[index].recieve();

    if (bytesRecv == SOCKET_ERROR || bytesRecv == 0)
    {
        if (bytesRecv == SOCKET_ERROR)
            cout << "Error at recv(): " << WSAGetLastError() << endl;

        removeSocket(index);
        return;
    }
}

int Server::initialize()
{
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
}

int Server::Run()
{
    if (initialize() == 1)
        return 1;

    while (true)
    {
        fd_set waitRecv;
        FD_ZERO(&waitRecv);

        for (int i = 0; i < MAX_SOCKETS; i++)
        {
            if (sockets[i].getRecieveState() == LISTEN || sockets[i].getRecieveState() == RECEIVE)
                FD_SET(sockets[i].getID(), &waitRecv);
        }

        int nfd = select(0, &waitRecv, NULL, NULL, NULL);
        if (nfd == SOCKET_ERROR)
        {
            cout << "Select error\n";
            break;
        }

        for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
        {
            if (FD_ISSET(sockets[i].getID(), &waitRecv))
            {
                nfd--;
                if (sockets[i].getRecieveState() == LISTEN)
                    acceptConnection(i);
                else if (sockets[i].getRecieveState() == RECEIVE)
                    receiveMessage(i);
            }
        }

        for (int i = 0; i < MAX_SOCKETS; i++)
        {
            if (sockets[i].getSendState() == SEND)
                sendMessage(i);
        }
    }
    WSACleanup();

    return 0;
}

void Server::sendMessage(int index)
{
    SOCKET msgSocket = sockets[index].getID();
    char response[4096] = { 0 };

    string request(sockets[index].getBuffer());

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



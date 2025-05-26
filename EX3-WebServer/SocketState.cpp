#include "SocketState.h"

void SocketState::createNewSocket(SOCKET id, int what_to_response)
{
    this->lastActivityTime = time(0);
    this->id = id;
    this->recieve_state = what_to_response;
    this->send_state = IDLE;
    this->len = 0;
}

void SocketState::emptySocket()
{
    this->recieve_state = EMPTY;
    this->send_state = EMPTY;
}

int SocketState::recieve()
{
    int bytesRecv = recv(id, &buffer[len], sizeof(buffer) - len, 0);

    this->lastActivityTime = time(0);

    if (bytesRecv != SOCKET_ERROR && bytesRecv != 0)
    {
        buffer[len + bytesRecv] = '\0'; 
        cout << "Web Server: Recieved: " << bytesRecv << " bytes of \"" << &buffer[len] << "\" message.\n";

        len += bytesRecv;
        
        recieve_state = IDLE;
        send_state = SEND;
    }
    else
        closesocket(id);

    return bytesRecv;
}

int SocketState::sendMsg()
{
    SOCKET msgSocket = id;
    char response[4096] = { 0 };

    string request(buffer);

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
            string html((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());//string html;   char c;   while (file.get(c)) html += c;
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
        string options = "HTTP/1.1 200 OK\r\nAllow: GET, HEAD, POST, PUT, OPTIONS, DELETE, TRACE\r\nContent-Length: 0\r\n\r\n";
        send(msgSocket, options.c_str(), (int)options.length(), 0);
    }
    else if (request.find("DELETE") == 0)
    {
        //find file name
        size_t startPos = request.find("DELETE") + 7;
        size_t endPos = request.find(" ", startPos); 
        string path = request.substr(startPos, endPos - startPos);

        if (!path.empty() && path[0] == '/')
            path = path.substr(1);

        //try to remove the requested file to delete
        int result = remove(path.c_str());

        if (result == 0)    //success
        {
            string ok = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
            send(msgSocket, ok.c_str(), (int)ok.length(), 0);
        }
        else
        {
            string notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(msgSocket, notFound.c_str(), (int)notFound.length(), 0);
        }
    }
    else if (request.find("PUT") == 0)
    {
        //extract file name
        size_t startPath = request.find(" ") + 1;
        size_t endPath = request.find(" ", startPath);
        string filePath = "." + request.substr(startPath, endPath - startPath); 

        //extract body of request
        size_t bodyPos = request.find("\r\n\r\n");
        string body = (bodyPos != string::npos) ? request.substr(bodyPos + 4) : "";

        //write to file
        ofstream out(filePath);
        if (out.is_open()) {
            out << body;
            out.close();
            string ok = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
            send(msgSocket, ok.c_str(), (int)ok.length(), 0);
        }
        else {
            string error = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
            send(msgSocket, error.c_str(), (int)error.length(), 0);
        }
    }

    else if (request.find("HEAD") == 0)
    {
        string lang = "en"; //default language
        size_t langPos = request.find("lang=");
        if (langPos != string::npos)
        {
            lang = request.substr(langPos + 5, 2);
        }

        string fileName = "index_" + lang + ".html";
        ifstream file(fileName);
        if (!file.is_open())
        {
            string notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(msgSocket, notFound.c_str(), (int)notFound.length(), 0);
        }
        else
        {
            string html((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            string headers = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_string(html.length()) + "\r\n\r\n";
            send(msgSocket, headers.c_str(), (int)headers.length(), 0);
        }
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

    this->lastActivityTime = time(0);


    closesocket(msgSocket);
    return 0;

  
}

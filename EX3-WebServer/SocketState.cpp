#include "SocketState.h"

void SocketState::createNewSocket(SOCKET id, int what_to_response)
{
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
    int bytesRecv = recv(id, &buffer[len], sizeof(buffer) - len - 1, 0);

    if (bytesRecv != SOCKET_ERROR && bytesRecv != 0)
    {
        buffer[len + bytesRecv] = '\0'; // Null-terminate the received data
        cout << "Received:\n" << buffer << endl;

        len += bytesRecv;
        recieve_state = IDLE;
        send_state = SEND;
    }
    else
        closesocket(id);

    return bytesRecv;
}

//int SocketState::sendMsg()
//{
//    int bytesSent = send(id, badRequest.c_str(), (int)badRequest.length(), 0);
//
//    if (SOCKET_ERROR == bytesSent)
//    {
//        cout << "Web Server: Error at send(): " << WSAGetLastError() << endl;
//        return;
//    }
//    cout << "Web Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";
//    return 0;
//}

#include "Server.h"

//TODO:
// 1. SPLIT TO CLASSES
// 2. CHANGE CHAT-GPT THINGS TO NORMAL
// 3. CLIENT NOT STOP AFTER 1 ASK
// 4. ADD AND DELETE COMMENTS
// 5. Work on sendMsg() in SocketState: send() error check, return value, arragne it

int main()
{
	Server server;
	return server.Run();
}
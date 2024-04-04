#include "server_socket.h"

int main()
{
	if (debug::DisplaysMSG()) {
		std::print("[Info] Started in Debug Mode\n");
	}
	server::sock.Start();
}
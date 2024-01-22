#include <iostream>
#include "server_socket.h"
#include "packet.h"

int main()
{
	server::sock.Bind();
	server::sock.Listen();

	while (true) {
		server::sock.Accept();
	}
}
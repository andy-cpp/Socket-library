#include <iostream>

#include "Socket.h"

int main()
{
	Socket socket(ConnectionDetails("127.0.0.1", 80));

	if (!socket.bind() || !socket.listen()) {
		printf("Error starting server\n");
		exit(1);
	}
	if (!socket.setsockopt(SOL_SOCKET, SO_RCVTIMEO, 1)) {
		printf("Error setting receive timeout\n");
	}
	
	if (!socket.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)) {
		printf("Error setting reuseaddr\n");
	}

	while (true)
	{
		Socket sock = socket.accept(1);

		if (sock) {
			printf("new socket: %s\n", sock.m_details.m_ip.c_str());
		}
	}
	
}
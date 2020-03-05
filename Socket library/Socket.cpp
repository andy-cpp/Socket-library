#include "Socket.h"

#include <stdexcept>

bool bInitialized = false;

bool InitWSA()
{
	WSADATA wsaData;
	return (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
}

void InitCheck()
{
	if (!bInitialized) {
		if (!InitWSA())
		{
			throw std::runtime_error("Failed to initialize winsock!");
		}
		bInitialized = true;
	}
}

int(__stdcall* ORIG_connect)(SOCKET, const sockaddr*, int) = connect;
int(__stdcall* ORIG_bind)(SOCKET, const sockaddr*, int) = bind;
int(__stdcall* ORIG_listen)(SOCKET, int backlog) = listen;
SOCKET(__stdcall* ORIG_accept)(SOCKET, sockaddr*, int* len) = accept;
int(__stdcall* ORIG_send)(SOCKET sock, char const* buf, int size, int flags) = send;
int(__stdcall* ORIG_recv)(SOCKET sock, char* buf, int size, int flags) = recv;
int(__stdcall* ORIG_sendto)(SOCKET sock, char const* buf , int buflen,int flags, const sockaddr* addr, int addrlen) = sendto;
int(__stdcall* ORIG_recvfrom)(SOCKET sock, char* buf, int buflen, int flags, sockaddr* addr, int* addrlen) = recvfrom;

int(__stdcall* ORIG_setsockopt)(SOCKET socket, int level, int optname, char const* value, int len) = setsockopt;
int(__stdcall* ORIG_getsockopt)(SOCKET socket, int level, int optname, char* buf, int* buflen) = getsockopt;


Socket::Socket(ConnectionDetails const& details, type const& Type) : m_details(details)
{
	InitCheck();

	if (details.m_type == ConnectionDetails::type::IPV4)
	{
		int iType = 0;
		if (Type == type::TCP)
			iType = SOCK_STREAM;
		else if (Type == type::UDP)
			iType = SOCK_DGRAM;

		m_socket = socket(AF_INET, iType, 0);
	}
	else if (details.m_type == ConnectionDetails::type::IPV6)
	{
		int iType = 0;
		if (Type == type::TCP)
			iType = SOCK_STREAM;
		else if (Type == type::UDP)
			iType = SOCK_DGRAM;

		m_socket = socket(AF_INET6, iType, 0);
	}
	else
	{
		throw std::runtime_error("Invalid procotol in Socket::Socket(ConnectionDetails const&, type const&)");
	}
}

bool Socket::connect()
{
	InitCheck();

	if (m_details.m_type == ConnectionDetails::type::IPV4)
	{
		return ORIG_connect(m_socket, (sockaddr*)&m_details.m_v4, sizeof(m_details.m_v4)) == 0;
	}
	else if (m_details.m_type == ConnectionDetails::type::IPV6)
	{
		return ORIG_connect(m_socket, (sockaddr*)&m_details.m_v6, sizeof(m_details.m_v6)) == 0;
	}
	else {
		return false;
	}
}

bool Socket::close()
{
	return closesocket(m_socket) == 0;
}

bool Socket::bind()
{
	return ORIG_bind(m_socket, (sockaddr*)&m_details.m_v4, sizeof(m_details.m_v4)) == 0;	
}

bool Socket::listen()
{
	return ORIG_listen(m_socket, 0) == 0;
}


Socket Socket::accept(long timeoutmsec)
{
	FD_SET readfs;
	FD_ZERO(&readfs);
	FD_SET(m_socket, &readfs);

	struct timeval timeout;
	timeout.tv_usec = (timeoutmsec * 1000) /* ms to usec */; 
	timeout.tv_sec = 0;


	struct timeval* pTimeout = (timeoutmsec > 0) ? &timeout : 0;

	if (m_details.m_type == ConnectionDetails::type::IPV4)
	{
		struct sockaddr_in addr;
		int socklen = sizeof(addr);

		if (select(m_socket + 1, &readfs, 0, 0, pTimeout) > 0) {
			/* If we are ready to accept a socket */
			int socket = ORIG_accept(m_socket, (sockaddr*)&addr, &socklen);
			if (socket != SOCKET_ERROR) {
				Socket sock(ConnectionDetails(addr.sin_addr.S_un.S_addr, addr.sin_port));
				return sock;
			}
			else
				return Socket();
		}
	}
	else if (m_details.m_type == ConnectionDetails::type::IPV6)
	{
		struct sockaddr_in6 addr;
		int socklen = sizeof(addr);
	
		if (select(m_socket + 1, &readfs, 0, 0, pTimeout) > 0)
		{
			/* if we have a client to accept */
			int socket = ORIG_accept(m_socket, (sockaddr*)&addr, &socklen);
			if (socket != SOCKET_ERROR) {
				uint64_t uAddr[2] = { 0 };
				memcpy(&uAddr, &addr.sin6_addr.u.Byte, sizeof(uint64_t) * 2);
	
				Socket sock(ConnectionDetails(uAddr, addr.sin6_port));
				return sock;
			}
		}
	}
	else
	{
			throw std::runtime_error("Invalid ip protocol in Socket::accept()");
	}

	Socket sock;
	sock.m_socket = SOCKET_ERROR;

	return sock;
}


int Socket::send(std::string const& str, int size)
{
	if (size == -1)
		size = str.size();
	int bufsize = 4096;

	unsigned long long uTimes = size / bufsize;
	long double dTimes = size / bufsize;

	if (uTimes < 1 || dTimes > uTimes)
		++uTimes;

	int bytes_sent = 0;
	for (unsigned long long index = 0; index < uTimes; ++index)
	{
		char const* buffer = str.c_str() + (index * bufsize);
		auto size = str.size() - (index * bufsize);
		if (size > bufsize)
			size = bufsize;

		int status = ORIG_send(m_socket, buffer, size, 0);

		if (status < 0)
			break;

		if (status < bufsize) {
			bytes_sent += status;
		}

	}
	return bytes_sent;
}

int Socket::recv(std::string& buf)
{
	int bufsize = 4096;

	char* buffer = (char*)malloc(bufsize + 1);
	if (buffer == 0)
		throw std::runtime_error("Nullptr exception in Socket::recv(std::string&)");
	memset(buffer, 0, bufsize + 1);

	int bytes_received = 0;
	while (true)
	{

		int status = ORIG_recv(m_socket, buffer, bufsize, 0);
		
		if (status < 0)
			break;

		if (status < bufsize) {
			bytes_received += status;
			buf.append(buffer);
			break;
		}
		bytes_received += status;
		buf.append(buffer);
	}
	free(buffer);
	return bytes_received;
}

int Socket::sendto(ConnectionDetails const& details, std::string const& data, int size)
{
	if (size == -1)
		size = data.size();
	int bufsize = 4096;

	unsigned long long uTimes = size / bufsize;
	long double dTimes = size / bufsize;

	if (uTimes < 1 || dTimes > uTimes)
		++uTimes;

	int bytes_sent = 0;
	for (unsigned long long index = 0; index < uTimes; ++index)
	{
		char const* buffer = data.c_str() + (index * bufsize);
		auto size = data.size() - (index * bufsize);
		if (size > bufsize)
			size = bufsize;

		const sockaddr* pDetails = (m_details.m_type == ConnectionDetails::type::IPV4) ? (const sockaddr*)&m_details.m_v4 : (const sockaddr*)&m_details.m_v6;
		int addrlen = (m_details.m_type == ConnectionDetails::type::IPV4) ? sizeof(m_details.m_v4) : sizeof(m_details.m_v6);

		int status = ORIG_sendto(m_socket, buffer, size, 0, pDetails, addrlen);

		if (status < 0)
			break;

		if (status < bufsize) {
			bytes_sent += status;
		}
	}
	return bytes_sent;
}

int Socket::recvfrom(std::string& buf, ConnectionDetails& details)
{
	int bufsize = 65535;

	char* buffer = (char*)malloc(bufsize + 1);
	if (buffer == 0)
		throw std::runtime_error("Nullptr exception in Socket::recv(std::string&)");
	memset(buffer, 0, bufsize + 1);


	int addrlen = (details.m_type == ConnectionDetails::type::IPV4) ? sizeof details.m_v4 : sizeof details.m_v6;
	sockaddr* pAddr = (details.m_type == ConnectionDetails::type::IPV4) ? (sockaddr*)&details.m_v4 : (sockaddr*)&details.m_v6;

	int bytes_received = 0;
	while (true)
	{
		int status = ORIG_recvfrom(m_socket, buffer, bufsize,0, pAddr, &addrlen);

		if (status < 0)
			break;

		if (status < bufsize) {
			bytes_received += status;
			buf.append(buffer);
			break;
		}
		bytes_received += status;
		buf.append(buffer);

	}
	if (details.m_type == ConnectionDetails::type::IPV4)
	{

		details = ConnectionDetails(details.m_v4.sin_addr.S_un.S_addr, details.m_v4.sin_port);
	}
	else
	{
		uint64_t addr[2] = { 0 };
		memcpy(&addr, &details.m_v6.sin6_addr.u.Byte, sizeof(uint64_t) * 2);

		details = ConnectionDetails(addr, details.m_v6.sin6_port, ConnectionDetails::type::IPV6);
	}

	free(buffer);
	return bytes_received;
}

bool Socket::has_data(long timeoutmsec) const
{
	FD_SET readfs;
	FD_ZERO(&readfs);
	FD_SET(m_socket, &readfs);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = (timeoutmsec * 1000); /* ms to usec */

	const timeval * pTimeout = (timeoutmsec > 0) ? &timeout : 0;

	return select(m_socket + 1, &readfs, 0, 0, pTimeout) > 0;
}

bool Socket::invalid() const
{
	return m_socket == SOCKET_ERROR;
}

bool Socket::operator!() const
{
	return this->invalid();
}

Socket::operator bool() const
{
	return !this->invalid();
}

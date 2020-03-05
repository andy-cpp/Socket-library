#pragma once

#include <string>

#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class ConnectionDetails
{
public:
	enum class type {
		IPV4, IPV6
	};

	ConnectionDetails() {

	}

	ConnectionDetails(const ConnectionDetails& other);

	ConnectionDetails& operator=(const ConnectionDetails& other);

	ConnectionDetails(unsigned long uip, unsigned short uPort, type const& Type = type::IPV4);

	ConnectionDetails(uint64_t addr[2], unsigned short uPort, type const& Type = type::IPV6);

	ConnectionDetails(std::string const& ip, unsigned short uPort, type const& Type = type::IPV4);

	bool operator!()
	{
		return (m_port == 0 || m_ip.empty());
	}

	operator bool()
	{
		return (m_port != 0 && !m_ip.empty());
	}
public:
	std::string m_ip;
	unsigned short m_port;
	type m_type;

	struct sockaddr_in6 m_v6;
	struct sockaddr_in m_v4;
};


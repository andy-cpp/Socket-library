#include "ConnectionDetails.h"

#include <exception>
#include <stdexcept>

ConnectionDetails::ConnectionDetails(const ConnectionDetails& other)
{
	memcpy(this, &other, sizeof other);
}

ConnectionDetails& ConnectionDetails::operator=(const ConnectionDetails& other)
{
	m_type = other.m_type;
	m_ip = other.m_ip;
	m_port = other.m_port;
	m_v4 = other.m_v4;
	m_v6 = other.m_v6;

	return *this;
}

ConnectionDetails::ConnectionDetails(unsigned long uip, unsigned short uPort, type const& Type)
{
	m_type = Type;

	char buf[32] = { 0 };
	if (inet_ntop(AF_INET, &uip, (char*)&buf, 32) != (char*)&buf) {
		throw std::runtime_error("Error converting ip address to ipv6 in ConnectionDetails::ConnectionDetails(unsigned long, unsigned short, type const&)");
	}

	m_ip = buf;
	m_v4.sin_family = AF_INET;
	m_v4.sin_port = uPort;
	m_v4.sin_addr.S_un.S_addr = uip;
}

ConnectionDetails::ConnectionDetails(uint64_t ipv6addr[2], unsigned short uPort, type const& Type)
{
	m_type = Type;

	char buf[64] = { 0 };
	if (inet_ntop(AF_INET6, &ipv6addr, (char*)&buf, 64) != (char*)&buf) {
		throw std::runtime_error("Error converting ip address to ipv6 in ConnectionDetails::ConnectionDetails(uint64_t[2], unsigned short, type const&)");
	}
	m_ip = buf;
	m_v6.sin6_family = AF_INET6;
	m_v6.sin6_port = uPort;
	
	memcpy(&m_v6.sin6_addr.u.Byte, ipv6addr, sizeof(uint64_t) * 2);
}

ConnectionDetails::ConnectionDetails(std::string const& ip, unsigned short uPort, type const& Type) : m_type(Type), m_ip(ip)
{
	m_type = Type;

	if (Type == type::IPV4)
	{
		m_v4.sin_family = AF_INET;
		m_v4.sin_port = htons(uPort);

		if (inet_pton(AF_INET, ip.c_str(), &m_v4.sin_addr.S_un.S_addr) != 1) {
			throw std::runtime_error("Error converting ip address ConnectionDetails::ConnectionDetails(std::string const&, unsigned short, type const&)");
		}
	}
	else if (Type == type::IPV6)
	{
		m_v6.sin6_family = AF_INET6;
		m_v6.sin6_port = uPort;

		if (inet_pton(AF_INET6, ip.c_str(), &m_v6.sin6_addr.u.Byte) != 1) {
			throw std::runtime_error("Error converting ip address ConnectionDetails::ConnectionDetails(std::string const&, unsigned short, type const&)");
		}
	}
	else
	{
		throw std::runtime_error("Invalid ip protocol version ConnectionDetails::ConnectionDetails(std::string const&, unsigned short, type const&)");
	}


}

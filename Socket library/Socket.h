#pragma once

#include "ConnectionDetails.h"

bool InitWSA();


/* Function pointer due to namespace collisions */
extern int(__stdcall* ORIG_setsockopt)(SOCKET socket, int level, int optname, char const* value, int len);
extern int(__stdcall* ORIG_getsockopt)(SOCKET socket, int level, int optname, char* buf, int* buflen);


class Socket
{
public:
	/* Protocol type enum */
	enum class type{
		TCP = SOCK_STREAM, UDP = SOCK_DGRAM
	};

	/* Constructor */
	Socket() {
		memset(this, 0, sizeof *this);
	}

	/* Constructor */
	Socket(ConnectionDetails const& details, type const& Type = type::TCP);

	/* connect function */
	bool connect();

	/* close function */
	bool close();

	/* bind function */
	bool bind();

	/* listen function */
	bool listen();

	/* accept function */
	Socket accept(int iTimeout = 0);

	/* send function */
	int send(std::string const& str, unsigned int size = -1);

	/* recv function */
	int recv(std::string& buffer);

	/* setsockopt, returns true on success */
	template <typename T>
	bool setsockopt(int level, int optname, T const& value)
	{
		return ORIG_setsockopt(m_socket, level, optname, (char const*)&value, sizeof value) == 0;
	}

	
	/* getsockopt returns true on success */
	template <typename T>
	bool getsockopt(int level, int optname, T* pValue)
	{
		int buflen = sizeof T;

		return ORIG_getsockopt(m_socket, level, optname, pValue, buflen) == 0;
	}

	/* returns true if socket is invalid */
	bool invalid() const;

	/* boolean operator */
	bool operator!() const;

	/* boolean operator */
	operator bool() const;
public:
	int m_socket = SOCKET_ERROR;
	ConnectionDetails m_details;
};
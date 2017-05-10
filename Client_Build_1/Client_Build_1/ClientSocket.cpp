#include "stdafx.h"
#include "ClientSocket.h"


CClientSocket::CClientSocket()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we couldn't find a useable */
		/* winsock.dll. */
		return;
	}

}


CClientSocket::~CClientSocket()
{
	WSACleanup();
}

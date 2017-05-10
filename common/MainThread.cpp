#include "stdafx.h"
#include "MainThread.h"
#include "GateListen.h"

static CMainthread* m_Instance;
CMainthread::CMainthread()
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


CMainthread::~CMainthread()
{
	WSACleanup();
}

void CMainthread::FromClientMsg(WORD wHandle, WORD wCmd, char* buf, int Buflen)
{

}

void CMainthread::FromDBMsg(WORD wHandle, WORD wCmd, char * buf, int Buflen)
{

}

CMainthread* CMainthread::getInstance()
{
	if (!m_Instance)
	{
		m_Instance = new CMainthread();

	}
	return m_Instance;
}

void CMainthread::Execute()
{
	CGateSocketServer::getInstance();

	while (!Terminated())
	{
		if (CGateSocketServer::getInstance()->isActiveed())
		{
			DispenseGateMessage();

		};
	}
	CGateSocketServer* gate = CGateSocketServer::getInstance();
	delete gate;

}

void CMainthread::ClearAllMsg()
{

}

void CMainthread::DispenseGateMessage()
{

}

void CMainthread::DispenseDBMessage()
{

}

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
	//这种写法主要是为了避免编译器的error C2712: 无法在要求对象展开的函数中使用 __try这个错误
	Release();
}

void CMainthread::FromClientMsg(WORD wHandle, WORD wCmd, char* buf, int Buflen)
{
	pThreadMsgNode nNode = new ThreadMsgNode();
	nNode->ImCmd = wCmd;
	nNode->ImHandle = wHandle;
	nNode->ImBufLen = Buflen;
	memset(nNode->ImBuf, 0, Buflen);
	memmove_s(nNode->ImBuf,nNode->ImBufLen, buf, Buflen);
	EnterCriticalSection(&m_GateCS);
	__try
	{
		//这里说明队列里是否为空
		if (!m_BotCMNode)
			m_TopCMNode = nNode;
		m_BotCMNode = nNode;
	}
	__finally
	{
		LeaveCriticalSection(&m_GateCS);

	}
}

void CMainthread::FromDBMsg(WORD wHandle, WORD wCmd, char * buf, int Buflen)
{

	pThreadMsgNode nNode = new ThreadMsgNode();
	nNode->ImCmd = wCmd;
	nNode->ImHandle = wHandle;
	nNode->ImBufLen = Buflen;
	memset(nNode->ImBuf, 0, Buflen);
	memmove_s(nNode->ImBuf, nNode->ImBufLen, buf, Buflen);
	EnterCriticalSection(&m_DBCS);
	__try
	{
		//这里说明队列里是否为空
		if (!m_BotDBNode)
			m_TopDBNode = nNode;
		m_BotDBNode = nNode;
	}
	__finally
	{
		LeaveCriticalSection(&m_DBCS);

	}

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
	gate->close();

}

void CMainthread::ClearAllMsg()
{
	EnterCriticalSection(&m_GateCS);
	  __try
	  {
		  while (m_TopCMNode != nullptr)
		  {
			  pThreadMsgNode nNode = NULL;
		      nNode = m_TopCMNode;
			  m_TopCMNode = nNode->Next;
			  delete(m_TopCMNode);
			  
		  }
		  m_BotCMNode = nullptr;
	  }
	  __finally 
	  {
		  LeaveCriticalSection(&m_GateCS);
	  }



	  EnterCriticalSection(&m_DBCS);
	  __try
	  {
		  while (m_TopDBNode != nullptr)
		  {
			  pThreadMsgNode nNode = NULL;
			  nNode = m_TopDBNode;
			  m_TopDBNode = nNode->Next;
			  delete(m_TopDBNode);

		  }
		  m_BotDBNode = nullptr;
	  }
	  __finally
	  {
		  LeaveCriticalSection(&m_DBCS);
	  }
	  
	
}

void CMainthread::DispenseGateMessage()
{
	pThreadMsgNode nNode = nullptr;
	pThreadMsgNode nNext = nullptr;
	EnterCriticalSection(&m_GateCS);
	__try
	{
		nNode = m_TopCMNode;
		m_TopCMNode = nullptr;
		m_BotDBNode = nullptr;

	}
	__finally
	{
		LeaveCriticalSection(&m_GateCS);
	}

	while (nNode)
	{
		nNext = nNode->Next;
		switch (nNode->ImCmd)
		{
		case IM_SELECT_SERVER_REQ:

			break;
		
			
		} 
	}
}

void CMainthread::DispenseDBMessage()
{

}

void CMainthread::Release()
{
	if (!Terminated())
	{
		Terminate();
		SetEvent(&m_Event);
		WaitFor();
	}
	CloseHandle(&m_Event);
	__try
	{
		ClearAllMsg();
		//释放pig
		//释放DB
		//释放Gate
		CGateSocketServer* gs = CGateSocketServer::getInstance();
		if (gs)
			delete gs;
	}
	__finally
	{
		DeleteCriticalSection(&m_GateCS);
		DeleteCriticalSection(&m_DBCS);

	}
	WSACleanup();
}

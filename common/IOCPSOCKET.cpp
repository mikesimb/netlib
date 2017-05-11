#include "stdafx.h"
#include "IOCPSOCKET.h"

/************************************************************************/
/* CZQ_CustomIocpClientSocket定义                                       */
/************************************************************************/

CZQ_CustomIocpClientSocket::CZQ_CustomIocpClientSocket()
{
	InitializeCriticalSection(&_sendBufferCS);
}

CZQ_CustomIocpClientSocket::~CZQ_CustomIocpClientSocket()
{
	DeleteCriticalSection(&_sendBufferCS);
}

void CZQ_CustomIocpClientSocket::close()
{
	if (forceClose() != 0)
	{
		OnErrorEvent(WSAGetLastError());
	}
}

int CZQ_CustomIocpClientSocket::sendBuf(char* buf, int count, bool freeAfterSend /*= false*/)
{
	int result = 0;
	if (count <= 0 || _socket == INVALID_SOCKET)
	{
		if (freeAfterSend)
			free(buf);

		return result;
	}
	result = count;

	pSendQueueNode pNode = new SendQueueNode;

	pNode->iBufferLen = count;
	pNode->szBuf = (char*)malloc(count);
	memcpy(pNode->szBuf, buf, count);
	if (freeAfterSend)
	{
		free(buf);
	}
	pNode->iStartPosition = 0;
	pNode->next = nullptr;
	EnterCriticalSection(&_sendBufferCS);
	do {
		_totalSendDataCount += count;
		_totalBufferlen += count;

		if (_lastQueueNode != nullptr)
		{
			_lastQueueNode->next = pNode;

		}

		if (_firstQueueNode != nullptr)
		{
			_firstQueueNode->next = nullptr;

		}
		_lastQueueNode = pNode;

		if (!_isSending)
		{
			prepareSend(&_sendBlock, 0);
		}

	} while (0);
	LeaveCriticalSection(&_sendBufferCS);
	return result;
}

int CZQ_CustomIocpClientSocket::sendText(std::string s)
{
	int strlen = s.length();
	if (strlen > 0)
	{
		char* buf = (char*)malloc(strlen);
		memcpy(buf, s.c_str(), strlen);
		return sendBuf(buf, strlen, true);
	}
	else
		return 0;
}

std::string  CZQ_CustomIocpClientSocket::getRemoteAddress()
{
	return _remoteAddress;
}

int CZQ_CustomIocpClientSocket::getRemotePort()
{
	return _remotePort;
}

char* CZQ_CustomIocpClientSocket::getData()
{
	return _data;

}

int CZQ_CustomIocpClientSocket::getSendBufferLen()
{
	return _totalBufferlen;
}

uint32_t CZQ_CustomIocpClientSocket::getTotalSendCount()
{
	return _totalSendDataCount;
}

void CZQ_CustomIocpClientSocket::setTotalSendCount(uint32_t value)
{
	_totalSendDataCount = value;
}

int CZQ_CustomIocpClientSocket::forceClose()
{
	int  result = 0;
	if (_socket != INVALID_SOCKET)
	{
		do {
			result = closesocket(_socket);

		} while (0);
		_socket = INVALID_SOCKET;

		clearSendBuffer();
		memset(&_readBlock, 0, sizeof(Block));
		PostQueuedCompletionStatus(_hIOCP, 0, (DWORD)(this), (LPOVERLAPPED)DISCONNECT_FLAG);

	}
	return result;
}

void CZQ_CustomIocpClientSocket::setActiveTicket(uint64_t value)
{
	_uactiveTick = value;
}

uint64_t CZQ_CustomIocpClientSocket::getActiveTick()
{
	return _uactiveTick;
}

uint32_t CZQ_CustomIocpClientSocket::getSocketHandle()
{
	return _socketHandle;
}

void CZQ_CustomIocpClientSocket::update()
{

}

void CZQ_CustomIocpClientSocket::OnErrorEvent(int ErrorCode)
{
	if (ErrorCode != ERROR_IO_PENDING && _onSocketErrorEvent != nullptr)
	{
		_onSocketErrorEvent(this, ErrorCode);

	}

}

void CZQ_CustomIocpClientSocket::initialize()
{
	_isSending = false;
	_data = nullptr;
	_currentTick = 0;
	_totalBufferlen = 0;
	_totalSendDataCount = 0;

}

void CZQ_CustomIocpClientSocket::finalize()
{
	_isSending = false;
	_data = nullptr;
}

void CZQ_CustomIocpClientSocket::socketRead(void *buf, int count)
{
	if (_onReviceEvent != nullptr)
	{
		_onReviceEvent(this, buf, count);
	}
}

void CZQ_CustomIocpClientSocket::doClientRead(pBlock pRBlock, int transfered)
{
	if (transfered > 0)
	{
		do {
			pRBlock->Buffer[transfered] = 0;
			socketRead(pRBlock->Buffer, transfered);
		} while (0);
		if (_socket != INVALID_SOCKET)
		{
			prepareRecv(pRBlock);
		}
	}
}

void CZQ_CustomIocpClientSocket::doClientSend(pBlock pSBlock, int transfered)
{
	int sendLen = pSBlock->wsaBuffer.len - transfered;
	if (sendLen > 0)
	{
		memcpy(pSBlock->Buffer, &(pSBlock->Buffer[transfered]), sendLen);

	}
	prepareSend(pSBlock, sendLen);

}

void CZQ_CustomIocpClientSocket::prepareRecv(pBlock pRBlock)
{
	try
	{
		pRBlock->event = ZQ_seRead;
		pRBlock->wsaBuffer.len = MAX_IOCP_BUF_SIZE;
		pRBlock->wsaBuffer.buf = pRBlock->Buffer;
		memset(&(pRBlock->overlapped), 0, sizeof(OVERLAPPED));
		DWORD transfer = 0, flag = 0;
		if (_socket == INVALID_SOCKET || WSARecv(_socket, &(pRBlock->wsaBuffer), 1, &transfer, &flag, &(pRBlock->overlapped), nullptr))
		{
			int errorcode = WSAGetLastError();
			if (errorcode != ERROR_IO_PENDING)
			{
				OnErrorEvent(errorcode);
				close();
			}
		}
	}
	catch (...)
	{
		OnErrorEvent(GetLastError());
	}
}

void CZQ_CustomIocpClientSocket::prepareSend(pBlock pSBlock, int iSendlen)
{
	EnterCriticalSection(&_sendBufferCS);
	do {
		_isSending = false;
		pSendQueueNode pNode = nullptr;
		int iRemainlen = 0;//这个是什么意思?
		int iDatalen = 0;//这个是什么意思？
		int iErrCode = 0;
		while (_firstQueueNode != nullptr)
		{
			pNode = _firstQueueNode;
			iRemainlen = MAX_IOCP_BUF_SIZE - iSendlen;
			iDatalen = pNode->iBufferLen - pNode->iStartPosition;
			if (iDatalen > iRemainlen)
			{
				memcpy(&(pSBlock->Buffer[iSendlen]), &(pNode->szBuf[pNode->iStartPosition]), iRemainlen);
				iSendlen = MAX_IOCP_BUF_SIZE;
				pNode->iStartPosition += iRemainlen;
				break;
			}
			else
			{
				memcpy(&(pSBlock->Buffer[iSendlen]), &(pNode->szBuf[pNode->iStartPosition]), iRemainlen);
				iSendlen += iDatalen;
				_firstQueueNode = _firstQueueNode->next;
				if (_firstQueueNode == nullptr)
				{
					_lastQueueNode = nullptr;

				}

				free(pNode->szBuf);
				delete(pNode);
				pNode = nullptr;
			}//if
		}//while
		_totalBufferlen -= iSendlen;
		if (iSendlen > 0)
		{
			_isSending = false;
			pSBlock->event = ZQ_seWrite;
			pSBlock->wsaBuffer.len = iSendlen;
			pSBlock->wsaBuffer.buf = pSBlock->Buffer;
			memset(&(pSBlock->Buffer), 0, sizeof(OVERLAPPED));

			DWORD transfer;
			if (_socket == INVALID_SOCKET ||
				WSASend(_socket, &(pSBlock->wsaBuffer), 1, &transfer, 0, &(pSBlock->overlapped), nullptr) == SOCKET_ERROR)
			{
				iErrCode = WSAGetLastError();
				if (iErrCode != ERROR_IO_PENDING)
				{
					OnErrorEvent(iErrCode);
					close();
				}//if
			}//if
		}
	} while (0);
	LeaveCriticalSection(&_sendBufferCS);
}

void CZQ_CustomIocpClientSocket::clearSendBuffer()
{
	EnterCriticalSection(&_sendBufferCS);
	do {
		pSendQueueNode pNode = nullptr;
		while (_firstQueueNode != nullptr)
		{
			pNode = _firstQueueNode;
			_firstQueueNode = _firstQueueNode->next;
			free(pNode->szBuf);
			delete(pNode);
			pNode = nullptr;
		}
	} while (0);
	LeaveCriticalSection(&_sendBufferCS);
}

/************************************************************************/
/*    CZQ_CustomIOCPSocket实现                                               */
/************************************************************************/



CZQ_CustomIOCPSocket::CZQ_CustomIOCPSocket()
	:_hIOCP(0)
	, OnSocketError(nullptr)
	, OnConnect(nullptr)
	, OnDisConnect(nullptr)
{
	_IOCPSOCKET.socket = INVALID_SOCKET;
}


CZQ_CustomIOCPSocket::~CZQ_CustomIOCPSocket()
{
}

HANDLE CZQ_CustomIOCPSocket::getIOCPHandle()
{
	return _hIOCP;
}

IOCPSOCKET * CZQ_CustomIOCPSocket::getIOCPSOCKET()
{
	return &_IOCPSOCKET;
}

void CZQ_CustomIOCPSocket::setServerIP(std::string IPAddress)
{
	if (_IOCPSOCKET.socket == INVALID_SOCKET && (IPAddress.length() <= 16))
	{
		memset(_IOCPSOCKET.IP, 0, 16);
		memcpy(_IOCPSOCKET.IP, (char*)IPAddress.c_str(), IPAddress.length());

	}
}

int CZQ_CustomIOCPSocket::getServerPort()
{
	return _IOCPSOCKET.port;
}

void CZQ_CustomIOCPSocket::setServerPort(const int Port)
{
	if (_IOCPSOCKET.socket == INVALID_SOCKET)
		_IOCPSOCKET.port = Port;

}

bool CZQ_CustomIOCPSocket::isActiveed()
{
	return _IOCPSOCKET.socket != INVALID_SOCKET;

}

void CZQ_CustomIOCPSocket::open()
{
	setActive(true);
}

void CZQ_CustomIOCPSocket::close()
{
	setActive(false);
}

void CZQ_CustomIOCPSocket::socketErrorEvent(CZQ_CustomIocpClientSocket* clientsocket, int errorCode)
{
	if (OnSocketError != nullptr)
	{
		OnSocketError(clientsocket, errorCode);
	}
	if (errorCode != 0)
	{
		//OutputDebugString("SocketError Code = %d",ErrorCode);
		//这里可以打印错误信息
	}
}




/************************************************************************/
/*CZQ_AcceptThread                                                            */
/************************************************************************/
CZQ_AcceptThread::CZQ_AcceptThread(CZQ_CustomIocpServerSocket * parentSocket)
{
	_parentSocket = parentSocket;
	Initialize();

}

CZQ_AcceptThread::~CZQ_AcceptThread()
{
	do {
		if (!Terminated())
			Terminate();
		if (_parentSocket != nullptr)
		{
			if (_parentSocket->getIOCPSOCKET()->socket != INVALID_SOCKET)
			{
				do {
					closesocket(_parentSocket->getIOCPSOCKET()->socket);

				} while (0);
				_parentSocket->getIOCPSOCKET()->socket = INVALID_SOCKET;

			}
		}

	} while (0);
	_parentSocket = nullptr;

}

void CZQ_AcceptThread::Execute()
{
	if (_parentSocket == nullptr || _parentSocket->getIOCPSOCKET() == nullptr) return;
	if (listen(_parentSocket->getIOCPSOCKET()->socket, SOMAXCONN) != 0)
	{
		OutputDebugString("Window Socket Error:Listen");
		return;
	}
	int nport = 0;
	int addrlen = sizeof(SOCKADDR_IN);
	SOCKADDR_IN toAddr;
	SOCKET s = 0;

	char ipaddress[16];

	while (!Terminated())
	{
		try {
			if (_parentSocket->getIOCPSOCKET()->socket == INVALID_SOCKET) break;
			addrlen = sizeof(SOCKADDR_IN);
			memset(&toAddr, 0, addrlen);

			s = WSAAccept(_parentSocket->getIOCPSOCKET()->socket,
				(SOCKADDR*)&toAddr,
				(LPINT)&addrlen,
				//(LPCONDITIONPROC)&Condition,
				NULL,
				(DWORD_PTR)(_parentSocket));
			if (Terminated()) break;
			if (s != INVALID_SOCKET)
			{


				strcpy_s(ipaddress, inet_ntoa(toAddr.sin_addr));
				nport = ntohs(toAddr.sin_port);
				_parentSocket->socketAccept(s, ipaddress, nport);
			}
			else
			{
				_parentSocket->socketErrorEvent(nullptr, GetLastError());

			}
		}
		catch (...) {
			OutputDebugString("CZQ_AcceptThread.cpp Line 73 is Execption.");
		}
	}
}

int __stdcall Condition(IN LPWSABUF lpCallerId, IN LPWSABUF lpCallerData, IN LPQOS lpSQOS, IN LPQOS lpGQOS, IN LPWSABUF lpCalleeId, IN LPWSABUF lpCalleeData, OUT GROUP FAR * g, IN DWORD_PTR dwCallbackData)
{
	char ipAddress[16];
	auto p = (PSOCKADDR_IN)lpCallerId->buf;
	strcpy_s(ipAddress, inet_ntoa(p->sin_addr));
	int nPort = ntohs(p->sin_port);
	int nIP = inet_addr(ipAddress);
	if (((CZQ_CustomIocpServerSocket*)dwCallbackData)->isValidAddress(ipAddress, nIP, nPort))
	{
		return CF_ACCEPT;
	}
	else
		return CF_REJECT;
}

/************************************************************************/
/*   CZQ_WorkThread                                                         */
/************************************************************************/

CZQ_WorkThread::CZQ_WorkThread(CZQ_CustomIOCPSocket* parentSocket, bool boMaster /*= false*/)
{
	_parentSocket = parentSocket;
	_boMaster = boMaster;
	Initialize();


}

CZQ_WorkThread::~CZQ_WorkThread()
{
	while (!Terminated())
	{
		Terminate();
		WaitFor();
	}
}

void CZQ_WorkThread::Execute()
{
	if (_boMaster && (!_parentSocket->activestart()))
	{
		Sleep(200);
		_parentSocket->activestop();
		return;

	}
	pBlock pblock = nullptr;
	CZQ_CustomIocpClientSocket* clientsocket = nullptr;
	bool ret = false;
	DWORD dwBytesXfered = 0;
	while (!Terminated())
	{
		try
		{
			pblock = nullptr;
			clientsocket = nullptr;
			if (!_parentSocket->isActiveed()) break;
			ret = GetQueuedCompletionStatus(_parentSocket->getIOCPHandle(), &dwBytesXfered,
				(PULONG_PTR)&clientsocket, (LPOVERLAPPED*)(&pblock), INFINITE);
			if ((uint32_t)pblock == SHUTDOWN_FLAG)
			{
				Terminate();
			}
			if (Terminated())
			{
				if (clientsocket != nullptr)
					clientsocket->forceClose();
				break;
			}

			if ((uint32_t)pblock == DISCONNECT_FLAG  && clientsocket != nullptr)
			{
				_parentSocket->freeclient(clientsocket);
				continue;
			}//if

			if (!ret || dwBytesXfered == 0)
			{
				if (clientsocket)
					clientsocket->forceClose();
				continue;
			}//if

			if (clientsocket && pblock)
			{
				clientsocket->setActiveTicket(0);
				switch (pblock->event)
				{
				case ZQ_seWrite:
					clientsocket->doClientSend(pblock, dwBytesXfered);
					break;
				case ZQ_seRead:
					clientsocket->doClientRead(pblock, dwBytesXfered);
					break;
				default:
					_parentSocket->socketErrorEvent(clientsocket, 9999);
				}//switch
			}//if
		}//try
		catch (...)
		{
			_parentSocket->socketErrorEvent(clientsocket, GetLastError());
			continue;
		}//catch
	}//while

	PostQueuedCompletionStatus(_parentSocket->getIOCPHandle(), 0, 0, (LPOVERLAPPED)SHUTDOWN_FLAG);
	if (_boMaster)
		_parentSocket->activestop();

}
/************************************************************************/
/* CZQ_CustomClientSocketPool                                           */
/************************************************************************/


CZQ_CustomeClientSocketPool::CZQ_CustomeClientSocketPool()
	:_count(0)
	, _firstQueueNode(nullptr)
	, _lastQueueNode(nullptr)
{
}


CZQ_CustomeClientSocketPool::~CZQ_CustomeClientSocketPool()
{
	clear(nullptr, 0);
}

void CZQ_CustomeClientSocketPool::append(CZQ_CustomIocpClientSocket* customClientSocket)
{
	pCustomClientListSocketNode pNode = new CustomClientListSocketNode;
	pNode->clientSocket = customClientSocket;
	pNode->next = nullptr;

	if (_lastQueueNode != nullptr)
	{
		_lastQueueNode->next = pNode;
	}
	else
	{
		if (_firstQueueNode == nullptr)
		{
			_firstQueueNode = pNode;

		}
	}
	_lastQueueNode = pNode;
	_count++;

}

CZQ_CustomIocpClientSocket * CZQ_CustomeClientSocketPool::deQueue()
{
	CZQ_CustomIocpClientSocket * result = nullptr;
	if (_firstQueueNode != nullptr)
	{
		pCustomClientListSocketNode pNode = _firstQueueNode;
		result = pNode->clientSocket;
		_firstQueueNode = pNode->next;
		delete pNode;
		pNode = nullptr;
		_count++;
		if (_firstQueueNode == nullptr)
		{
			_lastQueueNode = nullptr;

		}
	}
	return result;
}

void CZQ_CustomeClientSocketPool::clear(DequeueEvent dequeueEvent, uint64_t param)
{
	pCustomClientListSocketNode pNode = nullptr;
	while (_firstQueueNode != nullptr)
	{
		if ((dequeueEvent == nullptr) || (dequeueEvent(_firstQueueNode->clientSocket, param)))
		{
			_count--;
			pNode = _firstQueueNode;
			_firstQueueNode = pNode->next;
			delete pNode;
			pNode = nullptr;
		}
		else
			break;

	}
	if (_firstQueueNode == nullptr)
	{
		_lastQueueNode = nullptr;
	}
}

int CZQ_CustomeClientSocketPool::getCount()
{
	return _count;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

CZQ_CustomIocpServerSocket::CZQ_CustomIocpServerSocket()
	:_clientCount(0)
	, _hashClientCount(0)
	, _freeClientPool(nullptr)
	, _timeOutCheckTick(0)
	, _acceptThread(nullptr)
	, _currentHandle(1000)
	, OnSocketRevice(nullptr)
	, OnCheckIPAddress(nullptr)
	, OnCreateSocketEvent(nullptr)
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	int processorsCount = sysinfo.dwNumberOfProcessors;
	if (processorsCount >= MAX_WORK_THREAD_SIZE)
	{
		processorsCount = MAX_WORK_THREAD_SIZE - 1;
	}
	_workThreads.resize(processorsCount);

	for (auto iter = _workThreads.begin(); iter != _workThreads.end(); ++iter)
	{
		auto t = *iter;
		t = nullptr;
	}

	_freeClientPool = new CZQ_CustomeClientSocketPool();

	InitializeCriticalSection(&_client_cs);
	_timeOutCheckTick = GetTickCount64();

	for (int i = 0; i < HASH_IPITEM_BUCKETS; i++)
	{
		_HandleBuckets[i] = nullptr;
	}
}


CZQ_CustomIocpServerSocket::~CZQ_CustomIocpServerSocket()
{
	close();
	_freeClientPool->clear(std::bind(&CZQ_CustomIocpServerSocket::disposeAllClient,
		this,
		std::placeholders::_1,
		std::placeholders::_2), MAX_FREE_TIMEOUT);
	delete _freeClientPool;
	_freeClientPool = nullptr;

	DeleteCriticalSection(&_client_cs);
	portItemClear();

	_activeConnectList.clear();

}

void CZQ_CustomIocpServerSocket::touch(ClientSocketTouchProc touchProc, uint32_t param)
{

	EnterCriticalSection(&_client_cs);
	do {
		for (int i = _activeConnectList.size() - 1; i >= 0; i--)
		{
			do {
				auto iter = _activeConnectList.at(i);
				if (touchProc != nullptr)
				{
					touchProc(iter, param);
				}
			} while (0);
		}
	} while (0);
	LeaveCriticalSection(&_client_cs);
}

void CZQ_CustomIocpServerSocket::close()
{
	setActive(false);
	EnterCriticalSection(&_client_cs);
	do {
		for (int i = _activeConnectList.size() - 1; i >= 0; i--)
		{
			auto clientsocket = _activeConnectList.at(i);
			freeclient(clientsocket);
			clientsocket = nullptr;
		}
		_freeClientPool->clear(std::bind(&CZQ_CustomIocpServerSocket::disposeAllClient, this, std::placeholders::_1, std::placeholders::_2), MAX_FREE_TIMEOUT);

	} while (0);
	LeaveCriticalSection(&_client_cs);
}

std::vector<CZQ_CustomIocpClientSocket*>* CZQ_CustomIocpServerSocket::getActiveConnects()
{
	return &_activeConnectList;
}

int CZQ_CustomIocpServerSocket::getCount()
{
	return _clientCount;
}

int CZQ_CustomIocpServerSocket::hashCount()
{
	return _hashClientCount;
}

int CZQ_CustomIocpServerSocket::poolCount()
{
	return _freeClientPool->getCount();
}

bool CZQ_CustomIocpServerSocket::isValidAddress(std::string ip, int nIP, int iport)
{
	bool result = true;
	if (_clientCount > MAX_CLIENT_COUNT)
	{
		result = false;
	}
	else if (OnCheckIPAddress && !OnCheckIPAddress(ip))
	{
		result = false;
	}
	return result;

}

void CZQ_CustomIocpServerSocket::socketAccept(SOCKET socket, char* IP, int port)
{
	EnterCriticalSection(&_client_cs);
	do {
		freeCorpseClient();
		CZQ_CustomIocpClientSocket* clientsocket = _freeClientPool->deQueue();
		if (clientsocket == nullptr)
		{
			if (OnCreateSocketEvent != nullptr)
				clientsocket = OnCreateSocketEvent();
			else
				clientsocket = new CZQ_CustomIocpClientSocket();
		}
		if (clientsocket == nullptr) return;
		clientsocket->_socketHandle = _currentHandle;
		clientsocket->_remoteAddress = IP;
		clientsocket->_remotePort = port;
		clientsocket->_socket = socket;
		clientsocket->_hIOCP = _hIOCP;
		clientsocket->_onSocketErrorEvent = std::bind(&CZQ_CustomIOCPSocket::socketErrorEvent, this, std::placeholders::_1, std::placeholders::_2);
		clientsocket->initialize();
		_activeConnectList.push_back(clientsocket);
		_clientCount++;

		portItemAdd(_currentHandle, clientsocket);

		if (clientsocket->_socket != INVALID_SOCKET)
		{
			if (CreateIoCompletionPort((HANDLE)socket, _hIOCP, (ULONG_PTR)clientsocket, 0) == 0)
			{
				socketErrorEvent(clientsocket, GetLastError());
				clientsocket->forceClose();
			}
			else
			{
				if (OnConnect)
					OnConnect(clientsocket);
				if (clientsocket->_socket != INVALID_SOCKET)
					clientsocket->prepareRecv(&clientsocket->_readBlock);

			}
		}
		pHashPortItem p = nullptr;
		do
		{
			_currentHandle++;
			if (_currentHandle < 1000)
				_currentHandle = 1000;
			p = *(portItemFind(_currentHandle));

		} while (p != nullptr);
	} while (0);
	LeaveCriticalSection(&_client_cs);
}

void CZQ_CustomIocpServerSocket::lock()
{
	EnterCriticalSection(&_client_cs);
}

void CZQ_CustomIocpServerSocket::unlock()
{
	LeaveCriticalSection(&_client_cs);
}

CZQ_CustomIocpClientSocket * CZQ_CustomIocpServerSocket::valueof(const uint32_t key)
{
	CZQ_CustomIocpClientSocket  * result = nullptr;
	pHashPortItem item = *(portItemFind(key));
	if (item != nullptr)
	{
		result = item->CustomSocket;
	}
	return result;
}

pHashPortItem * CZQ_CustomIocpServerSocket::portItemFind(uint32_t key)
{
	int hash = key % HASH_IPITEM_BUCKETS;
	pHashPortItem* result = &(_HandleBuckets[hash]);
	while ((*result) != nullptr)
	{
		if ((*result)->SocketHandle == key)
		{
			break;
		}
		else
			result = &((*result)->next);
	}
	return result;
}

void CZQ_CustomIocpServerSocket::portItemAdd(const uint32_t key, CZQ_CustomIocpClientSocket * CustomIOCPClientSocket)
{
	int hash = key % HASH_IPITEM_BUCKETS;
	pHashPortItem p = new  HashPortItem;
	p->SocketHandle = key;
	p->CustomSocket = CustomIOCPClientSocket;
	p->next = _HandleBuckets[hash];
	_HandleBuckets[hash] = p;
	_hashClientCount++;
}

void CZQ_CustomIocpServerSocket::portItemRemote(const uint32_t key)
{
	auto prev = portItemFind(key);
	pHashPortItem p = *prev;
	if (p != nullptr)
	{
		(*prev) = p->next;
		delete p;

	}
	_hashClientCount--;

}

void CZQ_CustomIocpServerSocket::portItemClear()
{
	pHashPortItem p = nullptr;
	pHashPortItem n = nullptr;

	for (int i = 0; i < HASH_IPITEM_BUCKETS; i++)
	{
		p = _HandleBuckets[i];
		while (p != nullptr)
		{
			n = p->next;
			delete p;
			p = n;

		}
		_HandleBuckets[i] = nullptr;

	}
	_hashClientCount = 0;

}

int CZQ_CustomIocpServerSocket::getPoolCount()
{
	if (_freeClientPool)
		return _freeClientPool->getCount();
	else
		return 0;

}

void CZQ_CustomIocpServerSocket::setActive(bool bActive)
{
	if (bActive)
	{
		if (_IOCPSOCKET.IP == "" || _IOCPSOCKET.port == 0 || _IOCPSOCKET.socket != INVALID_SOCKET)
		{
			return;
		}
		for (int i = _workThreads.size() - 1; i > 0; i--)
		{
			auto t = _workThreads[i];
			if (t != nullptr)
			{
				delete t;
				_workThreads[i] = nullptr;

			}
		}
		_workThreads[0] = new CZQ_WorkThread(this, true);
		SetThreadAffinityMask(_workThreads[0]->getThread(), 0x01);


	}
	else
	{
		PostQueuedCompletionStatus(_hIOCP, 0, 0, (LPOVERLAPPED)SHUTDOWN_FLAG);
		for (int i = _workThreads.size() - 1; i >= 0; i--)
		{
			auto t = _workThreads[i];
			if (t != nullptr)
			{
				delete t;
				_workThreads[i] = nullptr;

			}
		}
	}
}

void CZQ_CustomIocpServerSocket::freeCorpseClient()
{
	uint64_t  tick = GetTickCount();
	uint64_t   dtick = tick - _timeOutCheckTick;
	if (tick > MAX_CHECK_TIMEOUT)
	{
		_timeOutCheckTick = tick;
		tick = dtick / 100;
		if (_freeClientPool)
		{
			_freeClientPool->clear(std::bind(&CZQ_CustomIocpServerSocket::disposeTimeOutClient, this, std::placeholders::_1, std::placeholders::_2), tick);

		}

		for (int i = _activeConnectList.size() - 1; i > 0; i--)
		{
			auto pClient = _activeConnectList[i];
			pClient->_currentTick += tick;
			if (pClient->getActiveTick() == 0)
			{
				pClient->setActiveTicket(pClient->_currentTick);
			}//if
			else
				if (pClient->_currentTick - pClient->getActiveTick() > MAX_ACTIVE_TIMEOUT)
				{
					pClient->setActiveTicket(pClient->_currentTick);
					pClient->forceClose();
				}//if
		}//for
	}//if

}

void CZQ_CustomIocpServerSocket::freeclient(CZQ_CustomIocpClientSocket * clientSocket)
{
	EnterCriticalSection(&_client_cs);
	do {
		for (auto iter = _activeConnectList.begin(); iter != _activeConnectList.end(); iter++)
		{
			if ((*iter) == clientSocket)
			{
				_clientCount--;
				_freeClientPool->append(clientSocket);
				if (clientSocket->_socket != INVALID_SOCKET)
				{
					clientSocket->forceClose();
				}
				if (OnDisConnect != nullptr)
				{
					OnDisConnect(clientSocket);
				}
				clientSocket->finalize();
				_activeConnectList.erase(iter);
				break;
			}//if
		}//for
	} while (0);
	LeaveCriticalSection(&_client_cs);
}

bool CZQ_CustomIocpServerSocket::activestart()
{
	bool result = false;
	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (_hIOCP != 0)
	{
		do {
			_IOCPSOCKET.socket = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
			if (_IOCPSOCKET.socket != INVALID_SOCKET)
			{
				do {
					SOCKADDR addr;
					memset(&addr, 0, sizeof(SOCKADDR));
					SOCKADDR_IN * addr_in = (SOCKADDR_IN*)(&addr);
					addr.sa_family = PF_INET;
					if (_IOCPSOCKET.IP =="0.0.0.0")
					{
						addr_in->sin_addr.S_un.S_addr = INADDR_ANY;
					}
					else
					{
						addr_in->sin_addr.S_un.S_addr = inet_addr(_IOCPSOCKET.IP);
					}
					WSAHtons(_IOCPSOCKET.socket, _IOCPSOCKET.port, &addr_in->sin_port);
					int ret = ::bind(_IOCPSOCKET.socket, &addr, sizeof(SOCKADDR));

					if (ret == 0)
					{
						_acceptThread = new CZQ_AcceptThread(this);
						for (size_t i = 1; i < _workThreads.size(); ++i)
						{
							_workThreads[i] = new CZQ_WorkThread(this, false);

						}
						result = true;
					}
					else
					{
						int err = WSAGetLastError();
						socketErrorEvent(nullptr, err);
						startListenFailed(err);
					}
				} while (0);
				if (!result)
				{
					closesocket(_IOCPSOCKET.socket);
					_IOCPSOCKET.socket = INVALID_SOCKET;
				}
				else
				{
					startListenSuccess(&_IOCPSOCKET);
				}
			}//if
			else
			{
				int err = GetLastError();
				startListenFailed(err);
			}
		} while (0);
		if (!result)
		{
			HANDLE iocpHandle = _hIOCP;
			_hIOCP = 0;
			CloseHandle(iocpHandle);
		}
	}
	else
		socketErrorEvent(nullptr, GetLastError());

	return result;
}

void CZQ_CustomIocpServerSocket::activestop()
{
	if (_acceptThread != nullptr)
	{
		delete _acceptThread;
		_acceptThread = nullptr;
	}

	for (int i = _activeConnectList.size() - 1; i >= 0; i--)
	{
		auto Clientsocket = _activeConnectList[i];
		Clientsocket->forceClose();
	}

	if (_hIOCP != 0)
	{
		HANDLE iocphandle = _hIOCP;
		_hIOCP = 0;
		CloseHandle(iocphandle);
	}
}

bool CZQ_CustomIocpServerSocket::disposeTimeOutClient(CZQ_CustomIocpClientSocket * clientsocket, uint64_t param)
{
	bool result = false;
	clientsocket->_currentTick += param;
	if (clientsocket->getActiveTick() == 0)
	{
		clientsocket->setActiveTicket(clientsocket->_currentTick);

	}
	else
		if (clientsocket->_currentTick - clientsocket->getActiveTick() > MAX_FREE_TIMEOUT)
		{
			delete clientsocket;
			clientsocket = nullptr;
			result = true;

		}
	return result;
}

bool CZQ_CustomIocpServerSocket::disposeAllClient(CZQ_CustomIocpClientSocket * clientSocket, uint64_t param)
{
	delete clientSocket;
	clientSocket = nullptr;
	return true;
}


/************************************************************************/
/*            CZQ_IocpClientSocket                                            */
/************************************************************************/

CZQ_IocpClientSocket::CZQ_IocpClientSocket()
{
	m_WorkThread = nullptr;
	m_ActiveSocket = new CZQ_CustomIocpClientSocket();
	m_ActiveSocket->_hIOCP = this->_hIOCP;
	m_ActiveSocket->_onSocketErrorEvent = IOCP_EVENT_CALLBACK_2(CZQ_IocpClientSocket::socketErrorEvent,this);


}

CZQ_IocpClientSocket::~CZQ_IocpClientSocket()
{
	m_ActiveSocket->forceClose();
	setActive(false);
	delete(m_ActiveSocket);
}

void CZQ_IocpClientSocket::setActive(bool Value)
{
	if (Value)
	{
		if ((_IOCPSOCKET.socket != INVALID_SOCKET) || (_IOCPSOCKET.port = 0 )||(inet_addr(_IOCPSOCKET.IP)==INADDR_NONE))
		{
			return;
		}
		if (m_WorkThread)
		{
			m_WorkThread->Terminate();
			PostQueuedCompletionStatus(_hIOCP, 0, 0, LPOVERLAPPED(SHUTDOWN_FLAG));
			delete(m_WorkThread);
		}
		m_WorkThread = new CZQ_WorkThread(this,true);
	}
	else
	{
		if (m_WorkThread)
		{
			m_WorkThread->Terminate();
			PostQueuedCompletionStatus(_hIOCP, 0, 0, LPOVERLAPPED(SHUTDOWN_FLAG));
			delete(m_WorkThread);
			activestop();
		}
		_IOCPSOCKET.socket = INVALID_SOCKET;
	}
}

void CZQ_IocpClientSocket::freeclient(CZQ_CustomIocpClientSocket* client)
{
	if ((client == m_ActiveSocket) && (_IOCPSOCKET.socket != INVALID_SOCKET))
	{
		_IOCPSOCKET.socket = INVALID_SOCKET;
		if (OnDisConnect)
			OnDisConnect(client);
	}
}

bool CZQ_IocpClientSocket::activestart()
{
	int ErrorCode;
	SOCKADDR_IN  toAddr;
	bool Result;
	if ((_IOCPSOCKET.socket != INVALID_SOCKET) ||
		(_IOCPSOCKET.port == 0) ||
		(inet_addr(_IOCPSOCKET.IP) == INADDR_NONE)
		)
		return false;
	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (_hIOCP != 0)
	{
		_IOCPSOCKET.socket = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_IP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (_IOCPSOCKET.socket != INVALID_SOCKET)
		{
			memset(&toAddr, sizeof(toAddr), 0);
			toAddr.sin_family = PF_INET;
			WSAHtons(_IOCPSOCKET.socket, _IOCPSOCKET.port, &toAddr.sin_port);
			toAddr.sin_addr.S_un.S_addr = inet_addr(_IOCPSOCKET.IP);
		
			m_ActiveSocket->_socket = _IOCPSOCKET.socket;
			m_ActiveSocket->_remoteAddress = _IOCPSOCKET.IP;
			m_ActiveSocket->_remotePort = _IOCPSOCKET.port;
			m_ActiveSocket->_hIOCP = _hIOCP;

			if (CreateIoCompletionPort((HANDLE)_IOCPSOCKET.socket, _hIOCP, DWORD(m_ActiveSocket), 0) == 0)
			{
				m_ActiveSocket->forceClose();
				_IOCPSOCKET.socket = INVALID_SOCKET;
				return false;
			}
			ErrorCode = 0;
			Result = WSAConnect(_IOCPSOCKET.socket, (sockaddr*)&toAddr, sizeof(toAddr), nullptr, nullptr, nullptr, nullptr) == 0;
			if (!Result)
			{
				ErrorCode = WSAGetLastError();
				Result = ErrorCode == WSAEWOULDBLOCK;
			}
			if (!Result)
			{
				m_ActiveSocket->forceClose();
				_IOCPSOCKET.socket = INVALID_SOCKET;
				if (ErrorCode != 10061)
					m_ActiveSocket->OnErrorEvent(ErrorCode);
			}
			else
			{
				if (OnConnect)
					OnConnect(m_ActiveSocket);
				if (_IOCPSOCKET.socket != INVALID_SOCKET)
				{ 
					memset(&m_ActiveSocket->_readBlock, sizeof(Block), 0);
					m_ActiveSocket->prepareRecv(&m_ActiveSocket->_readBlock);
				}
			}
		}
	}
	return Result;
}

void CZQ_IocpClientSocket::activestop()
{
	HANDLE  hIocpHandle;
	if (m_ActiveSocket->_socket != INVALID_SOCKET)
	{
		m_ActiveSocket->forceClose();
		_IOCPSOCKET.socket = INVALID_SOCKET;
		if (OnDisConnect)
			OnDisConnect(m_ActiveSocket);
	}
	if (_hIOCP != 0)
	{
	    hIocpHandle = _hIOCP;
		_hIOCP = INVALID_HANDLE_VALUE;
		CloseHandle(hIocpHandle);
	}
}

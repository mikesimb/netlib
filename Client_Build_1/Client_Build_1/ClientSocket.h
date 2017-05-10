#pragma once
#include "IOCPSOCKET.h"
class CClientSocket :
	public CZQ_IocpClientSocket
{
public:
	CClientSocket();
	~CClientSocket();
};


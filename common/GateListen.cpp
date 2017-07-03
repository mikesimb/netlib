 #include "stdafx.h"
#include "GateListen.h"
#include <stdio.h>
#include <iostream>

/************************************************************************/
/*     CGateUser                                            */
/************************************************************************/
static   CGateSocketServer* G_GateListen;

CGateSocketServer::CGateSocketServer()
{
	OnCreateSocketEvent = IOCP_EVENT_CALLBACK_0(CGateSocketServer::OnCreateClientSocket, this);
	OnConnect = IOCP_EVENT_CALLBACK_1(CGateSocketServer::ClientSocketConnect, this);
	OnDisConnect = IOCP_EVENT_CALLBACK_1(CGateSocketServer::ClientSocketDisconnect, this);
	OnSocketError = IOCP_EVENT_CALLBACK_2(CGateSocketServer::ClientServerSocketError, this);
	LoadConfig();
}

CGateSocketServer::~CGateSocketServer()
{

}


void CGateSocketServer::SendToClient(WORD wHandle, char * Buf, int buflen)
{

}

CGateSocketServer* CGateSocketServer::getInstance()
{
		if (G_GateListen == nullptr)
		{
			G_GateListen = new CGateSocketServer();

		}
		return G_GateListen;
}

void CGateSocketServer::ClientSocketConnect(CZQ_CustomIocpClientSocket* socket)
{
	std::cout << "ClientSocketConnect" << std::endl;
	std::cout << "Connect Count is :" << getCount() << std::endl;
}

void CGateSocketServer::ClientSocketDisconnect(CZQ_CustomIocpClientSocket* socket)
{
	std::cout << "ClientSocketDisconnect" << std::endl;
	std::cout << "Connect Count is :" << getCount() << std::endl;
}

void CGateSocketServer::ClientServerSocketError(CZQ_CustomIocpClientSocket* socket, int ErrorCode)
{
	if(ErrorCode != 10061)
		std::cout << "ClientServerSocketError ErrorCode is :" << ErrorCode << std::endl;
}

CZQ_CustomIocpClientSocket* CGateSocketServer::OnCreateClientSocket()
{
	return  new CGateUser();
}

void CGateSocketServer::LoadConfig()
{
	setServerIP("10.246.133.89");// ("192.168.198.117");
	setServerPort(7001);
	open();

}


/************************************************************************/
/*     CGateUser                                                                 */
/************************************************************************/


CGateUser::CGateUser()
{
	m_ReviceBuffer = new char[MAX_RECEIVE_LENGTH];

}

CGateUser::~CGateUser()
{
	delete(m_ReviceBuffer);
}
void CGateUser::ProcClientMsg(char* buf, int buflen)
{
	pDefaultMessage pMsg = (pDefaultMessage)buf;
	if (buflen >= sizeof(DefaultMessage))
	{
		std::cout << "GateUser:" << pMsg->Ident<< std::endl;

		switch (pMsg->Ident)
		{
		case CM_SELECT_SERVER:
			if (m_AreaIdx == -1)
				SendErrorMsg("加载程序版本错误，请自动完成更新",SM_SELECT_SERVER);
			m_Sel_ServerName = buf[sizeof(DefaultMessage)];
			m_ClientVersioin = pMsg->Recog;
			SendCollectServerInfo();
			if ((pMsg->Param == 1) || (!SendPKWarking()))
			{
				ContinueSelectServer();
			}
			break;
		case  CM_MERCHANT_QUERY:
			switch (pMsg->Recog)
			{
			case SM_SELECT_SERVER:
				  close();
				  break;
			case ID_PK_WARNING:
				if (pMsg->Series == 1)
					ContinueSelectServer();
				else
					close();
				break;
			default:
				close();
				break;
			}
			break;
		}
			
	}
	else
	{
		std::cout <<getRemoteAddress()<< "封包不完成" << std::endl;
	}

}

void CGateUser::InitAreaIndex(int SelAreaIdx)
{

}

void CGateUser::SendErrorMsg(std::string ErrorMsg, int msgID)
{
	char* buf;
	int     buflen;
	if (ErrorMsg != "")
	{
		buflen = sizeof(DefaultMessage) + ErrorMsg.size() + 1;
		buf = new char[buflen];
		pDefaultMessage(buf)->Recog = msgID;
		pDefaultMessage(buf)->Ident = SM_MERCHANT_QUERY;
		pDefaultMessage(buf)->Param = 0;
		pDefaultMessage(buf)->Tag = 1;
		pDefaultMessage(buf)->Series = 0;
		memcpy_s(&buf[sizeof(DefaultMessage)], buflen - sizeof(DefaultMessage), ErrorMsg.c_str(), ErrorMsg.size());
		SendToClient(buf, buflen);
	}
	


}

void CGateUser::SendCollectServerInfo()
{

}

void CGateUser::ContinueSelectServer()
{

}

bool CGateUser::SendPKWarking()
{
	return false;
}

void CGateUser::SocketRead(void* buf, int count)
{
	bool boWarning;
	int ioffset, doffset, Packagelen;
	if (count + m_ReviceBufferLen > MAX_RECEIVE_LENGTH)
		return;
	memmove_s(&m_ReviceBuffer[m_ReviceBufferLen], m_ReviceBufferLen, buf, count);
	m_ReviceBufferLen += count;
	boWarning = false;
	ioffset = 0;
	while (m_ReviceBufferLen - ioffset > sizeof(ClientMessage))
	{
		if (pClientMessage(&m_ReviceBuffer[ioffset])->Sign == SEGMENTATION_IDENT)
		{
			Packagelen = sizeof(ClientMessage) + pClientMessage(&m_ReviceBuffer[ioffset])->DataLength;
			if (Packagelen >= MAX_RECEIVE_LENGTH)
			{
				ioffset = m_ReviceBufferLen;
				break;
			}
			if (ioffset + Packagelen > m_ReviceBufferLen)
			{
				break;
			}
			switch (pClientMessage(&m_ReviceBuffer[ioffset])->Cmd)
			{
			case  LM_DYN_ENCRYPT_CODE:
				doffset = sizeof(ClientMessage) + ioffset;
				ProcClientMsg(&m_ReviceBuffer[doffset], pClientMessage(&m_ReviceBuffer[ioffset])->DataLength);
				break;
			case LM_GET_ENCRYPT:
				InitAreaIndex(0);
				break;
			default:
				break;
			}//switch
			ioffset += Packagelen;
		}
		else
		{
			boWarning = true;
			ioffset++;
		}
	}
	if (boWarning)
	{
		std::cout << "不完整封包" << std::endl;
	}
	m_ReviceBufferLen -= ioffset;
	if (m_ReviceBufferLen > 0)
		memmove_s(&m_ReviceBuffer[ioffset], m_ReviceBufferLen - ioffset, &m_ReviceBuffer[0], m_ReviceBufferLen);
}

void CGateUser::Initialize()
{
	m_ReviceBufferLen = 0;
	m_ClientVersioin = -1;
}



void CGateUser::SendToClient(char* buf, int buflen, WORD iCmd)
{
	char*   Data;
	int       DataLen;
	if ((buf == nullptr) || (buflen < sizeof(DefaultMessage)))
		return;
	DataLen = sizeof(ClientMessage) + buflen;
	Data = new char[DataLen];
	memset(Data, 0, DataLen);
	pClientMessage(Data)->Sign = SEGMENTATION_IDENT;
	pClientMessage(Data)->Cmd = iCmd;
	pClientMessage(Data)->DataLength = buflen;
	pClientMessage(Data)->DataIndex = this->getSocketHandle();

	memmove_s(&Data[sizeof(ClientMessage)], DataLen - sizeof(ClientMessage), buf, buflen);

	sendBuf(Data, DataLen);


}

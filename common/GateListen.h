#pragma once
#include "IOCPSOCKET.h"
#include "type.h"
class  CGateSocketServer :public CZQ_CustomIocpServerSocket
{
public:
	 CGateSocketServer();
	~ CGateSocketServer();

	void SendToClient(WORD wHandle, char * Buf, int buflen);
	static CGateSocketServer* getInstance();
	

private:

	void ClientSocketConnect(CZQ_CustomIocpClientSocket* socket);
	void ClientSocketDisconnect(CZQ_CustomIocpClientSocket* socket);
	void ClientServerSocketError(CZQ_CustomIocpClientSocket* socket, int ErrorCode);
	CZQ_CustomIocpClientSocket* OnCreateClientSocket();
	void LoadConfig();

};


class CGateUser :public CZQ_CustomIocpClientSocket
{
private:

	char*			m_ReviceBuffer;
	//ÒÑ¾­´æ´¢µÄ»º´æ
	int				m_ReviceBufferLen;
	std::string  m_Sel_ServerName;
	int             m_ClientVersioin;
	int             m_AreaIdx;
	void          ProcClientMsg(char* buf, int buflen);
	void          InitAreaIndex(int SelAreaIdx);
	void          SendErrorMsg(std::string ErrorMsg, int msgID);
	void         SendCollectServerInfo();
	void         ContinueSelectServer();
	bool         SendPKWarking();
protected:
	virtual void SocketRead(void* buf, int count) ;
	void Initialize();
public:
	CGateUser();
	~CGateUser();
	void SendToClient(char* buf, int buflen, WORD iCmd = LM_DYN_ENCRYPT_CODE);


};

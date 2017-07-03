#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
//���Ӿ�̬��SOKCET��
#pragma comment(lib, "ws2_32.Lib")


#include <WinSock2.h>
#include <stdint.h>
#include <functional>
#include <process.h>
#include <vector>

#include "ZQ_Thread.h"
/************************************************************************/
/* IOCPSOKCET����Ĺ�������                                             */
/************************************************************************/


class CZQ_CustomIocpClientSocket;
//�ͻ��˵������������
#define MAX_CLIENT_COUNT		( 1000)
//IOCP�����Ļ����С
#define MAX_IOCP_BUF_SIZE		( 8 * 1024)
//IOCP�ķ������ر�״̬��ʶ
#define SHUTDOWN_FLAG			( 0xFFFFFFFF)
//IOCP�Ŀͻ������ӶϿ��ı�ʶ
#define DISCONNECT_FLAG			( 0xFFFFFFFE)
//�����������߳�����
#define MAX_WORK_THREAD_SIZE		( 16)
//�û����ӹ�ϣ��Ĵ�С
#define HASH_IPITEM_BUCKETS		( 2047 )
//���ĳ�ʱ���ʱ��
#define MAX_CHECK_TIMEOUT		( 30 * 1000)
//��ǰ����״̬�ļ��ʱ��
#define MAX_ACTIVE_TIMEOUT		( 10 * 60)
//�Ͽ����ӵ��û��ļ��ʱ��
#define MAX_FREE_TIMEOUT		( 3  * 60)
//�ͻ���������������յ�buffer��С
#define CLIENT_MAX_RECEIVE_SIZE ( 32 * 1024 )
//����������������ķ��ʹ�С
#define SERVER_MAX_SEND_SIZE    ( 64 * 1024 )
//�����һ���������·��ı�ʶֵ������ʱ�޸ġ�
#define MSG_SIGN                ( 0xFF44FF44)
//������֮��ͨѶЭ���ʶ��
#define SERVER_MSG_SIGN         ( 0xFF33FF33)

//�¼���������
enum  SocketEvent {
	ZQ_seInitIOPort = 0,
	ZQ_seInitSocekt,
	ZQ_seConnect,
	ZQ_seListen,
	ZQ_seAccept,
	ZQ_seWrite,
	ZQ_seRead
};
//iocpsocket�Ļ������ݽṹ��
typedef struct tagIOCPSOCKET {
	SOCKET socket;
	char   IP[16];
	uint16_t  port;
}IOCPSOCKET, *pIOCPSOCKET;

//��ɶ˿ڻ������ṹ
typedef struct _Block
{
	OVERLAPPED overlapped;
	WSABUF  wsaBuffer;
	SocketEvent  event;
	char Buffer[MAX_IOCP_BUF_SIZE];

}Block, *pBlock;


//���Ͷ��нڵ�
typedef struct _SendQueueNode
{
	char * szBuf;
	int iBufferLen;
	int iStartPosition;
	_SendQueueNode* next;
}SendQueueNode, *pSendQueueNode;
//��������ǿͻ������ӵ�HASH��Ŀ����Ϊ���ڶ��û��������¾���鴦���ӵĿͻ���
typedef struct _HASH_PORT_ITEM
{
	uint32_t SocketHandle;
	CZQ_CustomIocpClientSocket * CustomSocket;
	_HASH_PORT_ITEM* next;

}HashPortItem, *pHashPortItem;

//�ͻ���socket������ṹ����
typedef struct _CustomClientListSocketNode
{
	CZQ_CustomIocpClientSocket* clientSocket;
	_CustomClientListSocketNode * next;
}CustomClientListSocketNode, *pCustomClientListSocketNode;


//�ص���������
#define  SocketNotifyEvent std::function<void(CZQ_CustomIocpClientSocket*)>
#define  SocketErrorEvent std::function<void(CZQ_CustomIocpClientSocket* ,int&)>
#define	 ClientSocketTouchProc std::function<void(CZQ_CustomIocpClientSocket* ,uint32_t)>
#define  SocketReviceEvent   std::function<void(CZQ_CustomIocpClientSocket* ,void* ,int)>
#define  OnCheckValidEvent  std::function<bool(std::string)>

#define OnReviceEvent std::function<void(void*,int)>
#define OnSocketErrorEvent std::function<void(CZQ_CustomIocpClientSocket* ,int)>
#define OnAcceptEvent std::function<void(SOCKET , char* ,int)>

#define CreateCustomSocketEvent  std::function<CZQ_CustomIocpClientSocket*()>
#define OnProcessMsgEvent   std::function<void(MsgRef*,int);
#define OnProcessServerMsgEvent  std::function<void(ServerMsgHead*,int)>

#define ARRAY_SIZE(a)  (sizeof(a)/sizeof(a[0]))

//C++11 �ص��Ķ���
#define IOCP_EVENT_CALLBACK_0(__selector__, __target__, ...)  std::bind(&__selector__, __target__, ##__VA_ARGS__)
#define IOCP_EVENT_CALLBACK_1(__selector__, __target__, ...)  std::bind(&__selector__, __target__, std::placeholders::_1, ##__VA_ARGS__) 
#define IOCP_EVENT_CALLBACK_2(__selector__, __target__, ...)  std::bind(&__selector__, __target__, std::placeholders::_1, std::placeholders::_2, ##__VA_ARGS__)
#define IOCP_EVENT_CALLBACK_3(__selector__, __target__, ...)  std::bind(&__selector__, __target__, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ##__VA_ARGS__)

/************************************************************************/
/* CZQ_CustomIocpClientSocket����                                       */
/************************************************************************/

using namespace std;

class CZQ_CustomIocpClientSocket
{
public:
	//@@���캯��
      CZQ_CustomIocpClientSocket();
	//@@��������
	 ~CZQ_CustomIocpClientSocket();
	//@@�ر�����
	void close();
	//@@���ͻ���
	int sendBuf(char* buf, int count, bool freeAfterSend = false);
	//@@�����ı�
	int sendText(std::string  s);
	//@@��ȡԶ�̵�ַ
	std::string getRemoteAddress();
	//@@��ȡԶ�����Ӷ˿�
	int getRemotePort();
	//@@��ȡ����
	char* getData();
	//@@��ȡ���ͻ��峤��
	int getSendBufferLen();
	//@@�ܹ������˶��ٸ���
	uint32_t  getTotalSendCount();
	//@@�����ܹ����͵İ�����
	void  setTotalSendCount(uint32_t value);
	//@@ǿ�ƹر�
	int forceClose();
	//@@���ü����tick
	void setActiveTicket(uint64_t value);
	//@@��ȡ�����tick
	uint64_t getActiveTick();
	//@@��ȡSocket���
	uint32_t getSocketHandle();
	//@@����
	virtual void  update();

protected:
	//����������¼�
	void OnErrorEvent(int ErrorCode);
	//@@��ʼ��
	virtual void initialize();
	//@@��ֹ��
	virtual void finalize();
	//@@������
	virtual void socketRead(void *buf, int count);

private:
	//@@�ͻ��˶�ȡ
	void doClientRead(pBlock pRBlock, int transfered);
	//@@�ͻ��˷���
	void doClientSend(pBlock pSBlock, int transfered);
	//@@׼�����ն���
	void prepareRecv(pBlock pRBlock);
	//@@׼�����Ͷ���
	void prepareSend(pBlock pSBlock, int iSendlen);
	//@@������ͻ���
	void clearSendBuffer();

private://�����Ƕ���ĳ�Ա����
	HANDLE						        _hIOCP;
	SOCKET							    _socket;
	std::string						    _remoteAddress;// = "0.0.0.0";
	int									_remotePort;
	char*								_data;
	uint32_t							_socketHandle;
	Block							    _sendBlock;
	Block							    _readBlock;
	int									_totalBufferlen;
	RTL_CRITICAL_SECTION	            _sendBufferCS;
	bool								_isSending;
	uint64_t							_currentTick;
	uint32_t							_totalSendDataCount;
	pSendQueueNode			_firstQueueNode;
	pSendQueueNode			_lastQueueNode;
	OnSocketErrorEvent		_onSocketErrorEvent;
	SocketReviceEvent			_onReviceEvent;
	uint64_t							_uactiveTick;

	friend class CZQ_AcceptThread;
	friend class CZQ_WorkThread;
	friend class CZQ_CustomIocpServerSocket;
	friend class CZQ_IocpClientSocket;
};

/************************************************************************/
/* CZQ_CustomeIOCPSocket����                                           */
/************************************************************************/
class CZQ_CustomIOCPSocket
{
public:
	//@@���캯��
	CZQ_CustomIOCPSocket();
	//@@��������
	virtual ~CZQ_CustomIOCPSocket();

public:
	//����Ϊ�����Զ������¼�����
	//��������Ļص�����
	SocketErrorEvent  OnSocketError;
	//���ӳɹ�֪ͨ
	SocketNotifyEvent OnConnect;
	//�Ͽ������¼�
	SocketNotifyEvent OnDisConnect;

private :
	//����Ϊ��Ա��������
	HANDLE				 _hIOCP = nullptr;

	IOCPSOCKET			_IOCPSOCKET;

	
public:
	//@@�ж��Ƿ�����
	bool isActiveed();
	//@@����
	void open();
	//@@�ر�
	void close();
	//@@����������һ��ʵ�ַ�ʽ��
	virtual void setActive(bool bActive) = 0;
	//@@�ͷſͻ�������
	virtual void freeclient(CZQ_CustomIocpClientSocket* client) = 0;
	//@@����
	virtual bool activestart() = 0;
	//@@ֹͣ
	virtual void activestop() = 0;
	//@@SocektError�¼�֪ͨ��
	void socketErrorEvent(CZQ_CustomIocpClientSocket* clientsocket, int errorCode);

	HANDLE				 getIOCPHandle();
	IOCPSOCKET*		getIOCPSOCKET();
	std::string				getServerIP();
	void						setServerIP(std::string  IPAddress);
	int							getServerPort();
	void						setServerPort(const int Port);

	friend class CZQ_AcceptThread;
	friend class CZQ_WorkThread;
	friend class CZQ_CustomIocpServerSocket;
	friend class CZQ_IocpClientSocket;
};

/************************************************************************/
/*   CZQ_AcceptThread                                                         */
/************************************************************************/
class CZQ_CustomIocpServerSocket;
class CZQ_AcceptThread :public CZQ_Thread
{
public:
	//@@�̵߳Ĺ��캯��
	CZQ_AcceptThread(CZQ_CustomIocpServerSocket * parentSocket);
	//@@�̵߳���������
	virtual ~CZQ_AcceptThread();
	//@@�߳�ִ�к���
	void Execute();
private:
	CZQ_CustomIocpServerSocket* _parentSocket;

};

/************************************************************************/
/*   CZQ_WorkThread                                                         */
/************************************************************************/
class CZQ_WorkThread :public CZQ_Thread
{
public:
	//@@�̵߳Ĺ��캯��
	CZQ_WorkThread(CZQ_CustomIOCPSocket* parentSocket, bool boMaster = false);
	//@@�̵߳���������
	virtual ~CZQ_WorkThread();
	//@@�߳�ִ�к���
	void Execute();
private:
	bool _boMaster;
	//@@workThread��ӵ���ߡ�
	CZQ_CustomIOCPSocket* _parentSocket;

};

/************************************************************************/
/*   CZQ_CustomeClientSocketPool                                        */
/*�ͻ��˵Ĵ洢�����࣬��һ������*/
/************************************************************************/
#define  DequeueEvent std::function<bool(CZQ_CustomIocpClientSocket* ,uint64_t)>
class CZQ_CustomeClientSocketPool
{
public:
	//@@���캯��
	CZQ_CustomeClientSocketPool();
	//@@��������
	~CZQ_CustomeClientSocketPool();
	//@@���Ԫ��
	void append(CZQ_CustomIocpClientSocket* customClientSocket);
	//@@�Ӷ���ɾ��Ԫ��
	CZQ_CustomIocpClientSocket * deQueue();
	//@@��ն���
	void clear(DequeueEvent dequeueEvent, uint64_t param);
	//@@��ȡԪ�ظ���
	int getCount();
private:
	pCustomClientListSocketNode  _firstQueueNode;
	pCustomClientListSocketNode  _lastQueueNode;
	int _count;
};

/************************************************************************/
/* CZQ_IocpServerSocket                                                     */
/************************************************************************/

class CZQ_CustomIocpServerSocket :public  CZQ_CustomIOCPSocket
{
public:
	CZQ_CustomIocpServerSocket();
	~CZQ_CustomIocpServerSocket();

public:
	//�������յ���Ϣ���¼�
	SocketReviceEvent OnSocketRevice;
	//����Ƿ�����Ч��ַ���¼�
	OnCheckValidEvent OnCheckIPAddress;
	//����һ���ͻ������ӵ�SOCKET���¼�
	CreateCustomSocketEvent OnCreateSocketEvent;
	//@@���Ŀǰ����֪����ʲô��
	void touch(ClientSocketTouchProc touchProc, uint32_t param);
	//@@�رշ�������
	void close();
	//@@��ȡ��ǰ�ļ���������б�
	std::vector<CZQ_CustomIocpClientSocket*>* getActiveConnects();
	//��ȡ��ǰ�Ŀͻ�������������
	int getCount();
	//��ȡ��ǰ��HASH���������
	int hashCount();
	//��ȡ�û��ĳ����е�������
	int poolCount();
	//�ж��Ƿ�����Ч��ַ
	bool isValidAddress(std::string ip, int nIP, int iport);
	//�пͻ�������ACCEPT�¼��ĺ���������
	void socketAccept(SOCKET socket, char* IP, int port);
	//���������ɹ���
	inline virtual void startListenSuccess(const IOCPSOCKET * iocpsocket) {};
	//��������ʧ�ܡ�
	inline virtual void startListenFailed(int errCode) {};

protected:
	void lock();
	void unlock();
	CZQ_CustomIocpClientSocket * valueof(const uint32_t key);

private:
	RTL_CRITICAL_SECTION  _client_cs;
	std::vector<CZQ_CustomIocpClientSocket*> _activeConnectList;
	int _clientCount;
	int _hashClientCount;
	//������һ��hash��Ӧ�ÿ��Կ����޸�һ��
	HashPortItem * _HandleBuckets[HASH_IPITEM_BUCKETS];

	CZQ_CustomeClientSocketPool* _freeClientPool;
	uint64_t _timeOutCheckTick;
	//������һ�������߳�
	CZQ_AcceptThread* _acceptThread;

	std::vector<CZQ_WorkThread*> _workThreads;
	uint32_t _currentHandle;
	//�ҵ���Ӧ��hash����
	pHashPortItem * portItemFind(uint32_t key);
	//@@���һ�����ݽ���hash��
	void portItemAdd(const uint32_t key, CZQ_CustomIocpClientSocket * CustomIOCPClientSocket);
	//@@ɾ��һ�����ݴ�HASH����
	void portItemRemote(const uint32_t key);
	//@@�������HASH��
	void portItemClear();
	//��ȡ��ϣ���е�Ԫ��������
	int getPoolCount();
	//@@����
	virtual void setActive(bool bActive);
	//@@�ͷ����õĿͻ���
	void freeCorpseClient();
	//@@�ͷſͻ���
	virtual void freeclient(CZQ_CustomIocpClientSocket * clientSocket);
	//��������
	virtual bool activestart();
	//ֹͣ������
	virtual void activestop();
	//@@�ͷų�ʱ�Ŀͻ���
	bool disposeTimeOutClient(CZQ_CustomIocpClientSocket * clientsocket, uint64_t param);
	//@@�ͷ����еĿͻ���
	bool disposeAllClient(CZQ_CustomIocpClientSocket * clientSocket, uint64_t param);

};

/************************************************************************/
/*                   CZQ_IocpClientSocket                                    */
/************************************************************************/


class CZQ_IocpClientSocket :public CZQ_CustomIOCPSocket
{
public:
	CZQ_IocpClientSocket();
	~CZQ_IocpClientSocket();
	CZQ_CustomIocpClientSocket * m_ActiveSocket;
	CZQ_WorkThread * m_WorkThread;
	//@@�����ͻ���
	void setActive(bool Value) override;
	//�ͷſͻ���
	void freeclient(CZQ_CustomIocpClientSocket* client) override;
	//��������
	bool activestart() override;
	//@@ֹͣ
	void activestop() override;
	//���������¼���
	OnReviceEvent  m_OnReviceEvent;
};

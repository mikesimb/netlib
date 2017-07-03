#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
//链接静态的SOKCET库
#pragma comment(lib, "ws2_32.Lib")


#include <WinSock2.h>
#include <stdint.h>
#include <functional>
#include <process.h>
#include <vector>

#include "ZQ_Thread.h"
/************************************************************************/
/* IOCPSOKCET定义的公共部分                                             */
/************************************************************************/


class CZQ_CustomIocpClientSocket;
//客户端的最大连接数量
#define MAX_CLIENT_COUNT		( 1000)
//IOCP的最大的缓存大小
#define MAX_IOCP_BUF_SIZE		( 8 * 1024)
//IOCP的服务器关闭状态标识
#define SHUTDOWN_FLAG			( 0xFFFFFFFF)
//IOCP的客户端连接断开的标识
#define DISCONNECT_FLAG			( 0xFFFFFFFE)
//允许的最大工作线程数量
#define MAX_WORK_THREAD_SIZE		( 16)
//用户连接哈希表的大小
#define HASH_IPITEM_BUCKETS		( 2047 )
//最大的超时检测时间
#define MAX_CHECK_TIMEOUT		( 30 * 1000)
//当前激活状态的检测时间
#define MAX_ACTIVE_TIMEOUT		( 10 * 60)
//断开连接的用户的检测时间
#define MAX_FREE_TIMEOUT		( 3  * 60)
//客户端类允许的最大接收的buffer大小
#define CLIENT_MAX_RECEIVE_SIZE ( 32 * 1024 )
//服务器类允许的最大的发送大小
#define SERVER_MAX_SEND_SIZE    ( 64 * 1024 )
//这个是一个服务器下发的标识值可以随时修改。
#define MSG_SIGN                ( 0xFF44FF44)
//服务器之间通讯协议标识。
#define SERVER_MSG_SIGN         ( 0xFF33FF33)

//事件定义类型
enum  SocketEvent {
	ZQ_seInitIOPort = 0,
	ZQ_seInitSocekt,
	ZQ_seConnect,
	ZQ_seListen,
	ZQ_seAccept,
	ZQ_seWrite,
	ZQ_seRead
};
//iocpsocket的基本数据结构。
typedef struct tagIOCPSOCKET {
	SOCKET socket;
	char   IP[16];
	uint16_t  port;
}IOCPSOCKET, *pIOCPSOCKET;

//完成端口缓冲区结构
typedef struct _Block
{
	OVERLAPPED overlapped;
	WSABUF  wsaBuffer;
	SocketEvent  event;
	char Buffer[MAX_IOCP_BUF_SIZE];

}Block, *pBlock;


//发送队列节点
typedef struct _SendQueueNode
{
	char * szBuf;
	int iBufferLen;
	int iStartPosition;
	_SendQueueNode* next;
}SendQueueNode, *pSendQueueNode;
//这个队列是客户端连接的HASH表，目的是为了在多用户的条件下尽快查处连接的客户端
typedef struct _HASH_PORT_ITEM
{
	uint32_t SocketHandle;
	CZQ_CustomIocpClientSocket * CustomSocket;
	_HASH_PORT_ITEM* next;

}HashPortItem, *pHashPortItem;

//客户端socket的链表结构定义
typedef struct _CustomClientListSocketNode
{
	CZQ_CustomIocpClientSocket* clientSocket;
	_CustomClientListSocketNode * next;
}CustomClientListSocketNode, *pCustomClientListSocketNode;


//回调函数定义
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

//C++11 回调的定义
#define IOCP_EVENT_CALLBACK_0(__selector__, __target__, ...)  std::bind(&__selector__, __target__, ##__VA_ARGS__)
#define IOCP_EVENT_CALLBACK_1(__selector__, __target__, ...)  std::bind(&__selector__, __target__, std::placeholders::_1, ##__VA_ARGS__) 
#define IOCP_EVENT_CALLBACK_2(__selector__, __target__, ...)  std::bind(&__selector__, __target__, std::placeholders::_1, std::placeholders::_2, ##__VA_ARGS__)
#define IOCP_EVENT_CALLBACK_3(__selector__, __target__, ...)  std::bind(&__selector__, __target__, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ##__VA_ARGS__)

/************************************************************************/
/* CZQ_CustomIocpClientSocket定义                                       */
/************************************************************************/

using namespace std;

class CZQ_CustomIocpClientSocket
{
public:
	//@@构造函数
      CZQ_CustomIocpClientSocket();
	//@@析构函数
	 ~CZQ_CustomIocpClientSocket();
	//@@关闭连接
	void close();
	//@@发送缓冲
	int sendBuf(char* buf, int count, bool freeAfterSend = false);
	//@@发送文本
	int sendText(std::string  s);
	//@@获取远程地址
	std::string getRemoteAddress();
	//@@获取远程连接端口
	int getRemotePort();
	//@@获取数据
	char* getData();
	//@@获取发送缓冲长度
	int getSendBufferLen();
	//@@总共发送了多少个包
	uint32_t  getTotalSendCount();
	//@@设置总共发送的包个数
	void  setTotalSendCount(uint32_t value);
	//@@强制关闭
	int forceClose();
	//@@设置激活的tick
	void setActiveTicket(uint64_t value);
	//@@获取激活的tick
	uint64_t getActiveTick();
	//@@获取Socket句柄
	uint32_t getSocketHandle();
	//@@更新
	virtual void  update();

protected:
	//发生错误的事件
	void OnErrorEvent(int ErrorCode);
	//@@初始化
	virtual void initialize();
	//@@终止化
	virtual void finalize();
	//@@包解析
	virtual void socketRead(void *buf, int count);

private:
	//@@客户端读取
	void doClientRead(pBlock pRBlock, int transfered);
	//@@客户端发送
	void doClientSend(pBlock pSBlock, int transfered);
	//@@准备接收动作
	void prepareRecv(pBlock pRBlock);
	//@@准备发送动作
	void prepareSend(pBlock pSBlock, int iSendlen);
	//@@清楚发送缓存
	void clearSendBuffer();

private://以下是定义的成员变量
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
/* CZQ_CustomeIOCPSocket定义                                           */
/************************************************************************/
class CZQ_CustomIOCPSocket
{
public:
	//@@构造函数
	CZQ_CustomIOCPSocket();
	//@@析构函数
	virtual ~CZQ_CustomIOCPSocket();

public:
	//以下为定义自动定义事件二用
	//发生错误的回调函数
	SocketErrorEvent  OnSocketError;
	//连接成功通知
	SocketNotifyEvent OnConnect;
	//断开连接事件
	SocketNotifyEvent OnDisConnect;

private :
	//以下为成员变量定义
	HANDLE				 _hIOCP = nullptr;

	IOCPSOCKET			_IOCPSOCKET;

	
public:
	//@@判断是否被启动
	bool isActiveed();
	//@@启动
	void open();
	//@@关闭
	void close();
	//@@启动的另外一种实现方式。
	virtual void setActive(bool bActive) = 0;
	//@@释放客户端连接
	virtual void freeclient(CZQ_CustomIocpClientSocket* client) = 0;
	//@@启动
	virtual bool activestart() = 0;
	//@@停止
	virtual void activestop() = 0;
	//@@SocektError事件通知。
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
	//@@线程的构造函数
	CZQ_AcceptThread(CZQ_CustomIocpServerSocket * parentSocket);
	//@@线程的析构函数
	virtual ~CZQ_AcceptThread();
	//@@线程执行函数
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
	//@@线程的构造函数
	CZQ_WorkThread(CZQ_CustomIOCPSocket* parentSocket, bool boMaster = false);
	//@@线程的析构函数
	virtual ~CZQ_WorkThread();
	//@@线程执行函数
	void Execute();
private:
	bool _boMaster;
	//@@workThread的拥有者。
	CZQ_CustomIOCPSocket* _parentSocket;

};

/************************************************************************/
/*   CZQ_CustomeClientSocketPool                                        */
/*客户端的存储容器类，是一个链表。*/
/************************************************************************/
#define  DequeueEvent std::function<bool(CZQ_CustomIocpClientSocket* ,uint64_t)>
class CZQ_CustomeClientSocketPool
{
public:
	//@@构造函数
	CZQ_CustomeClientSocketPool();
	//@@析构函数
	~CZQ_CustomeClientSocketPool();
	//@@添加元素
	void append(CZQ_CustomIocpClientSocket* customClientSocket);
	//@@从顶部删除元素
	CZQ_CustomIocpClientSocket * deQueue();
	//@@清空队列
	void clear(DequeueEvent dequeueEvent, uint64_t param);
	//@@获取元素个数
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
	//服务器收到消息的事件
	SocketReviceEvent OnSocketRevice;
	//检查是否是有效地址的事件
	OnCheckValidEvent OnCheckIPAddress;
	//创建一个客户端连接的SOCKET的事件
	CreateCustomSocketEvent OnCreateSocketEvent;
	//@@这个目前还不知道是什么鬼
	void touch(ClientSocketTouchProc touchProc, uint32_t param);
	//@@关闭服务器。
	void close();
	//@@获取当前的激活的连接列表
	std::vector<CZQ_CustomIocpClientSocket*>* getActiveConnects();
	//获取当前的客户端链接数量。
	int getCount();
	//获取当前的HASH表的数量。
	int hashCount();
	//获取用户的池子中的数量。
	int poolCount();
	//判断是否是有效地址
	bool isValidAddress(std::string ip, int nIP, int iport);
	//有客户端请求ACCEPT事件的后续操作。
	void socketAccept(SOCKET socket, char* IP, int port);
	//启动监听成功。
	inline virtual void startListenSuccess(const IOCPSOCKET * iocpsocket) {};
	//启动监听失败。
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
	//这里是一个hash类应该可以考虑修改一下
	HashPortItem * _HandleBuckets[HASH_IPITEM_BUCKETS];

	CZQ_CustomeClientSocketPool* _freeClientPool;
	uint64_t _timeOutCheckTick;
	//这里是一个接受线程
	CZQ_AcceptThread* _acceptThread;

	std::vector<CZQ_WorkThread*> _workThreads;
	uint32_t _currentHandle;
	//找到相应的hash数据
	pHashPortItem * portItemFind(uint32_t key);
	//@@添加一个数据进入hash表
	void portItemAdd(const uint32_t key, CZQ_CustomIocpClientSocket * CustomIOCPClientSocket);
	//@@删除一个数据从HASH表中
	void portItemRemote(const uint32_t key);
	//@@清空整个HASH表
	void portItemClear();
	//获取哈希表中的元素数量。
	int getPoolCount();
	//@@启动
	virtual void setActive(bool bActive);
	//@@释放无用的客户端
	void freeCorpseClient();
	//@@释放客户端
	virtual void freeclient(CZQ_CustomIocpClientSocket * clientSocket);
	//激活启动
	virtual bool activestart();
	//停止服务器
	virtual void activestop();
	//@@释放超时的客户端
	bool disposeTimeOutClient(CZQ_CustomIocpClientSocket * clientsocket, uint64_t param);
	//@@释放所有的客户端
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
	//@@启动客户端
	void setActive(bool Value) override;
	//释放客户端
	void freeclient(CZQ_CustomIocpClientSocket* client) override;
	//激活启动
	bool activestart() override;
	//@@停止
	void activestop() override;
	//接收数据事件。
	OnReviceEvent  m_OnReviceEvent;
};

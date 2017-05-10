#pragma once
#include "ZQ_Thread.h"
typedef struct tagThreadMsgNode
{
	char*							  ImBuf;
	WORD						  ImBufLen;
	WORD						  ImCmd;
	WORD						  Imdent;
	WORD						  ImHandle;
	tagThreadMsgNode*   Next;
}ThreadMsgNode,*pThreadMsgNode;

class CMainthread :public CZQ_Thread
{
public:
	CMainthread();
	~CMainthread();
	void FromClientMsg(WORD wHandle, WORD wCmd, char* buf, int Buflen);
	void FromDBMsg(WORD wHandle, WORD wCmd, char * buf, int Buflen);

	static CMainthread* getInstance();
protected:
	void Execute();
private:
	void ClearAllMsg();
	void DispenseGateMessage();
	void DispenseDBMessage();


private:
	unsigned long    m_Event;
	int                      m_SessionID;
	pThreadMsgNode * m_TopCMNode;
	pThreadMsgNode * m_BotCMNode;

	CRITICAL_SECTION   m_GateCS;
	CRITICAL_SECTION   m_DBCS;





	
};


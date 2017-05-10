#pragma once

/*
����һ���߳���CZQ_Thread.
���ߣ�����ΰ
���߳���Ŀǰ���߱��߳�ͬ������
*/

#include <windows.h>
#include <string>

#define Max_Wait_for_Timeout    6000



class CZQ_Thread
{
public:
	CZQ_Thread();
	virtual ~CZQ_Thread();
public:
	//�����߳�
	virtual BOOL  Initialize(BOOL bSuspended = FALSE, unsigned int uStackSize = 0);

	//�̹߳���
	BOOL Suspend(void);

	//�ָ��߳�
	BOOL Resume(void);

	//��ֹ�߳�
	virtual void Terminate(void);

	//�ȴ��߳�
	void WaitFor(const DWORD dwTimeOut = Max_Wait_for_Timeout);
	//��ȡ�߳�ID
	const char * getThreadID() { return (char *)m_dwThreadID; }
	//��ȡ�߳̾����CHAR���͡�
	const char * getThreadHandle() { return (char *)m_hThread; }
	//��ȡ�߳̾����HANDLE����
	HANDLE  getThread() { return m_hThread; };
	//�߳��Ƿ���ֹ
	BOOL Terminated(void);
	//�����߳�����
	void SetThreadName(char * name);
protected:
	//ִ�й���
	virtual void Execute(void) = 0;
	//��ֹ�¼�
	virtual void OnTerminate(void) { OutputDebugString(ThreadName.c_str()); };
	//�Ƿ���ֹ
	volatile BOOL m_bTerminated;

private:
	//�߳�ID
	DWORD m_dwThreadID;
	//�߳̾��
	HANDLE m_hThread;
	//��ǰ�Ƿ����
	BOOL m_bIsSuspended;
	//�̵߳�����
	std::string ThreadName;
	//�̹߳���
	static unsigned WINAPI ThreadProc(LPVOID pParam);
};


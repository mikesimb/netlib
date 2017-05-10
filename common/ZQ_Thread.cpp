#pragma once
#include "stdafx.h"
#include "ZQ_Thread.h"

//���͹��캯��
CZQ_Thread::CZQ_Thread()
{
	m_dwThreadID = 0;
	m_hThread = INVALID_HANDLE_VALUE;
	m_bTerminated = TRUE;
	m_bIsSuspended = false;
	ThreadName = "";
}

//������������
CZQ_Thread::~CZQ_Thread()
{
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		if (m_bIsSuspended)
		{
			Resume();
		}
		Terminate();
		DWORD time = Max_Wait_for_Timeout;
		WaitFor(time);
	}
}

//��ʼ���߳�  ������á�
BOOL CZQ_Thread::Initialize(BOOL bSuspended /*= FALSE*/, unsigned int uStackSize /*= 0*/)
{
	unsigned int dwThreadID;
	int iFlag = 0;
	//���������ô���ù���ı�־
	if (bSuspended)
	{
		iFlag = CREATE_SUSPENDED;
		m_bIsSuspended = true;

	}
	else
	{
		m_bIsSuspended = false;
	}
	//�����߳�
	HANDLE hThread = (HANDLE)::CreateThread(NULL, uStackSize, (LPTHREAD_START_ROUTINE)ThreadProc, (void*)this, iFlag, (LPDWORD)&dwThreadID);

	if (hThread == INVALID_HANDLE_VALUE)
		//�������ʧ��
		return false;
	m_dwThreadID = dwThreadID;
	m_hThread = hThread;
	return true;

}

//�̹߳���
BOOL CZQ_Thread::Suspend(void)
{
	//ִ�й����̲߳�����
	if (SuspendThread(m_hThread) != (DWORD)-1)
	{
		//�ɹ�����
		m_bIsSuspended = TRUE;
		return TRUE;
	}
	else
	{
		//����ʧ��
		m_bIsSuspended = FALSE;
		return FALSE;
	}
}

//�ָ��̡߳�
BOOL CZQ_Thread::Resume(void)
{
	//�����ж�����̴߳��ڷǹ���״̬��ô�˳���
	if (!m_bIsSuspended)
	{
		return TRUE;
	}
	//�ָ��߳�
	if (ResumeThread(m_hThread) != (DWORD)-1)
	{
		//�ָ��ɹ�
		m_bIsSuspended = FALSE;
		return TRUE;
	}
	else
	{
		//�ָ�ʧ��
		m_bIsSuspended = TRUE;
		return  FALSE;
	}
}


//��ֹ�߳�
void CZQ_Thread::Terminate(void)
{
	//ǿ������m_bTerminated = true;
	::InterlockedExchange((LONG *)(&m_bTerminated), TRUE);


}

//�ȴ��߳�
void CZQ_Thread::WaitFor(const DWORD dwTimeOut /*= MAX_WAIT_FOR_TIME*/)
{
	//����߳̾����Ч
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		//�ȴ��߳�
		WaitForSingleObject(m_hThread, dwTimeOut);
	}
}


//�߳��Ƿ���ֹ
BOOL CZQ_Thread::Terminated(void)
{
	return  m_bTerminated;
}

//�߳�����
void CZQ_Thread::SetThreadName(char * name)
{
	ThreadName = name;
}

//�̵߳��ù��̡�
unsigned WINAPI  CZQ_Thread::ThreadProc(LPVOID pParam)
{
	CZQ_Thread * pThead = (CZQ_Thread *)pParam;
	::InterlockedExchange((LONG *)&(pThead->m_bTerminated), FALSE);
	try
	{


		pThead->Execute();

	}
	catch (...)
	{

	}
	try
	{
		pThead->OnTerminate();
	}
	catch (...)
	{

	}
	CloseHandle(pThead->getThread());
	pThead->m_hThread = INVALID_HANDLE_VALUE;
	pThead->m_dwThreadID = 0;
	pThead->m_bTerminated = true;
	return 0;
}
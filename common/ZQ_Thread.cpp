#pragma once
#include "stdafx.h"
#include "ZQ_Thread.h"

//类型构造函数
CZQ_Thread::CZQ_Thread()
{
	m_dwThreadID = 0;
	m_hThread = INVALID_HANDLE_VALUE;
	m_bTerminated = TRUE;
	m_bIsSuspended = false;
	ThreadName = "";
}

//类型析构函数
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

//初始化线程  不许调用。
BOOL CZQ_Thread::Initialize(BOOL bSuspended /*= FALSE*/, unsigned int uStackSize /*= 0*/)
{
	unsigned int dwThreadID;
	int iFlag = 0;
	//如果挂起，那么设置挂起的标志
	if (bSuspended)
	{
		iFlag = CREATE_SUSPENDED;
		m_bIsSuspended = true;

	}
	else
	{
		m_bIsSuspended = false;
	}
	//创建线程
	HANDLE hThread = (HANDLE)::CreateThread(NULL, uStackSize, (LPTHREAD_START_ROUTINE)ThreadProc, (void*)this, iFlag, (LPDWORD)&dwThreadID);

	if (hThread == INVALID_HANDLE_VALUE)
		//如果创建失败
		return false;
	m_dwThreadID = dwThreadID;
	m_hThread = hThread;
	return true;

}

//线程挂起
BOOL CZQ_Thread::Suspend(void)
{
	//执行挂起线程操作。
	if (SuspendThread(m_hThread) != (DWORD)-1)
	{
		//成功挂起
		m_bIsSuspended = TRUE;
		return TRUE;
	}
	else
	{
		//挂起失败
		m_bIsSuspended = FALSE;
		return FALSE;
	}
}

//恢复线程。
BOOL CZQ_Thread::Resume(void)
{
	//首先判断如果线程处于非挂起状态那么退出；
	if (!m_bIsSuspended)
	{
		return TRUE;
	}
	//恢复线程
	if (ResumeThread(m_hThread) != (DWORD)-1)
	{
		//恢复成功
		m_bIsSuspended = FALSE;
		return TRUE;
	}
	else
	{
		//恢复失败
		m_bIsSuspended = TRUE;
		return  FALSE;
	}
}


//终止线程
void CZQ_Thread::Terminate(void)
{
	//强制设置m_bTerminated = true;
	::InterlockedExchange((LONG *)(&m_bTerminated), TRUE);


}

//等待线程
void CZQ_Thread::WaitFor(const DWORD dwTimeOut /*= MAX_WAIT_FOR_TIME*/)
{
	//如果线程句柄有效
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		//等待线程
		WaitForSingleObject(m_hThread, dwTimeOut);
	}
}


//线程是否被终止
BOOL CZQ_Thread::Terminated(void)
{
	return  m_bTerminated;
}

//线程名字
void CZQ_Thread::SetThreadName(char * name)
{
	ThreadName = name;
}

//线程调用过程。
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
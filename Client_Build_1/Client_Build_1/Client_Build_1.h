
// Client_Build_1.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CClient_Build_1App: 
// �йش����ʵ�֣������ Client_Build_1.cpp
//

class CClient_Build_1App : public CWinApp
{
public:
	CClient_Build_1App();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CClient_Build_1App theApp;
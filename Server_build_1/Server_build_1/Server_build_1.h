
// Server_build_1.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CServer_build_1App: 
// �йش����ʵ�֣������ Server_build_1.cpp
//

class CServer_build_1App : public CWinApp
{
public:
	CServer_build_1App();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CServer_build_1App theApp;
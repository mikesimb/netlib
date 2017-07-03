
// Client_Build_1Dlg.h : 头文件
//

#pragma once

#include "ClientSocket.h"


// CClient_Build_1Dlg 对话框
class CClient_Build_1Dlg : public CDialogEx
{
// 构造
public:
	CClient_Build_1Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_BUILD_1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CClientSocket* m_client;
	SocketNotifyEvent m_onClientConnect;
	SocketNotifyEvent m_onDisconnect;
	SocketErrorEvent m_onError;

	void OnConnect(CZQ_CustomIocpClientSocket* client);
	void OnDisconnect(CZQ_CustomIocpClientSocket* client);
	void OnError(CZQ_CustomIocpClientSocket* client, int& ErrorCode);



	
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnClose();
};

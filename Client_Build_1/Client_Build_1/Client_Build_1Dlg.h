
// Client_Build_1Dlg.h : ͷ�ļ�
//

#pragma once

#include "ClientSocket.h"


// CClient_Build_1Dlg �Ի���
class CClient_Build_1Dlg : public CDialogEx
{
// ����
public:
	CClient_Build_1Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_BUILD_1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	CClientSocket* m_client;
	SocketNotifyEvent m_onClientConnect;
	SocketNotifyEvent m_onDisconnect;
	SocketErrorEvent m_onError;

	void OnConnect(CZQ_CustomIocpClientSocket* client);
	void OnDisconnect(CZQ_CustomIocpClientSocket* client);
	void OnError(CZQ_CustomIocpClientSocket* client, int& ErrorCode);



	
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnClose();
};

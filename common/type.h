#pragma once
#include <windef.h>
#define MAX_GROUP_COUNT							32     // ÿ��������������
#define MAX_DBSERVER_COUNT					30     // ÿ��LoginGate�����ӵ�����DBServer����
#define MAX_AREA_COUNT								300    // ���ɺϲ���������

#define WM_UPDATE_HUMAN_COUNT			($400 + 200) // ���·���������  wParam=PGS_Human_Count, lParam=����
#define WM_DEBUG_MESSAGE						($400 + 201) // ������Ϣ���    wParam=PChar��Ϣ
#define WM_NETWORK_MESSAGE					($400 + 205)
#define WM_CONNECT_COUNT						($400 + 202) // Client����״̬  wParam=������, lParam=���ӳ�
#define WM_RESTORE_WINDOW					($400 + 203) // ���ڻ�ԭ
#define WM_GROUP_DISCONNECT					($400 + 204) // DB�ж�����

#define CONFIG_FILE_NAME							"LoginGate.ini"
#define IPADDRESS_CONFIG							"IpAddress.txt"
#define  IM_SELECT_SERVER_REQ					10000



#define    LM_DYN_ENCRYPT_CODE  23     // ����Ǽ��ܵ�
#define    LM_GET_ENCRYPT            24    // ȡ��̬�ӽ��ܳ���
#define    SM_SERVER_LIST              4001  // �������б�
#define    CM_SELECT_SERVER         4002  // ѡ�������
#define    SM_SELECT_SERVER         4002   // ѡ�����������
#define    SM_Collect_ServerInfo      22968			     

#define    SM_SELCHR                     4017  // ����ѡ���ɫ������Ϸ

#define    SM_HACKER_CHARACTOR   499     // ��������

#define    SM_MERCHANT_QUERY       2831   // NPC�����������
#define    CM_MERCHANT_QUERY       1110  // ����

#define     SEGMENTATION_IDENT     0xFF44FF44
#define     SEGMENTATION_SIGN       0xAA55AA55

#define     GDM_PING      1000
#define     GDM_SELECT_SERVER    1001
#define     GDM_PIG_MESSAGE      1002   // PIG��������Ϣ

#define     DGM_PING        2000
#define     DGM_SELECT_SERVER     2001
#define     DGM_SDOA_OPEN         2002   // ��Э�����SDOA�Ŀ���
#define     DGM_SDOA_CLOSE        2003

#define     DGM_DirectStaticAuth     2011   // ֱ�Ӿ�̬������֤
#define     MAX_RECEIVE_LENGTH   256
#define     ID_PK_WARNING            4030


typedef struct tagServerInfo
{
	std::string Serverinfo;
	unsigned long LastUpdateTick;
		int             Humancount;
}ServerInfo,*pServerInfo;
typedef struct tagDefaultMessage
{
	int                    Recog;
	WORD              Ident;
	WORD              Param;
	WORD              Tag;
	WORD               Series;


}DefaultMessage,*pDefaultMessage;

typedef struct tagClientMeesage
{
	unsigned long    Sign;
	BYTE                 ReservByte;
	BYTE                 Cmd;
	WORD               DataLength;
	unsigned long    DataIndex;


}ClientMessage, *pClientMessage;
typedef struct SelectGroupInfo
{
	unsigned long ciSessionID;
	int					  iencodeidx;
	WORD            wSocketHandle;
	WORD            wGatePort;
	unsigned long ciGateIP;
	WORD            wAreaID;
	byte                bGroupNo;
	byte                bErrorType;
	char                szPostfix[8];
}*pSelectGroupinfo;



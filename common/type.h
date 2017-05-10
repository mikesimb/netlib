#pragma once
#include <windef.h>
#define MAX_GROUP_COUNT							32     // 每个区最多的组数量
#define MAX_DBSERVER_COUNT					30     // 每个LoginGate可连接的最多的DBServer数量
#define MAX_AREA_COUNT								300    // 最多可合并的区数量

#define WM_UPDATE_HUMAN_COUNT			($400 + 200) // 更新服务器人数  wParam=PGS_Human_Count, lParam=组编号
#define WM_DEBUG_MESSAGE						($400 + 201) // 调试信息输出    wParam=PChar信息
#define WM_NETWORK_MESSAGE					($400 + 205)
#define WM_CONNECT_COUNT						($400 + 202) // Client连接状态  wParam=并发数, lParam=连接池
#define WM_RESTORE_WINDOW					($400 + 203) // 窗口还原
#define WM_GROUP_DISCONNECT					($400 + 204) // DB中断连接

#define CONFIG_FILE_NAME							"LoginGate.ini"
#define IPADDRESS_CONFIG							"IpAddress.txt"
#define  IM_SELECT_SERVER_REQ					10000



#define    LM_DYN_ENCRYPT_CODE  23     // 封包是加密的
#define    LM_GET_ENCRYPT            24    // 取动态加解密程序
#define    SM_SERVER_LIST              4001  // 服务器列表
#define    CM_SELECT_SERVER         4002  // 选择服务器
#define    SM_SELECT_SERVER         4002   // 选择服务器返回
#define    SM_Collect_ServerInfo      22968			     

#define    SM_SELCHR                     4017  // 返回选择角色进入游戏

#define    SM_HACKER_CHARACTOR   499     // 特征码检测

#define    SM_MERCHANT_QUERY       2831   // NPC发出的输入框
#define    CM_MERCHANT_QUERY       1110  // 返回

#define     SEGMENTATION_IDENT     0xFF44FF44
#define     SEGMENTATION_SIGN       0xAA55AA55

#define     GDM_PING      1000
#define     GDM_SELECT_SERVER    1001
#define     GDM_PIG_MESSAGE      1002   // PIG服务器信息

#define     DGM_PING        2000
#define     DGM_SELECT_SERVER     2001
#define     DGM_SDOA_OPEN         2002   // 由协议控制SDOA的开关
#define     DGM_SDOA_CLOSE        2003

#define     DGM_DirectStaticAuth     2011   // 直接静态密码认证
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




// ShortMessageDlg.h : 头文件
//

#pragma once

#include "SmsTraffic.h"
#include "atltime.h"

#include <Windows.h>
#include <wininet.h>
#define MAXBLOCKSIZE 1024
#pragma   comment   (lib,   "wininet.lib")

#include "mysql.h"
#pragma comment(lib,"libmySQL.lib")

//#define	WM_RECVDATA		WM_USER+1
struct RECVPARAM
{
	SOCKET sock;
	HWND hwnd;
};

// CShortMessageDlg 对话框
class CShortMessageDlg : public CDialogEx
{
// 构造
public:
	CShortMessageDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CShortMessageDlg();	// 标准析构函数

// 对话框数据
	enum { IDD = IDD_SHORTMESSAGE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CSmsTraffic* m_pSmsTraffic;
	
	CString m_sComm;
	CString m_sDb;
	CString m_sDbIp;
	UINT m_nDbPort;
	CString m_sDbPassword;
	CString m_sDbUser;
	UINT m_nMaxContent;
	UINT m_nMaxMessage;
	UINT m_nTimeRead;
	CString m_sSmsc;
	UINT m_nTimeWrite;
	CString m_sPhpPassword;
	UINT m_nPort;
	CString m_sPhpUrl;

	CString m_sNumber;
	CString m_sContent;
	bool m_bComm;
	bool m_bDb;
	afx_msg void OnBnClickedBtnConnect();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	void MoveCursor(void);
	void ShowLog(CString str);
	CTime m_time;
	CString m_sLog;
	CString m_temp;
	afx_msg void OnBnClickedBtnReset();
	afx_msg void OnBnClickedBtnSend();
	NOTIFYICONDATA m_nid;
	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);
	bool SendShortMessage(CString number, CString content);
	bool InitSocket(void);
	//afx_msg LRESULT OnRecvData(WPARAM wParam,LPARAM lParam);
	static DWORD WINAPI RecvProc(LPVOID lpParameter);

private:
	SOCKET m_socket;
	MYSQL mysql;
public:
	CString GetHtml(CString url);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedCancel();
	bool CheckDbConneted(void);
	void Send(void);
	void Recieve(void);
	void SetConfig(void);
	void GetConfig(void);
	afx_msg void OnBnClickedBtnConfig();
	void UpdateListSend(void);
	CListCtrl m_cListSend;
	CString m_sDbTime;
	afx_msg void OnBnClickedBtnCheckDb();
	bool Query(CString sql);
	int m_nSendCount;
	CString m_sSub;
	void UpdateSubMessage(void);
	afx_msg void OnBnClickedBtnSendAll();
	CString M2W(LPCSTR lpcszStr);
	afx_msg void OnBnClickedBtnReadAll();
	UINT GetCurrentTime(void);
};

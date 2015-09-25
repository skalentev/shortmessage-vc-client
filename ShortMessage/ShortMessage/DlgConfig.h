#pragma once


// CDlgConfig 对话框
class CShortMessageDlg;
class CDlgConfig : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgConfig)

public:
	CDlgConfig(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgConfig();

// 对话框数据
	enum { IDD = IDD_DLG_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CShortMessageDlg* m_pShortMessage;
	CString m_scComm;
	CString m_scDb;
	CString m_scDbIp;
	CString m_scDbPassword;
	CString m_scDbUser;
	UINT m_ncMaxContent;
	UINT m_ncMaxMessage;
	UINT m_ncTimeRead;
	CString m_scSmsc;
	UINT m_ncTimeWrite;
	UINT m_ncPort;
	virtual BOOL OnInitDialog();
	UINT m_ncDbPort;
	CString m_scPhpUrl;
};

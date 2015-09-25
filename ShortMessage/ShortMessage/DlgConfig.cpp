// DlgConfig.cpp : 实现文件
//

#include "stdafx.h"
#include "ShortMessage.h"
#include "DlgConfig.h"
#include "afxdialogex.h"

#include "ShortMessageDlg.h"
// CDlgConfig 对话框

IMPLEMENT_DYNAMIC(CDlgConfig, CDialogEx)

CDlgConfig::CDlgConfig(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgConfig::IDD, pParent)
{
	m_pShortMessage = NULL;
	m_scPhpUrl = _T("");
}

CDlgConfig::~CDlgConfig()
{
}

void CDlgConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMM, m_scComm);
	DDX_Text(pDX, IDC_EDIT_DB, m_scDb);
	DDX_Text(pDX, IDC_EDIT_DB_IP, m_scDbIp);
	DDX_Text(pDX, IDC_EDIT_DB_PASSWORD, m_scDbPassword);
	DDX_Text(pDX, IDC_EDIT_DB_USER, m_scDbUser);
	DDX_Text(pDX, IDC_EDIT_MAX_CONTENT, m_ncMaxContent);
	DDV_MinMaxUInt(pDX, m_ncMaxContent, 1, 70);
	DDX_Text(pDX, IDC_EDIT_MAX_MESSAGE, m_ncMaxMessage);
	DDX_Text(pDX, IDC_EDIT_READ, m_ncTimeRead);
	DDX_Text(pDX, IDC_EDIT_SMSC, m_scSmsc);
	DDX_Text(pDX, IDC_EDIT_WRITE, m_ncTimeWrite);
	DDX_Text(pDX, IDC_EDIT_PORT, m_ncPort);
	DDX_Text(pDX, IDC_EDIT_DB_PORT, m_ncDbPort);
	DDX_Text(pDX, IDC_EDIT_PHP_URL, m_scPhpUrl);
}


BEGIN_MESSAGE_MAP(CDlgConfig, CDialogEx)
END_MESSAGE_MAP()


// CDlgConfig 消息处理程序


BOOL CDlgConfig::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_scComm = m_pShortMessage->m_sComm;
	m_scDb = m_pShortMessage->m_sDb;
	m_scDbIp = m_pShortMessage->m_sDbIp;
	m_ncDbPort = m_pShortMessage->m_nDbPort;
	m_scDbPassword = m_pShortMessage->m_sDbPassword;
	m_scDbUser = m_pShortMessage->m_sDbUser;
	m_ncMaxContent = m_pShortMessage->m_nMaxContent;
	m_ncMaxMessage = m_pShortMessage->m_nMaxMessage;
	m_ncTimeRead = m_pShortMessage->m_nTimeRead;
	m_scSmsc = m_pShortMessage->m_sSmsc;
	m_ncTimeWrite = m_pShortMessage->m_nTimeWrite;
	m_ncPort = m_pShortMessage->m_nPort;
	m_scPhpUrl = m_pShortMessage->m_sPhpUrl;

	UpdateData(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

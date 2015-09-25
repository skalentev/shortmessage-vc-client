
// ShortMessageDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ShortMessage.h"
#include "ShortMessageDlg.h"
#include "afxdialogex.h"
#include "DlgConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CShortMessageDlg 对话框


BOOL WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_OEMCP,NULL,lpcwszStr,-1,NULL,0,NULL,FALSE);
	/*if(dwSize < dwMinSize)
	{
		return FALSE;
	}*/
	WideCharToMultiByte(CP_OEMCP,NULL,lpcwszStr,-1,lpszStr,dwSize,NULL,FALSE);
	return TRUE;
 }

BOOL MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize)
{
    DWORD dwMinSize;
    dwMinSize = MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, NULL, 0);
    if(dwSize < dwMinSize)
    {
		return FALSE;
    }
    MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);  
	return TRUE;
}

const int MESSAGE_TIMER_READ = 20101111;//读取间隔
const int MESSAGE_TIMER_WRITE = 20101112;//写入间隔
const int MESSAGE_TIMER_SENDCOUNT = 20101113;//统计周期
void *pShortMessage;
#define UM_TRAYNOTIFY WM_USER + 11

CShortMessageDlg::CShortMessageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CShortMessageDlg::IDD, pParent)
	, m_pSmsTraffic(NULL)
	, m_sNumber(_T(""))
	, m_sContent(_T(""))
	, m_bComm(false)
	, m_bDb(false)
	, m_time(0)
	, m_sLog(_T(""))
	, m_temp(_T(""))
	, m_sPhpPassword(_T("http://www.iisquare.com/"))
	, m_nSendCount(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	memset(&m_nid, 0, sizeof(m_nid)); // Initialize NOTIFYICONDATA struct
	m_nid.cbSize = sizeof(m_nid);
	m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	m_pSmsTraffic = new CSmsTraffic;
	m_sDbTime = _T("0000-00-00 00:00:00");
	m_sSub = _T("0");
}

CShortMessageDlg::~CShortMessageDlg()
{
	KillTimer(MESSAGE_TIMER_SENDCOUNT);
	if (m_pSmsTraffic != NULL)
	{
		delete m_pSmsTraffic;
	}
	if(m_bComm){
		KillTimer(MESSAGE_TIMER_WRITE);
		KillTimer(MESSAGE_TIMER_READ);
		CloseComm();
	}
	m_nid.hIcon = NULL;
	Shell_NotifyIcon(NIM_DELETE, &m_nid);

	mysql_close(&mysql);
}

void CShortMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NUMBER, m_sNumber);
	DDX_Text(pDX, IDC_EDIT_CONTENET, m_sContent);
	DDX_Text(pDX, IDC_EDIT_LOG, m_sLog);
	DDX_Control(pDX, IDC_LIST_SEND, m_cListSend);
	DDX_Text(pDX, IDC_EDIT_DB_TIME, m_sDbTime);
	DDX_Text(pDX, IDC_EDIT_SUB, m_sSub);
}

BEGIN_MESSAGE_MAP(CShortMessageDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CShortMessageDlg::OnBnClickedBtnConnect)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_RESET, &CShortMessageDlg::OnBnClickedBtnReset)
	ON_BN_CLICKED(IDC_BTN_SEND, &CShortMessageDlg::OnBnClickedBtnSend)
	ON_MESSAGE(UM_TRAYNOTIFY, &CShortMessageDlg::OnTrayNotify)
	//ON_MESSAGE(WM_RECVDATA,OnRecvData)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDCANCEL, &CShortMessageDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_CONFIG, &CShortMessageDlg::OnBnClickedBtnConfig)
	ON_BN_CLICKED(IDC_BTN_CHECK_DB, &CShortMessageDlg::OnBnClickedBtnCheckDb)
	ON_BN_CLICKED(IDC_BTN_SEND_ALL, &CShortMessageDlg::OnBnClickedBtnSendAll)
	ON_BN_CLICKED(IDC_BTN_READ_ALL, &CShortMessageDlg::OnBnClickedBtnReadAll)
END_MESSAGE_MAP()


// CShortMessageDlg 消息处理程序

BOOL CShortMessageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	m_nid.hWnd = GetSafeHwnd();
	m_nid.uCallbackMessage = UM_TRAYNOTIFY;

	// Set tray icon and tooltip
	m_nid.hIcon = m_hIcon;

	// Set tray notification tip information
	CString strToolTip = _T("互联信息系统客户端");
	_tcsncpy_s(m_nid.szTip, strToolTip, strToolTip.GetLength());
	Shell_NotifyIcon(NIM_ADD, &m_nid);

	pShortMessage = this;

	GetConfig();

	if(!AfxSocketInit())
	{
		AfxMessageBox(_T("加载套接字库失败！"));
		return FALSE;
	}
	InitSocket();
	RECVPARAM *pRecvParam=new RECVPARAM;
	pRecvParam->sock=m_socket;
	pRecvParam->hwnd=m_hWnd;
	HANDLE hThread=CreateThread(NULL,0,RecvProc,(LPVOID)pRecvParam,0,NULL);
	CloseHandle(hThread);

	mysql_init(&mysql);
	char ip[20]= {0};
	char user[20]= {0};
	char password[20]= {0};
	char db[20]= {0};
	WCharToMByte(m_sDbIp.GetBuffer(0),ip,m_sDbIp.GetLength() + 1/sizeof(ip[0]));
	WCharToMByte(m_sDbUser.GetBuffer(0),user,m_sDbUser.GetLength() + 1/sizeof(user[0]));
	WCharToMByte(m_sDbPassword.GetBuffer(0),password,m_sDbPassword.GetLength() + 1/sizeof(password[0]));
	WCharToMByte(m_sDb.GetBuffer(0),db,m_sDb.GetLength() + 1/sizeof(db[0]));

	if(mysql_real_connect(&mysql,ip,user,password,db,m_nDbPort,NULL,0)){
		CheckDbConneted();
		if(!Query(_T("set names 'gb2312'"))){
			AfxMessageBox(_T("选择字符集失败！"));
		};
	}else{
		AfxMessageBox(_T("数据库连接失败！")); 
		return FALSE;
	}

	m_cListSend.SetExtendedStyle(LVS_EX_FLATSB|LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_ONECLICKACTIVATE|LVS_EX_GRIDLINES);
	m_cListSend.InsertColumn(1,_T("手机号码"),LVCFMT_CENTER,120);
	m_cListSend.InsertColumn(2,_T("短信内容"),LVCFMT_CENTER,330);

	if(!SetTimer(MESSAGE_TIMER_SENDCOUNT,m_nTimeWrite * 1000,NULL)){
		AfxMessageBox(_T("统计时钟设置失败！")); 
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CShortMessageDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CShortMessageDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



LRESULT CShortMessageDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam)
{
 UINT uMsg = (UINT)lParam;

 switch(uMsg)
 {
 case WM_RBUTTONUP:
  {
   //右键处理
  CMenu menuTray;
  CPoint point;
  int id;
  GetCursorPos(&point);
  
  menuTray.LoadMenu(IDR_MENU_TRAY);
  id = menuTray.GetSubMenu(0)->TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
#if 0
  CString strInfo;
  strInfo.Format(L"menuid %d", id);
  LPCTSTR strtmp;
  strtmp = strInfo.GetBuffer(0);
  MessageBox(strtmp, L"test");
#endif
  switch(id){
   case IDR_TRAY_EXIT:
    //OnOK();
	OnBnClickedCancel();
    break;
   case IDR_TRAY_RESTORE:

    //窗口前端显示
    SetForegroundWindow();
    ShowWindow(SW_SHOWNORMAL);
    break;
   default:
    break;
  }
  break;
  }
 case WM_LBUTTONDBLCLK:
  SetForegroundWindow();
  ShowWindow(SW_SHOWNORMAL);
  break;
 default:
  break;
 }
 return 0;
}

void CShortMessageDlg::OnBnClickedBtnConnect()
{
	UpdateData();
	if(m_bComm){
		if(CloseComm()){
			KillTimer(MESSAGE_TIMER_WRITE);
			KillTimer(MESSAGE_TIMER_READ);
			m_bComm = false;
			GetDlgItem(IDC_BTN_CONNECT)->SetWindowTextW(_T("连接"));
			m_temp.Format(_T("端口%s已关闭！"),m_sComm);
			this->ShowLog(m_temp);
		}else{
			m_temp.Format(_T("端口%s关闭异常！"),m_sComm);
			MessageBox(m_temp);
		}
	}else{
		if(OpenComm((LPSTR)(LPCTSTR)m_sComm,9600)){
			if(gsmInit()){
				SetTimer(MESSAGE_TIMER_READ,m_nTimeRead * 1000,NULL);
				SetTimer(MESSAGE_TIMER_WRITE,(int)((m_nTimeWrite * 1000) / m_nMaxMessage),NULL);
				m_temp.Format(_T("成功连接至%s端口..."),m_sComm);
				this->ShowLog(m_temp);
				m_bComm = true;
				GetDlgItem(IDC_BTN_CONNECT)->SetWindowTextW(_T("断开"));
				UpdateSubMessage();
			}else{
				m_temp.Format(_T("端口%s上没有发现MODEM!"),m_sComm);
				MessageBox(m_temp);
			}
		}else{
			m_temp.Format(_T("无法打开%s端口！"),m_sComm);
			MessageBox(m_temp);
		}
	}
	UpdateData(FALSE);
}


void CShortMessageDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == MESSAGE_TIMER_SENDCOUNT){
		m_nSendCount = 0;
		UpdateSubMessage();
	}
	if(nIDEvent == MESSAGE_TIMER_WRITE){
		Send();
	}
	if(nIDEvent == MESSAGE_TIMER_READ){
		Recieve();
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CShortMessageDlg::MoveCursor(void)
{
	CEdit *pEdit = (CEdit *)GetDlgItem(IDC_EDIT_LOG); 
	if(pEdit)
	{ 
		  pEdit-> SetFocus(); 
		  pEdit-> SetSel(m_sLog.GetLength(),m_sLog.GetLength(),FALSE); 
	}
}


void CShortMessageDlg::ShowLog(CString str)
{
	UpdateData();
	m_time = CTime::GetCurrentTime();
	str.Format(_T("[%d-%d-%d %d:%d:%d]") + str + _T("\r\n"),m_time.GetYear(),m_time.GetMonth(),m_time.GetDay(),m_time.GetHour(),m_time.GetMinute(),m_time.GetSecond());
	if(m_sLog.GetLength() > 20*1024){
		m_sLog = str;
	}else{
		m_sLog += str;
	}
	UpdateData(FALSE);
	this->MoveCursor();
}


void CShortMessageDlg::OnBnClickedBtnReset()
{
	m_sNumber = _T("");
	m_sContent = _T("");
	UpdateData(FALSE);
}


void CShortMessageDlg::OnBnClickedBtnSend()
{
	UpdateData();
	if(!SendShortMessage(m_sNumber,m_sContent))MessageBox(_T("短信发送失败，请稍后再试！"));;
}


bool CShortMessageDlg::SendShortMessage(CString number, CString content)
{
	CString smsc = m_sSmsc;
	if(!m_bComm){
		m_temp.Format(_T("短信猫未启动，短信无法发送！"));
		this->ShowLog(m_temp);
		return false;
	}
	if(smsc.GetLength() < 1){
		m_temp.Format(_T("短信中心号码不合法，短信无法发送！"));
		this->ShowLog(m_temp);
		return false;
	}

	SM_PARAM SmParam;
	memset(&SmParam, 0, sizeof(SM_PARAM));

	// 去掉号码前的"+"
	if(smsc[0] == '+')  smsc = smsc.Mid(1);
	if(number[0] == '+')  number = number.Mid(1);

	// 在号码前加"86"
	if(smsc.Left(2) != _T("86") && smsc.GetLength() >= 11)  smsc = _T("86") + smsc;
	if(number.Left(2) != _T("86") && number.GetLength() >= 11)  number = _T("86") + number;
	
	// 填充短消息结构
	int i;
	for(i = 0;i < smsc.GetLength();i++)
	{
		SmParam.SCA[i] = smsc.GetAt(i);
	}
	for(i = 0;i < number.GetLength();i++)
	{
		SmParam.TPA[i] = number.GetAt(i);
	}

	SmParam.TP_PID = 0;
	SmParam.TP_DCS = GSM_UCS2;

	// 发送短消息
	int count = content.GetLength() / m_nMaxContent + 1;
	int sub = content.GetLength() % m_nMaxContent;
	if(count > 1 && sub == 0)count--;

	if(m_nSendCount + count > m_nMaxMessage){
		return false;
	}

	m_temp.Format(_T("发送至%s，信息内容：%s"),number,content);
	this->ShowLog(m_temp);

	CTime t = CTime::GetCurrentTime();
	SmParam.TP_ID = t.GetSecond() + t.GetMinute();
	CString str;
	for(i = 0; i < count; i++)
	{
		m_nSendCount++;
		if(i == count){
			str = content.Mid(i * m_nMaxContent, sub);
		}else{
			str = content.Mid(i * m_nMaxContent, m_nMaxContent);
		}
		WideCharToMultiByte(CP_ACP,0,str, str.GetLength() + 1,SmParam.TP_UD,160 ,NULL,NULL);
		SmParam.TP_COUNT = count;
		SmParam.TP_CURRENT =  i + 1;
		m_pSmsTraffic->PutSendMessage(&SmParam);
	}
	UpdateSubMessage();
	return true;
}


bool CShortMessageDlg::InitSocket(void)
{
	m_socket=socket(AF_INET,SOCK_DGRAM,0);
	if(INVALID_SOCKET==m_socket)
	{
		MessageBox(_T("套接字创建失败！"));
		return false;
	}
	SOCKADDR_IN addrSock;
	addrSock.sin_family=AF_INET;
	addrSock.sin_port=htons(6000);
	addrSock.sin_addr.S_un.S_addr=htonl(INADDR_ANY);

	int retval;
	retval=bind(m_socket,(SOCKADDR*)&addrSock,sizeof(SOCKADDR));
	if(SOCKET_ERROR==retval)
	{
		closesocket(m_socket);
		MessageBox(_T("绑定失败!"));
		return false;
	}
	return true;
}

DWORD WINAPI CShortMessageDlg::RecvProc(LPVOID lpParameter)
{
	SOCKET sock=((RECVPARAM*)lpParameter)->sock;
	HWND hwnd=((RECVPARAM*)lpParameter)->hwnd;
	delete lpParameter;

	CShortMessageDlg *shortMessage = (CShortMessageDlg *)pShortMessage;

	SOCKADDR_IN addrFrom;
	int len=sizeof(SOCKADDR);

	char recvBuf[200];
	char tempBuf[300];
	memset(recvBuf,0,sizeof(recvBuf));
	memset(tempBuf,0,sizeof(tempBuf));
	int retval;
	CString str;
	WCHAR wc[200];
	while(TRUE)
	{
		retval=recvfrom(sock,recvBuf,200,0,(SOCKADDR*)&addrFrom,&len);
		if(SOCKET_ERROR==retval)break;
		sprintf(tempBuf,"来自%s的消息:%s",inet_ntoa(addrFrom.sin_addr),recvBuf);
		//::PostMessage(hwnd,WM_RECVDATA,0,(LPARAM)tempBuf);
		::MultiByteToWideChar(CP_ACP,0,(LPSTR)tempBuf,-1,wc,200);
		str.Format(_T("%s"),wc);
		if(str.Find(shortMessage->m_sPhpPassword) != -1)
		{
			str.Replace(shortMessage->m_sPhpPassword,_T("-触发指令.发送短信-"));
			//shortMessage->Send();
		}
		shortMessage->ShowLog(str);
	}
	return 0;
}

CString CShortMessageDlg::GetHtml(CString url)
{
	try{
		CString str;
		//char *Url = url.GetBuffer(url.GetLength());
		HINTERNET hSession = InternetOpen(_T("RookIE/1.0"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (hSession != NULL)
		{
			HINTERNET handle2 = InternetOpenUrl(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
			if (handle2 != NULL)
			{
				byte Temp[MAXBLOCKSIZE];
				memset(Temp,0,sizeof(Temp));
				ULONG Number = 1;
				while (Number > 0)
				{
					InternetReadFile(handle2, Temp, MAXBLOCKSIZE - 1, &Number);
					CString gb(Temp);
					str += gb;
				}
				InternetCloseHandle(handle2);
				handle2 = NULL;
			}
			InternetCloseHandle(hSession);
			hSession = NULL;
		}
		return str.Left(str.GetLength()/2);
	}catch(int i){
		return _T("No Response!");
	}
}


void CShortMessageDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if(nType == SIZE_MINIMIZED){
		ShowWindow(SW_HIDE);
	}
}


void CShortMessageDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


bool CShortMessageDlg::CheckDbConneted(void)
{
	if(mysql_stat(&mysql)){
		m_time = CTime::GetCurrentTime();
		m_sDbTime.Format(_T("%d-%d-%d %d:%d:%d"),m_time.GetYear(),m_time.GetMonth(),m_time.GetDay(),m_time.GetHour(),m_time.GetMinute(),m_time.GetSecond());
		UpdateData(FALSE);
		if(!m_bDb){
			m_bDb = true;
			this->ShowLog(_T("数据库连接成功！"));
		}
	}else{
		if(m_bDb){
			m_bDb = false;
			this->ShowLog(_T("数据库连接失败！"));
		}
		return false;
	}
	return true;
}


void CShortMessageDlg::Send(void)
{
	UpdateListSend();
	if(!m_bComm)return;
	if(!CheckDbConneted())return;
	CString sql,str;
	sql = _T("select * from `message_shortmessage` where `sm_rors` = 1 and `sm_state` = 0 limit 1");
	Query(sql);
	MYSQL_RES *result = mysql_use_result(&mysql);
	MYSQL_ROW row;
	CString condition = _T("`sm_id` = 0");
	while(row = mysql_fetch_row(result))
	{
		if(!SendShortMessage(CString(row[1]), CString(row[2])))break;
		str.Format(_T(" or `sm_id` = %d"),atoi(row[0]));
		condition += str;
	}
	mysql_free_result(result);
	sql.Format(_T("update `message_shortmessage` set `sm_state`=1,`sm_dtime`=%d where %s"),GetCurrentTime(),condition);
	Query(sql);
}


void CShortMessageDlg::Recieve(void)
{
	UpdateListSend();
	if(!m_bComm)return;
	if(!CheckDbConneted())return;
	SM_PARAM SmParam;
	CString strTime;
	CString strNumber;
	CString strContent;

	// 取接收到的短消息
	while(m_pSmsTraffic->GetRecvMessage(&SmParam))
	{
		strNumber = SmParam.TPA;
		strContent = SmParam.TP_UD;
		strTime = _T("20") + CString(&SmParam.TP_SCTS[0],2) 
			+ _T("-") + CString(&SmParam.TP_SCTS[2],2) 
			+ _T("-") + CString(&SmParam.TP_SCTS[4],2)
			+ _T(" ") + CString(&SmParam.TP_SCTS[6],2) 
			+ _T(":") + CString(&SmParam.TP_SCTS[8],2) 
			+ _T(":") + CString(&SmParam.TP_SCTS[10],2);

		// 去掉号码前的"86"
		if(strNumber.Left(2) == "86")  strNumber = strNumber.Mid(2);
		m_temp.Format(_T("于%s时接收到来自%s的信息：%s"),strTime,strNumber,strContent);
		this->ShowLog(m_temp);
		CString url;
		url = m_sPhpUrl + "cpp/get.php?pw=iisquare.com&n=" + strNumber + "&m=" + strContent + "&t=" + strTime;
		ShowLog(GetHtml(url));
	}
}


void CShortMessageDlg::SetConfig(void)
{
	CDlgConfig dlg;
	dlg.m_pShortMessage = this;
	if(dlg.DoModal() == IDOK){
		CString uri = _T(".\\config.ini");
		CString fm;
		WritePrivateProfileString(_T("COM"), _T("comPort") ,dlg.m_scComm, uri);
		fm.Format(_T("%d"),dlg.m_ncTimeRead);
		WritePrivateProfileString(_T("COM"),_T("timeRead"),fm,uri);
		fm.Format(_T("%d"),dlg.m_ncTimeWrite);
		WritePrivateProfileString(_T("COM"),_T("timeWrite"),fm,uri);

		WritePrivateProfileString(_T("ShortMessage"), _T("smsc") , dlg.m_scSmsc, uri);
		fm.Format(_T("%d"),dlg.m_ncMaxContent);
		WritePrivateProfileString(_T("ShortMessage"),_T("maxContent"),fm,uri);
		fm.Format(_T("%d"),dlg.m_ncMaxMessage);
		WritePrivateProfileString(_T("ShortMessage"),_T("maxMessage"),fm,uri);

		WritePrivateProfileString(_T("DataBase"), _T("ip") , dlg.m_scDbIp, uri);
		fm.Format(_T("%d"),dlg.m_ncDbPort);
		WritePrivateProfileString(_T("DataBase"),_T("port"),fm,uri);
		WritePrivateProfileString(_T("DataBase"), _T("user") , dlg.m_scDbUser, uri);
		WritePrivateProfileString(_T("DataBase"), _T("password") , dlg.m_scDbPassword, uri);
		WritePrivateProfileString(_T("DataBase"), _T("db") , dlg.m_scDb, uri);
		
		fm.Format(_T("%d"),dlg.m_ncPort);
		WritePrivateProfileString(_T("Client"),_T("port"),fm,uri);

		WritePrivateProfileString(_T("PHP"), _T("url") , dlg.m_scPhpUrl, uri);

		MessageBox(_T("配置成功，下次启动时生效！"));
	}
}


void CShortMessageDlg::GetConfig(void)
{
	CString uri = _T(".\\config.ini");
	
	GetPrivateProfileString(_T("COM"), _T("comPort") , _T("COM3"),m_sComm.GetBufferSetLength(MAX_PATH), MAX_PATH, uri);
	m_sComm.ReleaseBuffer();
	m_nTimeRead = GetPrivateProfileInt(_T("COM"),_T("timeRead"),200,uri);
	m_nTimeWrite = GetPrivateProfileInt(_T("COM"),_T("timeWrite"),3600,uri);

	GetPrivateProfileString(_T("ShortMessage"), _T("smsc") , _T("13800534500"),m_sSmsc.GetBufferSetLength(MAX_PATH), MAX_PATH, uri);
	m_sSmsc.ReleaseBuffer();
	m_nMaxContent = GetPrivateProfileInt(_T("ShortMessage"),_T("maxContent"),70,uri);
	m_nMaxMessage = GetPrivateProfileInt(_T("ShortMessage"),_T("maxMessage"),300,uri);

	GetPrivateProfileString(_T("DataBase"), _T("ip") , _T("localhost"),m_sDbIp.GetBufferSetLength(MAX_PATH), MAX_PATH, uri);
	m_sDbIp.ReleaseBuffer();
	m_nDbPort = GetPrivateProfileInt(_T("DataBase"),_T("port"),3306,uri);
	GetPrivateProfileString(_T("DataBase"), _T("user") , _T("root"),m_sDbUser.GetBufferSetLength(MAX_PATH), MAX_PATH, uri);
	m_sDbUser.ReleaseBuffer();
	GetPrivateProfileString(_T("DataBase"), _T("password") , _T("root"),m_sDbPassword.GetBufferSetLength(MAX_PATH), MAX_PATH, uri);
	m_sDbPassword.ReleaseBuffer();
	GetPrivateProfileString(_T("DataBase"), _T("db") , _T("database"),m_sDb.GetBufferSetLength(MAX_PATH), MAX_PATH, uri);
	m_sDb.ReleaseBuffer();;

	m_nPort = GetPrivateProfileInt(_T("Client"),_T("port"),6000,uri);

	GetPrivateProfileString(_T("PHP"), _T("url") , _T("http://localhost/"),m_sPhpUrl.GetBufferSetLength(MAX_PATH), MAX_PATH, uri);
	m_sPhpUrl.ReleaseBuffer();
}


void CShortMessageDlg::OnBnClickedBtnConfig()
{
	// TODO: 在此添加控件通知处理程序代码
	SetConfig();
}


void CShortMessageDlg::UpdateListSend(void)
{
	m_cListSend.DeleteAllItems();
	int n = 0;
	int i = m_pSmsTraffic->m_nSendIn;
	int j = m_pSmsTraffic->m_nSendOut;
	while (j != i)
	{
		m_cListSend.InsertItem(n,_T(""));
		m_cListSend.SetItemText(n,0,CString(m_pSmsTraffic->m_SmSend[j].TPA));
		m_cListSend.SetItemText(n,1,CString(m_pSmsTraffic->m_SmSend[j].TP_UD));
		j++;
		if (j >= MAX_SM_SEND)  j = 0;
		n++;
	}
	UpdateData(FALSE);
}


void CShortMessageDlg::OnBnClickedBtnCheckDb()
{
	// TODO: 在此添加控件通知处理程序代码
	CheckDbConneted();
}


bool CShortMessageDlg::Query(CString sql)
{
	char temp[10240]= {0};
	WCharToMByte(sql.GetBuffer(sql.GetLength()),temp,sql.GetLength() + 1 / sizeof(temp[0]));
	if(mysql_real_query(&mysql,temp,strlen(temp)) != 0){
		return false;
	};
	return true;
}


void CShortMessageDlg::UpdateSubMessage(void)
{
	m_sSub.Format(_T("%d"), m_nMaxMessage - m_nSendCount);
	UpdateData(FALSE);
}


void CShortMessageDlg::OnBnClickedBtnSendAll()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	Send();
}


CString CShortMessageDlg::M2W(LPCSTR lpcszStr)
{
	wchar_t wText[10240] = {L""};
	MByteToWChar(lpcszStr,wText,sizeof(wText)/sizeof(wText[0]));
	return CString(wText);
}


void CShortMessageDlg::OnBnClickedBtnReadAll()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	Recieve();
}


UINT CShortMessageDlg::GetCurrentTime(void)
{
	CTime start(1970,01,01,8,0,0);
	CTime end = CTime::GetCurrentTime();
	CTimeSpan span = end - start;
	return span.GetTotalSeconds();
}

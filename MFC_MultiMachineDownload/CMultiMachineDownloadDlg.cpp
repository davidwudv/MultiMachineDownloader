
// CMultiMachineDownloadDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFC_MultiMachineDownload.h"
#include "CMultiMachineDownloadDlg.h"
#include "afxdialogex.h"
#include "TaskConfig.h"
#include "DownloadTask.h"
#include "HttpDownload.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int Exist(CString strPath);
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMultiMachineDownloadDlg 对话框



CMultiMachineDownloadDlg::CMultiMachineDownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMultiMachineDownloadDlg::IDD, pParent)
	, m_strURL(_T(""))
	, m_strSavePath(_T(""))
	, m_iThreads(0)
	, m_strIP(_T(""))
	, m_pDownloadTask(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CMultiMachineDownloadDlg::~CMultiMachineDownloadDlg()
{
	if(m_pDownloadTask != nullptr)
		delete m_pDownloadTask;
}

void CMultiMachineDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_URL, m_strURL);
	DDX_Text(pDX, IDC_EDIT_SAVEPAHT, m_strSavePath);
	DDX_Text(pDX, IDC_EDIT_THREADS, m_iThreads);
	DDX_Text(pDX, IDC_EDIT_MACHINEIP, m_strIP);
	DDX_Control(pDX, IDC_DOWNLOAD_OUTPUT_LIST, m_cListBoxDownloadOutPut);
	DDX_Control(pDX, IDC_IP_LIST, m_cIPList);
	DDX_Control(pDX, IDC_BUTTON_DELETEIP, m_cButtonDeleteIP);
	DDX_Control(pDX, IDC_BUTTON_START, m_cButtonStart);
	DDX_Control(pDX, IDC_BUTTON_Stop, m_cButtonStop);
	DDX_Control(pDX, IDC_DOWNLOAD_PROGRESS, m_cProgress);
}

BEGIN_MESSAGE_MAP(CMultiMachineDownloadDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CMultiMachineDownloadDlg::OnClickedButtonStart)
	ON_MESSAGE(WM_USER_DOWNLOAD_FINISHED, OnDownloadFinished)
	ON_BN_CLICKED(IDC_BUTTON_ADDIP, &CMultiMachineDownloadDlg::OnBnClickedButtonAddip)
	ON_BN_CLICKED(IDC_BUTTON_DELETEIP, &CMultiMachineDownloadDlg::OnBnClickedButtonDeleteip)
	ON_LBN_SETFOCUS(IDC_IP_LIST, &CMultiMachineDownloadDlg::OnLbnSetfocusIpList)
	ON_LBN_SELCANCEL(IDC_IP_LIST, &CMultiMachineDownloadDlg::OnLbnSelcancelIpList)
	ON_BN_CLICKED(IDC_BUTTON_Stop, &CMultiMachineDownloadDlg::OnBnClickedButtonStop)
	ON_MESSAGE(WM_USER_DOWNLOAD_STOP, OnDownloadStop)
	ON_EN_CHANGE(IDC_EDIT_URL, &CMultiMachineDownloadDlg::OnEnChangeEditUrl)
END_MESSAGE_MAP()


// CMultiMachineDownloadDlg 消息处理程序

BOOL CMultiMachineDownloadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
		
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//m_strURL = "http://dldir1.qq.com/qqfile/qq/QQ2013/QQ2013SP3/8557/QQ2013SP3.exe";
	//m_strURL = "http://127.0.0.1/fuck.exe";
	m_strURL = "http://skype.tom.com/download/SkypeSetup.exe";
	//m_strSavePath = "D:\\QQ2013SP3.exe";
	m_strSavePath = "D:\\SkypeSetup.exe";
	m_iThreads = 2;
	m_cButtonDeleteIP.EnableWindow(FALSE);
	m_cButtonStop.EnableWindow(FALSE);
	m_cProgress.SetRange(0, 100);

	UpdateData(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMultiMachineDownloadDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMultiMachineDownloadDlg::OnPaint()
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
HCURSOR CMultiMachineDownloadDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMultiMachineDownloadDlg::OnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	TaskConfig* taskConfig(nullptr);
	int exist = Exist(m_strSavePath);
	if(exist == 0)
	{
		taskConfig = new TaskConfig();
		taskConfig->LoadFromFile(m_strSavePath + _T(".conf"));
	}
	else if(exist == 1)
	{
		AfxMessageBox(_T("此文件已经下载过了!\n"));
		return;
	}
	else
		taskConfig = new TaskConfig(m_strURL, m_iThreads);
	m_pDownloadTask = new DownloadTask(m_strSavePath, taskConfig, this);
	m_pDownloadTask->Start();
	m_cButtonStart.EnableWindow(FALSE);
	m_cButtonStop.EnableWindow(TRUE);
}


void CMultiMachineDownloadDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
	m_pDownloadTask->Stop();
}

LRESULT CMultiMachineDownloadDlg::OnDownloadFinished(WPARAM wParam, LPARAM lParam)
{
	DownloadTask* myTask = (DownloadTask*)(lParam);
	if(myTask->GetCurrentThreadSum() == 0)
		delete myTask;
	AfxMessageBox(_T("下载完成！"));
	/*******待修改*********/
	m_pDownloadTask = nullptr;

	m_cButtonStart.EnableWindow(TRUE);
	m_cButtonStop.EnableWindow(FALSE);
	return 0;
}

LRESULT CMultiMachineDownloadDlg::OnDownloadStop(WPARAM wParam, LPARAM lParam)
{
	DownloadTask* myTask = (DownloadTask*)(lParam);
	if(myTask->GetCurrentThreadSum() == 0)
	{
		delete myTask;
		/*******待修改*********/
		m_pDownloadTask = nullptr;
		m_cButtonStop.EnableWindow(FALSE);
		m_cButtonStart.EnableWindow(TRUE);
	}
	return 0;
}

void CMultiMachineDownloadDlg::OnBnClickedButtonAddip()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_cIPList.AddString(m_strIP);
	m_strIP.Empty();
	UpdateData(FALSE);
}


void CMultiMachineDownloadDlg::OnBnClickedButtonDeleteip()
{
	// TODO: 在此添加控件通知处理程序代码
	int index = m_cIPList.GetCurSel();
	m_cIPList.DeleteString(index);
	if(m_cIPList.GetCount() == 0)
		m_cButtonDeleteIP.EnableWindow(FALSE);
}


void CMultiMachineDownloadDlg::OnLbnSetfocusIpList()
{
	// TODO: 在此添加控件通知处理程序代码
	m_cButtonDeleteIP.EnableWindow(TRUE);
}

void CMultiMachineDownloadDlg::OnLbnSelcancelIpList()
{
	// TODO: 在此添加控件通知处理程序代码
	m_cButtonDeleteIP.EnableWindow(FALSE);
}




void CMultiMachineDownloadDlg::OnEnChangeEditUrl()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	/*CString strFileName;
	int nPos = m_strURL.ReverseFind('/');
	if(nPos > 0)
		strFileName = m_strURL.Right(m_strURL.GetLength() - nPos - 1);
	m_strSavePath += strFileName;
	UpdateData(FALSE);*/
}

/************************************************************************/
/* 返回0表示此文件曾经下载过但未下载完成；    */
/* 返回1表示此文件曾经下载过且已经下载完成；*/
/* 返回-1表示此文件从未下载过；                        */
/************************************************************************/
int Exist(CString strPath)
{
	CFileFind finder;
	if(finder.FindFile(strPath))
	{
		if(finder.FindFile(strPath + _T(".conf")))
			return 0;
		else
			return 1;
	}
	return -1;
}
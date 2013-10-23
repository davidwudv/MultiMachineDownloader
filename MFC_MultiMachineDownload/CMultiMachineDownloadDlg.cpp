
// CMultiMachineDownloadDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MFC_MultiMachineDownload.h"
#include "CMultiMachineDownloadDlg.h"
#include "afxdialogex.h"
#include "TaskConfigFile.h"
#include "HttpDownload.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CMultiMachineDownloadDlg �Ի���



CMultiMachineDownloadDlg::CMultiMachineDownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMultiMachineDownloadDlg::IDD, pParent)
	, m_strURL(_T(""))
	, m_strSavePath(_T(""))
	, m_iThreads(0)
	, m_strIP(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMultiMachineDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_URL, m_strURL);
	DDX_Text(pDX, IDC_EDIT_SAVEPAHT, m_strSavePath);
	DDX_Text(pDX, IDC_EDIT_THREADS, m_iThreads);
	DDX_Text(pDX, IDC_EDIT_MACHINEIP, m_strIP);
	DDX_Control(pDX, IDC_DOWNLOAD_OUTPUT_LIST, m_cListBoxDownloadOutPut);
}

BEGIN_MESSAGE_MAP(CMultiMachineDownloadDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CMultiMachineDownloadDlg::OnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_SUSPEND, &CMultiMachineDownloadDlg::OnBnClickedButtonSuspend)
	ON_MESSAGE(WM_USER_DOWNLOAD_FINISHED, OnDownloadFinished)
END_MESSAGE_MAP()


// CMultiMachineDownloadDlg ��Ϣ�������

BOOL CMultiMachineDownloadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_strURL = "http://dldir1.qq.com/qqfile/qq/QQ2013/QQ2013SP3/8557/QQ2013SP3.exe";
	m_strSavePath = "D:\\";
	m_iThreads = 5;
	UpdateData(FALSE);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMultiMachineDownloadDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMultiMachineDownloadDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMultiMachineDownloadDlg::OnClickedButtonStart()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
#ifdef DEBUG
	DWORD dwThreadID;
	dwThreadID = ::GetCurrentThreadId();
	TRACE("Thread %d download finished%d��\n", dwThreadID, dwThreadID);
#endif
	HttpDownload* myDownload = new HttpDownload(m_strURL, m_strSavePath, m_iThreads, this);
	myDownload->Download(nullptr);
}


void CMultiMachineDownloadDlg::OnBnClickedButtonSuspend()
{
	
}

LRESULT CMultiMachineDownloadDlg::OnDownloadFinished(WPARAM wParam, LPARAM lParam)
{
	HttpDownload* hd = (HttpDownload*)lParam;
	::AfxMessageBox(_T("������ɣ�"));
	delete hd;
	return 0;
}
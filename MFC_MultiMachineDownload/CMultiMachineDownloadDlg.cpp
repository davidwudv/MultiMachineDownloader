
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

UINT threadFun(LPVOID lParam);

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
	m_cButtonDeleteIP.EnableWindow(FALSE);
	m_cButtonStop.EnableWindow(FALSE);
	m_cProgress.SetRange(0, 100);

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
	CFileFind finder;
	 m_pDownloadTask = new HttpDownload(m_strURL, m_strSavePath, m_iThreads, this);
	AfxBeginThread(threadFun, m_pDownloadTask);
	m_cButtonStart.EnableWindow(FALSE);
	m_cButtonStop.EnableWindow(TRUE);
}


void CMultiMachineDownloadDlg::OnBnClickedButtonStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_pDownloadTask->m_Stop = TRUE;
	m_cButtonStop.EnableWindow(FALSE);
	m_cButtonStart.EnableWindow(TRUE);
}

LRESULT CMultiMachineDownloadDlg::OnDownloadFinished(WPARAM wParam, LPARAM lParam)
{
	HttpDownload* hd = (HttpDownload*)lParam;
	::AfxMessageBox(_T("������ɣ�"));
	m_cButtonStart.EnableWindow(TRUE);
	m_cButtonStop.EnableWindow(FALSE);
	delete hd;

	return 0;
}

LRESULT CMultiMachineDownloadDlg::OnDownloadStop(WPARAM wParam, LPARAM lParam)
{
	CFile configFile(m_pDownloadTask->m_strSavePath + ".conf", CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive);
	CArchive ar(&configFile, CArchive::store);
	m_pDownloadTask->m_pConfigInfo->Serialize(ar);//�����ļ���Ϣ
	HttpDownload* hd = (HttpDownload*)lParam;
	delete hd;

	return 0;
}

void CMultiMachineDownloadDlg::OnBnClickedButtonAddip()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	m_cIPList.AddString(m_strIP);
	m_strIP.Empty();
	UpdateData(FALSE);
}


void CMultiMachineDownloadDlg::OnBnClickedButtonDeleteip()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int index = m_cIPList.GetCurSel();
	m_cIPList.DeleteString(index);
	if(m_cIPList.GetCount() == 0)
		m_cButtonDeleteIP.EnableWindow(FALSE);
}


void CMultiMachineDownloadDlg::OnLbnSetfocusIpList()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_cButtonDeleteIP.EnableWindow(TRUE);
}


void CMultiMachineDownloadDlg::OnLbnSelcancelIpList()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_cButtonDeleteIP.EnableWindow(FALSE);
}

UINT threadFun(LPVOID lParam)
{
	HttpDownload* download = static_cast<HttpDownload*>(lParam);
	download->Download(nullptr);

	return 0;
}



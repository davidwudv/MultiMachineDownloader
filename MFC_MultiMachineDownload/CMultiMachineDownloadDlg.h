
// CMultiMachineDownloadDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "resource.h"
#include "DownloadTask.h"
#include "afxcmn.h"

#define WM_USER_DOWNLOAD_FINISHED WM_USER + 0x100 //�����������ʱ���͵���Ϣ
#define WM_USER_DOWNLOAD_STOP WM_USER + 0x200//ֹͣ��������ʱ���͵���Ϣ'
class HttpDownload;

// CMultiMachineDownloadDlg �Ի���
class CMultiMachineDownloadDlg : public CDialogEx
{
// ����
public:
	CMultiMachineDownloadDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CMultiMachineDownloadDlg();

// �Ի�������
	enum { IDD = IDD_MFC_MULTIMACHINEDOWNLOAD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnDownloadFinished(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDownloadStop(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	CString m_strURL;
	CString m_strSavePath;
	short m_iThreads;
	CString m_strIP;
	afx_msg void OnClickedButtonStart();
	CListBox m_cListBoxDownloadOutPut;

private:
	DownloadTask* m_pDownloadTask;
public:
	afx_msg void OnBnClickedButtonAddip();
	CListBox m_cIPList;
	afx_msg void OnBnClickedButtonDeleteip();
	CButton m_cButtonDeleteIP;
	afx_msg void OnLbnSetfocusIpList();
	afx_msg void OnLbnSelcancelIpList();
	CButton m_cButtonStart;
	afx_msg void OnBnClickedButtonStop();
	CButton m_cButtonStop;
	CProgressCtrl m_cProgress;
};


// CMultiMachineDownloadDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "resource.h"
#define WM_USER_DOWNLOAD_FINISHED WM_USER + 0x100 //�����������ʱ���͵���Ϣ


// CMultiMachineDownloadDlg �Ի���
class CMultiMachineDownloadDlg : public CDialogEx
{
// ����
public:
	CMultiMachineDownloadDlg(CWnd* pParent = NULL);	// ��׼���캯��

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
	DECLARE_MESSAGE_MAP()
public:
	CString m_strURL;
	CString m_strSavePath;
	short m_iThreads;
	CString m_strIP;
	afx_msg void OnClickedButtonStart();
	afx_msg void OnBnClickedButtonSuspend();
	CListBox m_cListBoxDownloadOutPut;

private:
	//HttpDownload* TaskList
};

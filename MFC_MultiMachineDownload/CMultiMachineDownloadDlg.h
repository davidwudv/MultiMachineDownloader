
// CMultiMachineDownloadDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"


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
	DECLARE_MESSAGE_MAP()
public:
	CString m_pstrURL;
	CString m_pstrSavePath;
	short m_iThreads;
	CString m_pstrIP;
	afx_msg void OnClickedButtonStart();
	afx_msg void OnBnClickedButtonSuspend();
	CListBox m_cListBoxDownloadOutPut;
};

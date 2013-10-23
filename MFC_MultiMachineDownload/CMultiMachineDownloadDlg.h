
// CMultiMachineDownloadDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "resource.h"
#define WM_USER_DOWNLOAD_FINISHED WM_USER + 0x100 //下载任务完成时发送的消息


// CMultiMachineDownloadDlg 对话框
class CMultiMachineDownloadDlg : public CDialogEx
{
// 构造
public:
	CMultiMachineDownloadDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MFC_MULTIMACHINEDOWNLOAD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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

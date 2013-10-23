#pragma once
#include "TaskConfigFile.h"
#include "CMultiMachineDownloadDlg.h"


class HttpDownload
{
public:
	//CHttpDownload(void);
	//CHttpDownload(CTaskConfigFile*, CString,CString, CString, DWORD, INTERNET_PORT);
	HttpDownload(CString strURL, CString strSavePath, SHORT sThreadsSum, CMultiMachineDownloadDlg* pWnd);
	~HttpDownload(void);

	BOOL Download(TaskConfigFile*);
	BOOL m_Stop;//暂停下载任务
	BOOL m_Finished;//下载完成
	CMultiMachineDownloadDlg* m_pWnd;//主窗口

private:
	HttpDownload(const HttpDownload&);

	TaskConfigFile* m_pConfigInfo;//配置文件信息
	CString m_strSavePath;//文件保存路径
	CString m_strObject, m_strServer, m_strURL;//strServer用于保存服务器地址，strObject用于保存文件对象名称
	DWORD m_dwServiceType, m_dwStatus;//dwServiceType用于保存服务类型，dwStatus用于保存提交GET请求返回的状态号
	INTERNET_PORT m_nPort;//用于保存服务器端口号
	SHORT m_sThreadsSum;//线程总数
	CWinThread* m_threadsList[10];//同一任务最多支持10个线程同时下载
	CCriticalSection m_cs;//临界区

	BOOL GetInfor();//下载前准备,获取文件信息
	static UINT ReceiveData(LPVOID);//开始接收数据
	VOID Release();
};


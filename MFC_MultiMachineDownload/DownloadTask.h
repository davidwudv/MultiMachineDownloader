#pragma once
#include "CMultiMachineDownloadDlg.h"
#include "TaskConfig.h"
#include "HttpDownload.h"

#define DOWNLOAD_STOP -1

class CMultiMachineDownloadDlg;
class HttpDownload;

class DownloadTask
{
public:
	DownloadTask(CString savePath, TaskConfig* taskConfig, CMultiMachineDownloadDlg* masterDialog);
	~DownloadTask(void);
	SHORT GetCurrentThreadSum() { return m_currentThread; }
	long ReduceCurrentThreads() { return InterlockedDecrement(&m_currentThread); }//当前线程数减一
	void SaveConfig() { m_config->SaveToFile(m_savePath + _T(".conf")); }
	void LoadConfig() { m_config->LoadFromFile(m_savePath + _T(".conf")); }
	void DeleteConfig();//删除配置文件
	//void DeleteAllThreads();
	void Stop();//停止下载任务
	bool IsStop() { return m_bIsStop; }
	void Start();//开始下载
	 
private:
	DownloadTask(const DownloadTask&);

public:
	CMultiMachineDownloadDlg* m_masterDialog;//主窗口
	TaskConfig* m_config;//当前任务配置信息
	CString m_savePath;//下载文件的保存路径
	CCriticalSection m_cs;//临界区
	CCriticalSection m_cs2;
	CWinThread* m_threadList[10];//当前运行的线程列表
	HttpDownload* m_downloadList[10];
	volatile bool m_bFinished;
private:
	long m_currentThread;//当前任务运行的线程数
	bool m_bIsStop;
	
};


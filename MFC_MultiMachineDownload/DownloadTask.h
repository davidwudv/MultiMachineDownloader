#pragma once
#include "TaskConfig.h"

class DownloadTask
{
public:
	DownloadTask(CString savePath, TaskConfig* taskConfig);
	~DownloadTask(void);
	SHORT GetCurrentThreadSum() { return m_currentThread; }
	void SaveConfig() { m_config->SaveToFile(m_savePath + _T(".conf")); }
	//void LoadConfig() { m_config->LoadFromFile(m_savePath + _T(".conf")); }
	void Stop();//停止下载任务
	bool IsStop() { return m_bIsStop; }
	void Start();//开始下载
private:
	DownloadTask(const DownloadTask&);

public:
	TaskConfig* m_config;//当前任务配置信息
	CString m_savePath;//下载文件的保存路径
	CCriticalSection m_cs;//临界区
	CWinThread* m_threadList[10];//当前运行的线程列表
private:
	SHORT m_currentThread;//当前任务运行的线程数
	bool m_bIsStop;
	
};


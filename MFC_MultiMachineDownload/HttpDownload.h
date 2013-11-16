#pragma once
#include "DownloadTask.h"

class HttpDownload
{
public:
	HttpDownload(DownloadTask* downloadTask, short currentThread);
	~HttpDownload(void);
	int Start();//开始下载
	int StartEx();

private:
	bool QueryStatusCode(CString strResponse, UINT &nStatusCode);

public:
	DownloadTask* m_downloadTask;//标识隶属于哪一个下载任务
	TaskConfig* m_config;//任务配置信息
	short m_currentThreadIndex;//当前线程的索引
	
private:
	CStringA m_strReferer;//源资源地址
};


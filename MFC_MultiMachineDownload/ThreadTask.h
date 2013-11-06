#pragma once
#include "TaskConfigFile.h"
#include "HttpDownload.h"

class HttpDownload;

class ThreadTask
{
public:
	ThreadTask(void);
	ThreadTask(HttpDownload*);
	ThreadTask(UINT64 start, UINT64 size, HttpDownload* download);
	UINT64 m_blockStart;//资源分块的开始坐标
	UINT64 m_blockSize;//资源分块的大小
	UINT64 m_blockHasDownloadSize;//本分块已经下载的大小
	BOOL m_isLastBlock;//标识是否是最后一个分块
	HttpDownload* m_downloadInfor;//指示此线程隶属于哪个任务

	~ThreadTask(void);
private:
	VOID Release();
};


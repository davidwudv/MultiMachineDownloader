#pragma once
#include "TaskConfigFile.h"
#include "HttpDownload.h"

class ThreadTask
{
public:
	ThreadTask(void);
	ThreadTask(HttpDownload*);
	ThreadTask(ULONGLONG start, ULONGLONG size, HttpDownload* download);
	ULONGLONG m_blockStart;//资源分块的开始坐标
	ULONGLONG m_blockSize;//资源分块的大小
	BOOL m_isLastBlock;//标识是否是最后一个分块
	HttpDownload* m_downloadInfor;//指示此线程隶属于哪个任务

	~ThreadTask(void);
private:
	VOID Release();
};


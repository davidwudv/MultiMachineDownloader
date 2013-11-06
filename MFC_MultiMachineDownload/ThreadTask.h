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
	UINT64 m_blockStart;//��Դ�ֿ�Ŀ�ʼ����
	UINT64 m_blockSize;//��Դ�ֿ�Ĵ�С
	UINT64 m_blockHasDownloadSize;//���ֿ��Ѿ����صĴ�С
	BOOL m_isLastBlock;//��ʶ�Ƿ������һ���ֿ�
	HttpDownload* m_downloadInfor;//ָʾ���߳��������ĸ�����

	~ThreadTask(void);
private:
	VOID Release();
};


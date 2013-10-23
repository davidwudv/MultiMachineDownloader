#pragma once
#include "TaskConfigFile.h"
#include "HttpDownload.h"

class ThreadTask
{
public:
	ThreadTask(void);
	ThreadTask(HttpDownload*);
	ThreadTask(ULONGLONG start, ULONGLONG size, HttpDownload* download);
	ULONGLONG m_blockStart;//��Դ�ֿ�Ŀ�ʼ����
	ULONGLONG m_blockSize;//��Դ�ֿ�Ĵ�С
	BOOL m_isLastBlock;//��ʶ�Ƿ������һ���ֿ�
	HttpDownload* m_downloadInfor;//ָʾ���߳��������ĸ�����

	~ThreadTask(void);
private:
	VOID Release();
};


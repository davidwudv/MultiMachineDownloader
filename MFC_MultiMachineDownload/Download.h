#pragma once
#include "DownloadTask.h"

class Download
{
public:
	Download(DownloadTask* downloadTask, short currentThread);
	~Download(void);
	int Start();//��ʼ����

public:
	DownloadTask* m_downloadTask;//��ʶ��������һ����������
	TaskConfig* m_config;//����������Ϣ
	short m_currentThreadIndex;//��ǰ�̵߳�����
	
};


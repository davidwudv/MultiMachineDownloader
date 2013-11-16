#pragma once
#include "DownloadTask.h"

class HttpDownload
{
public:
	HttpDownload(DownloadTask* downloadTask, short currentThread);
	~HttpDownload(void);
	int Start();//��ʼ����
	int StartEx();

private:
	bool QueryStatusCode(CString strResponse, UINT &nStatusCode);

public:
	DownloadTask* m_downloadTask;//��ʶ��������һ����������
	TaskConfig* m_config;//����������Ϣ
	short m_currentThreadIndex;//��ǰ�̵߳�����
	
private:
	CStringA m_strReferer;//Դ��Դ��ַ
};


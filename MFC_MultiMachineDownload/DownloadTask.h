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
	void Stop();//ֹͣ��������
	bool IsStop() { return m_bIsStop; }
	void Start();//��ʼ����
private:
	DownloadTask(const DownloadTask&);

public:
	TaskConfig* m_config;//��ǰ����������Ϣ
	CString m_savePath;//�����ļ��ı���·��
	CCriticalSection m_cs;//�ٽ���
	CWinThread* m_threadList[10];//��ǰ���е��߳��б�
private:
	SHORT m_currentThread;//��ǰ�������е��߳���
	bool m_bIsStop;
	
};


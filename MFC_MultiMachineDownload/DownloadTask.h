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
	long ReduceCurrentThreads() { return InterlockedDecrement(&m_currentThread); }//��ǰ�߳�����һ
	void SaveConfig() { m_config->SaveToFile(m_savePath + _T(".conf")); }
	void LoadConfig() { m_config->LoadFromFile(m_savePath + _T(".conf")); }
	void DeleteConfig();//ɾ�������ļ�
	//void DeleteAllThreads();
	void Stop();//ֹͣ��������
	bool IsStop() { return m_bIsStop; }
	void Start();//��ʼ����
	 
private:
	DownloadTask(const DownloadTask&);

public:
	CMultiMachineDownloadDlg* m_masterDialog;//������
	TaskConfig* m_config;//��ǰ����������Ϣ
	CString m_savePath;//�����ļ��ı���·��
	CCriticalSection m_cs;//�ٽ���
	CCriticalSection m_cs2;
	CWinThread* m_threadList[10];//��ǰ���е��߳��б�
	HttpDownload* m_downloadList[10];
	volatile bool m_bFinished;
private:
	long m_currentThread;//��ǰ�������е��߳���
	bool m_bIsStop;
	
};


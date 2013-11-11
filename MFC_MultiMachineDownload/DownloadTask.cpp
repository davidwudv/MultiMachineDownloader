#include "stdafx.h"
#include "DownloadTask.h"
#include "Download.h"

using std::auto_ptr;
UINT downloadThreadFun(LPVOID lParam);

DownloadTask::DownloadTask(CString savePath, TaskConfig* taskConfig):
	m_savePath(savePath + taskConfig->GetFileName()), m_config(taskConfig), m_bIsStop(false), m_currentThread(0)
{
	for(int i = 0; i < 10; ++i)
		m_threadList[i] = nullptr;
	if(m_config->m_bSupportResume)	
		SaveConfig();
	CFile file(m_savePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);
	file.SetLength(m_config->GetFileLength());
	file.Close();
}

DownloadTask::~DownloadTask(void)
{
	if(m_config->m_bSupportResume)	
		SaveConfig();
	delete m_config;
	m_config = nullptr;
}

void DownloadTask::Start()
{
	short threadSum = m_config->GetDownloadThreadsSum();
	for(int i = 0; i < threadSum; ++i)
	{
		Download* myDownload = new Download(this, i);
		++m_currentThread;
		AfxBeginThread(downloadThreadFun, myDownload);
	}
	m_bIsStop = false;
}

/************************************************************************/
/* ���޸ġ���Ҫ���һ��ģ̬�Ի�����ѯ���û��Ƿ�ֹͣ����*/
/************************************************************************/
void DownloadTask::Stop()
{
	CSingleLock singleLock(&m_cs);
	singleLock.Lock();
	if(m_config->m_bSupportResume)
		SaveConfig();
	else
	{
		AfxMessageBox(_T("���ļ���֧�ֶϵ��������Ƿ�Ҫֹͣ���أ�\n"));
		return;
	}
	m_bIsStop = true;
}

UINT downloadThreadFun(LPVOID lParam)
{
	auto_ptr<Download> pDownload((Download*)lParam);
begin:
	int errorCode(0);
	errorCode = pDownload->Start();
	if(errorCode == ERROR_INTERNET_CONNECTION_RESET | errorCode == ERROR_INTERNET_TIMEOUT)//���������ӱ����û����ӳ�ʱ
		goto begin;
	return 0;
}
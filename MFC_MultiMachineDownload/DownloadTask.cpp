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
/* 待修改。需要添加一个模态对话框来询问用户是否停止下载*/
/************************************************************************/
void DownloadTask::Stop()
{
	CSingleLock singleLock(&m_cs);
	singleLock.Lock();
	if(m_config->m_bSupportResume)
		SaveConfig();
	else
	{
		AfxMessageBox(_T("此文件不支持断点续传，是否要停止下载？\n"));
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
	if(errorCode == ERROR_INTERNET_CONNECTION_RESET | errorCode == ERROR_INTERNET_TIMEOUT)//服务器连接被重置或连接超时
		goto begin;
	return 0;
}
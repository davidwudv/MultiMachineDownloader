#include "stdafx.h"
#include "DownloadTask.h"
#include "HttpDownload.h"

using std::auto_ptr;
UINT downloadThreadFun(LPVOID lParam);

DownloadTask::DownloadTask(CString savePath, TaskConfig* taskConfig, CMultiMachineDownloadDlg* masterDialog):
	m_savePath(savePath), m_config(taskConfig), m_bIsStop(false), m_currentThread(0), m_masterDialog(masterDialog), m_bFinished(false)
{
	for(int i = 0; i < 10; ++i)
	{
		m_threadList[i] = nullptr;
		m_downloadList[i] = nullptr;
	}
	if(m_config->m_bSupportResume)	
		SaveConfig();
	CFile file(m_savePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);
	file.SetLength(m_config->GetFileLength());
	file.Close();
}

DownloadTask::~DownloadTask(void)
{
	if(m_config->m_bSupportResume)	
	{
		if(m_bFinished)
			DeleteConfig();
		else
		{
			SaveConfig();
#ifdef DEBUG
			m_masterDialog->m_cListBoxDownloadOutPut.InsertString(0, _T("save config!"));
#endif
		}
	}
	delete m_config;
	m_config = nullptr;
}

void DownloadTask::Start()
{
	short threadSum = m_config->GetDownloadThreadsSum();
	for(int i = 0; i < threadSum; ++i)
	{
		HttpDownload* myDownload = new HttpDownload(this, i);
		m_downloadList[i] = myDownload;
		++m_currentThread;
		m_threadList[i] = AfxBeginThread(downloadThreadFun, myDownload);
	}
	m_bIsStop = false;
}

/************************************************************************/
/* 待修改。需要添加一个模态对话框来询问用户是否停止下载*/
/************************************************************************/
void DownloadTask::Stop()
{
	CSingleLock singleLock(&m_cs2);
	int threadSum = m_config->GetDownloadThreadsSum();

	if(!m_config->m_bSupportResume)
	{
		AfxMessageBox(_T("此文件不支持断点续传，是否要停止下载？\n"));
		return;
	}
	for(int i = 0; i < threadSum; ++i)
	{
		if(m_downloadList[i] != nullptr)
			m_downloadList[i]->m_socket.ShutDown(2);
	}
	singleLock.Lock();
	m_bIsStop = true;
}

void DownloadTask::DeleteConfig()
{
	DeleteFile(m_savePath + _T(".conf"));
}

//void DownloadTask::DeleteAllThreads()
//{
//	for (int i = 0; i < m_currentThread; ++i)
//	{
//		if(m_threadList[i] != nullptr)
//			delete m_threadList[i];
//	}
//}

UINT downloadThreadFun(LPVOID lParam)
{
	auto_ptr<HttpDownload> pDownload((HttpDownload*)lParam);
begin:
	int errorCode(0);
	//if(pDownload->m_config->GetDownloadThreadsSum() > 2)
	//{
		errorCode = pDownload->StartEx();
		if(errorCode == WSAECONNRESET || errorCode == WSAENOTCONN /*|| errorCode == ERROR_INTERNET_TIMEOUT*/ || errorCode == 0)
		{
#ifdef DEBUG
			CString str;
			str.Format(_T("Thread %d restart!"), pDownload->m_currentThreadIndex);
			pDownload->m_downloadTask->m_masterDialog->m_cListBoxDownloadOutPut.InsertString(0, str);
#endif
			goto begin;
		}
//	}
//	else
//	{
//		errorCode = pDownload->Start();
//		if(errorCode == ERROR_INTERNET_CONNECTION_RESET | errorCode == ERROR_INTERNET_TIMEOUT || errorCode == 0)//服务器连接被重置或连接超时
//		{
//#ifdef DEBUG
//			CString str;
//			str.Format(_T("Thread %d restart!"), pDownload->m_currentThreadIndex);
//#endif
//			goto begin;
//		}
//	}
	
	pDownload->m_downloadTask->ReduceCurrentThreads();
	if(pDownload->m_downloadTask->GetCurrentThreadSum() == 0)
	{
		if(pDownload->m_downloadTask->IsStop())
			PostMessage(pDownload->m_downloadTask->m_masterDialog->m_hWnd, WM_USER_DOWNLOAD_STOP, 0, (LPARAM)pDownload->m_downloadTask);
		else if(pDownload->m_downloadTask->m_bFinished)
			PostMessage(pDownload->m_downloadTask->m_masterDialog->m_hWnd, WM_USER_DOWNLOAD_FINISHED, 0, (LPARAM)pDownload->m_downloadTask);
	}
	pDownload->m_downloadTask->m_downloadList[pDownload->m_currentThreadIndex] = nullptr;
	return 0;
}
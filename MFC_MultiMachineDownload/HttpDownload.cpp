#include "stdafx.h"
#include "HttpDownload.h"
#include "ThreadTask.h"

#define HTTP_STATUS_REQUEST_RANGE_NOT_STATISFIABLE 416 //服务器不支持断点续传
#define BUFFSIZE 5120 //一次接收5KB

using std::auto_ptr;

//CHttpDownload::CHttpDownload(CTaskConfigFile* configInfo, CString strURL,
//							 CString strServer, CString strObject, DWORD dwServiceType, INTERNET_PORT nPort):
//m_pConfigInfo(configInfo), m_strURL(strURL), m_strServer(strServer), m_strObject(strObject), 
//	m_dwServiceType(dwServiceType),m_nPort(nPort)
//{
//}

HttpDownload::HttpDownload(CString strURL, CString strSavePath, SHORT sThreadsSum, CMultiMachineDownloadDlg* pWnd): 
	m_strURL(strURL), m_strSavePath(strSavePath),m_pConfigInfo(nullptr), m_sThreadsSum(sThreadsSum), m_Stop(FALSE), m_pWnd(pWnd), m_Finished(FALSE)
{
	::AfxParseURL(m_strURL, m_dwServiceType, m_strServer, m_strObject, m_nPort);
	for(SHORT i = 0; i < sThreadsSum; ++i)
	{
		m_threadsList[i] = nullptr;
	}
}

HttpDownload::HttpDownload(const HttpDownload& hd)
{
}

HttpDownload::~HttpDownload(void)
{
	Release();
}

VOID HttpDownload::Release()
{
	if(m_pConfigInfo)
	{
		delete m_pConfigInfo;
		m_pConfigInfo = nullptr;
	}

	for(int i =0; i < m_sThreadsSum; ++i)
	{
		if(m_threadsList[i])
			delete m_threadsList[i];
	}
}

BOOL HttpDownload::GetInfor()
{
	CInternetSession innetSession;
	auto_ptr<CHttpFile> pHttpFile;
	auto_ptr<CHttpConnection> pHttpConn;
	
	try
	{
		pHttpConn.reset(innetSession.GetHttpConnection(m_strServer, m_nPort));
		pHttpFile.reset(pHttpConn->OpenRequest(CHttpConnection::HTTP_VERB_HEAD, m_strObject));
		if(!pHttpFile.get())
		{
#ifdef DEBUG
			TRACE("Open request error!\n");
			::AfxMessageBox(_T("Open request error!\n"));
#endif
			innetSession.Close();
			return FALSE;
		}

		CString strRange(_T("Range: bytes=10-\r\n"));//用于判断服务器是否支持断点续传
		if(!pHttpFile->AddRequestHeaders(strRange))
		{
#ifdef DEBUG
			TRACE("AddRequestHeaders() error.\n");
#endif
		}
		if(!pHttpFile->SendRequest())
		{
#ifdef DEBUG
			TRACE("SendRequest()) error.\n");
#endif
		}
		pHttpFile->QueryInfoStatusCode(m_dwStatus);
		if(m_dwStatus >= 200 && m_dwStatus < 300)//请求成功
		{
			DWORD dwFileSize;
			pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, dwFileSize);
			m_pConfigInfo = new TaskConfigFile(m_strURL, m_strSavePath, dwFileSize, m_sThreadsSum);
			m_pConfigInfo->m_strFileName = pHttpFile->GetFileName();
			m_pConfigInfo->m_strSavePath += pHttpFile->GetFileName();
			//Download();
		}
		else//单线程
		{
#ifdef DEBUG
			TRACE("Send request error!\n");
			::AfxMessageBox(_T("服务器不支持断点续传！"));
#endif
			m_sThreadsSum = 1;//单线程下载
			DWORD dwFileSize;
			pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, dwFileSize);
			m_pConfigInfo = new TaskConfigFile(m_strURL, m_strSavePath, dwFileSize, m_sThreadsSum);
			m_pConfigInfo->m_strFileName = pHttpFile->GetFileName();
			m_pConfigInfo->m_strSavePath += pHttpFile->GetFileName();
		}
		

	}
	catch(CInternetException* internetEx)
	{
		internetEx->ReportError();
		internetEx->Delete();
		innetSession.Close();
	}
	catch(CException* e)
	{
		e->ReportError();
		e->Delete();
		innetSession.Close();
	}

	innetSession.Close();
	return TRUE;
}

UINT HttpDownload::ReceiveData(LPVOID pParam)
{
	CHAR* pBuff(nullptr);
	auto_ptr<ThreadTask> pTaskInfo(static_cast<ThreadTask*>(pParam));
	auto_ptr<CHttpFile> httpFile;
	CInternetSession session;
#ifdef DEBUG
	DWORD dwThreadID;
	dwThreadID = ::GetCurrentThreadId();
#endif

	try
	{
		//统一以二进制流传输
		DWORD dwFlags = INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;
		CString strRange;
		DWORD dwStatus(0);
		ULONGLONG sizeValue(0);//本分块已下载的大小
		pTaskInfo->m_downloadInfor->m_pConfigInfo->m_mapBlockDownloadedSize.Lookup(pTaskInfo->m_blockStart, sizeValue);
		ULONGLONG start(pTaskInfo->m_blockStart + sizeValue);
		ULONGLONG end(pTaskInfo->m_blockStart + pTaskInfo->m_downloadInfor->m_pConfigInfo->m_lBlockSize);
		if(pTaskInfo->m_isLastBlock)
			strRange.Format(_T("Range: bytes=%llu-\r\n"), start);
		else	
			strRange.Format(_T("Range: bytes=%llu-%llu\r\n"), start, end);

		httpFile.reset(static_cast<CHttpFile*>(session.OpenURL(pTaskInfo->m_downloadInfor->m_pConfigInfo->m_strLink, 1, dwFlags)));
		if(!httpFile->AddRequestHeaders(strRange))
		{
#ifdef DEBUG
			TRACE("AddRequestHeaders() error.\n");
#endif
		}
		if(!httpFile->SendRequest())
		{
#ifdef DEBUG
			TRACE("SendRequest()) error.\n");
#endif
		}
		httpFile->QueryInfoStatusCode(dwStatus);

		if(dwStatus >= 200 && dwStatus < 300)//请求成功
		{
			CFile downloadFile(pTaskInfo->m_downloadInfor->m_pConfigInfo->m_strSavePath, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive);
			CFile configFile(pTaskInfo->m_downloadInfor->m_pConfigInfo->m_strSavePath + ".tmp", CFile::modeCreate | CFile::modeWrite| CFile::shareExclusive);
			CArchive ar(&configFile, CArchive::store);
			UINT dwRead(1);
			DWORD dwDownloadStatus;

			//downloadFile.SetLength(pTaskInfo->m_downloadInfor->m_pConfigInfo->m_lFileSize);
			downloadFile.Seek(start, CHttpFile::begin);
			pBuff = new CHAR[BUFFSIZE];
#ifdef DEBUG
			TRACE("Thread %d, starting download......\n", dwThreadID);
#endif
			while(dwRead && !pTaskInfo->m_downloadInfor->m_Stop)
			{
				::ZeroMemory(pBuff, BUFFSIZE);//清空缓冲区
				dwRead = httpFile->Read(pBuff, BUFFSIZE);
#ifdef DEBUG
				TRACE("Thread %d, received %llu byte！\n", dwThreadID, dwRead);
#endif
				pTaskInfo->m_downloadInfor->m_cs.Lock();//进入临界区
				downloadFile.Write(pBuff, dwRead);
				dwDownloadStatus = pTaskInfo->m_downloadInfor->m_pConfigInfo->AddDownloadedSize(pTaskInfo->m_blockStart, sizeValue);//更新已下载的大小
				if(DOWNLOAD_FINISHED == dwDownloadStatus)
					pTaskInfo->m_downloadInfor->m_Finished = TRUE;
				pTaskInfo->m_downloadInfor->m_pConfigInfo->Serialize(ar);//保存文件信息
				pTaskInfo->m_downloadInfor->m_cs.Unlock();//离开临界区
			}

#ifdef DEBUG
			TRACE("Thread %d download finished！\n", dwThreadID);
#endif
			downloadFile.Close();
			configFile.Close();
		}
		else
		{
#ifdef DEBUG
			TRACE("Request error in ReceiveData funtion!\n");
			::AfxMessageBox(_T("fuck!"));
#endif
		}
	}
	catch (CMemoryException* e)
	{
		e->ReportError();
		e->Delete();
		delete[] pBuff;
	}
	catch (CFileException* e)
	{
		e->ReportError();
		e->Delete();
		delete[] pBuff;
	}
	catch (CException* e)
	{
		e->ReportError();
		e->Delete();
		delete[] pBuff;
	}

	delete[] pBuff;
	if(pTaskInfo->m_downloadInfor->m_Finished == TRUE)
	{
		::PostMessage(pTaskInfo->m_downloadInfor->m_pWnd->m_hWnd, WM_USER_DOWNLOAD_FINISHED, 0, (LPARAM)pTaskInfo->m_downloadInfor);
	}
	return 0;
}

BOOL HttpDownload::Download(TaskConfigFile* configInfo)
{
	if(!GetInfor())
		::AfxMessageBox(_T("无法获取文件信息，请确认下载链接是否有错。"));
	if(configInfo)
		m_pConfigInfo = configInfo;
	POSITION pos = m_pConfigInfo->m_mapBlockDownloadedSize.GetStartPosition();
	for(SHORT i = 0; i < m_sThreadsSum; ++i)
	{
		ThreadTask* myTask = new ThreadTask(this);
		m_pConfigInfo->m_mapBlockDownloadedSize.GetNextAssoc(pos, myTask->m_blockStart, myTask->m_blockSize);
		if(i == m_sThreadsSum -1)
			myTask->m_isLastBlock = TRUE;
		m_threadsList[i] = ::AfxBeginThread(ReceiveData, (LPVOID)myTask);
#ifdef DEBUG
		TRACE("Thread %d start...\n", m_threadsList[i]->m_nThreadID);
#endif
	}
	return TRUE;
}

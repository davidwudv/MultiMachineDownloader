#include "stdafx.h"
#include "HttpDownload.h"
#include "ThreadTask.h"
#include <atldbcli.h>
#include <tchar.h>

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

	/*for(int i =0; i < m_sThreadsSum; ++i)
	{
	if(m_threadsList[i])
	{
	m_threadsList[i]->Delete();
	m_threadsList[i] = nullptr;
	}
	}*/
}

//BOOL HttpDownload::GetInfor()
//{
//	CInternetSession innetSession;
//	auto_ptr<CHttpFile> pHttpFile;
//	auto_ptr<CHttpConnection> pHttpConn;
//	
//	try
//	{
//		pHttpConn.reset(innetSession.GetHttpConnection(m_strServer, m_nPort));
//		pHttpFile.reset(pHttpConn->OpenRequest(CHttpConnection::HTTP_VERB_HEAD, m_strObject));
//		if(!pHttpFile.get())
//		{
//#ifdef DEBUG
//			TRACE("Open request error!\n");
//			::AfxMessageBox(_T("Open request error!\n"));
//#endif
//			innetSession.Close();
//			return FALSE;
//		}
//
//		CString strRange(_T("Range: bytes=10-\r\n"));//用于判断服务器是否支持断点续传
//		if(!pHttpFile->AddRequestHeaders(strRange))
//		{
//#ifdef DEBUG
//			TRACE("AddRequestHeaders() error.\n");
//#endif
//		}
//		if(!pHttpFile->SendRequest())
//		{
//#ifdef DEBUG
//			TRACE("SendRequest()) error.\n");
//#endif
//		}
//		pHttpFile->QueryInfoStatusCode(m_dwStatus);
//		if(m_dwStatus >= 200 && m_dwStatus < 300)//请求成功
//		{
//			DWORD dwFileSize;
//			pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, dwFileSize);
//			m_pConfigInfo = new TaskConfigFile(m_strURL, m_strSavePath, dwFileSize, m_sThreadsSum);
//			m_pConfigInfo->m_strFileName = pHttpFile->GetFileName();
//			m_pConfigInfo->m_strSavePath += pHttpFile->GetFileName();
//			//Download();
//		}
//		else//单线程
//		{
//#ifdef DEBUG
//			TRACE("Send request error!\n");
//			::AfxMessageBox(_T("服务器不支持断点续传！"));
//#endif
//			m_sThreadsSum = 1;//单线程下载
//			DWORD dwFileSize;
//			pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, dwFileSize);
//			m_pConfigInfo = new TaskConfigFile(m_strURL, m_strSavePath, dwFileSize, m_sThreadsSum);
//			m_pConfigInfo->m_strFileName = pHttpFile->GetFileName();
//			m_pConfigInfo->m_strSavePath += pHttpFile->GetFileName();
//		}
//		
//
//	}
//	catch(CInternetException* internetEx)
//	{
//		internetEx->ReportError();
//		internetEx->Delete();
//	}
//	catch(CException* e)
//	{
//		e->ReportError();
//		e->Delete();
//	}
//
//	innetSession.Close();
//	return TRUE;
//}

BOOL HttpDownload::GetInfor()
{
	CSocket socket;
	CHAR buff[1024];
	UINT nStatusCode(0);

	//构造Request head
	CStringA strRequest(_T("HEAD ") + m_strObject + _T(" HTTP/1.1\r\n")
										+ _T("Host: ") + m_strServer + _T("\r\n")
										+ _T("Accept: */*\r\n")
										+ _T("Pragma: no-cache\r\n")
										+ _T("Cache-Control: no-cache\r\n")
										+ _T("Connection: close\r\n")
										+ _T("Range: bytes=10-\r\n"));
	if(!m_strReferer.IsEmpty())
		strRequest += _T("Referer: ") + m_strReferer + _T("\r\n");
	strRequest += _T("\r\n");

	if(socket.m_hSocket != NULL)
		socket.Close();
	socket.Create();
	if(!socket.Connect(m_strServer, m_nPort))
	{
		::AfxMessageBox(_T("socket connect error!\n"));
		DWORD errorCode = ::WSAGetLastError();
		return FALSE;
	}
#ifdef DEBUG
	else
		TRACE("Connected.\n");
#endif
	int result = socket.Send(strRequest, strRequest.GetLength());
#ifdef DEBUG
	TRACE("Request has been send, size: %d Byte\n", result);
#endif
	//strRequest.ReleaseBuffer(); 
	::ZeroMemory(buff, 1024);
	int ret = socket.Receive(buff, 1024);
#ifdef DEBUG
	TRACE("Received %d.\n", ret);
#endif

	if(ret == 0)
	{
		::AfxMessageBox(_T("与服务器的连接被关闭！\n"));
		int errorCode = WSAGetLastError();
#ifdef DEBUG
		TRACE("error code: %d.", errorCode);
#endif
		return FALSE;
	}
	CString strResponse(buff);
	if(strResponse.IsEmpty())
	{
		::AfxMessageBox(_T("Response is Empty!"));
		return FALSE;
	}
	BOOL queryResult = QueryStatusCode(strResponse, nStatusCode);
	ASSERT(queryResult);
	//return FALSE;
		
	switch(nStatusCode)
	{
	case HTTP_STATUS_OK:
	case HTTP_STATUS_CREATED:             //201  object created, reason = new URI
	case HTTP_STATUS_ACCEPTED:            //202  async completion (TBS)
 	case HTTP_STATUS_PARTIAL:             //203  partial completion
 	case HTTP_STATUS_NO_CONTENT:         // 204  no info to return
 	case HTTP_STATUS_RESET_CONTENT:       //205  request completed, but clear form
 	case HTTP_STATUS_PARTIAL_CONTENT:  //206  partial GET furfilled
		{
			int nPos = strResponse.Find(_T("Content-Length"));
			CString strFileLength = strResponse.Mid(nPos + strlenT(_T("Content-Length: ")));
			nPos = strFileLength.Find(_T("\r\n"));
			strFileLength.Left(nPos);
			strFileLength.Trim();
			UINT64 lFileSize(_ttoi64(strFileLength) + 10);
			/*获取文件名*/
			CString strFileName;
			nPos = m_strURL.ReverseFind('/');
			strFileName = m_strURL.Right(m_strURL.GetLength() - nPos - 1);
			m_strSavePath += strFileName;
			m_pConfigInfo = new TaskConfigFile(m_strURL, m_strSavePath, lFileSize, m_sThreadsSum);
			m_pConfigInfo->m_strFileName = strFileName;
			break;
		}
  	case HTTP_STATUS_AMBIGUOUS:           //300  server couldn't decide what to return
  	case HTTP_STATUS_MOVED:              //301  object permanently moved
  	case HTTP_STATUS_REDIRECT:            //302  object temporarily moved
  	case HTTP_STATUS_REDIRECT_METHOD:    //303  redirection w/ new access method
  	case HTTP_STATUS_NOT_MODIFIED:        //304  if-modified-since was not modified
  	case HTTP_STATUS_USE_PROXY:           //305  redirection to proxy, location header specifies proxy to use
  	case HTTP_STATUS_REDIRECT_KEEP_VERB:  //307  HTTP/1.1: keep same verb
		{
			break;
		}
	default:
		return FALSE;
	}
	
	try
	{
		CFile configFile(m_strSavePath + ".conf", CFile::modeCreate | CFile::modeWrite);
		CFile downloadFile(m_strSavePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);
		CArchive ar(&configFile, CArchive::store);
		m_pConfigInfo->Serialize(ar);

		downloadFile.SetLength(m_pConfigInfo->m_lFileSize);
	}
	catch (CMemoryException* e)
	{
		e->ReportError();
		e->Delete();
	}
	catch (CFileException* e)
	{
		int erroCode = WSAGetLastError();
		e->ReportError();
		e->Delete();
	}
	catch (CException* e)
	{
		e->ReportError();
		e->Delete();
	}
	

	return TRUE;
}

BOOL HttpDownload::QueryStatusCode(CString strResponse, UINT &nStatusCode )
{
	if(strResponse.Find(_T("HTTP")) < 0)
		return FALSE;
	CString strStatusCode = strResponse.Mid(9, 3);
	nStatusCode = _ttoi(strStatusCode);

	return nStatusCode > 0 ? TRUE : FALSE;
}

UINT HttpDownload::ReceiveData(LPVOID pParam)
{
#ifdef DEBUG
	DWORD dwThreadID;
	dwThreadID = GetCurrentThreadId();
#endif
	auto_ptr<ThreadTask> pTaskInfo(static_cast<ThreadTask*>(pParam));
	CSocket socket;
	//CSingleLock singleLock(&pTaskInfo->m_downloadInfor->m_cs);
	CHAR buff[BUFFSIZE];
	CStringA strRange;
	UINT64 sizeValue(0);//本分块已下载的大小
	CStringA strObject(pTaskInfo->m_downloadInfor->m_strObject);
	CStringA strServer(pTaskInfo->m_downloadInfor->m_strServer);
	CStringA strReferer(pTaskInfo->m_downloadInfor->m_strReferer);
	INTERNET_PORT nPort(pTaskInfo->m_downloadInfor->m_nPort);
	CStringA strRequest("HEAD " + strObject + " HTTP/1.1\r\n"
										 + "Host: " + strServer + "\r\n"
										 + "Accept: */*\r\n"
										 + "Pragma: no-cache\r\n"
										 + "Cache-Control: no-cache\r\n"
										 + "Connection: close\r\n"
										 + "Range: bytes=10-\r\n");
	if(!strReferer.IsEmpty())
		strRequest += "Referer: " + strReferer + "\r\n";
	strRequest += "\r\n";
	pTaskInfo->m_downloadInfor->m_pConfigInfo->m_mapBlockDownloadedSize.Lookup(pTaskInfo->m_blockStart, sizeValue);
	UINT64 start(pTaskInfo->m_blockStart + sizeValue);
	UINT64 end(pTaskInfo->m_blockStart + pTaskInfo->m_downloadInfor->m_pConfigInfo->m_lBlockSize);
	if(pTaskInfo->m_isLastBlock)
		strRange.Format("Range: bytes=%llu-\r\n", start);
	else	
		strRange.Format("Range: bytes=%llu-%llu\r\n", start, end);

	socket.Create();
	if(!socket.Connect(pTaskInfo->m_downloadInfor->m_strServer, nPort))
	{
		::AfxMessageBox(_T("socket connect error!\n"));
		DWORD errorCode = ::WSAGetLastError();
		return FALSE;
	}
#ifdef DEBUG
	else
		TRACE("Thread %d connected.\n", dwThreadID);
#endif
	::ZeroMemory(buff, BUFFSIZE);
	
#ifdef DEBUG
	int sendSize = socket.Send(strRequest, strRequest.GetLength());
	TRACE("Request has been send, size: %d Byte\n", sendSize);
	int receiveSize = socket.Receive(buff, BUFFSIZE);
	TRACE("Received %d Byte\n");
#else
	socket.Send(strRequest, strRequest.GetLength());
	socket.Receive(buff, BUFFSIZE);
#endif
	CString strResponse(buff);
	UINT dwStatusCode;
	QueryStatusCode(strResponse, dwStatusCode);
	if(dwStatusCode >= 200 && dwStatusCode < 300)
	{
		/*截取Response Head，此部分不写入文件*/
		int index = strResponse.Find(_T("\r\n\r\n"));
		CString strResponseHead = strResponse.Mid(0, index + 2);
		int size = strResponseHead.GetLength();

		try
		{
			UINT dwRead(1);
			CFile downloadFile(pTaskInfo->m_downloadInfor->m_pConfigInfo->m_strSavePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyNone );

			downloadFile.Seek(start, CFile::begin);
#ifdef DEBUG
			TRACE("Thread %d start download....\n", dwThreadID);
#endif
			while(!pTaskInfo->m_downloadInfor->m_Stop && !pTaskInfo->m_downloadInfor->m_Finished)
			{
				::ZeroMemory(buff, BUFFSIZE);//清空缓冲区
				dwRead = socket.Receive(buff, BUFFSIZE);
#ifdef DEBUG
				TRACE("Thread %d, received %d byte\n", dwThreadID, dwRead);
#endif
				if(dwRead < 0)
				{
					int erroCode = WSAGetLastError();
#ifdef DEBUG
					AfxMessageBox(_T("error code: %d"), erroCode);
#endif
					return -1;
				}

				pTaskInfo->m_downloadInfor->m_cs.Lock();//进入临界区
				downloadFile.Write(buff, dwRead);
				pTaskInfo->m_downloadInfor->m_Finished = pTaskInfo->m_downloadInfor->m_pConfigInfo->AddDownloadedSize(pTaskInfo->m_blockStart, static_cast<UINT64>(dwRead));//更新已下载的大小
				float progress = 100 * ((float)pTaskInfo->m_downloadInfor->m_pConfigInfo->m_lSumDownloadedSize / pTaskInfo->m_downloadInfor->m_pConfigInfo->m_lFileSize);
#ifdef DEBUG
				TRACE("Download progress: %%%.2f\n", progress);
#endif
				pTaskInfo->m_downloadInfor->m_pWnd->m_cProgress.SetPos(progress);
				pTaskInfo->m_downloadInfor->m_cs.Unlock();//离开临界区
				//pTaskInfo->m_downloadInfor->WriteDataToFile(downloadFile, buff, dwRead, pTaskInfo.get());
			}

			if(pTaskInfo->m_downloadInfor->m_Stop)
				::PostMessage(pTaskInfo->m_downloadInfor->m_pWnd->m_hWnd, WM_USER_DOWNLOAD_STOP, 0, (LPARAM)pTaskInfo->m_downloadInfor);
			downloadFile.Close();
			socket.Close();
		}
		catch (CMemoryException* e)
		{
			e->ReportError();
			e->Delete();
		}
		catch (CFileException* e)
		{
			int erroCode = WSAGetLastError();
			e->ReportError();
			e->Delete();
		}
		catch (CException* e)
		{
			e->ReportError();
			e->Delete();
		}
	}
	
	if(pTaskInfo->m_downloadInfor->m_Finished == TRUE)
	{
		::PostMessage(pTaskInfo->m_downloadInfor->m_pWnd->m_hWnd, WM_USER_DOWNLOAD_FINISHED, 0, (LPARAM)pTaskInfo->m_downloadInfor);
	}
	
	return 0;
}

VOID HttpDownload::WriteDataToFile(CFile& file, CHAR* buff, UINT writeSize, ThreadTask* pTask)
{
	CSingleLock singleLock(&m_cs);
	singleLock.Lock();//进入临界区
	file.Write(buff, writeSize);
	m_Finished = m_pConfigInfo->AddDownloadedSize(pTask->m_blockStart, static_cast<UINT64>(writeSize));//更新已下载的大小
	float progress = 100 * ((float)pTask->m_downloadInfor->m_pConfigInfo->m_lSumDownloadedSize / pTask->m_downloadInfor->m_pConfigInfo->m_lFileSize);
	m_pWnd->m_cProgress.SetPos(progress);
#ifdef DEBUG
	TRACE("Download progress: %%%.2f\n", progress);
#endif
}

//UINT HttpDownload::ReceiveData(LPVOID pParam)
//{
//	CHAR* pBuff(nullptr);
//	auto_ptr<ThreadTask> pTaskInfo(static_cast<ThreadTask*>(pParam));
//	auto_ptr<CHttpFile> httpFile;
//	CInternetSession session;
//#ifdef DEBUG
//	DWORD dwThreadID;
//	dwThreadID = ::GetCurrentThreadId();
//#endif
//
//	try
//	{
//		//统一以二进制流传输
//		DWORD dwFlags = INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;
//		CString strRange;
//		DWORD dwStatus(0);
//		UINT64 sizeValue(0);//本分块已下载的大小
//		pTaskInfo->m_downloadInfor->m_pConfigInfo->m_mapBlockDownloadedSize.Lookup(pTaskInfo->m_blockStart, sizeValue);
//		UINT64 start(pTaskInfo->m_blockStart + sizeValue);
//		UINT64 end(pTaskInfo->m_blockStart + pTaskInfo->m_downloadInfor->m_pConfigInfo->m_lBlockSize);
//		if(pTaskInfo->m_isLastBlock)
//			strRange.Format(_T("Range: bytes=%llu-\r\n"), start);
//		else	
//			strRange.Format(_T("Range: bytes=%llu-%llu\r\n"), start, end);
//
//		httpFile.reset(static_cast<CHttpFile*>(session.OpenURL(pTaskInfo->m_downloadInfor->m_pConfigInfo->m_strLink, 1, dwFlags)));
//		if(!httpFile->AddRequestHeaders(strRange))
//		{
//#ifdef DEBUG
//			TRACE("AddRequestHeaders() error.\n");
//#endif
//		}
//		if(!httpFile->SendRequest())
//		{
//#ifdef DEBUG
//			TRACE("SendRequest() error.\n");
//#endif
//		}
//		httpFile->QueryInfoStatusCode(dwStatus);
//
//		if(dwStatus >= 200 && dwStatus < 300)//请求成功
//		{
//			CFile downloadFile(pTaskInfo->m_downloadInfor->m_pConfigInfo->m_strSavePath, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive);
//			CFile configFile(pTaskInfo->m_downloadInfor->m_pConfigInfo->m_strSavePath + ".tmp", CFile::modeCreate | CFile::modeWrite| CFile::shareExclusive);
//			CArchive ar(&configFile, CArchive::store);
//			UINT dwRead(1);
//			BOOL downloadFinished;
//			CSingleLock singleLock(&pTaskInfo->m_downloadInfor->m_cs);
//
//			//downloadFile.SetLength(pTaskInfo->m_downloadInfor->m_pConfigInfo->m_lFileSize);
//			downloadFile.Seek(start, CHttpFile::begin);
//			pBuff = new CHAR[BUFFSIZE];
//#ifdef DEBUG
//			TRACE("Thread %d, starting download......\n", dwThreadID);
//#endif
//			while(dwRead && !pTaskInfo->m_downloadInfor->m_Stop)
//			{
//				::ZeroMemory(pBuff, BUFFSIZE);//清空缓冲区
//				dwRead = httpFile->Read(pBuff, BUFFSIZE);
//#ifdef DEBUG
//				TRACE("Thread %d, received %llu byte！\n", dwThreadID, dwRead);
//#endif
//				singleLock.Lock();//进入临界区
//				downloadFile.Write(pBuff, dwRead);
//				downloadFinished = pTaskInfo->m_downloadInfor->m_pConfigInfo->AddDownloadedSize(pTaskInfo->m_blockStart, sizeValue);//更新已下载的大小
//				if(downloadFinished)
//					pTaskInfo->m_downloadInfor->m_Finished = TRUE;
//				pTaskInfo->m_downloadInfor->m_pConfigInfo->Serialize(ar);//保存文件信息
//				singleLock.Unlock();//离开临界区
//			}
//
//#ifdef DEBUG
//			TRACE("Thread %d download finished！\n", dwThreadID);
//#endif
//			downloadFile.Close();
//			configFile.Close();
//		}
//		else
//		{
//#ifdef DEBUG
//			TRACE("Request error in ReceiveData funtion!\n");
//			::AfxMessageBox(_T("fuck!"));
//#endif
//		}
//	}
//	catch (CMemoryException* e)
//	{
//		e->ReportError();
//		e->Delete();
//	}
//	catch (CFileException* e)
//	{
//		e->ReportError();
//		e->Delete();
//	}
//	catch (CException* e)
//	{
//		e->ReportError();
//		e->Delete();
//	}
//
//	delete[] pBuff;
//	if(pTaskInfo->m_downloadInfor->m_Finished == TRUE)
//	{
//		::PostMessage(pTaskInfo->m_downloadInfor->m_pWnd->m_hWnd, WM_USER_DOWNLOAD_FINISHED, 0, (LPARAM)pTaskInfo->m_downloadInfor);
//	}
//	return 0;
//}

BOOL HttpDownload::Download(TaskConfigFile* configInfo)
{
	if(configInfo != nullptr)
		m_pConfigInfo = configInfo;
	else
	{
		CString strFileName;
		int nPos = m_strURL.ReverseFind('/');
		strFileName = m_strURL.Right(m_strURL.GetLength() - nPos - 1);
		m_strSavePath += strFileName;

		CFileFind finder;
		if(finder.FindFile(m_strSavePath))
		{
			if(finder.FindFile(m_strSavePath + _T(".conf")))
			{
				m_pConfigInfo = new TaskConfigFile();
				CFile configFile(m_strSavePath + _T(".conf"), CFile::modeRead);
				CArchive ar(&configFile, CArchive::load);
				m_pConfigInfo->Serialize(ar);
				m_sThreadsSum = m_pConfigInfo->m_sThreadsSum;
			}
			else
			{
				AfxMessageBox(_T("此文件已经下载过了!\n"));
			}
		}
		else if(!GetInfor())
		{
			::AfxMessageBox(_T("无法获取文件信息，请确认下载链接是否有错。"));
			return FALSE;
		}
	}
	
	POSITION pos = m_pConfigInfo->m_mapBlockDownloadedSize.GetStartPosition();
	for(SHORT i = 0; i < m_sThreadsSum; ++i)
	{
		try
		{
			ThreadTask* myTask = new ThreadTask(this);
			myTask->m_blockSize = m_pConfigInfo->m_lBlockSize;
			m_pConfigInfo->m_mapBlockDownloadedSize.GetNextAssoc(pos, myTask->m_blockStart, myTask->m_blockHasDownloadSize);
			if(i == m_sThreadsSum -1)
				myTask->m_isLastBlock = TRUE;
			m_threadsList[i] = ::AfxBeginThread(ReceiveData, (LPVOID)myTask);
#ifdef DEBUG
			TRACE("Thread %d start...\n", m_threadsList[i]->m_nThreadID);
#endif
		}
		catch (CMemoryException* e)
		{
			e->ReportError();
			e->Delete();
		}
		catch (CFileException* e)
		{
			e->ReportError();
			e->Delete();
		}
		catch (CException* e)
		{
			e->ReportError();
			e->Delete();
		}
	}
	return TRUE;
}

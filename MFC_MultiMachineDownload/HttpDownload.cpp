#include "stdafx.h"
#include "HttpDownload.h"

#define HTTP_STATUS_REQUEST_RANGE_NOT_STATISFIABLE 416 //服务器不支持断点续传

//CHttpDownload::CHttpDownload(CTaskConfigFile* configInfo, CString strURL,
//							 CString strServer, CString strObject, DWORD dwServiceType, INTERNET_PORT nPort):
//m_pConfigInfo(configInfo), m_strURL(strURL), m_strServer(strServer), m_strObject(strObject), 
//	m_dwServiceType(dwServiceType),m_nPort(nPort)
//{
//}

CHttpDownload::CHttpDownload(CString strURL, CString strSavePath, SHORT sThreadsSum): 
	m_strURL(strURL), m_strSavePath(strSavePath)
{
	::AfxParseURL(m_strURL, m_dwServiceType, m_strServer, m_strObject, m_nPort);
	for(SHORT i = 0; i < sThreadsSum; ++i)
	{
		m_threadsList[i] = nullptr;
	}
}

CHttpDownload::~CHttpDownload(void)
{
	if(!m_pConfigInfo)
		delete m_pConfigInfo;
}

BOOL CHttpDownload::GetInfor()
{
	CInternetSession innetSession;
	CHttpFile* pHttpFile(nullptr);
	CHttpConnection* pHttpConn(nullptr);
	
	try
	{
		pHttpConn = innetSession.GetHttpConnection(m_strServer, m_nPort);
		pHttpFile = pHttpConn->OpenRequest(CHttpConnection::HTTP_VERB_GET, m_strObject);
		if(!pHttpFile)
		{
#ifdef DEBUG
			TRACE0("Open request error!");
#endif
			delete pHttpConn;
			innetSession.Close();
			return FALSE;
		}

		CString strRange(_T("Range: bytes=10-\r\n"));//用于判断服务器是否支持断点续传
		pHttpFile->AddRequestHeaders(strRange);
		pHttpFile->SendRequest();
		pHttpFile->QueryInfoStatusCode(m_dwStatus);
		if(m_dwStatus >= 200 && m_dwStatus < 300)//请求成功
		{
			ULONGLONG lFileSize = pHttpFile->GetLength();
			m_pConfigInfo = new CTaskConfigFile(m_strURL, m_strSavePath, lFileSize, m_sThreadsSum);
			m_pConfigInfo->m_strFileName = pHttpFile->GetFileName();
			Download();
		}
		else
		{
#ifdef DEBUG
			TRACE0("Send request error!");
			::AfxMessageBox(_T("服务器不支持断点续传！"));
#endif
			delete pHttpFile;
			delete pHttpConn;
			innetSession.Close();
			return FALSE;
		}


	}
	catch(CInternetException* internetEx)
	{
		internetEx->ReportError();
		internetEx->Delete();
		if(pHttpConn != nullptr)
			delete pHttpConn;
		if(pHttpFile != nullptr)
			delete pHttpFile;
	}
	

	return TRUE;
}

UINT CHttpDownload::ReceiveData(LPVOID pParam)
{
	CTaskConfigFile* fileInfo = static_cast<CTaskConfigFile*>(pParam);
	CInternetSession session;
	CHttpFile* httpFile;
	//统一以二进制流传输
	DWORD dwFlags = INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;

	httpFile = static_cast<CHttpFile*>(session.OpenURL(fileInfo->m_strLink, 1, dwFlags));

	return 0;
}

BOOL CHttpDownload::Download(CTaskConfigFile* configInfo = nullptr)
{
	if(configInfo)
		m_pConfigInfo = configInfo;
	for(SHORT i = 0; i < m_pConfigInfo->m_sThreadsSum; ++i)
	{
		m_threadsList[i] = ::AfxBeginThread(ReceiveData, (LPVOID)m_pConfigInfo);
	}
	return TRUE;
}

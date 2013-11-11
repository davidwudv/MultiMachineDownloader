#include "stdafx.h"
#include "Download.h"

#define BUFFSIZE 5120 //一次接收5KB
using std::auto_ptr;

Download::Download(DownloadTask* downloadTask, short currentThread):
	m_downloadTask(downloadTask), m_config(downloadTask->m_config), m_currentThreadIndex(currentThread)
{
}

Download::~Download(void)
{
}

int Download::Start()
{
	int errorCode(0);
	CInternetSession session;
	auto_ptr<CHttpConnection> pHttpConnection;
	auto_ptr<CHttpFile> pHttpFile;
	CSingleLock singleLock(&m_downloadTask->m_cs);

	try
	{
		DWORD dwStatusCode(0);
		UINT64 start(m_config->m_block[m_currentThreadIndex]->m_ulStart + m_config->m_block[m_currentThreadIndex]->m_ulDownloadedSize);
		UINT64 end(m_config->m_block[m_currentThreadIndex]->m_ulStart + m_config->m_block[m_currentThreadIndex]->m_ulBlockSize - 1);
		pHttpConnection.reset(session.GetHttpConnection(m_config->GetServerString(), INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_TRANSFER_ASCII, m_config->GetPort()));
		pHttpFile.reset(pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, m_config->GetObjectString()));
		pHttpFile->AddRequestHeaders(_T("Pragma: no-cache\r\n"));
		//pHttpFile->AddRequestHeaders(_T("Connection: close\r\n"));
		pHttpFile->AddRequestHeaders(_T("Accept: */*\r\n"));
		if(m_config->m_bSupportResume)
		{
			CString range;
			if(m_config->m_block[m_currentThreadIndex]->m_bIsLastBlock)
				range.Format(_T("Range: bytes=%llu-\r\n"), start);
			else
				range.Format(_T("Range: bytes=%llu-%llu\r\n"), start, end);
			pHttpFile->AddRequestHeaders(range);
		}

#ifdef DEBUG
		TRACE("Begin send request\n");
#endif
		pHttpFile->SendRequest();
#ifdef DEBUG
		TRACE("End send request\n");
#endif
		pHttpFile->QueryInfoStatusCode(dwStatusCode);
		if(dwStatusCode >= 200 && dwStatusCode < 300)
		{
			char receiveBuff[BUFFSIZE + 1];
			UINT readSize(1);
			CFile downloadFile(m_downloadTask->m_savePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyNone);
			downloadFile.Seek(start, CFile::begin);
			while(readSize)
			{
				ZeroMemory(receiveBuff, BUFFSIZE + 1);
				readSize = pHttpFile->Read(receiveBuff, BUFFSIZE);
				singleLock.Lock();//进入临界区
				downloadFile.Write(receiveBuff, readSize);
				m_config->m_block[m_currentThreadIndex]->m_ulDownloadedSize += readSize;//更新本分块已下载大小
				m_config->m_ulSumDownloadedSize += readSize;//更新已下载总大小
#ifdef DEBUG
				float downloadPercent;
				downloadPercent = 100 * ((float)m_config->m_ulSumDownloadedSize / m_config->GetFileLength());
				TRACE("Thread %d: Receive %d, downloaded %.2f%%\n", m_currentThreadIndex, readSize, downloadPercent);
#endif
				singleLock.Unlock();//离开临界区
				if(m_downloadTask->IsStop())
					break;
			}
			downloadFile.Close();
		}

	}
	catch (CInternetException* e)
	{
		errorCode = WSAGetLastError();
		//e->ReportError();
		e->Delete();
	}
	catch (CFileException* e)
	{
		errorCode = WSAGetLastError();
		e->ReportError();
		e->Delete();
	}
	catch (CException* e)
	{
		errorCode = WSAGetLastError();
		e->ReportError();
		e->Delete();
	}

	pHttpFile->Close();
	pHttpConnection->Close();
	session.Close();
	return errorCode;
}

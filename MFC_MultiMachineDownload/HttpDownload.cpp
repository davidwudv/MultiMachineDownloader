#include "stdafx.h"
#include "HttpDownload.h"
#include <atldbcli.h>

#define BUFFSIZE 3072 //һ�ν���3KB
//#define ERROR_HTTP_REDIRECTED 2
using std::auto_ptr;

HttpDownload::HttpDownload(DownloadTask* downloadTask, short currentThread):
	m_downloadTask(downloadTask), m_config(downloadTask->m_config), m_currentThreadIndex(currentThread)
{
}

HttpDownload::~HttpDownload(void)
{
}

int HttpDownload::StartEx()
{
	int errorCode(1);
	CSingleLock singleLock(&m_downloadTask->m_cs);
	CSingleLock singleLock2(&m_downloadTask->m_cs2);
	CStringA strObject(m_config->GetObjectString());
	CStringA strServer(m_config->GetServerString());
	INTERNET_PORT nPort(m_config->GetPort());
	UINT64 start(m_config->m_block[m_currentThreadIndex]->m_ulStart + m_config->m_block[m_currentThreadIndex]->m_ulDownloadedSize);
	UINT64 end(m_config->m_block[m_currentThreadIndex]->m_ulStart + m_config->m_block[m_currentThreadIndex]->m_ulBlockSize - 1);
	int blockSize(m_config->m_block[m_currentThreadIndex]->m_ulBlockSize - m_config->m_block[m_currentThreadIndex]->m_ulDownloadedSize);
	if(blockSize == 0)
		return errorCode;

	int timeOut = 3000;//3s
	m_socket.SetSockOpt(SO_RCVTIMEO, &timeOut, sizeof(int));
	CHAR buff[BUFFSIZE + 1];
	UINT nStatusCode(0);
begin:
	/**** ����Request head ****/
	CStringA strRequest("GET " + strObject + " HTTP/1.1\r\n"
										 + "Host: " + strServer + "\r\n"
										 + "Accept: */*\r\n"
										 + "Pragma: no-cache\r\n"
										 + "Cache-Control: no-cache\r\n"
										 + "Connection: close\r\n");
	if(!m_strReferer.IsEmpty())
		strRequest += "Referer: " + m_strReferer + "\r\n";
	if(m_config->m_bSupportResume)
	{
		CStringA range;
		if(m_config->m_block[m_currentThreadIndex]->m_bIsLastBlock)
			range.Format("Range: bytes=%llu-\r\n", start);
		else
			range.Format("Range: bytes=%llu-%llu\r\n", start, end);
		strRequest += range;
	}
	strRequest += "\r\n";

	if(m_socket.m_hSocket != NULL)
		m_socket.Close();
	m_socket.Create();
	if(!m_socket.Connect(CString(strServer), nPort))
	{
		errorCode = ::WSAGetLastError();
		::AfxMessageBox(_T("socket connect error! error code: %d\n"), errorCode);
		return errorCode;
	}
#ifdef DEBUG
	else
		TRACE("Connected.\n");
#endif
	int result = m_socket.Send(strRequest, strRequest.GetLength());
#ifdef DEBUG
	TRACE("Request has been send, size: %d Byte\n", result);
#endif
	::ZeroMemory(buff, BUFFSIZE + 1);
	int receviedSize = m_socket.Receive(buff, BUFFSIZE);
#ifdef DEBUG
	TRACE("Received %d resposen.\n", receviedSize);
#endif
	if(receviedSize <= 0)
	{
		errorCode = WSAGetLastError();
		//::AfxMessageBox(_T("������������ӱ����ã�error code: %d\n"), errorCode);
		return errorCode;
	}

	CString strResponse(buff);
	int responseHeadSize(0);
	//if(strResponse.IsEmpty())
	//{
	//	//::AfxMessageBox(_T("Response is empty!"));
	//	errorCode = WSAGetLastError();
	//	return errorCode;
	//}
	bool queryResult = QueryStatusCode(strResponse, nStatusCode);
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
			/*��ȡResponse Head���˲��ֲ�д���ļ�*/
			int index = strResponse.Find(_T("\r\n\r\n"));
			CString strResponseHead = strResponse.Mid(0, index + 4);
			responseHeadSize = strResponseHead.GetLength();
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
			int index1 = strResponse.Find(_T("Location: "));
			int index2 = strResponse.Find(_T("\r\n"), index1);
			m_strReferer = strResponse.Mid(index1, index2 - index1);
			goto begin;
		}
	default:
		return -1;
	}

	try
	{
		CFile downloadFile(m_downloadTask->m_savePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyNone);
		int readSize(BUFFSIZE);//ÿ�ι̶����յ�bytes
		float downloadPercent;
		downloadFile.Seek(start, CFile::begin);
		singleLock.Lock();
		downloadFile.Write(buff + responseHeadSize, receviedSize - responseHeadSize);
		m_config->m_block[m_currentThreadIndex]->m_ulDownloadedSize += (receviedSize - responseHeadSize);//���±��ֿ������ش�С
		m_config->m_ulSumDownloadedSize += (receviedSize - responseHeadSize);//�����������ܴ�С
		singleLock.Unlock();
#ifdef DEBUG
		CString str;
		str.Format(_T("Thread %d begin receive data..."), m_currentThreadIndex);
		m_downloadTask->m_masterDialog->m_cListBoxDownloadOutPut.InsertString(0, str);
#endif
		while(receviedSize > 0)
		{
			::ZeroMemory(buff, BUFFSIZE + 1);
			/*** ÿ�ι̶�����BUFFSIZE bytes, �����²���С��BUFFSIZE��ֻ�������µ�bytes, �������Ա���ERROR_INVALID_USER_BUFFER ***/
			readSize = blockSize > BUFFSIZE ? BUFFSIZE : blockSize;
			receviedSize = m_socket.Receive(buff, readSize);
			readSize -= receviedSize;
			while(readSize && (receviedSize > 0))
			{
				int rs = m_socket.Receive(buff + receviedSize, readSize);
				if(rs == 0)//���ӱ��ж�
					break;
				else if(rs == SOCKET_ERROR)//����
				{
					receviedSize = rs;
					break;
				}
				receviedSize += rs;
				readSize -= rs;
			}

			if(receviedSize > 0)
			{
				blockSize -= receviedSize;
#ifdef DEBUG
				TRACE("Thread %d: Receive %d bytes,", m_currentThreadIndex, receviedSize);
#endif
				singleLock.Lock();//�����ٽ���
				downloadFile.Write(buff, receviedSize);
				m_config->m_block[m_currentThreadIndex]->m_ulDownloadedSize += receviedSize;//���±��ֿ������ش�С
				m_config->m_ulSumDownloadedSize += receviedSize;//�����������ܴ�С
				if(m_config->m_ulSumDownloadedSize == m_config->GetFileLength())
					m_downloadTask->m_bFinished = true;//�������
				downloadPercent = 100 * ((float)m_config->m_ulSumDownloadedSize / m_config->GetFileLength());
				m_downloadTask->m_masterDialog->m_cProgress.SetPos(downloadPercent);
				singleLock.Unlock();//�뿪�ٽ���
#ifdef DEBUG
				TRACE(" downloaded %.2f%%\n", downloadPercent);
#endif
				if(m_downloadTask->GetCurrentThreadSum() > 1)
					Sleep(100);//�ó�ʱ��Ƭ
			}
			singleLock2.Lock();
			if(m_downloadTask->IsStop())
				break;
			singleLock2.Unlock();
		}
		downloadFile.Close();
		
		/*if(errorCode == WSAECONNRESET || errorCode == WSAENOTCONN)
		goto begin;*/
	}
	/*catch (CInternetException* e)
	{
		errorCode = WSAGetLastError();
		e->Delete();
	}*/
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
	m_socket.Close();

	if( (receviedSize <= 0) && (blockSize > 0) && (!m_downloadTask->IsStop()) )//���ӱ��жϣ���δ�������
		errorCode = WSAGetLastError();
#ifdef DEBUG
		CString str;
		if(errorCode == WSAENOTCONN)
			str.Format(_T("Thread %d WSAENOTCONN!"), m_currentThreadIndex);
		else if(errorCode == WSAECONNRESET)
			str.Format(_T("Thread %d WSAECONNRESET!"), m_currentThreadIndex);
		/*else if(errorCode == ERROR_INTERNET_TIMEOUT)
			str.Format(_T("Thread %d ERROR_INTERNET_TIMEOUT!"), m_currentThreadIndex);*/
		else if(errorCode == 1)
		{
			if(blockSize == 0)
				str.Format(_T("Thread %d download finished!"), m_currentThreadIndex);
			else if(m_downloadTask->IsStop())
				str.Format(_T("Thread %d stop!"), m_currentThreadIndex);
		}
		else
			str.Format(_T("Thread %d has been stop but unknown why."), m_currentThreadIndex);
		m_downloadTask->m_masterDialog->m_cListBoxDownloadOutPut.InsertString(0, str);
#endif
	return errorCode;
}

bool HttpDownload::QueryStatusCode(CString strResponse, UINT &nStatusCode )
{
	if(strResponse.Find(_T("HTTP")) < 0)
		return false;
	CString strStatusCode = strResponse.Mid(9, 3);
	nStatusCode = _ttoi(strStatusCode);

	return nStatusCode > 0 ? true : false;
}

int HttpDownload::Start()
{
	if(m_config->m_block[m_currentThreadIndex]->m_ulBlockSize <= m_config->m_block[m_currentThreadIndex]->m_ulDownloadedSize)
		return 0;//�˷ֿ��Ѿ����������
	int errorCode(1);
	CInternetSession session;
	auto_ptr<CHttpConnection> pHttpConnection;
	auto_ptr<CHttpFile> pHttpFile;
	CSingleLock singleLock(&m_downloadTask->m_cs);
	CSingleLock singleLock2(&m_downloadTask->m_cs2);

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
		CString str;
		str.Format(_T("Thread %d begin send request"), m_currentThreadIndex);
		m_downloadTask->m_masterDialog->m_cListBoxDownloadOutPut.InsertString(0, str);
#endif
		pHttpFile->SendRequest();
#ifdef DEBUG
		str.Format(_T("Thread %d end send request"), m_currentThreadIndex);
		m_downloadTask->m_masterDialog->m_cListBoxDownloadOutPut.InsertString(0, str);
#endif
		pHttpFile->QueryInfoStatusCode(dwStatusCode);
		if(dwStatusCode >= 200 && dwStatusCode < 300)
		{
			char receiveBuff[BUFFSIZE + 1];
			UINT readSize(1);
			CFile downloadFile(m_downloadTask->m_savePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyNone);
			downloadFile.Seek(start, CFile::begin);
#ifdef DEBUG
			str.Format(_T("Thread %d begin receive data..."), m_currentThreadIndex);
			m_downloadTask->m_masterDialog->m_cListBoxDownloadOutPut.InsertString(0, str);
#endif
			while(readSize)
			{
				ZeroMemory(receiveBuff, BUFFSIZE + 1);
				readSize = pHttpFile->Read(receiveBuff, BUFFSIZE);
				if(readSize > 0)
				{
					singleLock.Lock();//�����ٽ���
					downloadFile.Write(receiveBuff, readSize);
					m_config->m_block[m_currentThreadIndex]->m_ulDownloadedSize += readSize;//���±��ֿ������ش�С
					m_config->m_ulSumDownloadedSize += readSize;//�����������ܴ�С
					if(m_config->m_ulSumDownloadedSize == m_config->GetFileLength())
						m_downloadTask->m_bFinished = true;//�������
					float downloadPercent;
					downloadPercent = 100 * ((float)m_config->m_ulSumDownloadedSize / m_config->GetFileLength());
					m_downloadTask->m_masterDialog->m_cProgress.SetPos(downloadPercent);
#ifdef DEBUG
					TRACE("Thread %d: Receive %d, downloaded %.2f%%\n", m_currentThreadIndex, readSize, downloadPercent);
#endif
					singleLock.Unlock();//�뿪�ٽ���
					if(m_downloadTask->GetCurrentThreadSum() > 1)
						Sleep(100);//�ó�ʱ��Ƭ
				}
				singleLock2.Lock();
				if(m_downloadTask->IsStop())
					break;
				singleLock2.Unlock();
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
#ifdef DEBUG
	CString str;
	if(errorCode == 1)
	{
		if(m_config->m_block[m_currentThreadIndex]->m_ulBlockSize == m_config->m_block[m_currentThreadIndex]->m_ulDownloadedSize)
			str.Format(_T("Thread %d download finished!"), m_currentThreadIndex);
		else if(m_downloadTask->IsStop())
			str.Format(_T("Thread %d stop!"), m_currentThreadIndex);
	}
	else if(errorCode == ERROR_INTERNET_CONNECTION_RESET)
		str.Format(_T("Thread %d ERROR_INTERNET_CONNECTION_RESET!"), m_currentThreadIndex);
	else if(errorCode == ERROR_INTERNET_TIMEOUT)
		str.Format(_T("Thread %d ERROR_INTERNET_TIMEOUT!"), m_currentThreadIndex);
	else
		str.Format(_T("Thread %d has been stop but unknown why!"), m_currentThreadIndex);
	m_downloadTask->m_masterDialog->m_cListBoxDownloadOutPut.InsertString(0, str);
#endif
	return errorCode;
}

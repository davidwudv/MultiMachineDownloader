#include "stdafx.h"
#include "TaskConfig.h"

#define HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE 416//不支持断点续传
using std::auto_ptr;
IMPLEMENT_SERIAL(TaskConfig, CObject, 0)

TaskConfig::TaskConfig(void): 
m_bSupportResume(true), m_sThreadsSum(5), m_ulFileLength(0ULL), m_ulSumDownloadedSize(0ULL)
{
	for(int i = 0; i < 10; ++i)
		m_block[i] = nullptr;
}

TaskConfig::TaskConfig(CString downloadUrl, short threadsSum): 
	m_strLink(downloadUrl), m_ulSumDownloadedSize(0ULL), m_sThreadsSum(threadsSum), m_ulFileLength(0ULL), m_bSupportResume(true)
{
	for(int i = 0; i < 10; ++i)
		m_block[i] = nullptr;
	InitConfig();
}

TaskConfig::TaskConfig(const TaskConfig& config):
	m_strLink(config.m_strLink), m_strFileName(config.m_strFileName), m_ulFileLength(config.m_ulFileLength), m_sThreadsSum(config.m_sThreadsSum),
	m_ulSumDownloadedSize(config.m_ulSumDownloadedSize), m_bSupportResume(config.m_bSupportResume)
{
	for(int i = 0; i < 10; ++i)
	{
		if(config.m_block[i] != nullptr)
		{
			m_block[i] = new Block(config.m_block[i]->m_ulStart, config.m_block[i]->m_ulBlockSize);
			m_block[i]->m_ulDownloadedSize = config.m_block[i]->m_ulDownloadedSize;
		}
	}
}

TaskConfig::~TaskConfig(void)
{
	for(int i = 0; i < m_sThreadsSum; ++i)
	{
		if(m_block[i] != nullptr)
		{
			delete m_block[i];
			m_block[i] = nullptr;
		}
	}
}

bool TaskConfig::InitConfig()
{
	begin:
	CInternetSession session;
	auto_ptr<CHttpConnection> pHttpConnection;
	auto_ptr<CHttpFile> pHttpFile;

	try
	{
		if(AfxParseURL(m_strLink, m_dwServiceType, m_strServer, m_strObject, m_nPort) == FALSE)
		{
			AfxMessageBox(_T("链接解析失败，请检查输入的链接是否正确\n"));
			return false;
		}
		pHttpConnection.reset(session.GetHttpConnection(m_strServer, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_TRANSFER_ASCII, m_nPort));
		pHttpFile.reset(pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_HEAD, m_strObject));
		if(m_bSupportResume == true)
			pHttpFile->AddRequestHeaders(_T("Range: bytes=100-\r\n"));
		pHttpFile->AddRequestHeaders(_T("Pragma: no-cache\r\n"));
		pHttpFile->AddRequestHeaders(_T("Connection: close\r\n"));
		pHttpFile->AddRequestHeaders(_T("Accept: */*\r\n"));
		DWORD dwStatusCode;
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
			CString strFileLength;
			m_strFileName = pHttpFile->GetFileName();
			pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, strFileLength);
			m_ulFileLength = _ttoi64(strFileLength);
			InitBlockInfo();
		}
		else if(dwStatusCode == HTTP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE)
		{
			m_bSupportResume = false;
			m_sThreadsSum = 1;
			session.Close();
			pHttpConnection->Close();
			pHttpFile->Close();
			goto begin;
		}
		else
		{
			AfxMessageBox(_T("无法获取资源！"));
			return false;
		}
	}
	catch (CInternetException* e)
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
	
	session.Close();
	pHttpConnection->Close();
	pHttpFile->Close();
}

void TaskConfig::InitBlockInfo()
{
	if(m_sThreadsSum == 1)
	{
		Block* block = new Block(0ULL, m_ulFileLength);
		block->m_bIsLastBlock = true;
		m_block[0] = block;
		return;
	}
	else
	{
		UINT64 blockSize = m_ulFileLength / m_sThreadsSum;
		UINT64 lastBlockSize = blockSize + (m_ulFileLength % m_sThreadsSum);//余下部分由最后一个线程负责下载
		for(int i = 0; i < m_sThreadsSum; ++i)
		{
			Block* block;
			if(i == 0)
				block = new Block(0ULL, blockSize);
			else if(i == m_sThreadsSum - 1)
			{
				block = new Block(i * blockSize, lastBlockSize);
				block->m_bIsLastBlock = true;
			}
			else
				block = new Block(i * blockSize, blockSize);
			m_block[i] = block;
		}
	}
}

void TaskConfig::SaveToFile(CString strPath)
{
	try
	{
		CFile file(strPath, CFile::modeCreate | CFile::modeWrite);
		CArchive ar(&file, CArchive::store);
		this->Serialize(ar);
		ar.Close();
		file.Close();
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

void TaskConfig::LoadFromFile(CString strPath)
{
	try
	{
		CFile file(strPath, CFile::modeRead);
		CArchive ar(&file, CArchive::load);
		this->Serialize(ar);
		ar.Close();
		file.Close();
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

void TaskConfig::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);
	if(ar.IsStoring())
	{
		ar << m_strLink << m_strFileName << m_ulFileLength << m_sThreadsSum << m_ulSumDownloadedSize
			<< m_strServer << m_strObject << m_dwServiceType << m_nPort;
		for(int i = 0; i < m_sThreadsSum; ++i)
		{
			if(m_block[i] != nullptr)
				ar << m_block[i]->m_ulStart << m_block[i]->m_ulBlockSize << m_block[i]->m_ulDownloadedSize << m_block[i]->m_bIsLastBlock;
		}
	}
	else
	{
		ar >> m_strLink >> m_strFileName >> m_ulFileLength >> m_sThreadsSum >> m_ulSumDownloadedSize
			>> m_strServer >> m_strObject >> m_dwServiceType >> m_nPort;
		for(int i = 0; i < m_sThreadsSum; ++i)
		{
			if(m_block[i] == nullptr)
				m_block[i] = new Block();
			ar >> m_block[i]->m_ulStart >> m_block[i]->m_ulBlockSize >> m_block[i]->m_ulDownloadedSize >> m_block[i]->m_bIsLastBlock;
		}
	}
}

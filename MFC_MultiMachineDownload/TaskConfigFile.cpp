#include "stdafx.h"
#include "TaskConfigFile.h"
//#include <utility>

//IMPLEMENT_DYNCREATE(CTaskConfigFile, CObject)
IMPLEMENT_SERIAL(CTaskConfigFile, CObject, 0)

CTaskConfigFile::CTaskConfigFile(void):
	m_strLink(""), m_strFileName(""),m_strSavePath(""), m_lFileSize(0L), m_lBlockSize(0L), 
	m_lSumDownloadedSize(0L), m_sThreadsSum(0)
{
}

CTaskConfigFile::CTaskConfigFile(CString strLink, CString strSavePath, LONG64 lFileSize, SHORT sThreads = 5):
	m_strLink(strLink), m_strFileName(""),m_strSavePath(strSavePath), m_lFileSize(lFileSize), 
	m_lSumDownloadedSize(0L), m_sThreadsSum(sThreads), m_mapBlockDownloadedSize(sThreads)
{
	m_lBlockSize = m_lFileSize / m_sThreadsSum;
	LONG64 surplus = m_lFileSize % m_sThreadsSum;//余下部分由最后一个线程负责下载
	for(int i = 0; i < m_sThreadsSum; ++i)
	{
		LONG64 start;
		LONG64 end;
		if(i == 0)
			start = i * m_lBlockSize;
		else
			start = i * m_lBlockSize + 1;
		
		if(i == m_sThreadsSum - 1)
			end = (i + 1) * m_lBlockSize + surplus;
		else
			end = (i + 1) * m_lBlockSize;
		m_mapBlockDownloadedSize.SetAt(start, end);
	}
}

CTaskConfigFile::CTaskConfigFile(const CTaskConfigFile& configFile)
{
	m_strFileName = configFile.m_strFileName;
	m_strLink = configFile.m_strLink;
	m_strSavePath = configFile.m_strSavePath;
	m_lBlockSize = configFile.m_lBlockSize;
	m_lFileSize = configFile.m_lFileSize;
	m_lSumDownloadedSize = configFile.m_lSumDownloadedSize;
	m_sThreadsSum = configFile.m_sThreadsSum;
	//m_mapBlockDownloadedSize = configFile.m_mapBlockDownloadedSize;
}

CTaskConfigFile::~CTaskConfigFile(void)
{
}

VOID CTaskConfigFile::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);
	m_mapBlockDownloadedSize.Serialize(ar);
	if(ar.IsStoring())
		ar << m_strLink << m_strFileName << m_strSavePath << m_lFileSize << 
			m_lBlockSize << m_lSumDownloadedSize << m_sThreadsSum;
	else
		ar >> m_strLink >> m_strFileName >> m_strSavePath >> m_lFileSize >>
		m_lBlockSize >> m_lSumDownloadedSize >> m_sThreadsSum;
}

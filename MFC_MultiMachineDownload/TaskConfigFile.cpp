#include "stdafx.h"
#include "TaskConfigFile.h"

IMPLEMENT_SERIAL(TaskConfigFile, CObject, 0)

TaskConfigFile::TaskConfigFile(void):
	m_strLink(""), m_strFileName(""),m_strSavePath(""), m_lFileSize(0L), m_lBlockSize(0L), 
	m_lSumDownloadedSize(0L), m_sThreadsSum(0)
{
}

TaskConfigFile::TaskConfigFile(CString strLink, CString strSavePath, UINT64 lFileSize, SHORT sThreads = 5):
	m_strLink(strLink), m_strFileName(""),m_strSavePath(strSavePath), m_lFileSize(lFileSize), 
	m_lSumDownloadedSize(0L), m_sThreadsSum(sThreads), m_mapBlockDownloadedSize(sThreads)
{
	if(sThreads == 1)
	{
		m_lBlockSize = m_lFileSize;
		m_lLastBlockSize = m_lFileSize;
		UINT64 startKey(0L);
		m_mapBlockDownloadedSize.SetAt(startKey, 0);
		return;
	}
	m_lBlockSize = m_lFileSize / m_sThreadsSum;
	m_lLastBlockSize = m_lBlockSize + (m_lFileSize % m_sThreadsSum);//余下部分由最后一个线程负责下载
	UINT64 startKey;

	for(int i = 0; i < m_sThreadsSum; ++i)
	{
		if(i == 0)
			startKey = 0;
		else
			startKey = i * m_lBlockSize + 1;
		m_mapBlockDownloadedSize.SetAt(startKey, 0);
	}
}

TaskConfigFile::TaskConfigFile(const TaskConfigFile& configFile)
{
}

TaskConfigFile::~TaskConfigFile(void)
{
}

UINT TaskConfigFile::AddDownloadedSize( UINT64 &blockIndex, UINT64 blockDownloadedSize )
{
	UINT64 oldValue;
	UINT64 newValue;
	m_mapBlockDownloadedSize.Lookup(blockIndex, oldValue);
	newValue = oldValue + blockDownloadedSize;
	m_mapBlockDownloadedSize.SetAt(blockIndex, newValue);//更新某分块已下载大小
	m_lSumDownloadedSize += blockDownloadedSize;//更新已下载总大小

	if(m_lSumDownloadedSize == m_lFileSize)
		return DOWNLOAD_FINISHED;
	return DOWNLOADING;
}

VOID TaskConfigFile::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);
	m_mapBlockDownloadedSize.Serialize(ar);
	if(ar.IsStoring())
		ar << m_strLink << m_strFileName << m_strSavePath << m_lFileSize << 
			m_lBlockSize << m_lLastBlockSize << m_lSumDownloadedSize << m_sThreadsSum;
	else
		ar >> m_strLink >> m_strFileName >> m_strSavePath >> m_lFileSize >>
		m_lBlockSize >> m_lLastBlockSize >> m_lSumDownloadedSize >> m_sThreadsSum;
}

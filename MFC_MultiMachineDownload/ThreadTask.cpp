#include "stdafx.h"
#include "ThreadTask.h"

ThreadTask::ThreadTask(void):
	m_blockStart(-1), m_blockSize(0L), m_downloadInfor(nullptr), m_isLastBlock(FALSE)
{
}

ThreadTask::ThreadTask(UINT64 start, UINT64 size, HttpDownload* download):
	m_blockStart(start), m_blockSize(size), m_blockHasDownloadSize(0), m_downloadInfor(download), m_isLastBlock(FALSE)
{
}

ThreadTask::ThreadTask(HttpDownload* download):
	m_blockSize(0), m_blockStart(-1), m_blockHasDownloadSize(0), m_downloadInfor(download), m_isLastBlock(FALSE)
{

}

VOID ThreadTask::Release()
{
	
}

ThreadTask::~ThreadTask(void)
{
	Release();
}

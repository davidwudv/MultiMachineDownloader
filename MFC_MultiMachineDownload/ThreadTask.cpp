#include "stdafx.h"
#include "ThreadTask.h"

ThreadTask::ThreadTask(void):
	m_blockStart(0L), m_blockSize(0L), m_downloadInfor(nullptr), m_isLastBlock(FALSE)
{
}

ThreadTask::ThreadTask(ULONGLONG start, ULONGLONG size, HttpDownload* download):
	m_blockStart(start), m_blockSize(size), m_downloadInfor(download), m_isLastBlock(FALSE)
{
}

ThreadTask::ThreadTask(HttpDownload* download):
	m_downloadInfor(download), m_isLastBlock(FALSE)
{

}

VOID ThreadTask::Release()
{
	if(m_downloadInfor)
	{
		delete m_downloadInfor;
		m_downloadInfor = nullptr;
	}
}

ThreadTask::~ThreadTask(void)
{
	Release();
}

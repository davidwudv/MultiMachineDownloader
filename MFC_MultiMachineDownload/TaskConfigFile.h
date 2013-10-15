#pragma once
class CTaskConfigFile: public CObject
{
	DECLARE_SERIAL(CTaskConfigFile)
public:
	CTaskConfigFile(void);
	CTaskConfigFile(CString strLink, CString strSavePath, ULONGLONG lFileSize, SHORT sThreads);
	//CTaskConfigFile(const CTaskConfigFile&);
	//CTaskConfigFile(CString strLink, CString strSavePath, ULONGLONG lFileSize);
	virtual ~CTaskConfigFile(void);

	CString m_strLink;//下载链接
	CString m_strFileName;//文件名
	CString m_strSavePath;//文件存储路径
	ULONGLONG m_lFileSize;//文件总大小
	ULONGLONG m_lBlockSize;//文件分块大小
	ULONGLONG m_lLastBlockSize;//最后一个分块大小
	ULONGLONG m_lSumDownloadedSize;//已下载的总大小
	SHORT m_sThreadsSum;//线程总数，默认为5
	
	//以字典映射记录各分块已下载的大小，key为分块的起始位置（字节），value为此分块已下载的大小
	CMap<ULONGLONG, ULONGLONG&, ULONGLONG, ULONGLONG&> m_mapBlockDownloadedSize;
	
	VOID Serialize(CArchive& ar);//序列化

};


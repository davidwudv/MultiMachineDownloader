#pragma once
#include <tchar.h>

class Block
{
public:
	Block(void): m_ulStart(0LL), m_ulBlockSize(0ULL), m_ulDownloadedSize(0ULL), m_bIsLastBlock(false) { }
	Block(UINT64 start, UINT64 blockSize): 
		m_ulStart(start), m_ulBlockSize(blockSize), m_ulDownloadedSize(0ULL), m_bIsLastBlock(false) { }
	
	UINT64 m_ulStart;//下载分块的开始位置
	UINT64 m_ulBlockSize;//分块的大小
	UINT64 m_ulDownloadedSize;//已经下载的大小
	bool m_bIsLastBlock;//标识是否是最后一个分块
};

class TaskConfig: public CObject
{
	DECLARE_SERIAL(TaskConfig)
public:
	TaskConfig(void);
	TaskConfig(CString downloadUrl, short threadsSum);
	TaskConfig(const TaskConfig&);
	~TaskConfig(void);
	CString GetDownloadLink() { return m_strLink; }
	CString GetFileName() { return m_strFileName; }
	UINT64 GetFileLength() { return m_ulFileLength; }
	SHORT GetDownloadThreadsSum() { return m_sThreadsSum; }
	CString GetObjectString() { return m_strObject; }
	CString GetServerString() { return m_strServer; }
	DWORD GetServiceType() { return m_dwServiceType; }
	INTERNET_PORT GetPort() { return m_nPort; }
	void SaveToFile(CString strPath);
	void LoadFromFile(CString strPath);
	void Serialize(CArchive& ar);//序列化

private:
	bool InitConfig();
	void InitBlockInfo();//初始化各个分块
	
private:
	CString m_strLink;//下载链接
	CString m_strFileName;//文件名
	UINT64 m_ulFileLength;//文件总大小
	SHORT m_sThreadsSum;//当前任务运行的线程总数，默认值为5，最大值为10
	CString m_strObject, m_strServer;//strServer用于保存服务器地址，strObject用于保存文件对象名称
	DWORD m_dwServiceType;//dwServiceType用于保存服务类型，dwStatus用于保存提交请求返回的状态号
	INTERNET_PORT m_nPort;//用于保存服务器端口号

public:
	Block* m_block[10];
	bool m_bSupportResume;//是否支持断点续传
	UINT64 m_ulSumDownloadedSize;//已下载的总大小
};


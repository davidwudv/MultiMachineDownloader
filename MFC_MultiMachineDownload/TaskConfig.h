#pragma once
#include <tchar.h>

class Block
{
public:
	Block(void): m_ulStart(0LL), m_ulBlockSize(0ULL), m_ulDownloadedSize(0ULL), m_bIsLastBlock(false) { }
	Block(UINT64 start, UINT64 blockSize): 
		m_ulStart(start), m_ulBlockSize(blockSize), m_ulDownloadedSize(0ULL), m_bIsLastBlock(false) { }
	
	UINT64 m_ulStart;//���طֿ�Ŀ�ʼλ��
	UINT64 m_ulBlockSize;//�ֿ�Ĵ�С
	UINT64 m_ulDownloadedSize;//�Ѿ����صĴ�С
	bool m_bIsLastBlock;//��ʶ�Ƿ������һ���ֿ�
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
	void Serialize(CArchive& ar);//���л�

private:
	bool InitConfig();
	void InitBlockInfo();//��ʼ�������ֿ�
	
private:
	CString m_strLink;//��������
	CString m_strFileName;//�ļ���
	UINT64 m_ulFileLength;//�ļ��ܴ�С
	SHORT m_sThreadsSum;//��ǰ�������е��߳�������Ĭ��ֵΪ5�����ֵΪ10
	CString m_strObject, m_strServer;//strServer���ڱ����������ַ��strObject���ڱ����ļ���������
	DWORD m_dwServiceType;//dwServiceType���ڱ���������ͣ�dwStatus���ڱ����ύ���󷵻ص�״̬��
	INTERNET_PORT m_nPort;//���ڱ���������˿ں�

public:
	Block* m_block[10];
	bool m_bSupportResume;//�Ƿ�֧�ֶϵ�����
	UINT64 m_ulSumDownloadedSize;//�����ص��ܴ�С
};


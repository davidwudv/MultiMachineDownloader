#pragma once

//#define DOWNLOADING 0
//#define DOWNLOAD_FINISHED 1

class TaskConfigFile: public CObject
{
	DECLARE_SERIAL(TaskConfigFile)
public:
	TaskConfigFile(void);
	TaskConfigFile(CString strLink, CString strSavePath, UINT64 lFileSize, SHORT sThreads);
	//CTaskConfigFile(CString strLink, CString strSavePath, UINT64 lFileSize);
	~TaskConfigFile(void);
private:
	TaskConfigFile(const TaskConfigFile&);

public:
	CString m_strLink;//��������
	CString m_strFileName;//�ļ���
	CString m_strSavePath;//�ļ��洢·��
	UINT64 m_lFileSize;//�ļ��ܴ�С
	UINT64 m_lBlockSize;//�ļ��ֿ��С
	UINT64 m_lLastBlockSize;//���һ���ֿ��С
	UINT64 m_lSumDownloadedSize;//�����ص��ܴ�С
	SHORT m_sThreadsSum;//�߳�������Ĭ��Ϊ5

	//���ֵ�ӳ���¼���ֿ������صĴ�С��keyΪ�ֿ����ʼλ�ã��ֽڣ���valueΪ�˷ֿ������صĴ�С
	CMap<UINT64, UINT64&, UINT64, UINT64> m_mapBlockDownloadedSize;
	BOOL AddDownloadedSize(UINT64& blockIndex, UINT64 blockDownloadedSize);
	//UINT64 SetDownloadedSize();
	

	VOID Serialize(CArchive& ar);//���л�


};


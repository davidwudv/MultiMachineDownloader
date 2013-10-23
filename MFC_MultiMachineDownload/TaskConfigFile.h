#pragma once

#define DOWNLOADING 0
#define DOWNLOAD_FINISHED 1

class TaskConfigFile: public CObject
{
	DECLARE_SERIAL(TaskConfigFile)
public:
	TaskConfigFile(void);
	TaskConfigFile(CString strLink, CString strSavePath, ULONGLONG lFileSize, SHORT sThreads);
	//CTaskConfigFile(CString strLink, CString strSavePath, ULONGLONG lFileSize);
	~TaskConfigFile(void);
private:
	TaskConfigFile(const TaskConfigFile&);

public:
	CString m_strLink;//��������
	CString m_strFileName;//�ļ���
	CString m_strSavePath;//�ļ��洢·��
	ULONGLONG m_lFileSize;//�ļ��ܴ�С
	ULONGLONG m_lBlockSize;//�ļ��ֿ��С
	ULONGLONG m_lLastBlockSize;//���һ���ֿ��С
	ULONGLONG m_lSumDownloadedSize;//�����ص��ܴ�С
	SHORT m_sThreadsSum;//�߳�������Ĭ��Ϊ5

	//���ֵ�ӳ���¼���ֿ������صĴ�С��keyΪ�ֿ����ʼλ�ã��ֽڣ���valueΪ�˷ֿ������صĴ�С
	CMap<ULONGLONG, ULONGLONG&, ULONGLONG, ULONGLONG> m_mapBlockDownloadedSize;
	DWORD AddDownloadedSize(ULONGLONG blockIndex, ULONGLONG& blockDownloadedSize);
	//ULONGLONG SetDownloadedSize();
	

	VOID Serialize(CArchive& ar);//���л�


};


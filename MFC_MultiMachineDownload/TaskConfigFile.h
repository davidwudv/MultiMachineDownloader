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

	CString m_strLink;//��������
	CString m_strFileName;//�ļ���
	CString m_strSavePath;//�ļ��洢·��
	ULONGLONG m_lFileSize;//�ļ��ܴ�С
	ULONGLONG m_lBlockSize;//�ļ��ֿ��С
	ULONGLONG m_lLastBlockSize;//���һ���ֿ��С
	ULONGLONG m_lSumDownloadedSize;//�����ص��ܴ�С
	SHORT m_sThreadsSum;//�߳�������Ĭ��Ϊ5
	
	//���ֵ�ӳ���¼���ֿ������صĴ�С��keyΪ�ֿ����ʼλ�ã��ֽڣ���valueΪ�˷ֿ������صĴ�С
	CMap<ULONGLONG, ULONGLONG&, ULONGLONG, ULONGLONG&> m_mapBlockDownloadedSize;
	
	VOID Serialize(CArchive& ar);//���л�

};


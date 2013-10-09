#pragma once
class CTaskConfigFile: public CObject
{
	//DECLARE_DYNCREATE(CTaskConfigFile)
	DECLARE_SERIAL(CTaskConfigFile)
public:
	CTaskConfigFile(void);
	CTaskConfigFile(CString strLink, CString strSavePath, LONG64 lFileSize, SHORT sThreads);
	CTaskConfigFile(const CTaskConfigFile&);
	//CTaskConfigFile(CString strLink, CString strSavePath, LONG64 lFileSize);
	virtual ~CTaskConfigFile(void);

	CString m_strLink;//��������
	CString m_strFileName;//�ļ���
	CString m_strSavePath;//�ļ��洢·��
	LONG64 m_lFileSize;//�ļ��ܴ�С
	LONG64 m_lBlockSize;//�ļ��ֿ��С
	LONG64 m_lSumDownloadedSize;//�����ص��ܴ�С
	SHORT m_sThreadsSum;//�߳�������Ĭ��Ϊ5
	
	//���ֵ�ӳ���¼���ֿ������صĴ�С��keyΪ�ֿ����ʼλ�ã��ֽڣ���valueΪ�˷ֿ������صĴ�С
	CMap<LONG64, LONG64&, LONG64, LONG64&> m_mapBlockDownloadedSize;

	VOID Serialize(CArchive& ar);
	//BOOL Save();//����������Ϣ���ļ�
	//BOOL Load(CString path);//���ļ�����������Ϣ
};


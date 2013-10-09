#pragma once
#include "TaskConfigFile.h"

class CHttpDownload
{
public:
	CHttpDownload(void);
	CHttpDownload(CTaskConfigFile);
	virtual ~CHttpDownload(void);

	CTaskConfigFile m_configInfo;//�����ļ���Ϣ
	CString m_strObject, m_strServer, m_strURL;//strServer���ڱ����������ַ��strObject���ڱ����ļ���������
	DWORD m_dwServiceType, m_dwStatus;//dwServiceType���ڱ���������ͣ�dwStatus���ڱ����ύGET���󷵻ص�״̬��
	INTERNET_PORT m_nPort;//���ڱ���������˿ں�
	BOOL Download();

private:
	CInternetSession m_innetSession;
	CHttpFile* m_pHttpFile;
	CHttpConnection* m_pHttpConn;
	BOOL PreDownload();//����ǰ׼��
	BOOL ReceiveData();//��ʼ��������
};


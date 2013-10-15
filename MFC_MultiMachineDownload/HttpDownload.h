#pragma once
#include "TaskConfigFile.h"

#define BUFFSIZE 10240 //һ�ν���10KB

class CHttpDownload
{
public:
	//CHttpDownload(void);
	//CHttpDownload(CTaskConfigFile*, CString,CString, CString, DWORD, INTERNET_PORT);
	CHttpDownload(CString,CString, SHORT);
	virtual ~CHttpDownload(void);

	BOOL Download(CTaskConfigFile*);

private:
	CTaskConfigFile* m_pConfigInfo;//�����ļ���Ϣ
	CString m_strSavePath;//�ļ�����·��
	CString m_strObject, m_strServer, m_strURL;//strServer���ڱ����������ַ��strObject���ڱ����ļ���������
	DWORD m_dwServiceType, m_dwStatus;//dwServiceType���ڱ���������ͣ�dwStatus���ڱ����ύGET���󷵻ص�״̬��
	INTERNET_PORT m_nPort;//���ڱ���������˿ں�
	SHORT m_sThreadsSum;//�߳�����
	CWinThread* m_threadsList[10];//ͬһ�������֧��10���߳�ͬʱ����

	BOOL GetInfor();//����ǰ׼��,��ȡ�ļ���Ϣ
	static UINT ReceiveData(LPVOID);//��ʼ��������
};


#pragma once
#include "TaskConfigFile.h"
#include "CMultiMachineDownloadDlg.h"
#include "ThreadTask.h"

class CMultiMachineDownloadDlg;
class ThreadTask;

class HttpDownload
{
	friend class CMultiMachineDownloadDlg;
public:
	//HttpDownload(void);
	//CHttpDownload(CTaskConfigFile*, CString,CString, CString, DWORD, INTERNET_PORT);
	HttpDownload(CString strURL, CString strSavePath, SHORT sThreadsSum, CMultiMachineDownloadDlg* pWnd);
	~HttpDownload(void);

	BOOL Download(TaskConfigFile*);
	static BOOL QueryStatusCode(CString strResponse, UINT &dwStatus);
	BOOL m_Stop;//��ͣ��������
	BOOL m_Finished;//�������
	CMultiMachineDownloadDlg* m_pWnd;//������

private:
	HttpDownload(const HttpDownload&);

	TaskConfigFile* m_pConfigInfo;//�����ļ���Ϣ
	CString m_strSavePath;//�ļ�����·��
	CString m_strObject, m_strServer, m_strURL;//strServer���ڱ����������ַ��strObject���ڱ����ļ���������
	CString m_strReferer;
	DWORD m_dwServiceType, m_dwStatus;//dwServiceType���ڱ���������ͣ�dwStatus���ڱ����ύGET���󷵻ص�״̬��
	INTERNET_PORT m_nPort;//���ڱ���������˿ں�
	SHORT m_sThreadsSum;//�߳�����
	CWinThread* m_threadsList[10];//ͬһ�������֧��10���߳�ͬʱ����
	CCriticalSection m_cs;//�ٽ���

	BOOL GetInfor();//����ǰ׼��,��ȡ�ļ���Ϣ
	static UINT ReceiveData(LPVOID);//��ʼ��������
	VOID WriteDataToFile(CFile& file, CHAR* buff, UINT writeSize, ThreadTask* pTask);
	VOID Release();
};


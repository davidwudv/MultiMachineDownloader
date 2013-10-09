#pragma once
#include "TaskConfigFile.h"

class CHttpDownload
{
public:
	CHttpDownload(void);
	CHttpDownload(CTaskConfigFile);
	virtual ~CHttpDownload(void);

	CTaskConfigFile m_configInfo;//配置文件信息
	CString m_strObject, m_strServer, m_strURL;//strServer用于保存服务器地址，strObject用于保存文件对象名称
	DWORD m_dwServiceType, m_dwStatus;//dwServiceType用于保存服务类型，dwStatus用于保存提交GET请求返回的状态号
	INTERNET_PORT m_nPort;//用于保存服务器端口号
	BOOL Download();

private:
	CInternetSession m_innetSession;
	CHttpFile* m_pHttpFile;
	CHttpConnection* m_pHttpConn;
	BOOL PreDownload();//下载前准备
	BOOL ReceiveData();//开始接收数据
};


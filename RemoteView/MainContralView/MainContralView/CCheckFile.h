#pragma once
#include"TcpSocket.h"

// CCheckFile 对话框

class CCheckFile : public CDialogEx
{
	DECLARE_DYNAMIC(CCheckFile)

public:
	CTcpSocket* GetTcpSocket();
	void SetTcpSocket(CTcpSocket* pTcpSocket);

private:
	CTcpSocket* m_pTcpClient = NULL;
	BOOL m_bRunning = FALSE;
	int nItem = 0;   //列数
	DWORD TotalFileSize=0; // 下载的文件总大小
	DWORD preFileSize = 0; //  当前已经下载的文件大小

public:
	CCheckFile(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CCheckFile();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DLG_FILE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_lstsFiles; //文件列表
	CProgressCtrl m_Progress; //文件上传进度条
	HANDLE hFileSrc; //打开文件的句柄
	HANDLE hFileDst;//目标文件句柄

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedReturn();
	afx_msg void OnBnClickedUpload();
	afx_msg void OnBnClickedDownload();

	void OnNMDblclkFiles(NMHDR* pNMHDR, LRESULT* pResult); //响应双击点击文件
	void GetDrivers(); //获取盘符
	void GetFilePath(CString& filePath); //获取文件路径
	void ShowDivers(LPBYTE pBuff, DWORD dwBuffSize);  //显示盘符
	void ShowFilePath(LPBYTE pBuff, DWORD dwBuffSizef); //显示文件路径

	void DownLoad(LPBYTE pBuff, DWORD dwBuffSize);  //下载文件
	void GetTolFileSize(LPBYTE pBuff, DWORD dwBuffSize);  //获得文件大小
	void DownLoadOver();  //文件下载完毕


};

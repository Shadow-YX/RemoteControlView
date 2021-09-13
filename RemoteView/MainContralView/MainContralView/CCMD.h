#pragma once
#include"TcpSocket.h"
#include "CLock.h"

// CCMD 对话框

class CCMD : public CDialogEx
{
	DECLARE_DYNAMIC(CCMD)

public:
	CTcpSocket* GetTcpSocket();
	void SetTcpSocket(CTcpSocket* pTcpSocket);

private:
	CTcpSocket* m_pTcpClient = NULL;
	BOOL m_bRunning = FALSE;
	HANDLE 	hRecvThread;
	CLock g_lock;

public:
	CCMD(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CCMD();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DLG_CMD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOpen();
	afx_msg void OnBnClickedStart();

	void ShowMsg(LPBYTE pBuff, DWORD dwBuffSize);



	CEdit m_edtShow;
};

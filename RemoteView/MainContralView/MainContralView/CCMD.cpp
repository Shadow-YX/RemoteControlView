// CCMD.cpp: 实现文件
//

#include "pch.h"
#include "MainContralView.h"
#include "CCMD.h"
#include "afxdialogex.h"
#include "pch.h"
#include "MainContralView.h"
#include "CCMD.h"
#include "afxdialogex.h"
#include <iostream>
using namespace std;
#define SEND_BUFF_SIZE 4096

// CCMD 对话框

IMPLEMENT_DYNAMIC(CCMD, CDialogEx)

CTcpSocket* CCMD::GetTcpSocket()
{
	return m_pTcpClient;
}

void CCMD::SetTcpSocket(CTcpSocket* pTcpSocket)
{
	m_pTcpClient = pTcpSocket;
}

CCMD::CCMD(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DLG_CMD, pParent)
{

}

CCMD::~CCMD()
{
	ExitThread(0);
}

void CCMD::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, EDT_SHOW, m_edtShow);
}


BEGIN_MESSAGE_MAP(CCMD, CDialogEx)
	ON_BN_CLICKED(BTN_OPEN, &CCMD::OnBnClickedOpen)
	ON_BN_CLICKED(BTN_START, &CCMD::OnBnClickedStart)
END_MESSAGE_MAP()


// CCMD 消息处理程序


BOOL CCMD::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//修改标题显示端口号
	CString csFmt;
	csFmt.Format("端口号:%d 正在被控制 ...", ntohs(m_pTcpClient->GetSockaddrIn().sin_port));
	SetWindowText(csFmt);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CCMD::OnBnClickedOpen()
{

	PKGHDR pkgSendCmd;
	pkgSendCmd.m_cmd = DT_CMD;
	pkgSendCmd.m_nLen = 0;
	m_pTcpClient->Send((char*)&pkgSendCmd, sizeof(pkgSendCmd));

}


void CCMD::OnBnClickedStart()
{
	CString cmdstr;
	GetDlgItemText(EDT_MSG, cmdstr); //读取输入命令

	cmdstr += "\r\n";

	PKGHDR pkgSendCmd;
	pkgSendCmd.m_cmd = DT_SENDCMD;
	pkgSendCmd.m_nLen = cmdstr.GetLength();
	m_pTcpClient->Send((char*)&pkgSendCmd, sizeof(pkgSendCmd));
	m_pTcpClient->Send(cmdstr.GetBuffer(), cmdstr.GetLength());

	SetDlgItemText(EDT_MSG, "");
}




void CCMD::ShowMsg(LPBYTE pBuff, DWORD dwBuffSize)
{
	CString strOldBuff;
	CString strBuff;
	strBuff += (char*)pBuff;;

	GetDlgItemText(EDT_SHOW, strOldBuff);
	SetDlgItemText(EDT_SHOW, strOldBuff + strBuff);
	m_edtShow.LineScroll(m_edtShow.GetLineCount());//自动滚屏

}

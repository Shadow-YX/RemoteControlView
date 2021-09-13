// CSetLocalInf.cpp: 实现文件
//

#include "pch.h"
#include "MainContralView.h"
#include "CSetLocalInf.h"
#include "afxdialogex.h"


// CSetLocalInf 对话框

IMPLEMENT_DYNAMIC(CSetLocalInf, CDialogEx)

CSetLocalInf::CSetLocalInf(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DLG_SETLOCALINF, pParent)
{

}

CSetLocalInf::~CSetLocalInf()
{
}

void CSetLocalInf::SetPort(UINT port)
{
	dwPort = port;
}

void CSetLocalInf::SetcsIp(CString ip)
{
	csIP = ip;
}

UINT CSetLocalInf::GetPort()
{
	return dwPort;
}

CString CSetLocalInf::GetIp()
{
	return csIP;
}

void CSetLocalInf::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSetLocalInf, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSetLocalInf::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSetLocalInf::OnBnClickedCancel)
END_MESSAGE_MAP()


// CSetLocalInf 消息处理程序


void CSetLocalInf::OnBnClickedOk()
{
	GetDlgItemText(EDT_LHOST, csIP);
	CString csPort;
	GetDlgItemText(EDT_LPORT, csPort);

	SetPort(atoi(csPort));
	CDialogEx::OnOK();
}


void CSetLocalInf::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}

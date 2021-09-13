#pragma once


// CSetLocalInf 对话框

class CSetLocalInf : public CDialogEx
{
	DECLARE_DYNAMIC(CSetLocalInf)

public:
	CSetLocalInf(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSetLocalInf();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = DLG_SETLOCALINF };
#endif
public:
	UINT dwPort = 0;
	CString csIP;

	void SetPort(UINT port);
	void SetcsIp(CString ip);
	UINT GetPort();
	CString GetIp();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	
};

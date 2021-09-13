
// MainContralViewView.h: CMainContralViewView 类的接口
//

#pragma once
#include<list>
#include"CLock.h"
#include"common.h"
#include "CCMD.h"
#include "CCheckFile.h"
#include "TcpSocket.h"
#include "CSetLocalInf.h"
#define  WM_HANDLECMD WM_USER+1

using namespace std;

class CMainContralViewView : public CView
{
protected: // 仅从序列化创建
	CMainContralViewView() noexcept;
	DECLARE_DYNCREATE(CMainContralViewView)

// 特性
public:
	CMainContralViewDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CMainContralViewView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
public:
	// 端口信息
	UINT dwPort=0 ;
	CString csIP;
	//标记
	bool isOpenCMDFlag=0;
	bool isOpenFileFlag = 0;
	bool isSocketFlag = 0;
	bool isControlMouseFlag = 0;
	bool isControlKeyBoardFlag = 0;

public:
	CCMD* pDlgCmd; //对话框
	CCheckFile* pDlgFile; //文件
	PSCREEN  pScreenShow=nullptr; //屏幕

public:
	CTcpSocket m_tcpSocket;
	HANDLE hThread;
	list<pair<PKGHDR, LPBYTE>> CommandLst;

	CLock CmdLstLock;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnHandlecmd(WPARAM wParam, LPARAM lParam);
public:
	static DWORD CALLBACK RecvThread(LPVOID lpParam);
	BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnControlConnect();
	afx_msg void OnControlStop();
	afx_msg void OnCmd();
	afx_msg void OnFilecheck();
	afx_msg void OnScreenOpen();
	afx_msg void OnScreenStop();

};

#ifndef _DEBUG  // MainContralViewView.cpp 中的调试版本
inline CMainContralViewDoc* CMainContralViewView::GetDocument() const
   { return reinterpret_cast<CMainContralViewDoc*>(m_pDocument); }
#endif


﻿
// MainContralView.h: MainContralView 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"       // 主符号


// CMainContralViewApp:
// 有关此类的实现，请参阅 MainContralView.cpp
//

class CMainContralViewApp : public CWinApp
{
public:
	CMainContralViewApp() noexcept;


// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 实现
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMainContralViewApp theApp;

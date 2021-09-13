
// MainContralViewView.cpp: CMainContralViewView 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MainContralView.h"
#endif

#include "MainContralViewDoc.h"
#include "MainContralViewView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainContralViewView

IMPLEMENT_DYNCREATE(CMainContralViewView, CView)

BEGIN_MESSAGE_MAP(CMainContralViewView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(MN_CONTROL_CONNECT, &CMainContralViewView::OnControlConnect)
	ON_COMMAND(MN_CONTROL_STOP, &CMainContralViewView::OnControlStop)
	ON_COMMAND(MN_CMD, &CMainContralViewView::OnCmd)
	ON_COMMAND(MN_FILECHECK, &CMainContralViewView::OnFilecheck)
	ON_MESSAGE(WM_HANDLECMD, &CMainContralViewView::OnHandlecmd)
	ON_COMMAND(MN_SCREEN_OPEN, &CMainContralViewView::OnScreenOpen)
	ON_COMMAND(MN_SCREEN_STOP, &CMainContralViewView::OnScreenStop)
END_MESSAGE_MAP()

// CMainContralViewView 构造/析构

CMainContralViewView::CMainContralViewView() noexcept
{
	// TODO: 在此处添加构造代码
	CCMD* pDlgCmd = nullptr; //对话框
	CCheckFile* pDlgFile=nullptr; //文件
	PSCREEN  pScreenShow=nullptr; //截屏
}

CMainContralViewView::~CMainContralViewView()
{
}

BOOL CMainContralViewView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMainContralViewView 绘图

void CMainContralViewView::OnDraw(CDC* pDC)
{
	CMainContralViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
	// 把屏幕显示到客户区
	RECT  rt;
	GetClientRect(&rt);
	if (pScreenShow != nullptr)
	{
		CDC dcMem;
		dcMem.CreateCompatibleDC(pDC);
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(pDC,
			pScreenShow->m_dwScreenWith,
			pScreenShow ->m_dwScreenHeigh);
		bmp.SetBitmapBits(pScreenShow ->m_dwDataLen, pScreenShow->m_data);
		pDC->SetStretchBltMode(HALFTONE);
		SetBrushOrgEx(pDC->m_hDC, 0, 0, NULL);
		dcMem.SelectObject(&bmp);


		pDC->StretchBlt(0, 0, rt.right - rt.left, rt.bottom - rt.top,
			&dcMem, 0, 0, pScreenShow ->m_dwScreenWith, pScreenShow ->m_dwScreenHeigh, SRCCOPY);
		
		//清理内存
		delete[](LPBYTE)pScreenShow;
		pScreenShow = NULL;
	}

}


// CMainContralViewView 打印

BOOL CMainContralViewView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMainContralViewView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CMainContralViewView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// CMainContralViewView 诊断

#ifdef _DEBUG
void CMainContralViewView::AssertValid() const
{
	CView::AssertValid();
}

void CMainContralViewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMainContralViewDoc* CMainContralViewView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMainContralViewDoc)));
	return (CMainContralViewDoc*)m_pDocument;
}
#endif //_DEBUG


// CMainContralViewView 消息处理程序

void CMainContralViewView::OnControlConnect()
{
	//启动服务器，创建连接
	BOOL bRet = m_tcpSocket.CreateSocket();

	CSetLocalInf SetLoacIInf;
	SetLoacIInf.DoModal();
	dwPort = SetLoacIInf.GetPort();
	csIP = SetLoacIInf.GetIp();

	bRet = m_tcpSocket.BindListen(csIP.GetBuffer(), dwPort);

	 //本地测试版
	//bRet = m_tcpSocket.BindListen("127.0.0.1", 9527);

	if (!bRet)
	{
		AfxMessageBox("服务器连接失败");
		return;
	}
	AfxMessageBox("主控端连接已启动！ 请打开被控端");

	//m_edtLIp.EnableWindow(FALSE);
	//m_edtLPort.EnableWindow(FALSE);
	//m_lstList.EnableWindow(TRUE);
	//m_bConectServer = TRUE;
	

	if (m_tcpSocket.Accept(&m_tcpSocket))
	{
		AfxMessageBox("有新的被控端已连接！");
	}
	else
	{
		AfxMessageBox("非法连接！！");
	}

	isSocketFlag = 1;

	//创建线程，接收服务端数据连接
	hThread = CreateThread(NULL, 0, RecvThread, this, 0, NULL);

	if (hThread == 0)
	{
		AfxMessageBox("线程创建失败失败");
		return;
	}
	else
	{
		CloseHandle(hThread);
	}
	

	return;
}

//用来接受被控端数据的线程
DWORD CALLBACK CMainContralViewView::RecvThread(LPVOID lpParam)
{
	CMainContralViewView* pThis = (CMainContralViewView*)lpParam;
	while (true)
	{
		//先接受数据包包头
		PKGHDR hdr;
		int nRet = pThis->m_tcpSocket.Recv((char*)&hdr, sizeof(hdr));
		
		//再收包体
		LPBYTE pBuff = NULL;
		if (hdr.m_nLen>0 )
		{
			int nRecved = 0;
			pBuff = new BYTE[hdr.m_nLen];
			while (nRecved < hdr.m_nLen)
			{
				nRet = pThis->m_tcpSocket.Recv(
					(char*)(pBuff + nRecved), hdr.m_nLen - nRecved	);
				nRecved += nRet;
			}
		}
		pThis->CmdLstLock.Lock();
		//将操作命令放入消息队列
		pThis->CommandLst.push_back(pair<PKGHDR, LPBYTE>(hdr, pBuff));
		pThis->CmdLstLock.UnLock();
		pThis->PostMessage(WM_HANDLECMD);

	}
	return 0;
}


LRESULT CMainContralViewView::OnHandlecmd(WPARAM wParam, LPARAM lParam)
{
	CmdLstLock.Lock();
	// 取出消息队列里的命令
	auto parCmd = CommandLst.front();
	CommandLst.pop_front();
	CmdLstLock.UnLock();

	switch (parCmd.first.m_cmd)
	{
	case DT_CMD:
	{
		//显示CMD命令
		pDlgCmd->ShowMsg(parCmd.second, parCmd.first.m_nLen);
	}
	break;
	case DT_DRIVERS:
	{
		//显示磁盘
		pDlgFile->ShowDivers(parCmd.second, parCmd.first.m_nLen);
	}
	break;
	case DT_FILEPATH:
	{	
		//显示文件信息
		pDlgFile->ShowFilePath(parCmd.second, parCmd.first.m_nLen);
	}
	break;
	case DT_FILE_DOWNLOAD:
	{
		//显示文件信息
		pDlgFile->DownLoad(parCmd.second, parCmd.first.m_nLen);
	}
	break;
	case DT_FILE_DOWNLOAD_OVER:
	{
		//显示文件信息
		pDlgFile->DownLoadOver();
	}
	break;
	case DT_FILE_SIZE:
	{
		//显示文件信息
		pDlgFile->GetTolFileSize(parCmd.second, parCmd.first.m_nLen);
	}
	break;
	case DT_SCREENSTART:
	{
		//处理屏幕命令，显示到客户区
		pScreenShow = (PSCREEN)parCmd.second;
		InvalidateRect(NULL, FALSE);
	}
	break;
	case DT_SCREENSTOP:
	{
		//关闭屏幕
		pScreenShow = nullptr;
	}
	break;
	}

	return 0;
}



void CMainContralViewView::OnControlStop()
{

	if (isSocketFlag == 0);
	{
		AfxMessageBox("请先启动服务器");
		return;
	}
	
	//清理资源
	m_tcpSocket.CloseSocket();
	isSocketFlag = 0;
	//m_lstList.DeleteAllItems();
	/*
	//重置对话框
	m_edtLIp.EnableWindow(TRUE);
	m_edtLPort.EnableWindow(TRUE);
	m_lstList.EnableWindow(FALSE);
	m_bConectServer = FALSE;
	*/
	AfxMessageBox("服务器已关闭");
}

void CMainContralViewView::OnScreenOpen()
{
	//开启远程桌面
	PKGHDR hdr;
	hdr.m_cmd = DT_SCREENSTART;
	hdr.m_nLen = 0;
	m_tcpSocket.Send((char*)&hdr, sizeof(hdr));

	// 打开键盘控制标记
	isControlMouseFlag = 1;
	//打开鼠标控制
	isControlKeyBoardFlag = 1;
}


void CMainContralViewView::OnScreenStop()
{
	//关闭远程桌面
	PKGHDR hdr;
	hdr.m_cmd = DT_SCREENSTOP;
	hdr.m_nLen = 0;
	m_tcpSocket.Send((char*)&hdr, sizeof(hdr));

	//关闭鼠标控制标记		
	isControlMouseFlag = 0;
	// 关闭键盘控制标记
	isControlKeyBoardFlag = 0;
}


void CMainContralViewView::OnCmd()
{
	//执行CMD
	if (isOpenCMDFlag == 0)
	{
		//CTcpSocket* pTcpSocket = (CTcpSocket*)m_lstList.GetItemData(m_nItem);
		if (isSocketFlag==1)
		{
			pDlgCmd = new CCMD();
			pDlgCmd->SetTcpSocket(&m_tcpSocket);
		}
		else
		{
			AfxMessageBox("未连接被控端");
			return;
		}
		pDlgCmd->Create(DLG_CMD, this);
		isOpenCMDFlag = 1;
	}
	pDlgCmd->ShowWindow(SW_SHOWNORMAL);
	

}


void CMainContralViewView::OnFilecheck()
{
	/// 文件查看
	if (isOpenFileFlag == 0)
	{
		//CTcpSocket* pTcpSocket = (CTcpSocket*)m_lstList.GetItemData(m_nItem);
		if (isSocketFlag == 1)
		{
			pDlgFile = new CCheckFile();
			pDlgFile->SetTcpSocket(&m_tcpSocket);
 		}
		else
		{
			AfxMessageBox("未连接被控端");
			return;
		}
		pDlgFile->Create(DLG_FILE, this);
		isOpenFileFlag = 1;
	}

	pDlgFile->ShowWindow(SW_SHOWNORMAL);

}

//处理鼠标键盘消息
BOOL CMainContralViewView::PreTranslateMessage(MSG* pMsg)
{

	switch (pMsg->message)
	{
	case WM_MOUSEMOVE:
	{
		if (isControlMouseFlag)
		{
			ONMOUSE pt(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam), MOUSEMOVE);
			PKGHDR  PkgPt;
			PkgPt.m_cmd = DT_MOUSEMOVE;
			PkgPt.m_nLen = sizeof(pt);

			m_tcpSocket.Send((char*)&PkgPt, sizeof(PkgPt));
			m_tcpSocket.Send((char*)&pt, sizeof(pt));
		}
	}

	break;
	case WM_LBUTTONDOWN:
	{
		if (isControlMouseFlag)
		{

			ONMOUSE pt(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam), LMOUSEDOWN);
			PKGHDR  PkgPt;
			PkgPt.m_cmd = DT_MOUSEMOVE;
			PkgPt.m_nLen = sizeof(pt);

			m_tcpSocket.Send((char*)&PkgPt, sizeof(PkgPt));
			m_tcpSocket.Send((char*)&pt, sizeof(pt));
		}
	}
	break;
	case WM_LBUTTONUP:
	{
		if 	(isControlMouseFlag )
		{

			ONMOUSE pt(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam), LMOUSEUP);
			PKGHDR  PkgPt;
			PkgPt.m_cmd = DT_MOUSEMOVE;
			PkgPt.m_nLen = sizeof(pt);

			m_tcpSocket.Send((char*)&PkgPt, sizeof(PkgPt));
			m_tcpSocket.Send((char*)&pt, sizeof(pt));
		}
	}
	break;
	case WM_RBUTTONDOWN:
	{
		if (isControlMouseFlag)
		{
			ONMOUSE pt(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam), RMOUSEDOWN);
			PKGHDR  PkgPt;
			PkgPt.m_cmd = DT_MOUSEMOVE;
			PkgPt.m_nLen = sizeof(pt);

			m_tcpSocket.Send((char*)&PkgPt, sizeof(PkgPt));
			m_tcpSocket.Send((char*)&pt, sizeof(pt));
		}

	}
	break;
	case WM_RBUTTONUP:
	{
		if (isControlMouseFlag)
		{
			ONMOUSE pt(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam), RMOUSEUP);
			PKGHDR  PkgPt;
			PkgPt.m_cmd = DT_MOUSEMOVE;
			PkgPt.m_nLen = sizeof(pt);

			m_tcpSocket.Send((char*)&PkgPt, sizeof(PkgPt));
			m_tcpSocket.Send((char*)&pt, sizeof(pt));
		}

	}
	break;
	case  WM_KEYDOWN:
	{
		if (isControlKeyBoardFlag )
		{
			PKGHDR  PkgChar;
			PkgChar.m_cmd = DT_KEYBOADEVENT;
			PkgChar.m_nLen = sizeof(WPARAM);

			m_tcpSocket.Send((char*)&PkgChar, sizeof(PkgChar));
			m_tcpSocket.Send((char*)&pMsg->wParam, sizeof(UINT));

		}

	}
	}
	return CView::PreTranslateMessage(pMsg);
}
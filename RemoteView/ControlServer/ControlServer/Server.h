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

	HANDLE hFileDst;//���ļ����
	HANDLE hFileSrc;//Ŀ���ļ����

public:
	static DWORD WINAPI SreenProc(LPVOID lpParam);
private:
	CTcpSocket m_tcpSocket;
	BOOL  isScreen = FALSE;

public:
	//CMD
	void OnCMD();
	void OnSendCMD(LPBYTE pCmd, DWORD dwSize);

	//�ļ����
	void OnGetFilePATH(LPBYTE szbuff, DWORD dwSize);
	void OnGetDrivers();

	//�ϴ�
	void OnCreatrFile(LPBYTE szbuff, DWORD dwSize); 
	void OnWriteFile(LPBYTE szbuff, DWORD dwSize);
	void OnWriteFileOver();

	//����
	void OnCreateUpDownFile(LPBYTE szbuff, DWORD dwSize);
	void OnWriteFileOverupDown();

	//��ʾ��Ļ
	void SendPreScreen();

	//  ��������¼�
	void MouseEvent(PONMOUSE MousePt);
	void MouseMove(PONMOUSE MousePt);  //��������ƶ�����
	void MouseLbuttonDown(PONMOUSE MousePt);  //��������������
	void MouseLbuttonUp(PONMOUSE MousePt);  //��������������
	void MouseRbuttonDown(PONMOUSE MousePt);  //��������Ҽ�����
	void MouseRbuttonUp(PONMOUSE MousePt);  //��������Ҽ�����
	
	//��������¼�
	void KeyBoardEvent(UINT* pChar);
};


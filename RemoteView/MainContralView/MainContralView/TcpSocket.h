#pragma once
#include <windows.h>
#include "common.h"

class CTcpSocket
{
public:
	CTcpSocket();
	~CTcpSocket();

	BOOL CreateSocket();
	BOOL BindListen(char* szIp, u_short nPort);
	BOOL Accept(CTcpSocket* pTcpSocket);
	BOOL Connect(char* szIp, u_short nPort);
	BOOL Recv(char* pBuff, int pLen);//收数据
	BOOL Send(char* pBuff, int pLen);//发数据


	void CloseSocket();

	const sockaddr_in& GetSockaddrIn()const;

	SOCKET m_socket;
	sockaddr_in m_si;
};


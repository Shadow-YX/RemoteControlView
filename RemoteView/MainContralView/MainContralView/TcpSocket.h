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
	BOOL Recv(char* pBuff, int pLen);//������
	BOOL Send(char* pBuff, int pLen);//������


	void CloseSocket();

	const sockaddr_in& GetSockaddrIn()const;

	SOCKET m_socket;
	sockaddr_in m_si;
};


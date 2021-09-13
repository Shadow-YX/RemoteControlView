#include "TcpSocket.h"


CTcpSocket::CTcpSocket()
{
}


CTcpSocket::~CTcpSocket()
{
	CloseSocket();
}

BOOL CTcpSocket::CreateSocket()
{
	m_socket = socket(
		AF_INET,
		SOCK_STREAM, //��ʽ 
		IPPROTO_TCP);//tcpЭ��
	if (m_socket == SOCKET_ERROR)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL CTcpSocket::BindListen(char* szIp, u_short nPort)
{
	// 	2) �󶨶˿�
	m_si.sin_family = AF_INET;
	m_si.sin_port = htons(nPort);
	m_si.sin_addr.S_un.S_addr = inet_addr(szIp);
	int nRet = bind(m_socket, (sockaddr*)&m_si, sizeof(m_si));
	if (nRet == SOCKET_ERROR)
	{
		return FALSE;
	}

	// 	3) ����
	nRet = listen(m_socket, SOMAXCONN);
	if (nRet == SOCKET_ERROR)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CTcpSocket::Accept(CTcpSocket* pTcpSocket)
{
	// 	4) ��������
	sockaddr_in siClient;
	int nSize = sizeof(siClient);
	SOCKET sockClient = accept(m_socket, (sockaddr*)&siClient, &nSize);
	if (sockClient == SOCKET_ERROR)
	{
		return FALSE;
	}

	pTcpSocket->m_socket = sockClient;
	pTcpSocket->m_si = siClient;
	return TRUE;
}

BOOL CTcpSocket::Connect(char* szIp, u_short nPort)
{
	sockaddr_in siServer;
	siServer.sin_family = AF_INET;
	siServer.sin_port = htons(nPort);
	siServer.sin_addr.S_un.S_addr = inet_addr(szIp);
	int nRet = connect(m_socket, (sockaddr*)&siServer, sizeof(siServer));
	if (nRet == SOCKET_ERROR)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CTcpSocket::Recv(char* pBuff, int pnLen/*���봫��, �����յ����ֽ���*/)
{
	int nRet = recv(m_socket, pBuff,  pnLen, 0);
	if (nRet == 0 || nRet == SOCKET_ERROR)
	{
		return FALSE;
	}

	return nRet;
}

BOOL CTcpSocket::Send(char* pBuff, int  pnLen/*���봫��, �������͵��ֽ���*/)
{
	int nRet = send(m_socket, pBuff, pnLen, 0);
	if (nRet == SOCKET_ERROR)
	{
		return FALSE;
	}

	return nRet;
}



void CTcpSocket::CloseSocket()
{
	closesocket(m_socket);
	m_socket = INVALID_SOCKET;
}

const sockaddr_in& CTcpSocket::GetSockaddrIn() const
{
	return m_si;
}


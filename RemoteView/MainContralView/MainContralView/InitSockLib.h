#pragma once
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

class CInitSockLib
{
public:
	CInitSockLib();
	~CInitSockLib();

private:
	static CInitSockLib m_sock;
};


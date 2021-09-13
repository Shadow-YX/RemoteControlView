// ControlServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include"Server.h"
using namespace std;
void RemoteUI();

int main()
{
	RemoteUI();
	while (TRUE)
	{
		Server tcpServer;
	
		int nPort = 0;
		char szIpAddress[20];
		cout << "输入连接IP地址: ";
		//这里设置被控端端口号
		cin >> szIpAddress;
		cout << "输入连接端口号: ";
		cin >> nPort;
		BOOL nRet=tcpServer.CreateServer(szIpAddress, nPort);

	
		//BOOL nRet = tcpServer.CreateServer((char*)"127.0.0.1", 9527);
		if (nRet )
		{
			//启动服务器
			tcpServer.RunServer();
		}
		cout << "IP地址或端口号输入错误或未打开主控端" << endl;

	}


	return 0;
}

void RemoteUI()
{
	cout << "            **************************************************************\r\n";
	cout << "            *							         *\r\n";
	cout << "            *			远程控制系统V1.0	                 *\r\n";
	cout << "            *							         *\r\n";
	cout << "            *							         *\r\n";
	cout << "            *							         *\r\n";
	cout << "            *							         *\r\n";
	cout << "            *							         *\r\n";
	cout << "            *							         *\r\n";
	cout << "            **************************************************************\r\n";

	
}

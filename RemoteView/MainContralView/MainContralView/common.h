#pragma once

enum DataType
{
	//登录
	DT_LOGIN, //登录信息
	//文件
	DT_FILE,   //获取文件信息
	DT_FILEPATH, //文件路径
	DT_GETDIRVERS, //获取盘符
	DT_DRIVERS, //返回盘符
	DT_GETFILEPATH,  //获取文件路径
	//文件上传/下载
	DT_FILE_ORI_PATH,  //文件源路径
	DT_FILE_TAR_PATH, //文件目标路径
	DT_FILE_UPLOAD, //上传文件
	DT_FILE_UPLOAD_OVER, //文件上传完毕
	DT_FILE_DOWNLOAD,//下载文件
	DT_FILE_DOWNLOAD_OVER,//下载文件完毕
	DT_FILE_SIZE, //文件的总大小
	//CMD
	DT_CMD,            //获取CMD命令行内容
	DT_SENDCMD,		 //发送CMD内容
	DT_SHOWCMD,  //显示CMD命令内容
	//截屏
	DT_SCREENSTART,    //开启屏幕控制
	DT_SCREENSTOP,   //关闭屏幕控制
	//IO操作
	DT_MOUSEMOVE,  //鼠标操作
	DT_KEYBOADEVENT, //键盘操作

};


//数据的包头
typedef struct tagPkgHeader
{
	tagPkgHeader()
	{
		m_nLen = 0;
	}

	DataType m_cmd;
	DWORD m_nLen;
}PKGHDR, * PPKGHDR;


//文件信息包头
typedef struct tagFileInfo
{
	TCHAR          FileName[MAX_PATH];  //文件名
	BOOL            FileType;//文件类型
	//DWORD   FileSize;  //文件大小
}FILEINFO, * PFILEINFO;

//屏幕命令包体
typedef struct tagScreenInfo
{
	DWORD m_dwScreenWith;
	DWORD m_dwScreenHeigh;
	DWORD m_dwDataLen;
	BYTE m_data[1];
}SCREEN, * PSCREEN;

//鼠标事件操作
enum MOUSEEVENT
{
	MOUSEMOVE, //鼠标移动
	LMOUSEDOWN, //左键按下
	RMOUSEDOWN, //右键按下
	LMOUSEUP, //左键弹起
	RMOUSEUP //右键弹起
};

//鼠标事件包头
typedef struct tagMouseEvent
{
	tagMouseEvent() { }
	tagMouseEvent(DWORD x, DWORD y, MOUSEEVENT cmd)
		:_x(x), _Y(y), _MouseCmd(cmd)
	{

	}
	MOUSEEVENT _MouseCmd;
	DWORD _x;
	DWORD _Y;

}ONMOUSE, * PONMOUSE;

//键盘事件包头
typedef struct tagKeyBoardEvent
{
	tagKeyBoardEvent() { }
	tagKeyBoardEvent(DWORD c) :
		_Char(c)
	{

	}
	DWORD _Char;

}ONKEYBOARD, * PONKEYBOARD;
#pragma once

enum DataType
{
	//��¼
	DT_LOGIN, //��¼��Ϣ
	//�ļ�
	DT_FILE,   //��ȡ�ļ���Ϣ
	DT_FILEPATH, //�ļ�·��
	DT_GETDIRVERS, //��ȡ�̷�
	DT_DRIVERS, //�����̷�
	DT_GETFILEPATH,  //��ȡ�ļ�·��
	//�ļ��ϴ�/����
	DT_FILE_ORI_PATH,  //�ļ�Դ·��
	DT_FILE_TAR_PATH, //�ļ�Ŀ��·��
	DT_FILE_UPLOAD, //�ϴ��ļ�
	DT_FILE_UPLOAD_OVER, //�ļ��ϴ����
	DT_FILE_DOWNLOAD,//�����ļ�
	DT_FILE_DOWNLOAD_OVER,//�����ļ����
	DT_FILE_SIZE, //�ļ����ܴ�С
	//CMD
	DT_CMD,            //��ȡCMD����������
	DT_SENDCMD,		 //����CMD����
	DT_SHOWCMD,  //��ʾCMD��������
	//����
	DT_SCREENSTART,    //������Ļ����
	DT_SCREENSTOP,   //�ر���Ļ����
	//IO����
	DT_MOUSEMOVE,  //������
	DT_KEYBOADEVENT, //���̲���

};


//���ݵİ�ͷ
typedef struct tagPkgHeader
{
	tagPkgHeader()
	{
		m_nLen = 0;
	}

	DataType m_cmd;
	DWORD m_nLen;
}PKGHDR, * PPKGHDR;


//�ļ���Ϣ��ͷ
typedef struct tagFileInfo
{
	TCHAR          FileName[MAX_PATH];  //�ļ���
	BOOL            FileType;//�ļ�����
	//DWORD   FileSize;  //�ļ���С
}FILEINFO, * PFILEINFO;

//��Ļ�������
typedef struct tagScreenInfo
{
	DWORD m_dwScreenWith;
	DWORD m_dwScreenHeigh;
	DWORD m_dwDataLen;
	BYTE m_data[1];
}SCREEN, * PSCREEN;

//����¼�����
enum MOUSEEVENT
{
	MOUSEMOVE, //����ƶ�
	LMOUSEDOWN, //�������
	RMOUSEDOWN, //�Ҽ�����
	LMOUSEUP, //�������
	RMOUSEUP //�Ҽ�����
};

//����¼���ͷ
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

//�����¼���ͷ
typedef struct tagKeyBoardEvent
{
	tagKeyBoardEvent() { }
	tagKeyBoardEvent(DWORD c) :
		_Char(c)
	{

	}
	DWORD _Char;

}ONKEYBOARD, * PONKEYBOARD;
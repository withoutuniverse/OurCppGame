﻿#pragma once
#include <winsock2.h>
#include <process.h>               //C中的线程 
#include <string>
#include <fstream>
#include <string>
#include "cocos2d.h"
#pragma comment(lib,"ws2_32.lib")


class GameClient
{
public:
	bool virtual init();
	void ClientProcess();    //启动客户处理
	static DWORD WINAPI Send(LPVOID lpParam);    //发送线程
	static DWORD WINAPI Receive(LPVOID lpParam);  //接受线程
	int SendMsg(char *buf);      //向服务器发送信息
	void SendCode(char sendbuf[]);
	void CleanUp();      //资源回收
	//SOCKET相关
public:
	SOCKET ClientSocket;
	struct sockaddr_in ServerAddr;

	DWORD SendThreadID;
	DWORD RecvThreadID;

	char RecvBuf[1024];
	char SendBuf[1024];
	//网络数据处理
public:

};
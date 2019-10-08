#pragma once

#include <QtWidgets/QWidget>
#include <QMessageBox>
#include "ui_QCmd.h"
#include <Windows.h>
#include <strsafe.h>
#include <process.h>

#define SEND_SIZE   256
#define RESULT_SIZE 2048

class QCmd : public QWidget
{
	Q_OBJECT

public:
	QCmd(QWidget *parent = Q_NULLPTR);
	// 处理后事
	~QCmd();
	// 错误提示
	VOID ShowError(LPCTSTR pszText);
	// 初始化程序 创建管道及程序
	BOOL Init();
	// 与cmd交互
	BOOL sendCmd(PCHAR psCmd, DWORD dwBuffersize);
	// 刷新出cmd的回显
	BOOL flushCmd(PCHAR psResultBuffer, DWORD dwBuffersize);

private:
	Ui::QCmdClass ui;
	// 四个HANDLE 用来创建两个管道
	HANDLE hReadPipe1, hWritePipe1, hReadPipe2, hWritePipe2;
	// 创建命令行进程之后拿到的pi 控制进程
	PROCESS_INFORMATION ProcessInformation = { 0 };
	// 准备缓存
	CHAR szSend[SEND_SIZE] = { 0 };
	CHAR szResult[RESULT_SIZE] = { 0 };
};

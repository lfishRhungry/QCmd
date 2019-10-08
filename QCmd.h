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
	// �������
	~QCmd();
	// ������ʾ
	VOID ShowError(LPCTSTR pszText);
	// ��ʼ������ �����ܵ�������
	BOOL Init();
	// ��cmd����
	BOOL sendCmd(PCHAR psCmd, DWORD dwBuffersize);
	// ˢ�³�cmd�Ļ���
	BOOL flushCmd(PCHAR psResultBuffer, DWORD dwBuffersize);

private:
	Ui::QCmdClass ui;
	// �ĸ�HANDLE �������������ܵ�
	HANDLE hReadPipe1, hWritePipe1, hReadPipe2, hWritePipe2;
	// ���������н���֮���õ���pi ���ƽ���
	PROCESS_INFORMATION ProcessInformation = { 0 };
	// ׼������
	CHAR szSend[SEND_SIZE] = { 0 };
	CHAR szResult[RESULT_SIZE] = { 0 };
};

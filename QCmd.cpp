#include "QCmd.h"

QCmd::QCmd(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	// -------------------------界面设计---------------------
	this->setWindowTitle("QCommandLine");
	this->setMinimumSize(800, 800);

	ui.groupInput->setTitle("Input your command here");
	ui.groupCur->setTitle("Current command");

	ui.buttonFlush->setText("Flush");
	ui.buttonClear->setText("Clear");

	ui.lineEditCur->setReadOnly(true);
	ui.plainTextEdit->setReadOnly(true);

	// -------------------------按键逻辑---------------------
	
	// 在输入框按下回车就是flush
	connect(ui.lineEditInput, &QLineEdit::returnPressed, [=]() {
		ui.buttonFlush->clicked();
		});

	// clear清楚输入和回显框
	connect(ui.buttonClear, &QPushButton::clicked, [=]() {
		ui.lineEditCur->setText("");
		ui.plainTextEdit->setPlainText("");
		// 回到输入框焦点
		ui.lineEditInput->setFocus();
		});

	// ----------------------------------主逻辑--------------------------------------

	connect(ui.buttonFlush, &QPushButton::clicked, [=]() {

		QString qsCmd = ui.lineEditInput->text();

		// 如果输入框为空就不输入数据直接刷剩余结果
		if (!qsCmd.isEmpty())
		{
			// 刷新现有命令框
			ui.lineEditCur->setText(qsCmd);
			// 发送命令 为了兼容中文cmd的GBK编码
			StringCchCopyA(szSend, SEND_SIZE, qsCmd.toLocal8Bit().data());
			if (!sendCmd(szSend, SEND_SIZE)) {
				ui.plainTextEdit->appendPlainText("[send command failed...]");
			}
			// 清空输入框
			ui.lineEditInput->setText("");
			// 等待命令执行
			Sleep(100);
		}

		// 刷回显结果
		if (!flushCmd(szResult, RESULT_SIZE)) {
			ui.plainTextEdit->appendPlainText("[flush result failed...]");
			return;
		}

		// 为了兼容中文cmd的GBK编码
		ui.plainTextEdit->appendPlainText(QString::fromLocal8Bit(szResult));

		});

	// 初始化
	if (!Init()) {
		this->close();
	}
	ui.plainTextEdit->appendPlainText("[Initiating successfully, send command now ~]");
	// 先flush一下显示初始化信息
	ui.buttonFlush->clicked();
}

// 创建管道及cmd程序
BOOL QCmd::Init() {

	// 创建匿名管道
	SECURITY_ATTRIBUTES securityAttributes = { 0 };
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.nLength = sizeof(securityAttributes);
	securityAttributes.lpSecurityDescriptor = NULL;
	if ((!::CreatePipe(&hReadPipe1, &hWritePipe1, &securityAttributes, 0))|
		(!::CreatePipe(&hReadPipe2, &hWritePipe2, &securityAttributes, 0))) {
		ShowError(TEXT("create pipe failed"));
		return FALSE;
	}

	// 创建cmd进程
	STARTUPINFO si = { 0 };
	// 隐藏窗口
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	// 设置标准输入输出和错误
	si.hStdInput = hReadPipe2;
	si.hStdOutput = si.hStdError = hWritePipe1;
	// 拼接启动命令行的命令
	TCHAR szCmdLine[256] = { 0 };
	GetSystemDirectory(szCmdLine, sizeof(szCmdLine));
	StringCchCat(szCmdLine, _countof(szCmdLine), TEXT("\\cmd.exe"));
	if (CreateProcess(szCmdLine, NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &ProcessInformation) == 0)
	{
		ShowError(TEXT("create cmd process failed"));
		return FALSE;
	}

	return TRUE;
}

// 处理后事
QCmd::~QCmd() {
	// 程序里面关于处理后事的部分（包括其他地方）
	// 都做得不够优雅和全面
	// 但是程序的重点不是这里 忍了（好吧是因为懒。。。）
	CloseHandle(hReadPipe1);
	CloseHandle(hReadPipe2);
	CloseHandle(hWritePipe1);
	CloseHandle(hWritePipe2);
	CloseHandle(ProcessInformation.hThread);
	CloseHandle(ProcessInformation.hProcess);
}

// 与cmd交互
// 注意 这里统一使用ANSI格式跟cmd打交道
BOOL QCmd::sendCmd(PCHAR psCmd, DWORD dwBuffersize) {

	DWORD dwBytesWrite = 0;

	// 注意我们的命令里面是没有换行符的 主动加入换行符比较安全
	StringCchCatA(psCmd, dwBuffersize, "\r\n");
	// 第三个参数填写要输入字符的数量即可
	if (!WriteFile(hWritePipe2, psCmd, lstrlenA(psCmd), &dwBytesWrite, NULL)) {
		return FALSE;
	}

	return(TRUE);
}

// 刷新出cmd的回显
BOOL QCmd::flushCmd(PCHAR psResultBuffer, DWORD dwBuffersize) {

	DWORD dwBytesResult = 0;
	RtlZeroMemory(psResultBuffer, dwBuffersize);

	// 先看有没有数据 防止没有数据还去读引起堵塞
	if (!PeekNamedPipe(hReadPipe1, psResultBuffer, dwBuffersize, &dwBytesResult, NULL, NULL)) {
		return FALSE;
	}

	// 刷新成功但是没有数据
	if (!dwBytesResult) {
		return TRUE;
	}

	// 有数据就读
	RtlZeroMemory(psResultBuffer, dwBuffersize);
	if (!ReadFile(hReadPipe1, psResultBuffer, dwBuffersize, &dwBytesResult, NULL)) {
		return FALSE;
	}

	return TRUE;
}


// 错误提示
VOID QCmd::ShowError(LPCTSTR pszText) {

	TCHAR szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, TEXT("%s Error[%d]\n"), pszText, ::GetLastError());
	QMessageBox::critical(this, "Error", QString::fromWCharArray(szErr));
}


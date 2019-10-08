#include "QCmd.h"

QCmd::QCmd(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	// -------------------------�������---------------------
	this->setWindowTitle("QCommandLine");
	this->setMinimumSize(800, 800);

	ui.groupInput->setTitle("Input your command here");
	ui.groupCur->setTitle("Current command");

	ui.buttonFlush->setText("Flush");
	ui.buttonClear->setText("Clear");

	ui.lineEditCur->setReadOnly(true);
	ui.plainTextEdit->setReadOnly(true);

	// -------------------------�����߼�---------------------
	
	// ��������»س�����flush
	connect(ui.lineEditInput, &QLineEdit::returnPressed, [=]() {
		ui.buttonFlush->clicked();
		});

	// clear�������ͻ��Կ�
	connect(ui.buttonClear, &QPushButton::clicked, [=]() {
		ui.lineEditCur->setText("");
		ui.plainTextEdit->setPlainText("");
		// �ص�����򽹵�
		ui.lineEditInput->setFocus();
		});

	// ----------------------------------���߼�--------------------------------------

	connect(ui.buttonFlush, &QPushButton::clicked, [=]() {

		QString qsCmd = ui.lineEditInput->text();

		// ��������Ϊ�վͲ���������ֱ��ˢʣ����
		if (!qsCmd.isEmpty())
		{
			// ˢ�����������
			ui.lineEditCur->setText(qsCmd);
			// �������� Ϊ�˼�������cmd��GBK����
			StringCchCopyA(szSend, SEND_SIZE, qsCmd.toLocal8Bit().data());
			if (!sendCmd(szSend, SEND_SIZE)) {
				ui.plainTextEdit->appendPlainText("[send command failed...]");
			}
			// ��������
			ui.lineEditInput->setText("");
			// �ȴ�����ִ��
			Sleep(100);
		}

		// ˢ���Խ��
		if (!flushCmd(szResult, RESULT_SIZE)) {
			ui.plainTextEdit->appendPlainText("[flush result failed...]");
			return;
		}

		// Ϊ�˼�������cmd��GBK����
		ui.plainTextEdit->appendPlainText(QString::fromLocal8Bit(szResult));

		});

	// ��ʼ��
	if (!Init()) {
		this->close();
	}
	ui.plainTextEdit->appendPlainText("[Initiating successfully, send command now ~]");
	// ��flushһ����ʾ��ʼ����Ϣ
	ui.buttonFlush->clicked();
}

// �����ܵ���cmd����
BOOL QCmd::Init() {

	// ���������ܵ�
	SECURITY_ATTRIBUTES securityAttributes = { 0 };
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.nLength = sizeof(securityAttributes);
	securityAttributes.lpSecurityDescriptor = NULL;
	if ((!::CreatePipe(&hReadPipe1, &hWritePipe1, &securityAttributes, 0))|
		(!::CreatePipe(&hReadPipe2, &hWritePipe2, &securityAttributes, 0))) {
		ShowError(TEXT("create pipe failed"));
		return FALSE;
	}

	// ����cmd����
	STARTUPINFO si = { 0 };
	// ���ش���
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	// ���ñ�׼��������ʹ���
	si.hStdInput = hReadPipe2;
	si.hStdOutput = si.hStdError = hWritePipe1;
	// ƴ�����������е�����
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

// �������
QCmd::~QCmd() {
	// ����������ڴ�����µĲ��֣����������ط���
	// �����ò������ź�ȫ��
	// ���ǳ�����ص㲻������ ���ˣ��ð�����Ϊ����������
	CloseHandle(hReadPipe1);
	CloseHandle(hReadPipe2);
	CloseHandle(hWritePipe1);
	CloseHandle(hWritePipe2);
	CloseHandle(ProcessInformation.hThread);
	CloseHandle(ProcessInformation.hProcess);
}

// ��cmd����
// ע�� ����ͳһʹ��ANSI��ʽ��cmd�򽻵�
BOOL QCmd::sendCmd(PCHAR psCmd, DWORD dwBuffersize) {

	DWORD dwBytesWrite = 0;

	// ע�����ǵ�����������û�л��з��� �������뻻�з��Ƚϰ�ȫ
	StringCchCatA(psCmd, dwBuffersize, "\r\n");
	// ������������дҪ�����ַ�����������
	if (!WriteFile(hWritePipe2, psCmd, lstrlenA(psCmd), &dwBytesWrite, NULL)) {
		return FALSE;
	}

	return(TRUE);
}

// ˢ�³�cmd�Ļ���
BOOL QCmd::flushCmd(PCHAR psResultBuffer, DWORD dwBuffersize) {

	DWORD dwBytesResult = 0;
	RtlZeroMemory(psResultBuffer, dwBuffersize);

	// �ȿ���û������ ��ֹû�����ݻ�ȥ���������
	if (!PeekNamedPipe(hReadPipe1, psResultBuffer, dwBuffersize, &dwBytesResult, NULL, NULL)) {
		return FALSE;
	}

	// ˢ�³ɹ�����û������
	if (!dwBytesResult) {
		return TRUE;
	}

	// �����ݾͶ�
	RtlZeroMemory(psResultBuffer, dwBuffersize);
	if (!ReadFile(hReadPipe1, psResultBuffer, dwBuffersize, &dwBytesResult, NULL)) {
		return FALSE;
	}

	return TRUE;
}


// ������ʾ
VOID QCmd::ShowError(LPCTSTR pszText) {

	TCHAR szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, TEXT("%s Error[%d]\n"), pszText, ::GetLastError());
	QMessageBox::critical(this, "Error", QString::fromWCharArray(szErr));
}


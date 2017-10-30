#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QtWidgets/QDialog>
#include "ui_mainframe.h"
#include <QSerialPort>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
	char strComName[50];
	QSerialPort::BaudRate baudRate;
	QSerialPort::DataBits dataBit;
	QSerialPort::Parity parity;
	QSerialPort::FlowControl flowControl;
	QSerialPort::StopBits stopBits;
}STRU_SERIAL_CONFIG_INFO;

class mainframe : public QDialog
{
	Q_OBJECT

public:
	mainframe(QWidget *parent = 0);
	~mainframe();

private:
	Ui::mainframeClass ui;

	void GetSerialPortInfo();
	void OperateData(unsigned char* pMsg);

	STRU_SERIAL_CONFIG_INFO m_stSerialConfigInfo;
	bool m_bHaveStarted;
	QSerialPort* m_pSerial;

	QByteArray m_Buffer;

	FILE* m_pFile;

	unsigned char* m_pBuffer;
	bool m_bFirstBin;
	unsigned short m_PackageSum;
	unsigned int m_validLength;
private slots:
	void Start();
	void ReceiveData();
};

#endif // MAINFRAME_H

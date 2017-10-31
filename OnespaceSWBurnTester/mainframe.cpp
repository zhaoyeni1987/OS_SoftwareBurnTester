#include "mainframe.h"
#include <QSerialPortInfo>
#include <string.h>
#include "crc.h"

mainframe::mainframe(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	m_bHaveStarted = false;

	QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();

	for (int i = 0; i < list.size(); i++)
	{
		ui.comboBoxSerialPort->addItem(list.at(i).portName());
	}

	ui.comboBoxBaud->setCurrentIndex(7);
	ui.comboBoxDataBit->setCurrentIndex(3);//set data bit 8
	ui.comboBoxCheckBit->setCurrentIndex(2);

	m_pBuffer = 0;

	m_bFirstBin = false;

	m_PackageSum = 0;

	m_validLength = 0;

	m_CurDownloadIndex = 0;

	m_lastPackage = 0;
}

mainframe::~mainframe()
{

}

void mainframe::OperateData(unsigned char* pMsg)
{
	if (pMsg[5] == 0xF0)
	{
		char msg[] = { 0x55,
					0xAA,
					0x05,
					0x00,
					0x01,
					0xF1,
					0x22,
					0xAA,
					0x11,
					0x00,
					0x00,
					0x00,
					0x00 };
		
		m_pSerial->write(msg, 13);
	}
	else if (pMsg[5] == 0xF2)
	{
		if (m_bFirstBin == false)
		{
			m_bFirstBin = true;

			char msg[] = { 0x55,
				0xAA,
				0x06,
				0x00,
				0x01,
				0xF3,
				0x22,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00 };
			msg[8] = (char)pMsg[6];
			msg[9] = (char)pMsg[7];
			msg[10] = (char)pMsg[8];
			msg[11] = (char)pMsg[9];

			m_PackageSum = pMsg[6] + pMsg[7] * 256;
			unsigned short PackageCur = pMsg[8] + pMsg[9] * 256;

			m_pBuffer = (unsigned char*)malloc(sizeof(unsigned char) * m_PackageSum * 250);
			memset(m_pBuffer, 0, sizeof(unsigned char) * sizeof(unsigned char) * m_PackageSum * 250);
			memcpy(m_pBuffer, pMsg + 11, 250);

			m_pSerial->write(msg, 14);
		}
		else
		{
			char msg[] = { 0x55,
				0xAA,
				0x06,
				0x00,
				0x01,
				0xF3,
				0x22,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00 };
			msg[8] = (char)pMsg[6];
			msg[9] = (char)pMsg[7];
			msg[10] = (char)pMsg[8];
			msg[11] = (char)pMsg[9];

			unsigned short PackageSum = msg[8] + msg[9] * 256;
			unsigned short PackageCur = pMsg[8] + pMsg[9] * 256;

			if (PackageCur >= (PackageSum - 1))
			{
				unsigned char validLength = pMsg[10];
				m_lastPackage = validLength;
				memcpy(m_pBuffer + PackageCur * 250, pMsg + 11, validLength);
				m_validLength = (m_PackageSum - 1) * 250 + validLength;
				m_pFile = fopen("temp.bin", "wb+");
				fwrite(m_pBuffer, 1, (m_PackageSum - 1) * 250 + validLength, m_pFile);
				fclose(m_pFile);
			}
			else
			{
				memcpy(m_pBuffer + PackageCur * 250, pMsg + 11, 250);
			}

			m_pSerial->write(msg, 14);
		}
	}
	else if (pMsg[5] == 0xF6)
	{
		char msg[] = { 0x55,
					0xAA,
					0x0A,
					0x00,
					0x01,
					0xF7,
					0x22,
					0x00,
					0x11,
					0x00,
					0x00,
					0x00,
					0x00,
					0x00,
					0x00, 
					0x00,
					0x00,
					0x00};
		
		unsigned int crc32 = CalCRC32(m_pBuffer, m_validLength);
		msg[8] = (char)(m_validLength & 0x000000FF);
		msg[9] = (char)((m_validLength >> 8) & 0x000000FF);
		msg[10] = (char)((m_validLength >> 16) & 0x000000FF);
		msg[11] = (char)((m_validLength >> 24) & 0x000000FF);

		msg[12] = (char)(crc32 & 0x000000FF);
		msg[13] = (char)((crc32 >> 8) & 0x000000FF);
		msg[14] = (char)((crc32 >> 16) & 0x000000FF);
		msg[15] = (char)((crc32 >> 24) & 0x000000FF);

		m_pSerial->write(msg, 18);
	}
	else if (pMsg[5] == 0xF4)
	{
		char msg[263] = { 0x55, 0xAA, 0xFF, 0x00, 0x01, 0xF5, 0x22, 0x00, 0x00, 0x00, 0xFA };
		unsigned short requireIndex = pMsg[6] + pMsg[7] * 256;
		msg[8] = (char)pMsg[6];
		msg[9] = (char)pMsg[7];

		if (requireIndex == (m_PackageSum - 1))	//last package
		{
			memcpy(&msg[11], m_pBuffer + 250 * requireIndex, m_lastPackage);
		}
		else
		{
			memcpy(&msg[11], m_pBuffer + 250 * requireIndex, 250);
		}
				
		m_pSerial->write(msg, 263);
	}
}

void mainframe::ReceiveData()
{
	QByteArray data = m_pSerial->readAll();

	m_Buffer += data;

	if ((m_Buffer[0] == (char)0x55)
		&& (m_Buffer[1] == (char)0xAA))
	{
		if (m_Buffer[2] == (char)((m_Buffer.length() - 8)))
		{
			OperateData((unsigned char*)m_Buffer.data());
			m_Buffer.clear();
		}
		else
		{
			m_Buffer = m_Buffer + data;
		}
	}
	else
	{
		if (m_Buffer.isEmpty())
		{
			m_Buffer.clear();
		}
		else
		{
			m_Buffer = m_Buffer + data;

			if ((m_Buffer[0] == (char)0x55)
				&& (m_Buffer[1] == (char)0xAA))
			{
				if (m_Buffer[2] == (char)((m_Buffer.length() - 8)))
				{
					OperateData((unsigned char*)m_Buffer.data());
					m_Buffer.clear();
				}
				else
				{
					m_Buffer = m_Buffer + data;
				}
			}
		}	
	}
}

void mainframe::GetSerialPortInfo()
{
	memset(&m_stSerialConfigInfo, 0, sizeof(STRU_SERIAL_CONFIG_INFO));

	QString strPortName = ui.comboBoxSerialPort->currentText();
	strcpy(m_stSerialConfigInfo.strComName, strPortName.toLatin1().data());

	QString strBaudrate = ui.comboBoxBaud->currentText();
	m_stSerialConfigInfo.baudRate = (QSerialPort::BaudRate)strBaudrate.toInt();

	m_stSerialConfigInfo.dataBit = (QSerialPort::DataBits)ui.comboBoxDataBit->currentText().toInt();

	m_stSerialConfigInfo.flowControl = QSerialPort::NoFlowControl;

	QString strParity = ui.comboBoxCheckBit->currentText();
	if (strParity == "NONE")
		m_stSerialConfigInfo.parity = QSerialPort::NoParity;
	else if (strParity == "ODD")
		m_stSerialConfigInfo.parity = QSerialPort::OddParity;
	else if (strParity == "EVEN")
		m_stSerialConfigInfo.parity = QSerialPort::EvenParity;
	else if (strParity == "MARK")
		m_stSerialConfigInfo.parity = QSerialPort::MarkParity;
	else if (strParity == "SPACE")
		m_stSerialConfigInfo.parity = QSerialPort::SpaceParity;
	QString strStopBits = ui.comboBoxStopBit->currentText();
	if (strStopBits == "1")
		m_stSerialConfigInfo.stopBits = QSerialPort::OneStop;
	else if (strStopBits == "1.5")
		m_stSerialConfigInfo.stopBits = QSerialPort::OneAndHalfStop;
	else if (strStopBits == "2")
		m_stSerialConfigInfo.stopBits = QSerialPort::TwoStop;
}

void mainframe::Start()
{
	if (m_bHaveStarted == false)		//if the process not started
	{
		GetSerialPortInfo();
		ui.groupBoxSerialSetting->setEnabled(false);

		m_pSerial = new QSerialPort(this);
		STRU_SERIAL_CONFIG_INFO serialInfo = m_stSerialConfigInfo;
		QString srtPortName(serialInfo.strComName);
		m_pSerial->setPortName(srtPortName);
		m_pSerial->setBaudRate(serialInfo.baudRate, QSerialPort::AllDirections);
		m_pSerial->setDataBits(serialInfo.dataBit);
		m_pSerial->setFlowControl(serialInfo.flowControl);
		m_pSerial->setParity(serialInfo.parity);
		m_pSerial->setStopBits(serialInfo.stopBits);
		m_pSerial->close();
		connect(m_pSerial, SIGNAL(readyRead()), this, SLOT(ReceiveData()));

		m_pSerial->open(QIODevice::ReadWrite);

		ui.pushButtonStart->setText("Stop");
		m_bHaveStarted = true;
	}
	else
	{
		ui.groupBoxSerialSetting->setEnabled(true);

		m_pSerial->close();
		delete m_pSerial;

		ui.pushButtonStart->setText("Start");
		m_bHaveStarted = false;
	}
}

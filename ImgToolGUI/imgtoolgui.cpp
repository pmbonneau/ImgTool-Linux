#include "imgtoolgui.h"
#include "FunctionsUtilities.h"
#include <QFileDialog>
#include <QMessageBox>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <algorithm>

using namespace std;
using namespace utilities;

ImgToolGUI::ImgToolGUI(QWidget *parent)
    : QMainWindow(parent), m_FirstFile(true)
{
	cout << "ImgTool v1.0 (1A0116d), © 2016 Pierre-Marc Bonneau, all rights reserved." << endl;
	cout << "Build style : RELEASE" << endl;
	cout << "Console interface -------------------------------------------------------" << endl;

	ui.setupUi(this);
	QObject::connect(ui.btnMenuOpenFile, SIGNAL(triggered()), this,
	SLOT(OpenFile()));

	QObject::connect(ui.btnExportImage, SIGNAL(clicked()), this,
	SLOT(ExportImage()));

	QObject::connect(ui.btnExportCert, SIGNAL(clicked()), this,
	SLOT(ExportCert()));
}

ImgToolGUI::~ImgToolGUI()
{

}

void ImgToolGUI::OpenFile()
{
	if (m_FirstFile == false)
	{
		Cleanup();
	}

	// https://www.youtube.com/watch?v=tKdfpA74HYY
	QString FilePath = QFileDialog::getOpenFileName(this, tr("Open File"), "/",".img3 (*.img3);;.dfu (*.dfu);;.* (*.*)");
	ui.txtFilePath->setText(FilePath);

	stringstream ssFilePath;
	string sFilePath;
	ssFilePath << FilePath.toStdString();
	getline(ssFilePath, sFilePath);
	cout << "Selected file path is : " << sFilePath << endl;
	ReadFile(sFilePath);
}

void ImgToolGUI::ExportImage()
{
	string sFilePath = "";
	QString FilePath = QFileDialog::getSaveFileName(this, "Save image file", "/", ".bin");
	sFilePath = FilePath.toStdString();
	sFilePath = sFilePath + ".bin";
	ofstream OutputFile(sFilePath.c_str(), std::ios::binary);
	OutputFile << m_ssExportImg3DATA.str();
	cout << "Image successfully exported to a file." << endl;
}

void ImgToolGUI::ExportCert()
{
	string sFilePath = "";
	QString FilePath = QFileDialog::getSaveFileName(this, "Save certificate file", "/", ".cer");
	sFilePath = FilePath.toStdString();
	sFilePath = sFilePath + ".cer";
	ofstream OutputFile(sFilePath.c_str(), std::ios::binary);
	OutputFile << m_ssExportImg3CERT.str();
	cout << "Certificate successfully exported to a file." << endl;
}

void ImgToolGUI::Cleanup()
{
	cout << "Entering the cleanup() routine." << endl;
    foreach(QLineEdit* TextBoxes, findChildren<QLineEdit*>())
    {
    	TextBoxes->clear();
    }

    m_ssExportImg3DATA.str(std::string());
	m_ssExportImg3DATA.clear();
	m_ssExportImg3CERT.str(std::string());
	m_ssExportImg3CERT.clear();
}

void ImgToolGUI::ReadFile(string FilePath)
{
	typedef unsigned char BYTE;
	string openfile;
	string p_ligne;
	ostringstream p_parametres;

	// http://www.dreamincode.net/forums/topic/170054-understanding-and-reading-binary-files-in-c/
	const char *filePath = FilePath.c_str();
	BYTE *FileBuffer;  //Pointeur aux données dans le buffer
	FILE *file = NULL;      // File pointer
	if ((file = fopen(filePath, "rb")) == NULL)
	{
		cout << "Could not open specified file." << endl;
	}
	else
	{
		cout << "File opened successfully." << endl;
	}

	long fileSize = getFileSize(file);

	cout << "Set file buffer size to : " << fileSize << endl;
	FileBuffer = new BYTE[fileSize];

	cout << "Buffer filled from file." << endl;
	fread(FileBuffer, fileSize, 1, file);

	stringstream ssImgHeader;
	for (int i = 0; i <= 20; i++)
	{
		// Add missing zeros when hex numbers are 0xF and lower.
		ssImgHeader << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
	}
	cout << "Img header extracted from buffer." << endl;

	string ImgHeader;
	string ImgFileType = "";
	ssImgHeader >> ImgHeader;
	ImgFileType = HexStringToChars((LittleToBigEndian(ImgHeader.substr(0,8))));
	if (ImgFileType != "Img3")
	{
		if (ImgFileType == "Img2")
		{
			cout << "IMG2 containers are not actually supported." << endl;
			QMessageBox::warning(this, "Message", "IMG2 containers are not actually supported.");
			return;
		}
		else if (ImgFileType == "Img4")
		{
			cout << "IMG4 containers are not actually supported." << endl;
			QMessageBox::warning(this, "Message", "IMG4 containers are not actually supported.");
			return;
		}
		else if (ImgFileType == "IM4P")
		{
			cout << "IM4P containers are not actually supported." << endl;
			QMessageBox::warning(this, "Message", "IM4P containers are not actually supported.");
			return;
		}
		else
		{
			cout << "The input file is invalid or not supported." << endl;
			QString FileTypeError("The input file is invalid or not supported.");
			QMessageBox::warning(this, "Message", FileTypeError);
			return;
		}
	}
	QString qsImgFileType = QString::fromStdString(ImgFileType);
	ui.txtImgVersion->setText(qsImgFileType);
	ui.txtImgVersion->setText(qsImgFileType);

	if (ImgFileType == "Img3")
	{
		cout << "Entering the ReadImg3() function." << endl;
		ReadImg3(FileBuffer);
	}
}

typedef unsigned char BYTE;
void ImgToolGUI::ReadImg3(BYTE* FileBuffer)
{
	cout << "Setting all variables." << endl;
	string ImgFileType = "";
	string Img3FullSize = "";
	string Img3FullSizeNoHeader = "";
	string Img3SignatureCheckArea = "";
	string Img3ID = "";
	string Img3TagTYPE = "";
	string Img3TotalLengthTYPE = "";
	string Img3DataLengthTYPE = "";
	string Img3TagDATA = "";
	string Img3TotalLengthDATA = "";
	string Img3DataLengthDATA = "";

	bool Encryption = false;

	// Vérification si l'image est un bootloader.
	bool Bootloader = false;

	string Img3TagVERS = "";
	string Img3TotalLengthVERS = "";
	string Img3DataLengthVERS = "";
	string Img3DataVERS = "";
	string Img3TagSEPO = "";
	string Img3TotalLengthSEPO = "";
	string Img3DataLengthSEPO = "";
	string Img3DataSEPO = "";
	string Img3TagCHIP = "";
	string Img3TotalLengthCHIP = "";
	string Img3DataLengthCHIP = "";
	string Img3DataCHIP = "";
	string Img3TagBORD = "";
	string Img3TotalLengthBORD = "";
	string Img3DataLengthBORD = "";
	string Img3DataBORD = "";
	string Img3TagSHSH = "";
	string Img3TotalLengthSHSH = "";
	string Img3DataLengthSHSH = "";
	string Img3DataSHSH = "";
	string Img3TagCERT = "";
	string Img3TotalLengthCERT = "";
	string Img3DataLengthCERT = "";
	string Img3DataCERT = "";
	string Img3TagSDOM = "";
	string Img3TotalLengthSDOM = "";
	string Img3DataLengthSDOM = "";
	string Img3DataSDOM = "";
	string Img3TagPROD = "";
	string Img3TotalLengthPROD = "";
	string Img3DataLengthPROD = "";
	string Img3DataPROD = "";
	string Img3TagCHIPcert = "";
	string Img3TotalLengthCHIPcert = "";
	string Img3DataLengthCHIPcert = "";
	string Img3DataCHIPcert = "";

	stringstream ssImgHeader;
	for (int i = 0; i <= 20; i++)
	{
		// Add missing zeros when hex numbers are 0xF and lower.
		ssImgHeader << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
	}

	string Img3Header;
	ssImgHeader >> Img3Header;
	cout << "Header : " << Img3Header << endl;

	ImgFileType = HexStringToChars((LittleToBigEndian(Img3Header.substr(0,8))));
	cout << "File type : " << ImgFileType << endl;
	QString qsImgFileType = QString::fromStdString(ImgFileType);
	ui.txtImg3Version->setText(qsImgFileType);
	ui.txtImg3Version->setText(qsImgFileType);

	Img3FullSize = (LittleToBigEndian(Img3Header.substr(8,16)));
	cout << "Full size : " << "0x" << Img3FullSize << endl;
	QString qsImg3FullSize = QString::fromStdString("0x" + Img3FullSize);
	ui.txtImg3FullSize->setText(qsImg3FullSize);
	ui.txtImg3FullSize->setText(qsImg3FullSize);

	Img3FullSizeNoHeader = (LittleToBigEndian(Img3Header.substr(16,24)));
	cout << "Full size without header : " << "0x" << Img3FullSizeNoHeader << endl;
	QString qsImg3FullSizeNoHeader = QString::fromStdString("0x" + Img3FullSizeNoHeader);
	ui.txtImg3FullSizeNoHeader->setText(qsImg3FullSizeNoHeader);
	ui.txtImg3FullSizeNoHeader->setText(qsImg3FullSizeNoHeader);

	Img3SignatureCheckArea = (LittleToBigEndian(Img3Header.substr(24,32)));
	cout << "Signature check area : " << "0x" << Img3SignatureCheckArea << endl;
	QString qsImg3SignatureCheckArea = QString::fromStdString("0x" + Img3SignatureCheckArea);
	ui.txtImg3SignatureCheckArea->setText(qsImg3SignatureCheckArea);
	ui.txtImg3SignatureCheckArea->setText(qsImg3SignatureCheckArea);

	Img3ID = HexStringToChars((LittleToBigEndian(Img3Header.substr(32,40))));
	if (Img3ID == "ibss")
	{
		Img3ID = "iBSS";
		Bootloader = true;
	}
	else if (Img3ID == "ibec")
	{
		Img3ID = "iBEC";
		Bootloader = true;
	}
	else if (Img3ID == "illb")
	{
		Img3ID = "LLB";
		Bootloader = true;
	}
	else if (Img3ID == "ibot")
	{
		Img3ID = "iBoot";
		Bootloader = true;
	}
	else if (Img3ID == "logo")
	{
		Img3ID = "Logo";
	}

	cout << "Img identifier : " << Img3ID << endl;
	QString qsImg3ID = QString::fromStdString(Img3ID);
	ui.txtImg3ID->setText(qsImg3ID);
	ui.txtImg3ID->setText(qsImg3ID);

	stringstream ssImg3TYPE;
	for (int i = 20; i <= 52; i++)
	{
		// Add missing zeros when hex numbers are 0xF and lower.
		ssImg3TYPE << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
	}

	string Img3TYPE;
	ssImg3TYPE >> Img3TYPE;
	cout << "Img3 tag TYPE RAW : " << "0x" << Img3TYPE << endl;

	Img3TagTYPE = HexStringToChars((LittleToBigEndian(Img3TYPE.substr(0,8))));
	cout << "Img3 tag TYPE : " << Img3TagTYPE << endl;
	QString qsImg3TagTYPE = QString::fromStdString(Img3TagTYPE);
	ui.txtImg3TagTYPE->setText(qsImg3TagTYPE);
	ui.txtImg3TagTYPE->setText(qsImg3TagTYPE);

	Img3TotalLengthTYPE = (LittleToBigEndian(Img3TYPE.substr(8,16)));
	cout << "Img3 tag TYPE total length : " << "0x" << Img3TotalLengthTYPE << endl;
	QString qsImg3LengthTYPE = QString::fromStdString("0x" + Img3TotalLengthTYPE);
	ui.txtImg3TotalLengthTYPE->setText(qsImg3LengthTYPE);
	ui.txtImg3TotalLengthTYPE->setText(qsImg3LengthTYPE);

	Img3DataLengthTYPE = (LittleToBigEndian(Img3TYPE.substr(16,24)));
	cout << "Img3 tag TYPE data length : " << "0x" << Img3DataLengthTYPE << endl;
	QString qsImg3DataLengthTYPE = QString::fromStdString("0x" + Img3DataLengthTYPE);
	ui.txtImg3DataLengthTYPE->setText(qsImg3DataLengthTYPE);
	ui.txtImg3DataLengthTYPE->setText(qsImg3DataLengthTYPE);

	stringstream ssImg3DATA;

	// Extraction de l'entête du tag DATA
	for (int i = 52; i <= 64; i++)
	{
		// Add missing zeros when hex numbers are 0xF and lower.
		ssImg3DATA << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
	}

	string Img3DATA;
	ssImg3DATA >> Img3DATA;
	cout << "Img3 tag DATA RAW : " << "0x" << Img3DATA << endl;

	Img3TagDATA = HexStringToChars((LittleToBigEndian(Img3DATA.substr(0,8))));
	cout << "Img3 tag DATA : " << Img3TagDATA << endl;
	QString qsImg3TagDATA = QString::fromStdString(Img3TagDATA);
	ui.txtImg3TagDATA->setText(qsImg3TagDATA);
	ui.txtImg3TagDATA->setText(qsImg3TagDATA);

	Img3TotalLengthDATA = (LittleToBigEndian(Img3DATA.substr(8,16)));
	cout << "Img3 tag DATA total length : " << "0x" << Img3TotalLengthDATA << endl;
	QString qsImg3TotalLengthDATA = QString::fromStdString("0x" + Img3TotalLengthDATA);
	ui.txtImg3TotalLengthDATA->setText(qsImg3TotalLengthDATA);
	ui.txtImg3TotalLengthDATA->setText(qsImg3TotalLengthDATA);

	Img3DataLengthDATA = (LittleToBigEndian(Img3DATA.substr(16,24)));
	cout << "Img3 tag DATA image length : " << "0x" << Img3DataLengthDATA << endl;
	QString qsImg3DataLengthDATA = QString::fromStdString("0x" + Img3DataLengthDATA);
	ui.txtImg3DataLengthDATA->setText(qsImg3DataLengthDATA);
	ui.txtImg3DataLengthDATA->setText(qsImg3DataLengthDATA);

	// Extraction de la partie data du tag DATA
	ssImg3DATA.clear();
	int Img3DATApos = 0;
	int Img3HeaderLength;
	int Img3DATATagHeaderLength;
	int Img3DataInfoDATApos;
	stringstream ssImg3DataInfoDATA;

	// Longeur de l'entête IMG3
	Img3HeaderLength = ConvertStringToDecInt(Img3FullSize) - ConvertStringToDecInt(Img3FullSizeNoHeader);

	// Longeur de l'entête du tag DATA
	Img3DATATagHeaderLength = ConvertStringToDecInt(Img3TotalLengthDATA) - ConvertStringToDecInt(Img3DataLengthDATA);

	// Position du tag DATA (on ajoute la longeur totale de TYPE à celle de l'entête IMG3).
	Img3DATApos = Img3HeaderLength + ConvertStringToDecInt(Img3TotalLengthTYPE);

	// Vérification de l'état d'encryption du conteneur Img3
	//stringstream ssCheckEncryptionPattern;
	//string CheckEncryptionPattern = "";
	//for (int i = Img3DATApos + Img3DATATagHeaderLength; i < Img3DATApos + Img3DATATagHeaderLength + 4; i++)
	//{
	//	ssCheckEncryptionPattern << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
	//}
	//ssCheckEncryptionPattern >> CheckEncryptionPattern;
	// cout << CheckEncryptionPattern << endl;

	// Vérifier si le conteneur est encrypté ou non, par défaut Encryption = false;
	//if (CheckEncryptionPattern != "0e0000ea")
	//{
	//	cout << "The input file is an encrypted Img3 container." << endl;
	//	cout << "The function to read encrypted Img3 containers is not completed." << endl;
	//	Encryption = true;
	//	QString ErrorEncryption("The function to read encrypted Img3 containers is not completed.");
	//	QMessageBox::warning(this, "Message", ErrorEncryption);
	//	return;
	//}

	// Récupération des données bytes par bytes à partir du buffer.
	//cout << Img3DATApos << endl;
	//cout << Img3DATATagHeaderLength << endl;

	// Apple Logo Bug
	//if (Img3ID == "Logo")
	//{
	//	Img3DATATagHeaderLength = Img3DATATagHeaderLength - 2;
	//	intImg3TotalLengthDATA = intImg3TotalLengthDATA - 2;
	//}
	int intImg3TotalLengthDATA = ConvertStringToDecInt(Img3TotalLengthDATA) - Img3DATATagHeaderLength;
	int Img3DATAImagepos = Img3DATATagHeaderLength + Img3DATApos;
	for (int i = Img3DATAImagepos; i < (Img3DATAImagepos + intImg3TotalLengthDATA); i++)
	{
		// Place les données de l'image dans une variable membre qui sera utilisé par la fonction ExportImage()
		m_ssExportImg3DATA << hex << FileBuffer[i];
	}

	if (Bootloader == true)
	{
		string Img3DataInfoDATA = "";
		string ImageInfo = "";
		string ImageSecurityFusing = "";
		string ImageVersion = "";

		// Endroit des informations de la section data du tag DATA
		Img3DataInfoDATApos = Img3DATApos + Img3DATATagHeaderLength + 512;

		// Récupération des informations de la section data du tag DATA
		for (int i = Img3DataInfoDATApos; i < Img3DataInfoDATApos + 384; i++)
		{
			ssImg3DataInfoDATA << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
		}

		ssImg3DataInfoDATA >> Img3DataInfoDATA;
		// cout << Img3DataInfoDATA << endl;

		// On extrait 128 caractères à partir de la position 0.
		ImageInfo = HexStringToChars(Img3DataInfoDATA.substr(0,128));
		QString qsImageInfo = QString::fromStdString(ImageInfo);
		ui.txtImageInfo->setText(qsImageInfo);

		ImageSecurityFusing = HexStringToChars(Img3DataInfoDATA.substr(128,128));
		QString qsImageSecurityFusing = QString::fromStdString(ImageSecurityFusing);
		ui.txtImageSecurityFusing->setText(qsImageSecurityFusing);

		ImageVersion = HexStringToChars(Img3DataInfoDATA.substr(256,128));
		QString qsImageVersion = QString::fromStdString(ImageVersion);
		ui.txtImageVersion->setText(qsImageVersion);
	}

	// Recherche de la position du tag «VERS»
	stringstream ssImg3VERS;
	int IntImg3FullSizeNoHeader = 0;
	int Img3HeaderSize = 0;
	int Img3VERSpos = 0;
	string sImg3VERSpos = "";
	int IntImg3FullSize = 0;
	IntImg3FullSizeNoHeader = ConvertStringToDecInt(Img3FullSizeNoHeader);
	IntImg3FullSize = ConvertStringToDecInt(Img3FullSize);
	Img3HeaderSize = IntImg3FullSize - IntImg3FullSizeNoHeader;
	Img3VERSpos = Img3HeaderSize + ConvertStringToDecInt(Img3TotalLengthTYPE) + ConvertStringToDecInt(Img3TotalLengthDATA);

	for (int i = Img3VERSpos; i <= (Img3VERSpos + 44); i++)
	{
		// Add missing zeros when hex numbers are 0xF and lower.
		ssImg3VERS << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
	}

	string Img3VERS;
	ssImg3VERS >> Img3VERS;

	Img3TagVERS = HexStringToChars((LittleToBigEndian(Img3VERS.substr(0,8))));

	if (Img3TagVERS == "VERS")
	{
		cout << "Img3 tag VERS RAW : " << "0x" << Img3VERS << endl;
		cout << "Img3 tag VERS : " << Img3TagVERS << endl;
		QString qsImg3TagVERS = QString::fromStdString(Img3TagVERS);
		ui.txtImg3TagVERS->setText(qsImg3TagVERS);
		ui.txtImg3TagVERS->setText(qsImg3TagVERS);

		Img3TotalLengthVERS = (LittleToBigEndian(Img3VERS.substr(8,16)));
		cout << "Img3 tag VERS total length : " << "0x" << Img3TotalLengthVERS  << endl;
		QString qsImg3TotalLengthVERS  = QString::fromStdString("0x" + Img3TotalLengthVERS);
		ui.txtImg3TotalLengthVERS ->setText(qsImg3TotalLengthVERS);
		ui.txtImg3TotalLengthVERS ->setText(qsImg3TotalLengthVERS );

		Img3DataLengthVERS = (LittleToBigEndian(Img3VERS.substr(16,24)));
		cout << "Img3 tag VERS data length : " << "0x" << Img3DataLengthVERS  << endl;
		QString qsImg3DataLengthVERS  = QString::fromStdString("0x" + Img3DataLengthVERS);
		ui.txtImg3DataLengthVERS->setText(qsImg3DataLengthVERS);
		ui.txtImg3DataLengthVERS->setText(qsImg3DataLengthVERS);

		Img3DataVERS = Img3VERS.substr(32,(32 + ConvertStringToDecInt(Img3DataLengthVERS)));
		Img3DataVERS = HexStringToChars(Img3DataVERS);
		QString qsImg3DataVERS  = QString::fromStdString(Img3DataVERS);
		ui.txtImg3DataVERS->setText(qsImg3DataVERS);
		ui.txtImg3DataVERS->setText(qsImg3DataVERS);
		cout << "Img3 tag VERS data : " << Img3DataVERS  << endl;
	}

	string sImg3SEPOpos = "";
	stringstream ssImg3SEPO;
	int Img3SEPOpos = 0;
	Img3SEPOpos = Img3VERSpos + ConvertStringToDecInt(Img3TotalLengthVERS);

	for (int i = Img3SEPOpos; i <= (Img3SEPOpos + 28); i++)
	{
		// Add missing zeros when hex numbers are 0xF and lower.
		ssImg3SEPO << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
	}

	string Img3SEPO;
	ssImg3SEPO >> Img3SEPO;

	Img3TagSEPO = HexStringToChars((LittleToBigEndian(Img3SEPO.substr(0,8))));

	if (Img3TagSEPO == "SEPO")
	{
		cout << "Img3 tag SEPO RAW : " << "0x" << Img3SEPO << endl;
		cout << "Img3 tag SEPO : " << Img3TagSEPO << endl;
		QString qsImg3TagSEPO = QString::fromStdString(Img3TagSEPO);
		ui.txtImg3TagSEPO->setText(qsImg3TagSEPO);
		ui.txtImg3TagSEPO->setText(qsImg3TagSEPO);

		Img3TotalLengthSEPO = (LittleToBigEndian(Img3SEPO.substr(8,16)));
		cout << "Img3 tag SEPO total length : " << "0x" << Img3TotalLengthSEPO  << endl;
		QString qsImg3TotalLengthSEPO  = QString::fromStdString("0x" + Img3TotalLengthSEPO);
		ui.txtImg3TotalLengthSEPO ->setText(qsImg3TotalLengthSEPO);
		ui.txtImg3TotalLengthSEPO ->setText(qsImg3TotalLengthSEPO);

		Img3DataLengthSEPO = (LittleToBigEndian(Img3SEPO.substr(16,24)));
		cout << "Img3 tag SEPO data length : " << "0x" << Img3DataLengthSEPO  << endl;
		QString qsImg3DataLengthSEPO  = QString::fromStdString("0x" + Img3DataLengthSEPO);
		ui.txtImg3DataLengthSEPO->setText(qsImg3DataLengthSEPO);
		ui.txtImg3DataLengthSEPO->setText(qsImg3DataLengthSEPO);

		Img3DataSEPO = (LittleToBigEndian(Img3SEPO.substr(24,(32 + ConvertStringToDecInt(Img3DataLengthSEPO)))));
		cout << "Img3 tag SEPO data : " << "0x" << Img3DataSEPO  << endl;
		QString qsImg3DataSEPO  = QString::fromStdString("0x" + Img3DataSEPO);
		ui.txtImg3DataSEPO->setText(qsImg3DataSEPO);
		ui.txtImg3DataSEPO->setText(qsImg3DataSEPO);
	}

	string sImg3CHIPpos = "";
	stringstream ssImg3CHIP;
	int Img3CHIPpos = 0;
	Img3CHIPpos = Img3SEPOpos + ConvertStringToDecInt(Img3TotalLengthSEPO);

	for (int i = Img3CHIPpos; i <= (Img3CHIPpos + 28); i++)
	{
		// Add missing zeros when hex numbers are 0xF and lower.
		ssImg3CHIP << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
	}

	string Img3CHIP;
	ssImg3CHIP >> Img3CHIP;

	Img3TagCHIP = HexStringToChars((LittleToBigEndian(Img3CHIP.substr(0,8))));

	if (Img3TagCHIP == "CHIP")
	{
		cout << "Img3 tag CHIP RAW : " << "0x" << Img3CHIP << endl;
		cout << "Img3 tag CHIP : " << Img3TagCHIP << endl;
		QString qsImg3TagCHIP = QString::fromStdString(Img3TagCHIP);
		ui.txtImg3TagCHIP->setText(qsImg3TagCHIP);
		ui.txtImg3TagCHIP->setText(qsImg3TagCHIP);

		Img3TotalLengthCHIP = (LittleToBigEndian(Img3CHIP.substr(8,16)));
		cout << "Img3 tag CHIP length : " << "0x" << Img3TotalLengthCHIP  << endl;
		QString qsImg3TotalLengthCHIP  = QString::fromStdString("0x" + Img3TotalLengthCHIP);
		ui.txtImg3TotalLengthCHIP ->setText(qsImg3TotalLengthCHIP);
		ui.txtImg3TotalLengthCHIP ->setText(qsImg3TotalLengthCHIP);

		Img3DataLengthCHIP = (LittleToBigEndian(Img3CHIP.substr(16,24)));
		cout << "Img3 tag CHIP data length : " << "0x" << Img3DataLengthCHIP  << endl;
		QString qsImg3DataLengthCHIP  = QString::fromStdString("0x" + Img3DataLengthCHIP);
		ui.txtImg3DataLengthCHIP->setText(qsImg3DataLengthCHIP);
		ui.txtImg3DataLengthCHIP->setText(qsImg3DataLengthCHIP);

		Img3DataCHIP = (LittleToBigEndian(Img3CHIP.substr(24,(32 + ConvertStringToDecInt(Img3DataLengthCHIP)))));
		cout << "Img3 tag CHIP data : " << "0x" << Img3DataCHIP  << endl;
		QString qsImg3DataCHIP  = QString::fromStdString("0x" + Img3DataCHIP);
		ui.txtImg3DataCHIP->setText(qsImg3DataCHIP);
		ui.txtImg3DataCHIP->setText(qsImg3DataCHIP);
	}

	string sImg3BORDpos = "";
	stringstream ssImg3BORD;
	int Img3BORDpos = 0;
	Img3BORDpos = Img3CHIPpos + ConvertStringToDecInt(Img3TotalLengthCHIP);

	for (int i = Img3BORDpos; i <= (Img3BORDpos + 28); i++)
	{
		// Add missing zeros when hex numbers are 0xF and lower.
		ssImg3BORD << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
	}

	string Img3BORD;
	ssImg3BORD >> Img3BORD;
	//cout << "Img3 tag BORD RAW : " << "0x" << Img3BORD << endl;

	Img3TagBORD = HexStringToChars((LittleToBigEndian(Img3BORD.substr(0,8))));

	if (Img3TagBORD == "BORD")
	{
		cout << "Img3 tag BORD RAW : " << "0x" << Img3BORD << endl;
		cout << "Img3 tag BORD : " << Img3TagBORD << endl;
		QString qsImg3TagBORD = QString::fromStdString(Img3TagBORD);
		ui.txtImg3TagBORD->setText(qsImg3TagBORD);
		ui.txtImg3TagBORD->setText(qsImg3TagBORD);

		Img3TotalLengthBORD = (LittleToBigEndian(Img3BORD.substr(8,16)));
		cout << "Img3 tag BORD length : " << "0x" << Img3TotalLengthBORD  << endl;
		QString qsImg3TotalLengthBORD  = QString::fromStdString("0x" + Img3TotalLengthBORD);
		ui.txtImg3TotalLengthBORD ->setText(qsImg3TotalLengthBORD);
		ui.txtImg3TotalLengthBORD ->setText(qsImg3TotalLengthBORD);

		Img3DataLengthBORD = (LittleToBigEndian(Img3BORD.substr(16,24)));
		cout << "Img3 tag BORD data length : " << "0x" << Img3DataLengthBORD  << endl;
		QString qsImg3DataLengthBORD  = QString::fromStdString("0x" + Img3DataLengthBORD);
		ui.txtImg3DataLengthBORD->setText(qsImg3DataLengthBORD);
		ui.txtImg3DataLengthBORD->setText(qsImg3DataLengthBORD);

		Img3DataBORD = (LittleToBigEndian(Img3BORD.substr(24,(32 + ConvertStringToDecInt(Img3DataLengthBORD)))));
		cout << "Img3 tag BORD data : " << "0x" << Img3DataBORD  << endl;
		QString qsImg3DataBORD  = QString::fromStdString("0x" + Img3DataBORD);
		ui.txtImg3DataBORD->setText(qsImg3DataBORD);
		ui.txtImg3DataBORD->setText(qsImg3DataBORD);
	}

	string sImg3SHSHpos = "";
	stringstream ssImg3SHSH;
	int Img3SHSHpos = 0;
	Img3SHSHpos = Img3BORDpos + ConvertStringToDecInt(Img3TotalLengthBORD);

	if (Encryption == true)
	{
		stringstream ssImg3KBAG1;
		stringstream ssImg3TotalLengthKBAG1;
		string sImg3TotalLengthKBAG1;
		string Img3KBAG1;
		// On sait que la position du tag KBAG est celle du tag SHSH lorsque le conteneur est encrypté
		int Img3KBAG1pos = Img3SHSHpos;
		int Img3TotalLengthKBAG1pos = Img3KBAG1pos + 4;
		int Img3TotalLengthKBAG1 = 0;

		// Trouver la longeur totale du tag KBAG1
		for (int i = Img3TotalLengthKBAG1pos; i < (Img3TotalLengthKBAG1pos + 4); i++)
		{
			// Add missing zeros when hex numbers are 0xF and lower.
			ssImg3TotalLengthKBAG1 << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
		}
		ssImg3TotalLengthKBAG1 >> sImg3TotalLengthKBAG1;
		// cout << sImg3TotalLengthKBAG1 << endl;
		Img3TotalLengthKBAG1 = ConvertStringToDecInt((LittleToBigEndian(sImg3TotalLengthKBAG1)));
		// cout << Img3TotalLengthKBAG1 << endl;

		for (int i = Img3KBAG1pos; i < (Img3KBAG1pos + Img3TotalLengthKBAG1); i++)
		{
			// Add missing zeros when hex numbers are 0xF and lower.
			ssImg3KBAG1 << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
		}

		ssImg3KBAG1 >> Img3KBAG1;
		cout << "Img3 KBAG1 : " << Img3KBAG1 << endl;

		// On est rendu ici, décortiquer le KBAG

	}


	for (int i = Img3SHSHpos; i <= (Img3SHSHpos + 28); i++)
	{
		// Add missing zeros when hex numbers are 0xF and lower.
		ssImg3SHSH << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
	}

	string Img3SHSH;
	ssImg3SHSH >> Img3SHSH;

	Img3TagSHSH = HexStringToChars((LittleToBigEndian(Img3SHSH.substr(0,8))));

	if (Img3TagSHSH == "SHSH")
	{
		cout << "Img3 tag SHSH RAW : " << "0x" << Img3SHSH << endl;
		cout << "Img3 tag SHSH : " << Img3TagSHSH << endl;
		QString qsImg3TagSHSH = QString::fromStdString(Img3TagSHSH);
		ui.txtImg3TagSHSH->setText(qsImg3TagSHSH);
		ui.txtImg3TagSHSH->setText(qsImg3TagSHSH);

		Img3TotalLengthSHSH = (LittleToBigEndian(Img3SHSH.substr(8,16)));
		cout << "Img3 tag SHSH total length : " << "0x" << Img3TotalLengthSHSH  << endl;
		QString qsImg3TotalLengthSHSH  = QString::fromStdString("0x" + Img3TotalLengthSHSH);
		ui.txtImg3TotalLengthSHSH ->setText(qsImg3TotalLengthSHSH);
		ui.txtImg3TotalLengthSHSH ->setText(qsImg3TotalLengthSHSH);

		Img3DataLengthSHSH = (LittleToBigEndian(Img3SHSH.substr(16,24)));
		cout << "Img3 tag SHSH data length : " << "0x" << Img3DataLengthSHSH  << endl;
		QString qsImg3DataLengthSHSH  = QString::fromStdString("0x" + Img3DataLengthSHSH);
		ui.txtImg3DataLengthSHSH->setText(qsImg3DataLengthSHSH);
		ui.txtImg3DataLengthSHSH->setText(qsImg3DataLengthSHSH);

		ssImg3SHSH.clear();
		int Img3SHSHTagHeaderLength;
		Img3SHSHTagHeaderLength = ConvertStringToDecInt(Img3TotalLengthSHSH) - ConvertStringToDecInt(Img3DataLengthSHSH);
		for (int i = Img3SHSHpos + Img3SHSHTagHeaderLength; i < (Img3SHSHpos + ConvertStringToDecInt(Img3TotalLengthSHSH)); i++)
		{
			// Add missing zeros when hex numbers are 0xF and lower.
			ssImg3SHSH << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
		}
		ssImg3SHSH >> Img3DataSHSH;
		cout << "Img3 tag SHSH data : " << "0x" << Img3DataSHSH  << endl;
		QString qsImg3DataSHSH  = QString::fromStdString("0x" + Img3DataSHSH);
		ui.txtImg3DataSHSH->setText(qsImg3DataSHSH);
		ui.txtImg3DataSHSH->setText(qsImg3DataSHSH);
	}


	string sImg3CERTpos = "";
	stringstream ssImg3CERT;
	int Img3CERTpos = 0;
	Img3CERTpos = Img3SHSHpos + ConvertStringToDecInt(Img3TotalLengthSHSH);

	for (int i = Img3CERTpos; i <= (Img3CERTpos + 28); i++)
	{
		// Add missing zeros when hex numbers are 0xF and lower.
		ssImg3CERT << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
	}

	string Img3CERT;
	ssImg3CERT >> Img3CERT;

	Img3TagCERT = HexStringToChars((LittleToBigEndian(Img3CERT.substr(0,8))));

	if (Img3TagCERT == "CERT")
	{
		cout << "Img3 tag CERT RAW: " << "0x" << Img3CERT << endl;
		cout << "Img3 tag CERT : " << Img3TagCERT << endl;
		QString qsImg3TagCERT = QString::fromStdString(Img3TagCERT);
		ui.txtImg3TagCERT->setText(qsImg3TagCERT);
		ui.txtImg3TagCERT->setText(qsImg3TagCERT);

		Img3TotalLengthCERT = (LittleToBigEndian(Img3CERT.substr(8,16)));
		cout << "Img3 tag CERT total length : " << "0x" << Img3TotalLengthCERT  << endl;
		QString qsImg3TotalLengthCERT  = QString::fromStdString("0x" + Img3TotalLengthCERT);
		ui.txtImg3TotalLengthCERT ->setText(qsImg3TotalLengthCERT);
		ui.txtImg3TotalLengthCERT ->setText(qsImg3TotalLengthCERT);

		Img3DataLengthCERT = (LittleToBigEndian(Img3CERT.substr(16,24)));
		cout << "Img3 tag CERT data length : " << "0x" << Img3DataLengthCERT  << endl;
		QString qsImg3DataLengthCERT  = QString::fromStdString("0x" + Img3DataLengthCERT);
		ui.txtImg3DataLengthCERT->setText(qsImg3DataLengthCERT);
		ui.txtImg3DataLengthCERT->setText(qsImg3DataLengthCERT);

		// Nettoyage de la variable avant de la réutiliser
		ssImg3CERT.str(std::string());
		ssImg3CERT.clear();

		// int Img3CERTTagHeaderLength;
		// Img3CERTTagHeaderLength = ConvertStringToDecInt(Img3TotalLengthCERT) - ConvertStringToDecInt(Img3DataLengthCERT);
		// Img3CERTTagHeaderLength = Img3CERTTagHeaderLength - 12; // La longeur totale - la longeur des données ne donne pas la longeur de l'entête de ce tag.
		int Img3CERTDatapos = Img3CERTpos + 12;
		for (int i = Img3CERTpos + 12; i < (Img3CERTDatapos + ConvertStringToDecInt(Img3DataLengthCERT)); i++)
		{
			// Add missing zeros when hex numbers are 0xF and lower.
			ssImg3CERT << setfill('0') << setw(2) << hex << (int)FileBuffer[i];
			m_ssExportImg3CERT << hex << FileBuffer[i];
		}
		ssImg3CERT >> Img3DataCERT;
		cout << "Img3 tag CERT data : " << "0x" << Img3DataCERT  << endl;

		// Trouvons la position du tag SDOM dans le certificat.
		size_t Img3SDOMpos = Img3DataCERT.find("4d4f4453"); //MODS, en little-endian.
		string Img3SDOM = Img3DataCERT.substr(Img3SDOMpos, 32);
		Img3TagSDOM = HexStringToChars((LittleToBigEndian(Img3SDOM.substr(0,8))));
		cout << "Img3 tag SDOM : " << Img3TagSDOM << endl;
		QString qsImg3TagSDOM = QString::fromStdString(Img3TagSDOM);
		ui.txtImg3TagSDOM->setText(qsImg3TagSDOM);
		ui.txtImg3TagSDOM->setText(qsImg3TagSDOM);

		Img3TotalLengthSDOM = (LittleToBigEndian(Img3SDOM.substr(8,16)));
		cout << "Img3 tag SDOM total length : " << "0x" << Img3TotalLengthSDOM  << endl;
		QString qsImg3TotalLengthSDOM  = QString::fromStdString("0x" + Img3TotalLengthSDOM);
		ui.txtImg3TotalLengthSDOM ->setText(qsImg3TotalLengthSDOM);
		ui.txtImg3TotalLengthSDOM ->setText(qsImg3TotalLengthSDOM);

		Img3DataLengthSDOM = (LittleToBigEndian(Img3SDOM.substr(16,24)));
		cout << "Img3 tag SDOM data length : " << "0x" << Img3DataLengthSDOM  << endl;
		QString qsImg3DataLengthSDOM  = QString::fromStdString("0x" + Img3DataLengthSDOM);
		ui.txtImg3DataLengthSDOM->setText(qsImg3DataLengthSDOM);
		ui.txtImg3DataLengthSDOM->setText(qsImg3DataLengthSDOM);

		Img3DataSDOM = (LittleToBigEndian(Img3SDOM.substr(24,32)));
		cout << "Img3 tag SDOM data : " << "0x" << Img3DataSDOM  << endl;
		QString qsImg3DataSDOM  = QString::fromStdString("0x" + Img3DataSDOM);
		ui.txtImg3DataSDOM->setText(qsImg3DataSDOM);
		ui.txtImg3DataSDOM->setText(qsImg3DataSDOM);


		// Trouvons la position du tag PROD dans le certificat.
		size_t Img3PRODpos = Img3DataCERT.find("444f5250"); //DORP, en little-endian.
		string Img3PROD = Img3DataCERT.substr(Img3PRODpos, 32);

		Img3TagPROD = HexStringToChars((LittleToBigEndian(Img3PROD.substr(0,8))));
		cout << "Img3 tag PROD : " << Img3TagPROD << endl;
		QString qsImg3TagPROD = QString::fromStdString(Img3TagPROD);
		ui.txtImg3TagPROD->setText(qsImg3TagPROD);
		ui.txtImg3TagPROD->setText(qsImg3TagPROD);

		Img3TotalLengthPROD = (LittleToBigEndian(Img3PROD.substr(8,16)));
		cout << "Img3 tag PROD total length : " << "0x" << Img3TotalLengthPROD  << endl;
		QString qsImg3TotalLengthPROD  = QString::fromStdString("0x" + Img3TotalLengthPROD);
		ui.txtImg3TotalLengthPROD ->setText(qsImg3TotalLengthPROD);
		ui.txtImg3TotalLengthPROD ->setText(qsImg3TotalLengthPROD);

		Img3DataLengthPROD = (LittleToBigEndian(Img3PROD.substr(16,24)));
		cout << "Img3 tag PROD data length : " << "0x" << Img3DataLengthPROD  << endl;
		QString qsImg3DataLengthPROD  = QString::fromStdString("0x" + Img3DataLengthPROD);
		ui.txtImg3DataLengthPROD->setText(qsImg3DataLengthPROD);
		ui.txtImg3DataLengthPROD->setText(qsImg3DataLengthPROD);

		Img3DataPROD = (LittleToBigEndian(Img3PROD.substr(24,32)));
		cout << "Img3 tag PROD data : " << "0x" << Img3DataPROD  << endl;
		QString qsImg3DataPROD  = QString::fromStdString("0x" + Img3DataPROD);
		ui.txtImg3DataPROD->setText(qsImg3DataPROD);
		ui.txtImg3DataPROD->setText(qsImg3DataPROD);

		// Trouvons la position du tag CHIP dans le certificat.
		size_t Img3CHIPcertpos = Img3DataCERT.find("50494843"); //PIHC, en little-endian.
		string Img3CHIPcert = Img3DataCERT.substr(Img3CHIPcertpos, 32);

		Img3TagCHIPcert = HexStringToChars((LittleToBigEndian(Img3CHIPcert.substr(0,8))));
		cout << "Img3 tag CHIP (in CERT) : " << Img3TagCHIPcert << endl;
		QString qsImg3TagCHIPcert = QString::fromStdString(Img3TagCHIPcert);
		ui.txtImg3TagCHIPcert->setText(qsImg3TagCHIPcert);
		ui.txtImg3TagCHIPcert->setText(qsImg3TagCHIPcert);

		Img3TotalLengthCHIPcert = (LittleToBigEndian(Img3CHIPcert.substr(8,16)));
		cout << "Img3 tag CHIP (in CERT) total length : " << "0x" << Img3TotalLengthCHIPcert  << endl;
		QString qsImg3TotalLengthCHIPcert  = QString::fromStdString("0x" + Img3TotalLengthCHIPcert);
		ui.txtImg3TotalLengthCHIPcert ->setText(qsImg3TotalLengthCHIPcert);
		ui.txtImg3TotalLengthCHIPcert ->setText(qsImg3TotalLengthCHIPcert);

		Img3DataLengthCHIPcert = (LittleToBigEndian(Img3CHIPcert.substr(16,24)));
		cout << "Img3 tag CHIP (in CERT) data length : " << "0x" << Img3DataLengthCHIPcert  << endl;
		QString qsImg3DataLengthCHIPcert  = QString::fromStdString("0x" + Img3DataLengthCHIPcert);
		ui.txtImg3DataLengthCHIPcert->setText(qsImg3DataLengthCHIPcert);
		ui.txtImg3DataLengthCHIPcert->setText(qsImg3DataLengthCHIPcert);

		Img3DataCHIPcert = (LittleToBigEndian(Img3CHIPcert.substr(24,32)));
		cout << "Img3 tag CHIP (in CERT) data : " << "0x" << Img3DataCHIPcert  << endl;
		QString qsImg3DataCHIPcert  = QString::fromStdString("0x" + Img3DataCHIPcert);
		ui.txtImg3DataCHIPcert->setText(qsImg3DataCHIPcert);
		ui.txtImg3DataCHIPcert->setText(qsImg3DataCHIPcert);
	}
	//else
	//{
	//	cout << "The input file is an encrypted Img3 container." << endl;
	//	cout << "The function to read encrypted Img3 containers is not completed." << endl;
	//	Encryption = true;
	//	QString ErrorEncryption("The function to read encrypted Img3 containers is not completed.");
	//	QMessageBox::warning(this, "Message", ErrorEncryption);
	//	m_FirstFile = false;
	//	return;
	//}
	m_FirstFile = false;
	cout << "Finished to read Img3 tags." << endl;
}

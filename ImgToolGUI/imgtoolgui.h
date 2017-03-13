#ifndef IMGTOOLGUI_H
#define IMGTOOLGUI_H

#include <QtGui/QMainWindow>
#include "ui_imgtoolgui.h"
#include <iostream>
#include <sstream>

using namespace std;

class ImgToolGUI : public QMainWindow
{
    Q_OBJECT

public:
    ImgToolGUI(QWidget *parent = 0);
    ~ImgToolGUI();
    void Cleanup();
    void ReadFile(string FilePath);

    typedef unsigned char BYTE;
    void ReadImg3(BYTE* FileBuffer);

private slots:
   	void OpenFile();
   	void ExportCert();
   	void ExportImage();

private:
    Ui::ImgToolGUIClass ui;
    //BYTE *m_FileBuffer; // Pointeur aux données dans le buffer
    bool m_FirstFile;
    stringstream m_ssExportImg3CERT;
    stringstream m_ssExportImg3DATA;
};

#endif // IMGTOOLGUI_H

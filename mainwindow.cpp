#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->setRange(0, 100);
    connect(ui->pushButton_login, &QPushButton::clicked, this, &MainWindow::slot_pushButton_login_onClicked);
    connect(ui->listWidget_dirs, &QListWidget::doubleClicked, this, &MainWindow::slot_listWidget_dirs_doubleClicked);
    connect(ui->pushButton_upload, &QPushButton::clicked, this, &MainWindow::slot_pushButton_upload_onClicked);
    connect(ui->pushButton_download, &QPushButton::clicked, this, &MainWindow::slot_pushButton_download_onClicked);
    mp_ftp = new FtpClient;
    connect(mp_ftp, &FtpClient::signal_out_updateDirNames, this, &MainWindow::slot_updateDirNames);
    connect(mp_ftp, &FtpClient::signal_out_updateFileNames, this, &MainWindow::slot_updateFileNames);
    connect(mp_ftp, &FtpClient::signal_out_downloadFileDone, this, &MainWindow::slot_downloadFileDone);
    connect(mp_ftp, &FtpClient::signal_out_uploadFileDone, this, &MainWindow::slot_uploadFileDone);
    connect(mp_ftp, &FtpClient::signal_out_dataTransferProgress, this, &MainWindow::slot_dataTransferProgress);
    connect(mp_ftp, &FtpClient::signal_out_updateState, this, &MainWindow::slot_updateFtpState);
}

MainWindow::~MainWindow()
{
    delete ui;
    if(mp_ftp)
    {
        delete mp_ftp;
        mp_ftp = NULL;
    }
}

void MainWindow::slot_pushButton_login_onClicked()
{
    mp_ftp->setFtpServer(ui->lineEdit_ftpHost->text(), ui->lineEdit_ftpPort->text().toInt(), ui->lineEdit_ftpUser->text(), ui->lineEdit_ftpPasswd->text());
}

void MainWindow::slot_pushButton_upload_onClicked()
{
    QString filename = QFileDialog::getOpenFileName(NULL, tr("上传文件"), "./");
    if(filename.isEmpty())
        return;
    if(mp_file)
    {
        if(mp_file->isOpen())
            mp_file->close();
        delete mp_file;
    }
    mp_file = new QFile(filename);
    if(mp_file->exists() && mp_file->open(QIODevice::ReadOnly))
    {
        emit mp_ftp->signal_in_uploadFile(mp_file, QFileInfo(*mp_file).fileName());
    }
    else
    {

    }
}

void MainWindow::slot_pushButton_download_onClicked()
{
    QListWidgetItem* item = ui->listWidget_Files->currentItem();
    if(!item) return;
    QString name = item->text();
    if(name.isEmpty())
        return;
    mp_file = new QFile(QString("./%1").arg(name));
    if(mp_file->open(QIODevice::WriteOnly))
    {
        emit mp_ftp->signal_in_downloadFile(name, mp_file);
    }
}

void MainWindow::slot_updateFileNames(QStringList names)
{
    ui->listWidget_Files->clear();
    ui->listWidget_Files->addItems(names);
}

void MainWindow::slot_updateDirNames(QStringList names)
{
    ui->listWidget_dirs->clear();
    ui->listWidget_dirs->addItem("..");
    ui->listWidget_dirs->addItems(names);
}

void MainWindow::slot_updateFtpState(bool isLinked)
{
    if(isLinked)
    {
        ui->pushButton_login->setEnabled(false);
        ui->listWidget_Files->setEnabled(true);
        ui->listWidget_dirs->setEnabled(true);
        ui->pushButton_download->setEnabled(true);
        ui->pushButton_upload->setEnabled(true);
    }
    else
    {
        ui->pushButton_login->setEnabled(true);
        ui->listWidget_Files->setEnabled(false);
        ui->listWidget_dirs->setEnabled(false);
        ui->pushButton_download->setEnabled(false);
        ui->pushButton_upload->setEnabled(false);
        ui->listWidget_Files->clear();
        ui->listWidget_dirs->clear();
    }
}

void MainWindow::slot_listWidget_dirs_doubleClicked(const QModelIndex &index)
{
    if(!mp_ftp) return;
    int dex = index.row();
    if(dex<0)
        return;
    QString name = ui->listWidget_dirs->item(dex)->text();
    if(name == "..")
        emit mp_ftp->signal_in_exitDir();
    else
        emit mp_ftp->signal_in_openDir(name);
}

void MainWindow::slot_downloadFileDone(bool error)
{
    ui->progressBar->setHidden(true);
    if(mp_file)
    {
        if(mp_file->isOpen())
        {
            mp_file->close();
        }
        delete mp_file;
        mp_file = NULL;
    }
    if(!error)
    {
        ui->progressBar->setValue(100);
        ui->progressBar->setHidden(false);
    }
    if(error)
        QMessageBox::warning(NULL, tr("文件下载"), tr("文件下载失败!"));
    else
        QMessageBox::information(NULL, tr("文件下载"), tr("文件下载成功!"));
    emit mp_ftp->signal_in_openDir("");
}

void MainWindow::slot_uploadFileDone(bool error)
{
    ui->progressBar->setHidden(true);
    if(mp_file)
    {
        if(mp_file->isOpen())
        {
            mp_file->close();
        }
        delete mp_file;
        mp_file = NULL;
    }
    if(!error)
    {
        ui->progressBar->setValue(100);
        ui->progressBar->setHidden(false);
    }
    if(error)
        QMessageBox::warning(NULL, tr("文件上传"), tr("文件上传失败!"));
    else
        QMessageBox::information(NULL, tr("文件上传"), tr("文件上传成功!"));
    emit mp_ftp->signal_in_openDir("");
}

void MainWindow::slot_dataTransferProgress(qint64 v, qint64 p)
{
    if(p>0)
    {
        qint64 process = (v*100)/p;
        qDebug()<<v<<p<<process;
        ui->progressBar->setValue((int)process);
    }
}

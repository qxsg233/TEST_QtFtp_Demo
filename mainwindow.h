#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QProgressDialog>
#include "ftpclient.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void slot_pushButton_login_onClicked();
    void slot_pushButton_upload_onClicked();
    void slot_pushButton_download_onClicked();
    void slot_updateFileNames(QStringList names);
    void slot_updateDirNames(QStringList names);
    void slot_updateFtpState(bool isLinked);
    void slot_listWidget_dirs_doubleClicked(const QModelIndex &index);
    void slot_downloadFileDone(bool error);
    void slot_uploadFileDone(bool error);
    void slot_dataTransferProgress(qint64 v, qint64 p);
private:
    Ui::MainWindow *ui;
    FtpClient* mp_ftp = NULL;
    QFile* mp_file = NULL;
};
#endif // MAINWINDOW_H

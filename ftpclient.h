#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <QObject>
#include <QDir>
#include "qftp.h"
class FtpClient : public QObject
{
    Q_OBJECT
signals:
    void signal_out_updateFileNames(QStringList names);
    void signal_out_updateDirNames(QStringList name);
    void signal_out_updateState(bool isLinked);

    void signal_in_openDir(QString name);
    void signal_in_exitDir();
    void signal_in_downloadFile(QString name, QIODevice* dev);
    void signal_in_uploadFile(QIODevice* dev, QString name);
    void signal_out_downloadFileDone(bool error);
    void signal_out_uploadFileDone(bool error);
    void signal_out_dataTransferProgress(qint64 v, qint64 p);
public:
    explicit FtpClient(QObject *parent = nullptr);
    ~FtpClient();
    void setFtpServer(const QString &host, const quint16 &port, const QString &username, const QString &passwd);
    static QString TranslateQFtpCommand(const int &cmd);
    static QString TranslateQFtpState(const int &state);
    void printLog(const QString &msg, const bool &isError=false, const bool &printInfo=false);
private slots:
    void slot_commandFinished(int, bool);
    void slot_commandStarted(int);
    void slot_rawCommandReply(int, const QString&);
    void slot_dataTransferProgress(qint64, qint64);
    void slot_done(bool);
    void slot_listInfo(const QUrlInfo&);
    void slot_stateChanged(int);

    void slot_openDir(const QString &name);
    void slot_exitDir();
    void slot_downloadFile(QString name, QIODevice* dev);
    void slot_uploadFile(QIODevice* dev, QString name);
private:
    QFtp* mp_ftp;
    QString m_hostName;
    quint16 m_port;
    QString m_username;
    QString m_passwd;
    int m_state;
    QStringList m_ftpCurFiles;
    QStringList m_ftpCurDirs;
    QStringList m_ftpCurDir;
};

#endif // FTPCLIENT_H

#include "ftpclient.h"
#include <QDebug>
#include <QTextCodec>
FtpClient::FtpClient(QObject *parent)
    : QObject{parent}
{
    connect(this, &FtpClient::signal_in_openDir, this, &FtpClient::slot_openDir);
    connect(this, &FtpClient::signal_in_exitDir, this, &FtpClient::slot_exitDir);
    connect(this, &FtpClient::signal_in_downloadFile, this, &FtpClient::slot_downloadFile);
    connect(this, &FtpClient::signal_in_uploadFile, this, &FtpClient::slot_uploadFile);
    mp_ftp = new QFtp();
    m_state = QFtp::Close;
    m_ftpCurDir.clear();
    connect(mp_ftp, &QFtp::commandFinished, this, &FtpClient::slot_commandFinished);
    connect(mp_ftp, &QFtp::commandStarted, this, &FtpClient::slot_commandStarted);
    connect(mp_ftp, &QFtp::rawCommandReply, this, &FtpClient::slot_rawCommandReply);
    connect(mp_ftp, &QFtp::dataTransferProgress, this, &FtpClient::slot_dataTransferProgress);
    connect(mp_ftp, &QFtp::done, this, &FtpClient::slot_done);
    connect(mp_ftp, &QFtp::listInfo, this, &FtpClient::slot_listInfo);
    connect(mp_ftp, &QFtp::stateChanged, this, &FtpClient::slot_stateChanged);
}

FtpClient::~FtpClient()
{
    if(mp_ftp)
    {
        mp_ftp->close();
        delete mp_ftp;
    }
    mp_ftp = NULL;
}

void FtpClient::setFtpServer(const QString &host, const quint16 &port, const QString &username, const QString &passwd)
{
    m_hostName = host;
    m_port = port;
    m_username = username;
    m_passwd = passwd;
    if(mp_ftp)
    {
        mp_ftp->connectToHost(m_hostName, port);
    }
}

QString FtpClient::TranslateQFtpCommand(const int &cmd)
{
    switch (cmd) {
    case QFtp::None:
        return "None";
    case QFtp::SetTransferMode:
        return "SetTransferMode";
    case QFtp::SetProxy:
        return "SetProxy";
    case QFtp::ConnectToHost:
        return "ConnectToHost";
    case QFtp::Login:
        return "Login";
    case QFtp::Close:
        return "Close";
    case QFtp::List:
        return "List";
    case QFtp::Cd:
        return "Cd";
    case QFtp::Get:
        return "Get";
    case QFtp::Put:
        return "Put";
    case QFtp::Remove:
        return "Remove";
    case QFtp::Mkdir:
        return "Mkdir";
    case QFtp::Rmdir:
        return "Rmdir";
    case QFtp::Rename:
        return "Rename";
    case QFtp::RawCommand:
        return "RawCommand";
    default:
        return "";
    }
    return "";
}

QString FtpClient::TranslateQFtpState(const int &state)
{
    switch (state) {
    case QFtp::Unconnected:
        return "Unconnected";
    case QFtp::HostLookup:
        return "HostLookup";
    case QFtp::Connecting:
        return "Connecting";
    case QFtp::Connected:
        return "Connected";
    case QFtp::LoggedIn:
        return "LoggedIn";
    case QFtp::Closing:
        return "Closing";
    default:
        return "";
    }
    return "";
}

void FtpClient::printLog(const QString &msg, const bool &isError, const bool &printInfo)
{
    qDebug()<<msg;
}

void FtpClient::slot_commandFinished(int id, bool isError)
{
    // qDebug()<<"slot_commandFinished"<<id<<isError;
    if(!mp_ftp) return;
    qDebug()<<TranslateQFtpCommand(mp_ftp->currentCommand());
    if(mp_ftp->currentCommand() == QFtp::ConnectToHost)
    {
        if(!isError)//连接成功就登录
        {
            mp_ftp->login(m_username, m_passwd);
            printLog(tr("服务器连接成功!"));
        }
        else
        {
            printLog(tr("服务器连接失败(%1)").arg(mp_ftp->errorString()), true, true);
            return;
        }
    }
    else if(mp_ftp->currentCommand() == QFtp::Login)
    {
        if(!isError)
        {
            printLog(tr("FTP登录成功!"));
            m_ftpCurDirs.clear();
            m_ftpCurFiles.clear();
            // m_ftpCurDir = "/";
            mp_ftp->list(m_ftpCurDir.join('/')+"/");
        }
        else
        {
            printLog(tr("FTP登录失败(%1)").arg(mp_ftp->errorString()), true, true);
            return;
        }
    }
    else if(mp_ftp->currentCommand() == QFtp::List)
    {
        if(!isError)
        {
            printLog(tr("更新列表: (%1)(%2)").arg(m_ftpCurDirs.join(',')).arg(m_ftpCurFiles.join(',')));
        }
        else
        {
            printLog(tr("更新列表失败(%1)").arg(mp_ftp->errorString()), true, true);
            return;
        }
    }
    else if(mp_ftp->currentCommand() == QFtp::Get)
    {
        emit signal_out_downloadFileDone(isError);
        qDebug()<< TranslateQFtpCommand(mp_ftp->currentCommand());
        printLog(tr("下载%1").arg(isError?"失败":"成功"), isError, true);
    }
    else if(mp_ftp->currentCommand() == QFtp::Put)
    {
        emit signal_out_uploadFileDone(isError);
        printLog(tr("上传%1").arg(isError?"失败":"成功"), isError, true);
    }
}

void FtpClient::slot_commandStarted(int id)
{
    // qDebug()<<"slot_commandStarted"<<id;
}

void FtpClient::slot_rawCommandReply(int id, const QString &str)
{
    qDebug()<<"slot_rawCommandReply"<<id<<str;
}

void FtpClient::slot_dataTransferProgress(qint64 v, qint64 p)
{
    // qDebug()<<"slot_dataTransferProgress"<<v<<p;
    emit signal_out_dataTransferProgress(v,p);
}

void FtpClient::slot_done(bool error)
{

}

void FtpClient::slot_listInfo(const QUrlInfo &info)
{
    if(info.isDir())
    {
        m_ftpCurDirs<<info.name();
    }
    else if(info.isFile())
    {
        m_ftpCurFiles<<info.name();
    }
    emit signal_out_updateDirNames(m_ftpCurDirs);
    emit signal_out_updateFileNames(m_ftpCurFiles);
}

void FtpClient::slot_stateChanged(int state)
{
    m_state = state;
    // qDebug()<<"slot_stateChanged"<<TranslateQFtpState(state);
    if(state == QFtp::LoggedIn)
    {
        emit signal_out_updateState(true);
    }
    else if(state == QFtp::Unconnected)
    {
        emit signal_out_updateState(false);
    }
}

void FtpClient::slot_openDir(const QString &name)
{
    if(mp_ftp)
    {
        // qDebug()<<"name"<<name;
        m_ftpCurDir.append(name);
        m_ftpCurDirs.clear();
        m_ftpCurFiles.clear();
        qDebug()<<m_ftpCurDir;
        mp_ftp->list(m_ftpCurDir.join('/')+"/");
    }
}

void FtpClient::slot_exitDir()
{
    if(mp_ftp)
    {
        m_ftpCurDirs.clear();
        m_ftpCurFiles.clear();
        if(!m_ftpCurDir.isEmpty())
        {
            m_ftpCurDir.removeLast();
            mp_ftp->list(m_ftpCurDir.join('/')+"/");
        }
        else
        {
            mp_ftp->close();
        }
    }
}

void FtpClient::slot_downloadFile(QString name, QIODevice* dev)
{
    qDebug()<<"slot_downloadFile"<<name;
    if(!(mp_ftp&&dev)) return;
    if(mp_ftp->currentCommand() != QFtp::None)
    {
        printLog(tr("当前状态(%1)无法下载").arg(TranslateQFtpCommand(mp_ftp->currentCommand())), true, true);
        return;
    }
    QString filename = m_ftpCurDir.join('/')+"/"+name;
    qDebug()<<"filename"<<filename;
    if(!dev->isOpen())
    {
        bool n_ret = dev->open(QIODevice::WriteOnly);
        if(!n_ret)
        {
            printLog(tr("无法打开文件"), true, true);
            return;
        }
    }
    mp_ftp->get(filename, dev);
}

void FtpClient::slot_uploadFile(QIODevice *dev, QString name)
{
    if(!(mp_ftp&&dev)) return;
    if(mp_ftp->currentCommand() != QFtp::None)
    {
        printLog(tr("当前状态(%1)无法上传").arg(TranslateQFtpCommand(mp_ftp->currentCommand())), true, true);
        return;
    }
    QString filename = m_ftpCurDir.join('/')+"/"+name;
    if(!dev->isOpen())
    {
        bool n_ret = dev->open(QIODevice::ReadOnly);
        if(!n_ret)
        {
            printLog(tr("无法打开文件"), true, true);
            return;
        }
    }
    mp_ftp->put(dev, filename);
}

#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QStringList>
#include "global.h"
#include "tcpmgr.h"
#include "filetcpmgr.h"

int main(int argc, char *argv[])
{
    // 在main函数中设置缩放比例 ，否则聊天窗口显示不全
    qputenv("QT_SCALE_FACTOR", "0.69");

    QApplication a(argc, argv);

    QStringList qssPaths;
    qssPaths << QDir::currentPath() + "/style/stylesheet.qss"
             << QCoreApplication::applicationDirPath() + "/style/stylesheet.qss"
             << QCoreApplication::applicationDirPath() + "/../style/stylesheet.qss"
             << QCoreApplication::applicationDirPath() + "/../../style/stylesheet.qss"
             << QCoreApplication::applicationDirPath() + "/../../../style/stylesheet.qss"
             << QCoreApplication::applicationDirPath() + "/../../../../style/stylesheet.qss"
             << ":/style/stylesheet.qss";

    for (const auto &qssPath : qssPaths) {
        QFile qss(QDir::cleanPath(qssPath));
        if (qss.open(QFile::ReadOnly)) {
            qDebug("open qss success");
            QString style = QString::fromUtf8(qss.readAll());
            if(!style.isEmpty() && style.at(0) == QChar::ByteOrderMark){
                style.remove(0, 1);
            }
            a.setStyleSheet(style);
            qss.close();
            break;
        }
    }

    // 获取当前应用程序的路径
    QString app_path = QCoreApplication::applicationDirPath();
    // 拼接文件名
    QString fileName = "config.ini";
    QString config_path = QDir::toNativeSeparators(app_path +
                                                   QDir::separator() + fileName);

    QSettings settings(config_path, QSettings::IniFormat);
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    gate_url_prefix = "http://"+gate_host+":"+gate_port;

    //启动tcp线程
    TcpThread tcpthread;
    //启动资源网络线程
    FileTcpThread file_tcp_thread;
    MainWindow w;
    w.show();
    return a.exec();
}

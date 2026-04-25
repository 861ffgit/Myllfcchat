#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "resetdialog.h"
#include "tcpmgr.h"
#include <QLayout>
#include <QMessageBox>
#include "filetcpmgr.h"

namespace {
QSize ResolvePageSize(QWidget *page)
{
    QSize page_size = page->sizeHint();
    if(!page_size.isValid() || page_size.width() <= 0 || page_size.height() <= 0) {
        page_size = page->size();
    }

    if(!page_size.isValid() || page_size.width() <= 0 || page_size.height() <= 0) {
        page_size = page->minimumSizeHint();
    }

    const QSize min_size = page->minimumSize();
    if(min_size.width() > 0 && min_size.height() > 0) {
        page_size = page_size.expandedTo(min_size);
    }

    const QSize max_size = page->maximumSize();
    if(max_size.width() > 0 && max_size.height() > 0
            && max_size.width() < QWIDGETSIZE_MAX && max_size.height() < QWIDGETSIZE_MAX) {
        page_size = page_size.boundedTo(max_size);
    }

    if(!page_size.isValid() || page_size.width() <= 0 || page_size.height() <= 0) {
        page_size = QSize(300, 500);
    }

    return page_size;
}

QString InfoTitle()
{
    return QString::fromUtf8(u8"\u63d0\u793a");
}
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _login_dlg(nullptr),
    _reg_dlg(nullptr),
    _reset_dlg(nullptr),
    _chat_dlg(nullptr),
    _ui_status(LOGIN_UI)
{
    ui->setupUi(this);

    _login_dlg = new LoginDialog(this);
    SetCentralPage(_login_dlg);
    AdjustAuthWindowSize(_login_dlg);

    connect(_login_dlg, &LoginDialog::switchRegister,
            this, &MainWindow::SlotSwitchReg, Qt::QueuedConnection);
    connect(_login_dlg, &LoginDialog::switchReset,
            this, &MainWindow::SlotSwitchReset, Qt::QueuedConnection);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_swich_chatdlg,
            this, &MainWindow::SlotSwitchChat);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_notify_offline,
            this, &MainWindow::SlotOffline);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_connection_closed,
            this, &MainWindow::SlotExcepConOffline);
    connect(FileTcpMgr::GetInstance().get(), &FileTcpMgr::sig_connection_closed,
            this, &MainWindow::SlotResServerConOffline);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetCentralPage(QWidget *page)
{
    if(page == nullptr) {
        return;
    }

    page->hide();
    page->setWindowFlags(Qt::Widget);
    page->setParent(this);

    setCentralWidget(page);
    page->show();
    page->raise();
}

void MainWindow::SlotSwitchReg()
{
    _reg_dlg = new RegisterDialog(this);
    connect(_reg_dlg, &RegisterDialog::sigSwitchLogin,
            this, &MainWindow::SlotSwitchLogin, Qt::QueuedConnection);

    SetCentralPage(_reg_dlg);
    AdjustAuthWindowSize(_reg_dlg);
    _ui_status = REGISTER_UI;
}

void MainWindow::SlotSwitchLogin()
{
    _login_dlg = new LoginDialog(this);
    SetCentralPage(_login_dlg);
    AdjustAuthWindowSize(_login_dlg);

    connect(_login_dlg, &LoginDialog::switchRegister,
            this, &MainWindow::SlotSwitchReg, Qt::QueuedConnection);
    connect(_login_dlg, &LoginDialog::switchReset,
            this, &MainWindow::SlotSwitchReset, Qt::QueuedConnection);
    _ui_status = LOGIN_UI;
}

void MainWindow::SlotSwitchReset()
{
    _ui_status = RESET_UI;
    _reset_dlg = new ResetDialog(this);
    SetCentralPage(_reset_dlg);
    AdjustAuthWindowSize(_reset_dlg);

    connect(_reset_dlg, &ResetDialog::switchLogin,
            this, &MainWindow::SlotSwitchLogin2, Qt::QueuedConnection);
}

void MainWindow::SlotSwitchLogin2()
{
    _login_dlg = new LoginDialog(this);
    SetCentralPage(_login_dlg);
    AdjustAuthWindowSize(_login_dlg);

    connect(_login_dlg, &LoginDialog::switchReset,
            this, &MainWindow::SlotSwitchReset, Qt::QueuedConnection);
    connect(_login_dlg, &LoginDialog::switchRegister,
            this, &MainWindow::SlotSwitchReg, Qt::QueuedConnection);
    _ui_status = LOGIN_UI;
}

void MainWindow::SlotSwitchChat()
{
    if(_chat_dlg == nullptr) {
        _chat_dlg = new ChatDialog(this);
    }

    SetCentralPage(_chat_dlg);
    setMinimumSize(QSize(1050, 900));
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    _ui_status = CHAT_UI;
    _chat_dlg->loadChatList();
}

void MainWindow::SlotOffline()
{
    QMessageBox::information(this, InfoTitle(),
                             QString::fromUtf8(u8"\u8d26\u53f7\u5df2\u5728\u5176\u4ed6\u8bbe\u5907\u767b\u5f55\uff0c\u5f53\u524d\u4f1a\u8bdd\u5c06\u8fd4\u56de\u767b\u5f55\u9875\u9762"));
    TcpMgr::GetInstance()->CloseConnection();
    offlineLogin();
}

void MainWindow::SlotExcepConOffline()
{
    QMessageBox::information(this, InfoTitle(),
                             QString::fromUtf8(u8"\u670d\u52a1\u5668\u8fde\u63a5\u5df2\u65ad\u5f00\uff0c\u5f53\u524d\u4f1a\u8bdd\u5c06\u8fd4\u56de\u767b\u5f55\u9875\u9762"));
    TcpMgr::GetInstance()->CloseConnection();
    FileTcpMgr::GetInstance()->CloseConnection();
    offlineLogin();
}

void MainWindow::SlotResServerConOffline()
{
    QMessageBox::information(this, InfoTitle(),
                             QString::fromUtf8(u8"\u8d44\u6e90\u670d\u52a1\u5668\u8fde\u63a5\u5df2\u65ad\u5f00\uff0c\u8bf7\u91cd\u65b0\u767b\u5f55"));
    TcpMgr::GetInstance()->CloseConnection();
    FileTcpMgr::GetInstance()->CloseConnection();
    offlineLogin();
}

void MainWindow::offlineLogin()
{
    if(_ui_status == LOGIN_UI) {
        return;
    }

    _login_dlg = new LoginDialog(this);
    SetCentralPage(_login_dlg);
    AdjustAuthWindowSize(_login_dlg);

    connect(_login_dlg, &LoginDialog::switchRegister,
            this, &MainWindow::SlotSwitchReg, Qt::QueuedConnection);
    connect(_login_dlg, &LoginDialog::switchReset,
            this, &MainWindow::SlotSwitchReset, Qt::QueuedConnection);
    _ui_status = LOGIN_UI;
}

void MainWindow::AdjustAuthWindowSize(QWidget *page)
{
    if(page == nullptr) {
        return;
    }

    const QSize page_size = ResolvePageSize(page);

    QSize frame_size = size();
    QSize central_size = centralWidget() ? centralWidget()->size() : QSize();
    if(!central_size.isValid() || central_size.width() <= 0 || central_size.height() <= 0) {
        central_size = page_size;
    }

    QSize target_size(page_size.width() + frame_size.width() - central_size.width(),
                      page_size.height() + frame_size.height() - central_size.height());

    setMinimumSize(target_size);
    setMaximumSize(target_size);
    resize(target_size);
}

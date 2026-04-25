#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include "logindialog.h"
#include "registerdialog.h"
#include "resetdialog.h"
#include "chatdialog.h"
/******************************************************************************
 *
 * @file       mainwindow.h
 * @brief      主界面功能 Function
 *
 * @author     GGB
 * @date       2025/12/30
 * @history
 *****************************************************************************/
namespace Ui {
class MainWindow;
}

enum UIStatus{
    LOGIN_UI,
    REGISTER_UI,
    RESET_UI,
    CHAT_UI
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void SlotSwitchReg();
    void SlotSwitchLogin();
    void SlotSwitchReset();
    void SlotSwitchLogin2();
    void SlotSwitchChat();
    void SlotOffline();
    void SlotExcepConOffline();
    void SlotResServerConOffline();

private:
    void SetCentralPage(QWidget *page);
    void AdjustAuthWindowSize(QWidget *page);
    void offlineLogin();
    Ui::MainWindow *ui;
    QPointer<LoginDialog> _login_dlg;
    QPointer<RegisterDialog> _reg_dlg;
    QPointer<ResetDialog> _reset_dlg;
    QPointer<ChatDialog> _chat_dlg;
    UIStatus _ui_status;
};

#endif // MAINWINDOW_H

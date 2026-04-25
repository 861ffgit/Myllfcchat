#include "friendinfopage.h"
#include "ui_friendinfopage.h"
#include <QDebug>

FriendInfoPage::FriendInfoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendInfoPage),_user_info(nullptr)
{
    ui->setupUi(this);
    setObjectName("friend_info_page");
    setStyleSheet(
        "QWidget#friend_info_page{background:#ffffff;border:1px solid #e8edf2;border-radius:20px;}"
        "QLabel#name_lb{color:#17212b;font:600 24px 'Microsoft YaHei';}"
        "QLabel#nick_tip,QLabel#bak_tip{color:#8a97a6;font:13px 'Microsoft YaHei';}"
        "QLabel#nick_lb,QLabel#bak_lb{color:#2a3340;font:14px 'Microsoft YaHei';}"
        "QPushButton#msg_chat,QPushButton#voice_chat,QPushButton#video_chat{background:transparent;border:none;padding:0px;}"
        "QPushButton#msg_chat[state='normal']{border-image:url(:/res/msg_chat_normal.png);}"
        "QPushButton#msg_chat[state='hover']{border-image:url(:/res/msg_chat_hover.png);}"
        "QPushButton#msg_chat[state='press']{border-image:url(:/res/msg_chat_press.png);}"
        "QPushButton#voice_chat[state='normal']{border-image:url(:/res/voice_chat_normal.png);}"
        "QPushButton#voice_chat[state='hover']{border-image:url(:/res/voice_chat_hover.png);}"
        "QPushButton#voice_chat[state='press']{border-image:url(:/res/voice_chat_press.png);}"
        "QPushButton#video_chat[state='normal']{border-image:url(:/res/video_chat_normal.png);}"
        "QPushButton#video_chat[state='hover']{border-image:url(:/res/video_chat_hover.png);}"
        "QPushButton#video_chat[state='press']{border-image:url(:/res/video_chat_press.png);}"
    );
    ui->msg_chat->SetState("normal","hover","press");
    ui->video_chat->SetState("normal","hover","press");
    ui->voice_chat->SetState("normal","hover","press");
}

FriendInfoPage::~FriendInfoPage()
{
    delete ui;
}

void FriendInfoPage::SetInfo(std::shared_ptr<UserInfo> user_info)
{
    _user_info = user_info;
    // 加载图片
    QPixmap pixmap(user_info->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->name_lb->setText(user_info->_name);
    ui->nick_lb->setText(user_info->_nick);
    ui->bak_lb->setText(user_info->_nick);
}

void FriendInfoPage::on_msg_chat_clicked()
{
    qDebug() << "msg chat btn clicked";
    emit sig_jump_chat_item(_user_info);
}

#include "logindialog.h"
#include "ui_logindialog.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRegularExpression>
#include "httpmgr.h"
#include "tcpmgr.h"
#include "filetcpmgr.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    ui->email_edit->SetBackgroundColor(QColor(0xf6, 0xfb, 0xf7), QColor(0xff, 0xff, 0xff));
    ui->pass_edit->SetBackgroundColor(QColor(0xf6, 0xfb, 0xf7), QColor(0xff, 0xff, 0xff));
    ui->email_edit->setTextMargins(14, 0, 14, 0);
    ui->pass_edit->setTextMargins(14, 0, 14, 0);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    showTip(QString(), true);
    ui->title_label->hide();
    ui->subtitle_label->hide();

    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);
    ui->forget_label->SetState("normal", "hover", "", "selected", "selected_hover", "");
    ui->forget_label->setCursor(Qt::PointingHandCursor);
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slot_forget_pwd);

    ui->pass_visible->setCursor(Qt::PointingHandCursor);
    ui->pass_visible->SetState("unvisible", "unvisible_hover", "", "visible",
                               "visible_hover", "");
    connect(ui->pass_visible, &ClickedLabel::clicked, this, [this]() {
        auto state = ui->pass_visible->GetCurState();
        if(state == ClickLbState::Normal){
            ui->pass_edit->setEchoMode(QLineEdit::Password);
        }else{
            ui->pass_edit->setEchoMode(QLineEdit::Normal);
        }
    });

    initHttpHandlers();

    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish,
            this, &LoginDialog::slot_login_mod_finish);
    connect(this, &LoginDialog::sig_connect_tcp,
            TcpMgr::GetInstance().get(), &TcpMgr::slot_tcp_connect);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_con_success,
            this, &LoginDialog::slot_tcp_con_finish);
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_login_failed,
            this, &LoginDialog::slot_login_failed);
    connect(this, &LoginDialog::sig_connect_res_server,
            FileTcpMgr::GetInstance().get(), &FileTcpMgr::slot_tcp_connect);
    connect(FileTcpMgr::GetInstance().get(), &FileTcpMgr::sig_con_success,
            this, &LoginDialog::slot_res_con_finish);

    initHead();
}

LoginDialog::~LoginDialog()
{
    qDebug() << "destruct LoginDlg";
    delete ui;
}

void LoginDialog::initHead()
{
    QPixmap originalPixmap(":/res/head_1.jpg");
    qDebug() << originalPixmap.size() << ui->head_label->size();

    originalPixmap = originalPixmap.scaled(ui->head_label->size(),
                                           Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    QPixmap roundedPixmap(ui->head_label->size());
    roundedPixmap.fill(Qt::transparent);

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addRoundedRect(QRectF(roundedPixmap.rect()), 10, 10);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, originalPixmap);

    ui->head_label->setPixmap(roundedPixmap);
}

void LoginDialog::initHttpHandlers()
{
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](const QJsonObject& jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"), false);
            enableBtn(true);
            return;
        }
        auto email = jsonObj["email"].toString();

        _si = std::make_shared<ServerInfo>();
        _si->_uid = jsonObj["uid"].toInt();
        _si->_chat_host = jsonObj["chathost"].toString();
        _si->_chat_port = jsonObj["chatport"].toString();
        _si->_token = jsonObj["token"].toString();
        _si->_res_host = jsonObj["reshost"].toString();
        _si->_res_port = jsonObj["resport"].toString();

        qDebug() << "email is " << email
                 << "uid is" << _si->_uid
                 << "chat host is" << _si->_chat_host
                 << "chat port is" << _si->_chat_port
                 << "token is" << _si->_token
                 << "res host is" << _si->_res_host
                 << "res port is" << _si->_res_port;
        emit sig_connect_tcp(_si);
    });
}

void LoginDialog::showTip(QString str, bool b_ok)
{
    if(str.isEmpty()){
        ui->err_tip->clear();
        ui->err_tip->setProperty("state", "empty");
        repolish(ui->err_tip);
        ui->err_tip->show();
        return;
    }

    if(b_ok){
        ui->err_tip->setProperty("state", "normal");
    }else{
        ui->err_tip->setProperty("state", "err");
    }

    ui->err_tip->setText(str);
    ui->err_tip->show();
    repolish(ui->err_tip);
}

void LoginDialog::slot_forget_pwd()
{
    qDebug() << "slot forget pwd";
    emit switchReset();
}

bool LoginDialog::checkUserValid()
{
    auto email = ui->email_edit->text();
    if(email.isEmpty()){
        qDebug() << "email empty";
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱不能为空"));
        return false;
    }

    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool LoginDialog::checkPwdValid()
{
    auto pwd = ui->pass_edit->text();
    if(pwd.length() < 6 || pwd.length() > 15){
        qDebug() << "Pass length invalid";
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
        return false;
    }

    QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
    bool match = regExp.match(pwd).hasMatch();
    if(!match){
        AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符且长度为(6~15)"));
        return false;
    }

    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

bool LoginDialog::enableBtn(bool enabled)
{
    ui->login_btn->setEnabled(enabled);
    ui->reg_btn->setEnabled(enabled);
    return true;
}

void LoginDialog::on_login_btn_clicked()
{
    qDebug() << "login btn clicked";
    if(checkUserValid() == false){
        return;
    }

    if(checkPwdValid() == false){
        return;
    }

    showTip(QString(), true);
    enableBtn(false);
    auto email = ui->email_edit->text();
    auto pwd = ui->pass_edit->text();

    QJsonObject json_obj;
    json_obj["email"] = email;
    json_obj["passwd"] = xorString(pwd);
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/user_login"),
                                        json_obj, ReqId::ID_LOGIN_USER, Modules::LOGINMOD);
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"), false);
        enableBtn(true);
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isNull()){
        showTip(tr("json解析错误"), false);
        enableBtn(true);
        return;
    }

    if(!jsonDoc.isObject()){
        showTip(tr("json解析错误"), false);
        enableBtn(true);
        return;
    }

    if(!_handlers.contains(id)){
        showTip(tr("参数错误"), false);
        enableBtn(true);
        return;
    }

    _handlers[id](jsonDoc.object());
}

void LoginDialog::slot_tcp_con_finish(bool bsuccess)
{
    if(bsuccess){
        showTip(tr("聊天服务连接成功，正在连接资源服务器..."), true);
        emit sig_connect_res_server(_si);
    }else{
        showTip(tr("网络异常"), false);
        enableBtn(true);
    }
}

void LoginDialog::slot_login_failed(int err)
{
    QString result = tr("登录失败, err is %1").arg(err);
    showTip(result, false);
    enableBtn(true);
}

void LoginDialog::slot_res_con_finish(bool bsuccess)
{
    if(bsuccess){
        showTip(tr("聊天服务连接成功，正在登录..."), true);

        QJsonObject jsonObj;
        jsonObj["uid"] = _si->_uid;
        jsonObj["token"] = _si->_token;

        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
        emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_CHAT_LOGIN, jsonData);
    }else{
        showTip(tr("网络异常"), false);
        enableBtn(true);
    }
}

void LoginDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void LoginDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()){
        showTip(QString(), true);
        return;
    }

    showTip(_tip_errs.first(), false);
}

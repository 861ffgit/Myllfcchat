#include "resetdialog.h"
#include "ui_resetdialog.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QRegularExpression>

#include "global.h"
#include "httpmgr.h"

ResetDialog::ResetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResetDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setFixedSize(size());
    const QColor input_background(0xff, 0xfa, 0xf5);
    const QColor input_focus_background(0xff, 0xff, 0xff);
    const QColor input_border(0xf0, 0xd2, 0xc1);
    const QColor input_focus_border(0xe0, 0xa4, 0x7f);

    ui->user_edit->SetBackgroundColor(input_background, input_focus_background);
    ui->email_edit->SetBackgroundColor(input_background, input_focus_background);
    ui->varify_edit->SetBackgroundColor(input_background, input_focus_background);
    ui->pwd_edit->SetBackgroundColor(input_background, input_focus_background);
    ui->user_edit->SetBorderColor(input_border, input_focus_border);
    ui->email_edit->SetBorderColor(input_border, input_focus_border);
    ui->varify_edit->SetBorderColor(input_border, input_focus_border);
    ui->pwd_edit->SetBorderColor(input_border, input_focus_border);
    ui->user_edit->setTextMargins(14, 0, 14, 0);
    ui->email_edit->setTextMargins(14, 0, 14, 0);
    ui->varify_edit->setTextMargins(14, 0, 14, 0);
    ui->pwd_edit->setTextMargins(14, 0, 14, 0);
    ui->varify_btn->SetThemeColors(QColor(0xf0, 0xb5, 0x92),
                                   QColor(0xe3, 0x9b, 0x73),
                                   QColor(0xf6, 0xde, 0xcf));
    ui->pwd_edit->setEchoMode(QLineEdit::Password);
    showTip(QString(), true);

    connect(ui->user_edit, &QLineEdit::editingFinished, this, [this]() { checkUserValid(); });
    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this]() { checkEmailValid(); });
    connect(ui->pwd_edit, &QLineEdit::editingFinished, this, [this]() { checkPassValid(); });
    connect(ui->varify_edit, &QLineEdit::editingFinished, this, [this]() { checkVarifyValid(); });

    initHandlers();
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reset_mod_finish,
            this, &ResetDialog::slot_reset_mod_finish);
}

ResetDialog::~ResetDialog()
{
    delete ui;
}

void ResetDialog::on_return_btn_clicked()
{
    qDebug() << "reset dialog return clicked";
    emit switchLogin();
}

void ResetDialog::on_varify_btn_clicked()
{
    qDebug() << "receive varify btn clicked";
    const auto email = ui->email_edit->text();
    if(!checkEmailValid()) {
        return;
    }

    QJsonObject json_obj;
    json_obj["email"] = email;
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/get_varifycode"),
                                        json_obj, ReqId::ID_GET_VARIFY_CODE, Modules::RESETMOD);
}

void ResetDialog::slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS) {
        showTip(QStringLiteral(u"\u7f51\u7edc\u8bf7\u6c42\u9519\u8bef"), false);
        return;
    }

    const QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isNull() || !jsonDoc.isObject()) {
        showTip(QStringLiteral(u"json\u89e3\u6790\u9519\u8bef"), false);
        return;
    }

    if(!_handlers.contains(id)) {
        showTip(QStringLiteral(u"\u53c2\u6570\u9519\u8bef"), false);
        return;
    }

    _handlers[id](jsonDoc.object());
}

bool ResetDialog::checkUserValid()
{
    if(ui->user_edit->text().isEmpty()) {
        AddTipErr(TipErr::TIP_USER_ERR, QStringLiteral(u"\u7528\u6237\u540d\u4e0d\u80fd\u4e3a\u7a7a"));
        return false;
    }

    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}

bool ResetDialog::checkPassValid()
{
    const auto pass = ui->pwd_edit->text();
    if(pass.length() < 6 || pass.length() > 15) {
        AddTipErr(TipErr::TIP_PWD_ERR, QStringLiteral(u"\u5bc6\u7801\u957f\u5ea6\u5e94\u4e3a6~15"));
        return false;
    }

    const QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
    if(!regExp.match(pass).hasMatch()) {
        AddTipErr(TipErr::TIP_PWD_ERR, QStringLiteral(u"\u4e0d\u80fd\u5305\u542b\u975e\u6cd5\u5b57\u7b26"));
        return false;
    }

    DelTipErr(TipErr::TIP_PWD_ERR);
    return true;
}

bool ResetDialog::checkEmailValid()
{
    const auto email = ui->email_edit->text();
    const QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    if(!regex.match(email).hasMatch()) {
        AddTipErr(TipErr::TIP_EMAIL_ERR, QStringLiteral(u"\u90ae\u7bb1\u5730\u5740\u4e0d\u6b63\u786e"));
        return false;
    }

    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool ResetDialog::checkVarifyValid()
{
    if(ui->varify_edit->text().isEmpty()) {
        AddTipErr(TipErr::TIP_VARIFY_ERR, QStringLiteral(u"\u9a8c\u8bc1\u7801\u4e0d\u80fd\u4e3a\u7a7a"));
        return false;
    }

    DelTipErr(TipErr::TIP_VARIFY_ERR);
    return true;
}

void ResetDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void ResetDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()) {
        showTip(QString(), true);
        return;
    }

    showTip(_tip_errs.first(), false);
}

void ResetDialog::initHandlers()
{
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](const QJsonObject& jsonObj) {
        if(jsonObj["error"].toInt() != ErrorCodes::SUCCESS) {
            showTip(QStringLiteral(u"\u9a8c\u8bc1\u7801\u53d1\u9001\u5931\u8d25"), false);
            return;
        }

        qDebug() << "email is" << jsonObj["email"].toString();
        showTip(QStringLiteral(u"\u9a8c\u8bc1\u7801\u5df2\u7ecf\u53d1\u9001\u5230\u90ae\u7bb1\uff0c\u8bf7\u6ce8\u610f\u67e5\u6536"), true);
    });

    _handlers.insert(ReqId::ID_RESET_PWD, [this](const QJsonObject& jsonObj) {
        if(jsonObj["error"].toInt() != ErrorCodes::SUCCESS) {
            showTip(QStringLiteral(u"\u91cd\u7f6e\u5bc6\u7801\u5931\u8d25"), false);
            return;
        }

        qDebug() << "email is" << jsonObj["email"].toString();
        qDebug() << "user uuid is" << jsonObj["uuid"].toString();
        showTip(QStringLiteral(u"\u5bc6\u7801\u91cd\u7f6e\u6210\u529f\uff0c\u8bf7\u8fd4\u56de\u767b\u5f55"), true);
    });
}

void ResetDialog::showTip(QString str, bool b_ok)
{
    if(str.isEmpty()) {
        ui->err_tip->clear();
        ui->err_tip->setProperty("state", "empty");
        ui->err_tip->show();
        repolish(ui->err_tip);
        return;
    }

    ui->err_tip->setProperty("state", b_ok ? "normal" : "err");
    ui->err_tip->setText(str);
    ui->err_tip->show();
    repolish(ui->err_tip);
}

void ResetDialog::on_sure_btn_clicked()
{
    bool valid = checkUserValid();
    if(!valid) { return; }

    valid = checkEmailValid();
    if(!valid) { return; }

    valid = checkPassValid();
    if(!valid) { return; }

    valid = checkVarifyValid();
    if(!valid) { return; }

    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = xorString(ui->pwd_edit->text());
    json_obj["varifycode"] = ui->varify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/reset_pwd"),
                                        json_obj, ReqId::ID_RESET_PWD, Modules::RESETMOD);
}

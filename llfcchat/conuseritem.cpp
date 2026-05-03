#include <conuseritem.h>
#include <ui_conuseritem.h>

#include <QPainter>
#include <QPainterPath>

namespace {
constexpr char kSelectedProperty[] = {'s','e','l','e','c','t','e','d',0};
constexpr char kHoveredProperty[] = {'h','o','v','e','r','e','d',0};
constexpr int kAccentWidth = 4;
constexpr int kAccentMargin = 10;
constexpr int kCardRadius = 12;
constexpr int kCardInset = 4;
}

ConUserItem::ConUserItem(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ConUserItem),
    _is_selected(false),
    _is_hovered(false)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CONTACT_USER_ITEM);
    setAttribute(Qt::WA_StyledBackground, true);
    setMouseTracking(true);
    ui->horizontalLayout->setContentsMargins(18, 4, 10, 4);
    ui->red_point->raise();
    ShowRedPoint(false);
    UpdateVisualState();
}

ConUserItem::~ConUserItem()
{
    delete ui;
}

QSize ConUserItem::sizeHint() const
{
    return QSize(270, 78);
}

void ConUserItem::SetInfo(std::shared_ptr<AuthInfo> auth_info)
{
    _info = std::make_shared<UserInfo>(auth_info);
    QPixmap pixmap(_info->_icon);
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::SetInfo(int uid, QString name, QString icon)
{
    _info = std::make_shared<UserInfo>(uid, name, icon);
    QPixmap pixmap(_info->_icon);
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::SetInfo(std::shared_ptr<AuthRsp> auth_rsp)
{
    _info = std::make_shared<UserInfo>(auth_rsp);
    QPixmap pixmap(_info->_icon);
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);
    ui->user_name_lb->setText(_info->_name);
}

void ConUserItem::SetVisualSelected(bool selected)
{
    if (_is_selected == selected) {
        return;
    }

    _is_selected = selected;
    UpdateVisualState();
}

void ConUserItem::ShowRedPoint(bool show)
{
    if (show) {
        ui->red_point->show();
        return;
    }

    ui->red_point->hide();
}

std::shared_ptr<UserInfo> ConUserItem::GetInfo()
{
    return _info;
}

void ConUserItem::paintEvent(QPaintEvent *event)
{
    ListItemBase::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF card_rect = rect().adjusted(kCardInset, kCardInset, -kCardInset, -kCardInset);
    QPainterPath card_path;
    card_path.addRoundedRect(card_rect, kCardRadius, kCardRadius);

    if (_is_selected) {
        painter.fillPath(card_path, QColor(224, 242, 230));
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(46, 168, 96));
        painter.drawRoundedRect(QRectF(card_rect.left() + kAccentMargin,
                                       card_rect.top() + 12,
                                       kAccentWidth,
                                       card_rect.height() - 24),
                                2,
                                2);

        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(188, 224, 198), 1));
        painter.drawRoundedRect(card_rect.adjusted(0.5, 0.5, -0.5, -0.5), kCardRadius, kCardRadius);
        return;
    }

    if (_is_hovered) {
        painter.fillPath(card_path, QColor(242, 247, 244));
    }
}

void ConUserItem::enterEvent(QEnterEvent *event)
{
    _is_hovered = true;
    UpdateVisualState();
    ListItemBase::enterEvent(event);
}

void ConUserItem::leaveEvent(QEvent *event)
{
    _is_hovered = false;
    UpdateVisualState();
    ListItemBase::leaveEvent(event);
}

void ConUserItem::UpdateVisualState()
{
    setProperty(kSelectedProperty, _is_selected);
    setProperty(kHoveredProperty, _is_hovered);
    repolish(this);
    update();
}

#include <statewidget.h>

#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>

namespace {
constexpr char kSelectedPrefix[] = {'s','e','l','e','c','t','e','d',0};
constexpr char kHoverSuffix[] = {'h','o','v','e','r',0};
constexpr char kHoverState[] = {'h','o','v','e','r',0};
constexpr char kPressedState[] = {'p','r','e','s','s','e','d',0};
constexpr char kStateProperty[] = {'s','t','a','t','e',0};
constexpr char kRedPointName[] = {'r','e','d','_','p','o','i','n','t',0};
constexpr char kSideChatLb[] = {'s','i','d','e','_','c','h','a','t','_','l','b',0};
constexpr char kSideContactLb[] = {'s','i','d','e','_','c','o','n','t','a','c','t','_','l','b',0};
constexpr char kSideSettingsLb[] = {'s','i','d','e','_','s','e','t','t','i','n','g','s','_','l','b',0};
constexpr char kChatIcon[] = {':','/','r','e','s','/','c','h','a','t','_','i','c','o','n','.','p','n','g',0};
constexpr char kChatIconHover[] = {':','/','r','e','s','/','c','h','a','t','_','i','c','o','n','_','h','o','v','e','r','.','p','n','g',0};
constexpr char kChatIconSelected[] = {':','/','r','e','s','/','c','h','a','t','_','i','c','o','n','_','p','r','e','s','s','.','p','n','g',0};
constexpr char kContactIcon[] = {':','/','r','e','s','/','c','o','n','t','a','c','t','_','l','i','s','t','.','p','n','g',0};
constexpr char kContactIconHover[] = {':','/','r','e','s','/','c','o','n','t','a','c','t','_','l','i','s','t','_','h','o','v','e','r','.','p','n','g',0};
constexpr char kContactIconSelected[] = {':','/','r','e','s','/','c','o','n','t','a','c','t','_','l','i','s','t','_','p','r','e','s','s','.','p','n','g',0};
constexpr char kSettingsIcon[] = {':','/','r','e','s','/','s','e','t','t','i','n','g','s','.','p','n','g',0};
constexpr char kSettingsIconHover[] = {':','/','r','e','s','/','s','e','t','t','i','n','g','s','_','h','o','v','e','r','.','p','n','g',0};
constexpr char kSettingsIconSelected[] = {':','/','r','e','s','/','s','e','t','t','i','n','g','s','_','p','r','e','s','s','.','p','n','g',0};
constexpr qreal kFrameInset = 1.0;
constexpr qreal kFrameRadius = 10.0;

QString sideBarIconPath(const QString &object_name, const QString &state_name)
{
    const bool is_selected = state_name.startsWith(QString::fromLatin1(kSelectedPrefix));
    const bool is_hover = state_name.endsWith(QString::fromLatin1(kHoverSuffix));

    if (object_name == QString::fromLatin1(kSideChatLb)) {
        if (is_selected) {
            return QString::fromLatin1(kChatIconSelected);
        }
        if (is_hover) {
            return QString::fromLatin1(kChatIconHover);
        }
        return QString::fromLatin1(kChatIcon);
    }

    if (object_name == QString::fromLatin1(kSideContactLb)) {
        if (is_selected) {
            return QString::fromLatin1(kContactIconSelected);
        }
        if (is_hover) {
            return QString::fromLatin1(kContactIconHover);
        }
        return QString::fromLatin1(kContactIcon);
    }

    if (object_name == QString::fromLatin1(kSideSettingsLb)) {
        if (is_selected) {
            return QString::fromLatin1(kSettingsIconSelected);
        }
        if (is_hover) {
            return QString::fromLatin1(kSettingsIconHover);
        }
        return QString::fromLatin1(kSettingsIcon);
    }

    return QString();
}

bool isSelectedSideBarState(const QString &state_name)
{
    return state_name.startsWith(QString::fromLatin1(kSelectedPrefix));
}

bool isActiveSideBarState(const QString &state_name)
{
    return state_name == QString::fromLatin1(kHoverState)
        || state_name == QString::fromLatin1(kPressedState);
}
}

StateWidget::StateWidget(QWidget *parent)
    : QWidget(parent)
    , _curstate(ClickLbState::Normal)
    , _red_point(nullptr)
    , _icon_size(15, 15)
{
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_StyledBackground, true);
    AddRedPoint();
}

void StateWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QString state_name = property(kStateProperty).toString();
    const QRectF background_rect = rect().adjusted(kFrameInset, kFrameInset,
                                                   -kFrameInset, -kFrameInset);

    if (isSelectedSideBarState(state_name)) {
        painter.setPen(QPen(QColor(63, 107, 89, 217), 1));
        painter.setBrush(QColor(33, 65, 53));
        painter.drawRoundedRect(background_rect, kFrameRadius, kFrameRadius);
    } else if (isActiveSideBarState(state_name)) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(196, 228, 208, 110));
        painter.drawRoundedRect(background_rect, kFrameRadius, kFrameRadius);
    }

    const QString icon_path = CurrentIconPath();
    if (icon_path.isEmpty()) {
        return;
    }

    QPixmap pixmap(icon_path);
    if (pixmap.isNull()) {
        return;
    }

    QSize draw_size = pixmap.size();
    draw_size.scale(_icon_size, Qt::KeepAspectRatio);

    QRect target_rect(QPoint(0, 0), draw_size);
    target_rect.moveCenter(rect().center());
    painter.drawPixmap(target_rect, pixmap);
}

void StateWidget::resizeEvent(QResizeEvent *event)
{
    UpdateRedPointPosition();
    QWidget::resizeEvent(event);
}

void StateWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (_curstate == ClickLbState::Selected) {
            QWidget::mousePressEvent(event);
            return;
        }

        if (_curstate == ClickLbState::Normal) {
            _curstate = ClickLbState::Selected;
            setProperty(kStateProperty, _selected_press);
            repolish(this);
            update();
        }

        return;
    }

    QWidget::mousePressEvent(event);
}

void StateWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (_curstate == ClickLbState::Normal) {
            setProperty(kStateProperty, _normal_hover);
            repolish(this);
            update();
        } else {
            setProperty(kStateProperty, _selected_hover);
            repolish(this);
            update();
        }
        emit clicked();
        return;
    }

    QWidget::mousePressEvent(event);
}

void StateWidget::enterEvent(QEnterEvent *event)
{
    if (_curstate == ClickLbState::Normal) {
        setProperty(kStateProperty, _normal_hover);
        repolish(this);
        update();
    } else {
        setProperty(kStateProperty, _selected_hover);
        repolish(this);
        update();
    }

    QWidget::enterEvent(event);
}

void StateWidget::leaveEvent(QEvent *event)
{
    if (_curstate == ClickLbState::Normal) {
        setProperty(kStateProperty, _normal);
        repolish(this);
        update();
    } else {
        setProperty(kStateProperty, _selected);
        repolish(this);
        update();
    }

    QWidget::leaveEvent(event);
}

void StateWidget::SetState(QString normal, QString hover, QString press,
                           QString select, QString select_hover, QString select_press)
{
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;

    _selected = select;
    _selected_hover = select_hover;
    _selected_press = select_press;

    setProperty(kStateProperty, normal);
    repolish(this);
}

void StateWidget::SetIconSet(QString normal, QString hover, QString press,
                             QString select, QString select_hover, QString select_press)
{
    _normal_icon = normal;
    _normal_hover_icon = hover;
    _normal_press_icon = press;

    _selected_icon = select;
    _selected_hover_icon = select_hover;
    _selected_press_icon = select_press;

    update();
}

void StateWidget::SetIconSize(const QSize &size)
{
    if (!size.isValid()) {
        return;
    }

    _icon_size = size;
    update();
}

ClickLbState StateWidget::GetCurState()
{
    return _curstate;
}

void StateWidget::ClearState()
{
    _curstate = ClickLbState::Normal;
    setProperty(kStateProperty, _normal);
    repolish(this);
    update();
}

void StateWidget::SetSelected(bool bselected)
{
    if (bselected) {
        _curstate = ClickLbState::Selected;
        setProperty(kStateProperty, _selected);
        repolish(this);
        update();
        return;
    }

    _curstate = ClickLbState::Normal;
    setProperty(kStateProperty, _normal);
    repolish(this);
    update();
}

void StateWidget::AddRedPoint()
{
    if (_red_point) {
        return;
    }

    _red_point = new QLabel(this);
    _red_point->setObjectName(QString::fromLatin1(kRedPointName));
    _red_point->setFixedSize(10, 10);
    _red_point->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    _red_point->setVisible(false);
    UpdateRedPointPosition();
}

void StateWidget::ShowRedPoint(bool show)
{
    if (_red_point) {
        _red_point->setVisible(show);
    }
}

QString StateWidget::CurrentIconPath() const
{
    const QString state_name = property(kStateProperty).toString();

    if (_normal_icon.isEmpty()) {
        return sideBarIconPath(objectName(), state_name);
    }

    if (state_name == _selected_press && !_selected_press_icon.isEmpty()) {
        return _selected_press_icon;
    }

    if (state_name == _selected_hover && !_selected_hover_icon.isEmpty()) {
        return _selected_hover_icon;
    }

    if (state_name == _selected && !_selected_icon.isEmpty()) {
        return _selected_icon;
    }

    if (state_name == _normal_press && !_normal_press_icon.isEmpty()) {
        return _normal_press_icon;
    }

    if (state_name == _normal_hover && !_normal_hover_icon.isEmpty()) {
        return _normal_hover_icon;
    }

    if (state_name == _normal && !_normal_icon.isEmpty()) {
        return _normal_icon;
    }

    return _normal_icon;
}

void StateWidget::UpdateRedPointPosition()
{
    if (!_red_point) {
        return;
    }

    const int right_margin = 3;
    const int top_margin = 3;
    _red_point->move(width() - _red_point->width() - right_margin, top_margin);
    _red_point->raise();
}

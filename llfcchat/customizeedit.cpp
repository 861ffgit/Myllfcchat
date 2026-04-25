#include <customizeedit.h>

#include <QFocusEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>

CustomizeEdit::CustomizeEdit(QWidget *parent):QLineEdit(parent),_max_len(0)
{
    _border_color = QColor(0xd8, 0xde, 0xe5);
    _focus_border_color = QColor(0xd2, 0xd9, 0xe0);
    _background_color = QColor(0xff, 0xff, 0xff);
    _focus_background_color = QColor(0xff, 0xff, 0xff);

    setFrame(false);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_StyledBackground, false);
    setStyleSheet("background: transparent; border: none;");
    setTextMargins(14, 1, 14, 1);

    QPalette pal = palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    pal.setColor(QPalette::Window, Qt::transparent);
    setPalette(pal);

    connect(this, &QLineEdit::textChanged, this, &CustomizeEdit::limitTextLength);
}

void CustomizeEdit::SetMaxLength(int maxLen)
{
    _max_len = maxLen;
}

void CustomizeEdit::SetBackgroundColor(const QColor &backgroundColor,
                                       const QColor &focusBackgroundColor)
{
    _background_color = backgroundColor;
    _focus_background_color = focusBackgroundColor;
    setStyleSheet("background: transparent; border: none;");
    update();
}

void CustomizeEdit::SetBorderColor(const QColor &borderColor,
                                   const QColor &focusBorderColor)
{
    _border_color = borderColor;
    _focus_border_color = focusBorderColor;
    update();
}

void CustomizeEdit::paintEvent(QPaintEvent *event)
{
    QRectF outer_rect = rect().adjusted(2.0, 2.0, -2.0, -2.0);
    QRectF inner_rect = outer_rect.adjusted(1.2, 1.2, -1.2, -1.2);

    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.setBrush(hasFocus() ? _focus_background_color : _background_color);
        painter.drawRoundedRect(outer_rect, 15, 15);
    }

    QLineEdit::paintEvent(event);

    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);

        QPainterPath outer_path;
        QPainterPath inner_path;
        outer_path.addRoundedRect(outer_rect, 15, 15);
        inner_path.addRoundedRect(inner_rect, 13.5, 13.5);
        painter.fillPath(outer_path.subtracted(inner_path),
                         hasFocus() ? _focus_border_color : _border_color);
    }
}

void CustomizeEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
    emit sig_foucus_out();
}

void CustomizeEdit::limitTextLength(QString text)
{
    if(_max_len <= 0){
        return;
    }

    QByteArray byteArray = text.toUtf8();

    if (byteArray.size() > _max_len) {
        byteArray = byteArray.left(_max_len);
        this->setText(QString::fromUtf8(byteArray));
    }
}

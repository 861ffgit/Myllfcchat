#include "chatnamelabel.h"

#include <QFontMetrics>
#include <QPainter>

namespace {
constexpr int kNameTextRaise = 3;
}

ChatNameLabel::ChatNameLabel(QWidget *parent)
    : QLabel(parent)
{
}

void ChatNameLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setFont(font());
    painter.setPen(palette().color(foregroundRole()));

    const QRect rect = contentsRect();
    const QFontMetrics metrics(font());
    const QString displayText = metrics.elidedText(text(), Qt::ElideRight, rect.width());
    const int textWidth = metrics.horizontalAdvance(displayText);

    int textLeft = rect.left();
    if(alignment() & Qt::AlignHCenter)
        textLeft = rect.left() + (rect.width() - textWidth) / 2;
    else if(alignment() & Qt::AlignRight)
        textLeft = rect.right() - textWidth + 1;

    const int baseline = rect.top() + metrics.ascent() - kNameTextRaise;
    painter.drawText(textLeft, baseline, displayText);
}

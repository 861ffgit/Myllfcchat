#include "bubbleframe.h"
#include <QPainter>
#include <QPainterPath>

namespace {
constexpr int kTailWidth = 8;
constexpr qreal kBubbleRadius = 10.0;
constexpr int kTailTop = 18;
constexpr int kTailHalfHeight = 4;

QPainterPath buildBubblePath(const QRectF &body, bool isSelf)
{
    QPainterPath bodyPath;
    bodyPath.addRoundedRect(body, kBubbleRadius, kBubbleRadius);

    const qreal seamX = isSelf ? body.right() - 0.5 : body.left() + 0.5;
    const qreal tipX = isSelf ? body.right() + kTailWidth : body.left() - kTailWidth;

    QPainterPath tailPath;
    tailPath.addPolygon(QPolygonF({
        QPointF(seamX, kTailTop - kTailHalfHeight),
        QPointF(seamX, kTailTop + kTailHalfHeight),
        QPointF(tipX, kTailTop)
    }));

    return bodyPath.united(tailPath);
}
}

BubbleFrame::BubbleFrame(ChatRole role, QWidget *parent) :QFrame(parent)
    ,m_role(role)
    ,m_margin(8)
{
    setAttribute(Qt::WA_TranslucentBackground);
    m_pHLayout = new QHBoxLayout();
    m_pHLayout->setSpacing(0);
    if(m_role == ChatRole::Self)
        m_pHLayout->setContentsMargins(m_margin, m_margin, kTailWidth + m_margin, m_margin);
    else
        m_pHLayout->setContentsMargins(kTailWidth + m_margin, m_margin, m_margin, m_margin);

    this->setLayout(m_pHLayout);
}

void BubbleFrame::setMargin(int margin)
{
    m_margin = qMax(0, margin);
    if (m_role == ChatRole::Self) {
        m_pHLayout->setContentsMargins(m_margin, m_margin, kTailWidth + m_margin, m_margin);
    } else {
        m_pHLayout->setContentsMargins(kTailWidth + m_margin, m_margin, m_margin, m_margin);
    }
    updateGeometry();
}

void BubbleFrame::setWidget(QWidget *w)
{
    if(m_pHLayout->count() > 0)
        return ;
    else{
        m_pHLayout->addWidget(w);
    }
}

void BubbleFrame::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const bool isSelf = (m_role == ChatRole::Self);
    const QColor bubbleColor = isSelf ? QColor(149, 236, 105) : QColor(255, 255, 255);
    const QColor borderColor = isSelf ? QColor(133, 217, 94) : QColor(228, 228, 228);

    QPen borderPen(borderColor, 1);
    borderPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(borderPen);
    painter.setBrush(bubbleColor);

    const QRectF body = isSelf
        ? QRectF(1, 1, this->width() - kTailWidth - 1, this->height() - 2)
        : QRectF(kTailWidth, 1, this->width() - kTailWidth - 1, this->height() - 2);

    painter.drawPath(buildBubblePath(body, isSelf));

    QFrame::paintEvent(e);
}

#include "ClickableLabel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
    , m_showOverlay(false)
    , m_hovered(false)
    , m_cornerFillColor(Qt::transparent)
{
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    setAutoFillBackground(false);
}

void ClickableLabel::setCornerFillColor(const QColor& color)
{
    m_cornerFillColor = color;
    update();
}

void ClickableLabel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
}

void ClickableLabel::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    update();
    QLabel::enterEvent(event);
}

void ClickableLabel::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QLabel::leaveEvent(event);
}

void ClickableLabel::paintEvent(QPaintEvent* event)
{
    const QPixmap labelPixmap = QLabel::pixmap(Qt::ReturnByValue);
    const QRect drawRect = rect().adjusted(0, 0, -1, -1);
    QPainterPath clipPath;
    clipPath.addRoundedRect(drawRect, 10, 10);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QColor baseColor = m_cornerFillColor.alpha() > 0
        ? m_cornerFillColor
        : palette().window().color();
    painter.fillRect(rect(), baseColor);

    if (!labelPixmap.isNull()) {
        painter.save();
        painter.setClipPath(clipPath);
        painter.drawPixmap(rect(), labelPixmap);
        painter.restore();
    } else {
        QLabel::paintEvent(event);
    }

    if (m_showOverlay && !m_overlayIcon.isNull()) {
        painter.save();
        painter.setClipPath(clipPath);
        if (m_hovered) {
            painter.fillPath(clipPath, QColor(0, 0, 0, 96));
        } else {
            painter.fillPath(clipPath, QColor(0, 0, 0, 56));
        }

        int iconSize = qMin(width(), height()) / 3;
        QRect iconRect(
            (width() - iconSize) / 2,
            (height() - iconSize) / 2,
            iconSize,
            iconSize
        );

        m_overlayIcon.paint(&painter, iconRect);
        painter.restore();
    }
}

void ClickableLabel::setIconOverlay(const QIcon& icon)
{
    m_overlayIcon = icon;
    update();
}

void ClickableLabel::showIconOverlay(bool show)
{
    m_showOverlay = show;
    update();
}

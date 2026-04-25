#include "roundedimagelabel.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

RoundedImageLabel::RoundedImageLabel(QWidget *parent)
    : QLabel(parent)
    , m_cornerRadius(8)
{
}

int RoundedImageLabel::cornerRadius() const
{
    return m_cornerRadius;
}

void RoundedImageLabel::setCornerRadius(int radius)
{
    m_cornerRadius = radius;
    update();
}

void RoundedImageLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRect drawRect = rect();
    QPainterPath clipPath;
    clipPath.addRoundedRect(drawRect, m_cornerRadius, m_cornerRadius);
    painter.setClipPath(clipPath);

    const QPixmap pixmapValue = pixmap(Qt::ReturnByValue);
    if (!pixmapValue.isNull()) {
        QPixmap scaled = pixmapValue.scaled(drawRect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const int offsetX = (scaled.width() - drawRect.width()) / 2;
        const int offsetY = (scaled.height() - drawRect.height()) / 2;
        painter.drawPixmap(drawRect, scaled, QRect(offsetX, offsetY, drawRect.width(), drawRect.height()));
        return;
    }

    painter.fillPath(clipPath, palette().window());
}

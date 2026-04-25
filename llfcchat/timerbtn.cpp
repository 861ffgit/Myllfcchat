#include "timerbtn.h"
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>

namespace {
QString getCodeText()
{
    return QString(QChar(0x83B7)) + QChar(0x53D6);
}
}

TimerBtn::TimerBtn(QWidget *parent):QPushButton(parent),_counter(10)
{
    _start_color = QColor(0x8e, 0xc8, 0xa0);
    _end_color = QColor(0x6f, 0xaf, 0x84);
    _disabled_color = QColor(0xc7, 0xe1, 0xcf);

    setAutoFillBackground(false);
    setAttribute(Qt::WA_StyledBackground, false);

    _timer = new QTimer(this);

    connect(_timer, &QTimer::timeout, [this](){
        _counter--;
        if(_counter <= 0){
            _timer->stop();
            _counter = 10;
            this->setText(getCodeText());
            this->setEnabled(true);
            return;
        }
        this->setText(QString::number(_counter));
    });
}

TimerBtn::~TimerBtn()
{
    _timer->stop();
}

void TimerBtn::SetThemeColors(const QColor &startColor,
                              const QColor &endColor,
                              const QColor &disabledColor)
{
    _start_color = startColor;
    _end_color = endColor;
    _disabled_color = disabledColor;
    update();
}

void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        this->setEnabled(false);
        this->setText(QString::number(_counter));
        _timer->start(1000);
        emit clicked();
    }
    QPushButton::mouseReleaseEvent(e);
}

void TimerBtn::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF draw_rect = rect().adjusted(3.0, 3.0, -3.0, -3.0);
    QLinearGradient gradient(draw_rect.topLeft(), draw_rect.topRight());
    gradient.setColorAt(0.0, _start_color);
    gradient.setColorAt(1.0, _end_color);

    painter.setPen(Qt::NoPen);
    painter.setBrush(isEnabled() ? QBrush(gradient) : QBrush(_disabled_color));
    painter.drawRoundedRect(draw_rect, 13, 13);

    QColor text_color = isEnabled() ? QColor(0xff, 0xff, 0xff) : QColor(0xff, 0xff, 0xff, 199);
    painter.setPen(text_color);
    painter.drawText(rect(), Qt::AlignCenter, text());
}

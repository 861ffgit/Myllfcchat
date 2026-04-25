#ifndef TIMERBTN_H
#define TIMERBTN_H

#include <QColor>
#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton
{
public:
    TimerBtn(QWidget *parent = nullptr);
    ~TimerBtn();
    void SetThemeColors(const QColor &startColor, const QColor &endColor, const QColor &disabledColor);

    void mouseReleaseEvent(QMouseEvent *e) override;
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QTimer *_timer;
    int _counter;
    QColor _start_color;
    QColor _end_color;
    QColor _disabled_color;
};

#endif // TIMERBTN_H

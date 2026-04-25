#pragma once
#include <QLabel>
#include <QWidget>
#include <QIcon>
#include <QColor>
#include <QObject>

class ClickableLabel :
                       public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent = nullptr);
    void setIconOverlay(const QIcon& icon);
    void showIconOverlay(bool show);
    void setCornerFillColor(const QColor& color);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

signals:
    void clicked();

private:
    QIcon m_overlayIcon;
    bool m_showOverlay;
    bool m_hovered;
    QColor m_cornerFillColor;
};

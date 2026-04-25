#ifndef CUSTOMIZEEDIT_H
#define CUSTOMIZEEDIT_H

#include <QColor>
#include <QLineEdit>

class CustomizeEdit: public QLineEdit
{
    Q_OBJECT
public:
    CustomizeEdit(QWidget *parent = nullptr);
    void SetMaxLength(int maxLen);
    void SetBackgroundColor(const QColor &backgroundColor, const QColor &focusBackgroundColor);
    void SetBorderColor(const QColor &borderColor, const QColor &focusBorderColor);
protected:
    void paintEvent(QPaintEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
private:
    void limitTextLength(QString text);

    int _max_len;
    QColor _border_color;
    QColor _focus_border_color;
    QColor _background_color;
    QColor _focus_background_color;
signals:
    void sig_foucus_out();
};

#endif // CUSTOMIZEEDIT_H

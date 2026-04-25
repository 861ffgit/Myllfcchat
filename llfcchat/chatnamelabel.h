#ifndef CHATNAMELABEL_H
#define CHATNAMELABEL_H

#include <QLabel>

class ChatNameLabel : public QLabel
{
public:
    explicit ChatNameLabel(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // CHATNAMELABEL_H

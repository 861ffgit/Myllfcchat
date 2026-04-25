#ifndef ROUNDEDIMAGELABEL_H
#define ROUNDEDIMAGELABEL_H

#include <QLabel>

class RoundedImageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit RoundedImageLabel(QWidget *parent = nullptr);

    int cornerRadius() const;
    void setCornerRadius(int radius);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_cornerRadius;
};

#endif // ROUNDEDIMAGELABEL_H

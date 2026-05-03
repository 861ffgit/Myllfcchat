#ifndef CONUSERITEM_H
#define CONUSERITEM_H

#include <QWidget>
#include "listitembase.h"
#include "userdata.h"

namespace Ui {
class ConUserItem;
}

class ConUserItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit ConUserItem(QWidget *parent = nullptr);
    ~ConUserItem();
    QSize sizeHint() const override;
    void SetInfo(std::shared_ptr<AuthInfo> auth_info);
    void SetInfo(std::shared_ptr<AuthRsp> auth_rsp);
    void SetInfo(int uid, QString name, QString icon);
    void SetVisualSelected(bool selected);
    void ShowRedPoint(bool show = false);
    std::shared_ptr<UserInfo> GetInfo();

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void UpdateVisualState();

    Ui::ConUserItem *ui;
    std::shared_ptr<UserInfo> _info;
    bool _is_selected;
    bool _is_hovered;
};

#endif // CONUSERITEM_H

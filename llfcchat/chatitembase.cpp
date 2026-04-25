#include "ChatItemBase.h"
#include "chatnamelabel.h"
#include <QFont>
#include <QFontMetrics>
#include "BubbleFrame.h"

namespace {
constexpr int kNameLabelExtraHeight = 1;
constexpr int kMessageVerticalSpacing = 0;
}

ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent)
    : QWidget(parent)
    , m_role(role)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_pNameLabel    = new ChatNameLabel();
    m_pNameLabel->setObjectName("chat_user_name");
    QFont font("Microsoft YaHei");
    font.setPointSize(9);
    m_pNameLabel->setFont(font);
    const int nameLabelHeight = QFontMetrics(font).height() + kNameLabelExtraHeight;
    m_pNameLabel->setFixedHeight(nameLabelHeight);

    m_pIconLabel    = new QLabel();
    m_pIconLabel->setScaledContents(true);
    m_pIconLabel->setFixedSize(40, 40);
    m_pIconLabel->setAlignment(Qt::AlignCenter);

    m_pBubble       = new QWidget();
    m_pBubble->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QGridLayout *pGLayout = new QGridLayout();
    pGLayout->setVerticalSpacing(kMessageVerticalSpacing);
    pGLayout->setHorizontalSpacing(8);
    pGLayout->setContentsMargins(12, 8, 12, 8);
    QSpacerItem*pSpacer = new QSpacerItem(48, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_pStatusLabel = new QLabel();
    m_pStatusLabel->setFixedSize(14,14);
    m_pStatusLabel->setScaledContents(true);

    if(m_role == ChatRole::Self)
    {
        m_pNameLabel->setContentsMargins(0,0,6,0);
        m_pNameLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
        pGLayout->addWidget(m_pNameLabel, 0,2, 1,1, Qt::AlignTop);
        pGLayout->addWidget(m_pIconLabel, 0, 3, 2,1, Qt::AlignTop);
        pGLayout->setRowMinimumHeight(0, nameLabelHeight);
        pGLayout->addItem(pSpacer, 1, 0, 1, 1);
        pGLayout->addWidget(m_pStatusLabel,1,1,1,1,Qt::AlignCenter);
        pGLayout->addWidget(m_pBubble, 1,2, 1,1, Qt::AlignTop);
        pGLayout->setColumnStretch(0, 3);
        pGLayout->setColumnStretch(1,0);
        pGLayout->setColumnStretch(2,4);
        pGLayout->setColumnStretch(3,0);
    }else{
        m_pNameLabel->setContentsMargins(6,0,0,0);
        m_pNameLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        pGLayout->addWidget(m_pIconLabel, 0, 0, 2,1, Qt::AlignTop);
        pGLayout->setRowMinimumHeight(0, nameLabelHeight);
        pGLayout->addWidget(m_pNameLabel, 0,1, 1,1, Qt::AlignTop);
        pGLayout->addWidget(m_pBubble, 1,1, 1,1, Qt::AlignTop);
        pGLayout->addItem(pSpacer, 2, 2, 1, 1);
        pGLayout->setColumnStretch(1, 4);
        pGLayout->setColumnStretch(2, 3);
    }
    this->setLayout(pGLayout);
}

void ChatItemBase::setUserName(const QString &name)
{
    m_pNameLabel->setText(name);
}

void ChatItemBase::setUserIcon(const QPixmap &icon)
{
    m_pIconLabel->setPixmap(icon);
}

void ChatItemBase::setWidget(QWidget *w)
{
    QGridLayout *pGLayout = (qobject_cast<QGridLayout *>)(this->layout());
    pGLayout->replaceWidget(m_pBubble, w);
    delete m_pBubble;
    m_pBubble = w;
}

void ChatItemBase::setStatus(int status)
{
    if(status == MsgStatus::UN_READ){
        m_pStatusLabel->setPixmap(QPixmap(":/res/unread.png"));
        return ;
    }

    if(status == MsgStatus::SEND_FAILED){
        m_pStatusLabel->setPixmap(QPixmap(":/res/send_fail.png"));
        return ;
    }

    if(status == MsgStatus::READED){
        m_pStatusLabel->setPixmap(QPixmap(":/res/readed.png"));
        return ;
    }

    m_pStatusLabel->clear();
}

QLabel* ChatItemBase::getIconLabel() {
    return m_pIconLabel;
}

QWidget* ChatItemBase::getBubble() {
    return m_pBubble;
}

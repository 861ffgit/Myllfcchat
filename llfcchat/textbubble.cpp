#include "textbubble.h"
#include <QAbstractTextDocumentLayout>
#include <QFontMetricsF>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextOption>
#include <QtMath>

TextBubble::TextBubble(ChatRole role, const QString &text, QWidget *parent)
    :BubbleFrame(role, parent)
{
    m_pTextEdit = new QTextEdit();
    m_pTextEdit->setReadOnly(true);
    m_pTextEdit->setFrameStyle(QFrame::NoFrame);
    m_pTextEdit->setAttribute(Qt::WA_TranslucentBackground);
    m_pTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    m_pTextEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_pTextEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pTextEdit->installEventFilter(this);
    QFont font("Microsoft YaHei");
    font.setPointSize(10);
    m_pTextEdit->setFont(font);
    m_pTextEdit->document()->setDocumentMargin(0);
    setPlainText(text);
    setWidget(m_pTextEdit);
    initStyleSheet();
    adjustTextHeight();
}

bool TextBubble::eventFilter(QObject *o, QEvent *e)
{
    if(m_pTextEdit == o && (e->type() == QEvent::Paint || e->type() == QEvent::Resize))
    {
        adjustTextHeight();
    }
    return BubbleFrame::eventFilter(o, e);
}

void TextBubble::setPlainText(const QString &text)
{
    constexpr int kMaxBubbleTextWidth = 380;
    constexpr int kMinBubbleTextWidth = 1;

    m_pTextEdit->setPlainText(text);

    QTextDocument *doc = m_pTextEdit->document();
    const QMargins margins = this->layout()->contentsMargins();
    doc->setTextWidth(-1);

    int targetWidth = qCeil(doc->idealWidth()) + 1;
    targetWidth = qBound(kMinBubbleTextWidth, targetWidth, kMaxBubbleTextWidth);

    m_pTextEdit->setFixedWidth(targetWidth);
    this->setFixedWidth(targetWidth + margins.left() + margins.right());
    adjustTextHeight();
}

void TextBubble::adjustTextHeight()
{
    QTextDocument *doc = m_pTextEdit->document();
    const QMargins margins = this->layout()->contentsMargins();
    doc->setTextWidth(m_pTextEdit->viewport()->width());

    const int textHeight = qCeil(doc->documentLayout()->documentSize().height());
    m_pTextEdit->setFixedHeight(textHeight + 1);
    setFixedHeight(m_pTextEdit->height() + margins.top() + margins.bottom());
}

void TextBubble::initStyleSheet()
{
    m_pTextEdit->setStyleSheet(
        "QTextEdit{"
        "background:transparent;"
        "border:none;"
        "color:#1f2326;"
        "padding:0px;"
        "selection-background-color:#c8efc2;"
        "}"
    );
}

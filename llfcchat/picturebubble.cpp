#include "PictureBubble.h"
#include <QLabel>
#include <QStyle>
#include <QTimer>

#define PIC_MAX_WIDTH 160
#define PIC_MAX_HEIGHT 90

PictureBubble::PictureBubble(const QPixmap &picture, ChatRole role, int total, QWidget *parent)
    :BubbleFrame(role, parent), m_state(TransferState::None), m_total_size(total)
{
    setMargin(8);

    m_pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);
    m_playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    m_downloadIcon = style()->standardIcon(QStyle::SP_ArrowDown);

    QWidget* container = new QWidget();
    container->setAttribute(Qt::WA_TranslucentBackground);
    container->setAttribute(Qt::WA_StyledBackground, false);
    container->setAutoFillBackground(false);
    m_vLayout = new QVBoxLayout(container);
    m_vLayout->setContentsMargins(0, 0, 0, 0);
    m_vLayout->setSpacing(6);

    m_picLabel = new ClickableLabel();
    m_picLabel->setScaledContents(true);
    m_picLabel->setCornerFillColor(role == ChatRole::Self ? QColor(149, 236, 105)
                                                     : QColor(255, 255, 255));
    QPixmap pix = picture.scaled(QSize(PIC_MAX_WIDTH, PIC_MAX_HEIGHT),
                                 Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_pixmapSize = pix.size();
    m_picLabel->setPixmap(pix);
    m_picLabel->setFixedSize(pix.size());

    connect(m_picLabel, &ClickableLabel::clicked,
            this, &PictureBubble::onPictureClicked);

    m_progressBar = new QProgressBar();
    m_progressBar->setFixedWidth(pix.width());
    m_progressBar->setFixedHeight(4);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    setState(TransferState::None);

    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "   border: none;"
        "   border-radius: 2px;"
        "   background-color: rgba(255, 255, 255, 0.45);"
        "}"
        "QProgressBar::chunk {"
        "   background-color: #07c160;"
        "   border-radius: 2px;"
        "}"
        );

    m_vLayout->addWidget(m_picLabel);
    m_vLayout->addWidget(m_progressBar);
    this->setWidget(container);
    adjustSize();
}

void PictureBubble::adjustSize()
{
    const QMargins margins = this->layout()->contentsMargins();
    int width = m_pixmapSize.width() + margins.left() + margins.right();
    int height = m_pixmapSize.height() + margins.top() + margins.bottom();

    if (!m_progressBar->isHidden()) {
        height += m_progressBar->height() + m_vLayout->spacing();
    }

    setFixedSize(width, height);
}

void PictureBubble::setProgress(int value, int total_value)
{
    if (m_total_size != total_value) {
        m_total_size = total_value;
    }
    float percent = (value / (m_total_size*1.0f))*100.0f;
    m_progressBar->setValue(int(percent));
    if (percent >= 100) {
        setState(TransferState::Completed);
    }
}

void PictureBubble::showProgress(bool show)
{
    if (show) {
        m_progressBar->show();
    }
    else {
        m_progressBar->hide();
    }

    adjustSize();
}

void PictureBubble::resumeState() {
    if (_msg_info->_transfer_type == TransferType::Download) {
        _msg_info->_transfer_state = TransferState::Downloading;
        m_state = TransferState::Downloading;
        updateIconOverlay();
        return;
    }

    if (_msg_info->_transfer_type == TransferType::Upload) {
        _msg_info->_transfer_state = TransferState::Uploading;
        m_state = TransferState::Uploading;
        updateIconOverlay();
        return;
    }
}

void PictureBubble::setState(TransferState state)
{
    m_state = state;
    if (_msg_info) {
        _msg_info->_transfer_state = state;
    }

    switch (state) {
    case TransferState::Downloading:
    case TransferState::Uploading:
    case TransferState::Paused:
        showProgress(true);
        break;
    case TransferState::Completed:
        QTimer::singleShot(1000, this, [this]() {
            showProgress(false);
        });
        break;
    case TransferState::None:
    case TransferState::Failed:
        showProgress(false);
        break;
    }

    updateIconOverlay();
}

void PictureBubble::setMsgInfo(std::shared_ptr<MsgInfo> msg)
{
    _msg_info = msg;
    if (_msg_info->_transfer_state == TransferState::Uploading) {
        setState(TransferState::Uploading);
        return;
    }

    if (_msg_info->_transfer_state == TransferState::Downloading) {
        setState(TransferState::Downloading);
        return;
    }
}

void PictureBubble::setDownloadFinish(std::shared_ptr<MsgInfo> msg,QString file_path) {
    Q_UNUSED(msg);
    m_progressBar->setValue(100);
    setState(TransferState::Completed);
    auto picture = QPixmap(file_path);
    QPixmap pix = picture.scaled(QSize(PIC_MAX_WIDTH, PIC_MAX_HEIGHT),
                                 Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_pixmapSize = pix.size();
    m_picLabel->setPixmap(pix);
    m_picLabel->setFixedSize(pix.size());
    adjustSize();
    updateIconOverlay();
}

void PictureBubble::onPictureClicked()
{
    switch (m_state) {
    case TransferState::Downloading:
    case TransferState::Uploading:
        setState(TransferState::Paused);
        emit pauseRequested(_msg_info->_unique_name, _msg_info->_transfer_type);
        break;

    case TransferState::Paused:
        resumeState();
        emit resumeRequested(_msg_info->_unique_name, _msg_info->_transfer_type);
        break;

    case TransferState::Failed:
        emit resumeRequested(_msg_info->_unique_name, _msg_info->_transfer_type);
        break;

    default:
        break;
    }
}

void PictureBubble::updateIconOverlay()
{
    switch (m_state) {
    case TransferState::Downloading:
    case TransferState::Uploading:
        m_picLabel->setIconOverlay(m_pauseIcon);
        m_picLabel->showIconOverlay(true);
        break;

    case TransferState::Paused:
        m_picLabel->setIconOverlay(m_playIcon);
        m_picLabel->showIconOverlay(true);
        break;

    case TransferState::Failed:
        m_picLabel->setIconOverlay(m_downloadIcon);
        m_picLabel->showIconOverlay(true);
        break;

    default:
        m_picLabel->showIconOverlay(false);
        break;
    }
}

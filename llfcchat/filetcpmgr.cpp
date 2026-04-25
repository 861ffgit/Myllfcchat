#include "filetcpmgr.h"
#include "usermgr.h"

#include <QAbstractSocket>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace {

bool ParseJsonObject(const QByteArray& data, QJsonObject& object)
{
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qDebug() << "Failed to create QJsonDocument.";
        return false;
    }

    object = jsonDoc.object();
    return true;
}

QString BuildChatImageDir(int peerUid)
{
    const int uid = UserMgr::GetInstance()->GetUid();
    const QString storageDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return storageDir + "/user/" + QString::number(uid) + "/chatimg/" + QString::number(peerUid);
}

void EnsureDirectory(const QString& dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QByteArray BuildImageDownloadRequest(const QString& name,
                                     int seq,
                                     qint64 currentSize,
                                     qint64 totalSize,
                                     int senderId,
                                     int receiverId,
                                     int messageId,
                                     const QString& clientPath = QString())
{
    QJsonObject jsonObj;
    jsonObj["name"] = name;
    jsonObj["seq"] = seq;
    jsonObj["trans_size"] = QString::number(currentSize);
    jsonObj["total_size"] = QString::number(totalSize);
    jsonObj["token"] = UserMgr::GetInstance()->GetToken();
    jsonObj["sender_id"] = senderId;
    jsonObj["receiver_id"] = receiverId;
    jsonObj["message_id"] = messageId;
    jsonObj["uid"] = UserMgr::GetInstance()->GetUid();
    if (!clientPath.isEmpty()) {
        jsonObj["client_path"] = clientPath;
    }

    return QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
}

} // namespace

FileTcpMgr::FileTcpMgr(QObject* parent)
    : QObject(parent),
      _host(""),
      _port(0),
      _b_recv_pending(false),
      _message_id(0),
      _message_len(0),
      _bytes_sent(0),
      _pending(false),
      _cwnd_size(0)
{
    registerMetaType();

    QObject::connect(&_socket, &QTcpSocket::connected, this, [this]() {
        qDebug() << "Connected to server!";
        emit sig_con_success(true);
    });

    QObject::connect(&_socket, &QTcpSocket::readyRead, this, [this]() {
        _buffer.append(_socket.readAll());

        forever {
            if (!_b_recv_pending) {
                if (_buffer.size() < FILE_UPLOAD_HEAD_LEN) {
                    return;
                }

                QDataStream stream(_buffer);
                stream.setVersion(QDataStream::Qt_5_0);
                stream.setByteOrder(QDataStream::BigEndian);
                stream >> _message_id >> _message_len;
                _buffer.remove(0, FILE_UPLOAD_HEAD_LEN);

                qDebug() << "Message ID:" << _message_id << ", Length:" << _message_len;
            }

            if (_buffer.size() < static_cast<int>(_message_len)) {
                _b_recv_pending = true;
                return;
            }

            _b_recv_pending = false;
            const QByteArray messageBody = _buffer.mid(0, _message_len);
            _buffer.remove(0, _message_len);

            qDebug() << "receive body msg is" << messageBody;
            handleMsg(static_cast<ReqId>(_message_id), static_cast<int>(_message_len), messageBody);
        }
    });

    QObject::connect(
        &_socket,
        QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
        this,
        [this](QAbstractSocket::SocketError socketError) {
            qDebug() << "Error:" << _socket.errorString();
            switch (socketError) {
            case QTcpSocket::ConnectionRefusedError:
                qDebug() << "Connection Refused!";
                emit sig_con_success(false);
                break;
            case QTcpSocket::RemoteHostClosedError:
                qDebug() << "Remote Host Closed Connection!";
                break;
            case QTcpSocket::HostNotFoundError:
                qDebug() << "Host Not Found!";
                emit sig_con_success(false);
                break;
            case QTcpSocket::SocketTimeoutError:
                qDebug() << "Connection Timeout!";
                emit sig_con_success(false);
                break;
            case QTcpSocket::NetworkError:
                break;
            default:
                break;
            }
        });

    QObject::connect(&_socket, &QTcpSocket::disconnected, this, [this]() {
        qDebug() << "Disconnected from server.";
        emit sig_connection_closed();
    });

    QObject::connect(this, &FileTcpMgr::sig_send_data, this, &FileTcpMgr::slot_send_data);

    QObject::connect(this, &FileTcpMgr::sig_close, this, &FileTcpMgr::slot_tcp_close);
    QObject::connect(this, &FileTcpMgr::sig_continue_upload_file, this, &FileTcpMgr::slot_continue_upload_file);
    QObject::connect(this, &FileTcpMgr::sig_continue_download_file, this, &FileTcpMgr::slot_continue_download_file);

    initHandlers();
}

void FileTcpMgr::registerMetaType()
{
    qRegisterMetaType<ServerInfo>("ServerInfo");
    qRegisterMetaType<std::shared_ptr<ServerInfo>>("std::shared_ptr<ServerInfo>");
    qRegisterMetaType<SearchInfo>("SearchInfo");
    qRegisterMetaType<std::shared_ptr<SearchInfo>>("std::shared_ptr<SearchInfo>");

    qRegisterMetaType<AddFriendApply>("AddFriendApply");
    qRegisterMetaType<std::shared_ptr<AddFriendApply>>("std::shared_ptr<AddFriendApply>");

    qRegisterMetaType<ApplyInfo>("ApplyInfo");
    qRegisterMetaType<std::shared_ptr<AuthInfo>>("std::shared_ptr<AuthInfo>");

    qRegisterMetaType<AuthRsp>("AuthRsp");
    qRegisterMetaType<std::shared_ptr<AuthRsp>>("std::shared_ptr<AuthRsp>");

    qRegisterMetaType<UserInfo>("UserInfo");
    qRegisterMetaType<std::vector<std::shared_ptr<TextChatData>>>(
        "std::vector<std::shared_ptr<TextChatData>>");
    qRegisterMetaType<std::vector<std::shared_ptr<ChatThreadInfo>>>(
        "std::vector<std::shared_ptr<ChatThreadInfo>>");
    qRegisterMetaType<std::shared_ptr<ChatThreadData>>("std::shared_ptr<ChatThreadData>");
    qRegisterMetaType<ReqId>("ReqId");
    qRegisterMetaType<MsgInfo>("MsgInfo");
    qRegisterMetaType<std::shared_ptr<MsgInfo>>("std::shared_ptr<MsgInfo>");
}

void FileTcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
    const auto find_iter = _handlers.find(id);
    if (find_iter == _handlers.end()) {
        qDebug() << "not found id [" << id << "] to handle";
        return;
    }

    find_iter.value()(id, len, data);
}

void FileTcpMgr::slot_send_data(ReqId reqId, QByteArray dataBytes)
{
    const quint16 id = static_cast<quint16>(reqId);
    const quint32 len = static_cast<quint32>(dataBytes.length());

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);
    out << id << len;
    block.append(dataBytes);
    const qint64 written = _socket.write(block);
    if (written < 0) {
        qWarning() << "FileTcpMgr write failed:" << _socket.errorString();
    }
}

void FileTcpMgr::slot_tcp_connect(std::shared_ptr<ServerInfo> si)
{
    qDebug() << "receive tcp connect signal";
    qDebug() << "Connecting to server...";
    _host = si->_res_host;
    _port = static_cast<uint16_t>(si->_res_port.toUInt());
    _socket.connectToHost(_host, _port);
}

FileTcpMgr::~FileTcpMgr() {}

void FileTcpMgr::SendData(ReqId reqId, QByteArray data)
{
    emit sig_send_data(reqId, data);
}

void FileTcpMgr::initHandlers()
{
    auto handleUploadResponse = [this](const QJsonObject& recvObj) {
        const QString name = recvObj["name"].toString();
        const int seq = recvObj["seq"].toInt();

        auto file_info = UserMgr::GetInstance()->GetTransFileByName(name);
        if (!file_info) {
            return;
        }

        const int sender =
            recvObj.contains("sender") ? recvObj["sender"].toInt() : file_info->_sender;
        const int receiver =
            recvObj.contains("receiver") ? recvObj["receiver"].toInt() : file_info->_receiver;

        file_info->_flighting_seqs.erase(seq);
        file_info->_rsp_seqs.insert(seq);

        while (file_info->_rsp_seqs.count(file_info->_last_confirmed_seq + 1)) {
            ++file_info->_last_confirmed_seq;
        }

        qDebug() << "recv:" << name << "file seq is" << seq;

        if (file_info->_last_confirmed_seq == file_info->_max_seq) {
            file_info->_rsp_size = file_info->_total_size;

            const QString imgDir = BuildChatImageDir(file_info->_sender);
            const QString destPath = imgDir + "/" + file_info->_unique_name;
            CopyFile(file_info->_text_or_url, destPath, imgDir);

            emit sig_update_upload_progress(file_info);
            UserMgr::GetInstance()->RmvTransFileByName(name);

            const auto free_file = UserMgr::GetInstance()->GetFreeUploadFile();
            if (free_file != nullptr) {
                BatchSend(free_file, free_file->_sender, free_file->_receiver);
            }
            return;
        }

        file_info->_rsp_size = file_info->_last_confirmed_seq * MAX_FILE_LEN;
        emit sig_update_upload_progress(file_info);

        if (!UserMgr::GetInstance()->TransFileIsUploading(name)) {
            return;
        }

        BatchSend(file_info, sender, receiver);
    };

    _handlers.insert(ID_UPLOAD_HEAD_ICON_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is" << id;

        QJsonObject recvObj;
        if (!ParseJsonObject(data, recvObj)) {
            return;
        }

        qDebug() << "data jsonobj is" << recvObj;

        if (!recvObj.contains("error")) {
            qDebug() << "icon upload failed, err is Json Parse Err" << ErrorCodes::ERR_JSON;
            return;
        }

        const int err = recvObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "icon upload failed, err is" << err;
            return;
        }

        const QString md5 = recvObj["md5"].toString();
        int seq = recvObj["seq"].toInt();
        const int trans_size = recvObj["trans_size"].toInt();
        const int uid = recvObj["uid"].toInt();
        const int total_size = recvObj["total_size"].toInt();
        const QString name = recvObj["name"].toString();

        qDebug() << "recv:" << name << "file trans_size is" << trans_size;

        if (total_size == trans_size) {
            UserMgr::GetInstance()->RmvUploadFile(name);
            return;
        }

        const auto file_info = UserMgr::GetInstance()->GetUploadInfoByName(name);
        if (!file_info) {
            return;
        }

        QFile file(file_info->filePath());
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Could not open file:" << file.errorString();
            return;
        }

        file.seek(trans_size);
        ++seq;

        const QByteArray buffer = file.read(MAX_FILE_LEN);
        QJsonObject sendObj;
        sendObj["md5"] = md5;
        sendObj["name"] = name;
        sendObj["seq"] = seq;
        sendObj["trans_size"] = buffer.size() + (seq - 1) * MAX_FILE_LEN;
        sendObj["total_size"] = total_size;
        sendObj["last"] =
            (buffer.size() + (seq - 1) * MAX_FILE_LEN >= total_size) ? 1 : 0;
        sendObj["data"] = QString::fromLatin1(buffer.toBase64());
        sendObj["last_seq"] = recvObj["last_seq"].toInt();
        sendObj["uid"] = uid;

        SendData(ID_UPLOAD_HEAD_ICON_REQ, QJsonDocument(sendObj).toJson(QJsonDocument::Compact));
    });

    _handlers.insert(ID_DOWN_LOAD_FILE_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is" << id << "data is" << data;

        QJsonObject jsonObj;
        if (!ParseJsonObject(data, jsonObj)) {
            return;
        }

        if (!jsonObj.contains("error")) {
            qDebug() << "parse download file response failed" << ErrorCodes::ERR_JSON;
            return;
        }

        const int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "download file failed, error is" << err;
            return;
        }

        const QString base64Data = jsonObj["data"].toString();
        const QString clientPath = jsonObj["client_path"].toString();
        const int seq = jsonObj["seq"].toInt();
        const bool is_last = jsonObj["is_last"].toBool();
        const qint64 total_size = jsonObj["total_size"].toString().toLongLong();
        const qint64 current_size = jsonObj["current_size"].toString().toLongLong();
        const QString name = jsonObj["name"].toString();
        const QString req_type = jsonObj["req_type"].toString();

        const auto file_info = UserMgr::GetInstance()->GetDownloadInfo(name);
        if (!file_info) {
            qDebug() << "file:" << name << "not found";
            return;
        }

        file_info->_current_size = static_cast<int>(current_size);
        file_info->_total_size = static_cast<int>(total_size);

        const QByteArray decodedData = QByteArray::fromBase64(base64Data.toUtf8());
        QFile file(clientPath);
        const QIODevice::OpenMode mode =
            (seq == 1) ? QIODevice::WriteOnly : (QIODevice::WriteOnly | QIODevice::Append);

        if (!file.open(mode)) {
            qDebug() << "Failed to open file for writing:" << clientPath;
            qDebug() << "Error:" << file.errorString();
            return;
        }

        const qint64 bytesWritten = file.write(decodedData);
        if (bytesWritten != decodedData.size()) {
            qDebug() << "Failed to write all data. Written:" << bytesWritten
                     << "Expected:" << decodedData.size();
        }

        file.close();

        qDebug() << "Successfully wrote" << bytesWritten << "bytes to file";
        qDebug() << "Progress:" << current_size << "/" << total_size
                 << "(" << (current_size * 100 / total_size) << "%)";

        if (is_last) {
            qDebug() << "File download completed:" << clientPath;
            UserMgr::GetInstance()->RmvDownloadFile(name);

            if (req_type == "self_icon") {
                emit sig_reset_label_icon(clientPath);
            }
            return;
        }

        file_info->_seq = seq + 1;
        FileTcpMgr::GetInstance()->SendDownloadInfo(file_info, req_type);
    });

    _handlers.insert(ID_FILE_INFO_SYNC_RSP, [this, handleUploadResponse](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is" << id;

        if (_cwnd_size > 0) {
            --_cwnd_size;
        }

        QJsonObject recvObj;
        if (!ParseJsonObject(data, recvObj)) {
            return;
        }

        qDebug() << "data jsonobj is" << recvObj;

        if (!recvObj.contains("error")) {
            qDebug() << "file upload failed, err is Json Parse Err" << ErrorCodes::ERR_JSON;
            return;
        }

        const int err = recvObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "file upload failed, err is" << err;
            return;
        }

        handleUploadResponse(recvObj);
    });

    _handlers.insert(ID_IMG_CHAT_UPLOAD_RSP, [this, handleUploadResponse](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is" << id;

        if (_cwnd_size > 0) {
            --_cwnd_size;
        }

        QJsonObject recvObj;
        if (!ParseJsonObject(data, recvObj)) {
            return;
        }

        qDebug() << "data jsonobj is" << recvObj;

        if (!recvObj.contains("error")) {
            qDebug() << "image upload failed, err is Json Parse Err" << ErrorCodes::ERR_JSON;
            return;
        }

        const int err = recvObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "image upload failed, err is" << err;
            return;
        }

        handleUploadResponse(recvObj);
    });

    _handlers.insert(
        ID_IMG_CHAT_CONTINUE_UPLOAD_RSP,
        [this, handleUploadResponse](ReqId id, int len, QByteArray data) {
            Q_UNUSED(len);
            qDebug() << "handle id is" << id;

            if (_cwnd_size > 0) {
                --_cwnd_size;
            }

            QJsonObject recvObj;
            if (!ParseJsonObject(data, recvObj)) {
                return;
            }

            qDebug() << "data jsonobj is" << recvObj;

            if (!recvObj.contains("error")) {
                qDebug() << "image continue upload failed, err is Json Parse Err"
                         << ErrorCodes::ERR_JSON;
                return;
            }

            const int err = recvObj["error"].toInt();
            if (err != ErrorCodes::SUCCESS) {
                qDebug() << "image continue upload failed, err is" << err;
                return;
            }

            handleUploadResponse(recvObj);
        });

    _handlers.insert(ID_IMG_CHAT_DOWN_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is" << id << "data is" << data;

        QJsonObject jsonObj;
        if (!ParseJsonObject(data, jsonObj)) {
            return;
        }

        if (!jsonObj.contains("error")) {
            qDebug() << "parse image download response failed" << ErrorCodes::ERR_JSON;
            return;
        }

        const int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "image download failed, error is" << err;
            return;
        }

        const QString base64Data = jsonObj["data"].toString();
        const int seq = jsonObj["seq"].toInt();
        const bool is_last = jsonObj["is_last"].toBool();
        const qint64 total_size = jsonObj["total_size"].toString().toLongLong();
        const qint64 current_size = jsonObj["current_size"].toString().toLongLong();
        const QString name = jsonObj["name"].toString();

        const auto file_info = UserMgr::GetInstance()->GetTransFileByName(name);
        if (!file_info) {
            qDebug() << "file:" << name << "not found";
            return;
        }

        file_info->_current_size = current_size;
        file_info->_rsp_size = current_size;
        file_info->_total_size = total_size;

        const QString clientPath = file_info->_text_or_url;
        const QString file_path = clientPath + "/" + name;
        const QByteArray decodedData = QByteArray::fromBase64(base64Data.toUtf8());

        QFile file(file_path);
        const QIODevice::OpenMode mode =
            (seq == 1) ? QIODevice::WriteOnly : (QIODevice::WriteOnly | QIODevice::Append);

        if (!file.open(mode)) {
            qDebug() << "Failed to open file for writing:" << file_path;
            qDebug() << "Error:" << file.errorString();
            return;
        }

        const qint64 bytesWritten = file.write(decodedData);
        if (bytesWritten != decodedData.size()) {
            qDebug() << "Failed to write all data. Written:" << bytesWritten
                     << "Expected:" << decodedData.size();
        }

        file.close();

        qDebug() << "Successfully wrote" << bytesWritten << "bytes to file";
        qDebug() << "Progress:" << current_size << "/" << total_size
                 << "(" << (current_size * 100 / total_size) << "%)";

        if (is_last) {
            qDebug() << "File download completed:" << file_path;
            UserMgr::GetInstance()->RmvTransFileByName(name);
            emit sig_download_finish(file_info, file_path);
            return;
        }

        file_info->_seq = seq + 1;
        file_info->_last_confirmed_seq = seq;
        if (file_info->_transfer_state == TransferState::Paused) {
            return;
        }

        const QByteArray send_data = BuildImageDownloadRequest(
            name,
            file_info->_seq,
            file_info->_current_size,
            file_info->_total_size,
            file_info->_sender,
            file_info->_receiver,
            file_info->_msg_id);
        FileTcpMgr::GetInstance()->SendData(ID_IMG_CHAT_DOWN_REQ, send_data);
        emit sig_update_download_progress(file_info);
    });

    _handlers.insert(ID_IMG_CHAT_DOWN_INFO_SYNC_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is" << id << "data is" << data;

        QJsonObject jsonObj;
        if (!ParseJsonObject(data, jsonObj)) {
            return;
        }

        if (!jsonObj.contains("error")) {
            qDebug() << "parse image download info response failed" << ErrorCodes::ERR_JSON;
            return;
        }

        const int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "image download info failed, error is" << err;
            return;
        }

        const int message_id = jsonObj["message_id"].toInt();
        const int thread_id = jsonObj["thread_id"].toInt();
        const int sender_id = jsonObj["sender_id"].toInt();
        const int recv_id = jsonObj["recv_id"].toInt();
        const QString name = jsonObj["name"].toString();
        const int msg_type = jsonObj["msg_type"].toInt();
        const int status = jsonObj["status"].toInt();
        Q_UNUSED(msg_type);
        Q_UNUSED(status);

        const qint64 total_size = jsonObj["total_size"].toString().toLongLong();
        const QString img_path_str = BuildChatImageDir(sender_id);

        auto file_info = UserMgr::GetInstance()->GetTransFileByName(name);
        if (!file_info) {
            file_info = std::make_shared<MsgInfo>(
                MsgType::IMG_MSG,
                img_path_str,
                CreateLoadingPlaceholder(200, 200),
                name,
                total_size,
                "");
            file_info->_msg_id = message_id;
            file_info->_sender = sender_id;
            file_info->_receiver = recv_id;
            file_info->_thread_id = thread_id;
            file_info->_transfer_type = TransferType::Download;
            file_info->_transfer_state = TransferState::Downloading;
            UserMgr::GetInstance()->AddTransFile(name, file_info);
        }

        EnsureDirectory(img_path_str);

        emit sig_update_download_progress(file_info);

        const QByteArray send_data = BuildImageDownloadRequest(
            name,
            file_info->_seq,
            0,
            file_info->_total_size,
            sender_id,
            recv_id,
            message_id,
            img_path_str);
        FileTcpMgr::GetInstance()->SendData(ID_IMG_CHAT_DOWN_REQ, send_data);
    });
}

void FileTcpMgr::CopyFile(QString src_path, QString dst_path, QString dst_dir)
{
    EnsureDirectory(dst_dir);
    if (QFile::exists(dst_path)) {
        QFile::remove(dst_path);
    }

    if (QFile::copy(src_path, dst_path)) {
        qDebug() << "copy file success";
    } else {
        qDebug() << "copy file failed";
    }
}

void FileTcpMgr::ContinueUploadFile(QString unique_name)
{
    emit sig_continue_upload_file(unique_name);
}

void FileTcpMgr::ContinueDownloadFile(QString unique_name)
{
    emit sig_continue_download_file(unique_name);
}

void FileTcpMgr::BatchSend(std::shared_ptr<MsgInfo> msg_info, int sender, int receiver)
{
    if (!msg_info) {
        return;
    }

    if (msg_info->_seq * MAX_FILE_LEN >= msg_info->_total_size) {
        qDebug() << "file has sent finished";
        return;
    }

    if (MAX_CWND_SIZE - _cwnd_size == 0) {
        return;
    }

    QFile file(msg_info->_text_or_url);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file:" << file.errorString();
        return;
    }

    file.seek(msg_info->_seq * MAX_FILE_LEN);

    while (MAX_CWND_SIZE - _cwnd_size > 0) {
        ++msg_info->_seq;
        msg_info->_flighting_seqs.insert(msg_info->_seq);

        const QByteArray buffer = file.read(MAX_FILE_LEN);
        if (buffer.isEmpty()) {
            break;
        }
        const QString base64Data = QString::fromLatin1(buffer.toBase64());

        msg_info->_current_size = buffer.size() + (msg_info->_seq - 1) * MAX_FILE_LEN;

        QJsonObject sendObj;
        sendObj["md5"] = msg_info->_md5;
        sendObj["name"] = msg_info->_unique_name;
        sendObj["seq"] = msg_info->_seq;
        sendObj["trans_size"] = QString::number(msg_info->_current_size);
        sendObj["total_size"] = QString::number(msg_info->_total_size);
        sendObj["last"] =
            (buffer.size() + (msg_info->_seq - 1) * MAX_FILE_LEN >= msg_info->_total_size) ? 1 : 0;
        sendObj["data"] = base64Data;
        sendObj["last_seq"] = msg_info->_max_seq;
        sendObj["uid"] = UserMgr::GetInstance()->GetUid();
        sendObj["message_id"] = msg_info->_msg_id;
        sendObj["sender"] = sender;
        sendObj["receiver"] = receiver;

        SendData(ID_FILE_INFO_SYNC_REQ, QJsonDocument(sendObj).toJson(QJsonDocument::Compact));
        ++_cwnd_size;

        if (sendObj["last"].toInt() == 1) {
            break;
        }
    }
}

void FileTcpMgr::slot_tcp_close()
{
    _socket.close();
}

void FileTcpMgr::slot_continue_upload_file(QString unique_name)
{
    const auto msg_info = UserMgr::GetInstance()->GetTransFileByName(unique_name);
    if (!msg_info) {
        return;
    }

    msg_info->_seq = msg_info->_last_confirmed_seq;

    if (msg_info->_seq * MAX_FILE_LEN >= msg_info->_total_size) {
        qDebug() << "file has sent finished";
        return;
    }

    if (MAX_CWND_SIZE - _cwnd_size == 0) {
        return;
    }

    QFile file(msg_info->_text_or_url);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file:" << file.errorString();
        return;
    }

    file.seek(msg_info->_seq * MAX_FILE_LEN);

    while (MAX_CWND_SIZE - _cwnd_size > 0) {
        ++msg_info->_seq;
        msg_info->_flighting_seqs.insert(msg_info->_seq);

        const QByteArray buffer = file.read(MAX_FILE_LEN);
        if (buffer.isEmpty()) {
            break;
        }
        const QString base64Data = QString::fromLatin1(buffer.toBase64());

        msg_info->_current_size = buffer.size() + (msg_info->_seq - 1) * MAX_FILE_LEN;

        QJsonObject sendObj;
        sendObj["md5"] = msg_info->_md5;
        sendObj["name"] = msg_info->_unique_name;
        sendObj["seq"] = msg_info->_seq;
        sendObj["trans_size"] = msg_info->_current_size;
        sendObj["total_size"] = msg_info->_total_size;
        sendObj["sender"] = msg_info->_sender;
        sendObj["receiver"] = msg_info->_receiver;
        sendObj["message_id"] = msg_info->_msg_id;
        sendObj["last"] =
            (buffer.size() + (msg_info->_seq - 1) * MAX_FILE_LEN >= msg_info->_total_size) ? 1 : 0;
        sendObj["data"] = base64Data;
        sendObj["last_seq"] = msg_info->_max_seq;
        sendObj["uid"] = UserMgr::GetInstance()->GetUid();

        SendData(ID_IMG_CHAT_CONTINUE_UPLOAD_REQ, QJsonDocument(sendObj).toJson(QJsonDocument::Compact));
        ++_cwnd_size;

        if (sendObj["last"].toInt() == 1) {
            break;
        }
    }
}

void FileTcpMgr::slot_continue_download_file(QString unique_name)
{
    const auto file_info = UserMgr::GetInstance()->GetTransFileByName(unique_name);
    if (!file_info) {
        return;
    }

    if (file_info->_current_size >= file_info->_total_size) {
        qDebug() << "file has received finished";
        return;
    }

    const QByteArray send_data = BuildImageDownloadRequest(
        unique_name,
        file_info->_seq,
        file_info->_current_size,
        file_info->_total_size,
        file_info->_sender,
        file_info->_receiver,
        file_info->_msg_id);
    FileTcpMgr::GetInstance()->SendData(ID_IMG_CHAT_DOWN_REQ, send_data);
}

void FileTcpMgr::CloseConnection()
{
    emit sig_close();
}

void FileTcpMgr::SendDownloadInfo(std::shared_ptr<DownloadInfo> download, QString req_type)
{
    QJsonObject jsonObj;
    jsonObj["name"] = download->_name;
    jsonObj["seq"] = download->_seq;
    jsonObj["trans_size"] = 0;
    jsonObj["total_size"] = 0;
    jsonObj["token"] = UserMgr::GetInstance()->GetToken();
    jsonObj["uid"] = UserMgr::GetInstance()->GetUid();
    jsonObj["client_path"] = download->_client_path;
    jsonObj["req_type"] = req_type;

    SendData(ID_DOWN_LOAD_FILE_REQ, QJsonDocument(jsonObj).toJson(QJsonDocument::Compact));
}

FileTcpThread::FileTcpThread()
{
    _file_tcp_thread = new QThread();
    FileTcpMgr::GetInstance()->moveToThread(_file_tcp_thread);
    QObject::connect(_file_tcp_thread, &QThread::finished, _file_tcp_thread, &QObject::deleteLater);
    _file_tcp_thread->start();
}

FileTcpThread::~FileTcpThread()
{
    _file_tcp_thread->quit();
}

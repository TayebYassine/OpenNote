#include "SingleInstanceController.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>
#include <QBuffer>
#include <QVariant>

namespace {
constexpr quint32 kMagic = 0x4F4E4F54;  

QDataStream::Version streamVersion() {
    return QDataStream::Qt_6_0;
}
}  

SingleInstanceController::SingleInstanceController(QString serverName, QObject* parent)
    : QObject(parent), m_serverName(std::move(serverName)) {}

SingleInstanceController::~SingleInstanceController() {
    if (m_server) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }
}

bool SingleInstanceController::tryForwardToPrimary(const QStringList& paths, int timeoutMs) {
    if (paths.isEmpty()) return false;

    QLocalSocket socket;
    socket.connectToServer(m_serverName);
    if (!socket.waitForConnected(timeoutMs)) return false;

    QByteArray payload;
    {
        QDataStream out(&payload, QIODevice::WriteOnly);
        out.setVersion(streamVersion());
        out << kMagic;
        out << paths;
    }

    QDataStream out(&socket);
    out.setVersion(streamVersion());
    out << quint32(payload.size());
    socket.write(payload);
    socket.flush();
    socket.waitForBytesWritten(timeoutMs);
    socket.disconnectFromServer();
    return true;
}

bool SingleInstanceController::startPrimary() {
    if (m_server) return true;

    m_server = new QLocalServer(this);

    
    QLocalServer::removeServer(m_serverName);

    if (!m_server->listen(m_serverName)) return false;

    connect(m_server, &QLocalServer::newConnection, this,
            &SingleInstanceController::onNewConnection);
    return true;
}

void SingleInstanceController::onNewConnection() {
    while (m_server && m_server->hasPendingConnections()) {
        QLocalSocket* socket = m_server->nextPendingConnection();
        if (!socket) continue;
        setupSocket(socket);
    }
}

void SingleInstanceController::setupSocket(QLocalSocket* socket) {
    socket->setParent(this);
    socket->setProperty("_opennote_expectedSize", QVariant::fromValue<quint32>(0));

    connect(socket, &QLocalSocket::readyRead, this, [this, socket]() {
        QDataStream in(socket);
        in.setVersion(streamVersion());

        quint32 expected = socket->property("_opennote_expectedSize").toUInt();
        if (expected == 0) {
            if (socket->bytesAvailable() < static_cast<qint64>(sizeof(quint32))) return;
            in >> expected;
            socket->setProperty("_opennote_expectedSize", expected);
        }

        if (socket->bytesAvailable() < expected) return;

        const QByteArray payload = socket->read(expected);
        socket->disconnectFromServer();
        socket->deleteLater();

        QBuffer buf;
        buf.setData(payload);
        if (!buf.open(QIODevice::ReadOnly)) return;
        QDataStream payloadIn(&buf);
        payloadIn.setVersion(streamVersion());

        quint32 magic = 0;
        QStringList paths;
        payloadIn >> magic;
        payloadIn >> paths;
        if (magic != kMagic) return;
        if (!paths.isEmpty()) emit filesReceived(paths);
    });

    connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
}


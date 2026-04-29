#ifndef OPENNOTE_LINUX_SINGLEINSTANCECONTROLLER_H
#define OPENNOTE_LINUX_SINGLEINSTANCECONTROLLER_H
#pragma once

#include <QObject>
#include <QStringList>

class QLocalServer;
class QLocalSocket;

class SingleInstanceController : public QObject {
    Q_OBJECT

public:
    explicit SingleInstanceController(QString serverName, QObject* parent = nullptr);
    ~SingleInstanceController() override;

    [[nodiscard]] QString serverName() const { return m_serverName; }

    [[nodiscard]] bool tryForwardToPrimary(const QStringList& paths, int timeoutMs = 250);
    [[nodiscard]] bool startPrimary();

signals:
    void filesReceived(const QStringList& paths);

private:
    void onNewConnection();
    void setupSocket(QLocalSocket* socket);

    QString m_serverName;
    QLocalServer* m_server = nullptr;
};

#endif  // OPENNOTE_LINUX_SINGLEINSTANCECONTROLLER_H


#pragma once

#include <QObject>
#include <QHostAddress>
#include <QHash>
#include <QString>

class QTcpServer;
class QTcpSocket;

class Lx200Server : public QObject {
    Q_OBJECT
public:
    explicit Lx200Server(QObject *parent = nullptr);

    bool start(quint16 port, const QHostAddress &address = QHostAddress::Any);
    void stop();

    bool isRunning() const;
    quint16 listeningPort() const;
    QHostAddress listeningAddress() const;
    int clientCount() const;

    void setCurrentPosition(double raDeg, double decDeg);
    void setLocation(double latitudeDeg, double longitudeDeg);

signals:
    void runningChanged(bool running);
    void clientConnected(const QHostAddress &peerAddress, quint16 peerPort);
    void clientDisconnected(const QHostAddress &peerAddress, quint16 peerPort);
    void errorOccurred(const QString &error);

    void gotoRequested(double raDeg, double decDeg);
    void stopRequested();
    void syncRequested(double raDeg, double decDeg);

private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();

private:
    void processBuffer(QTcpSocket *socket);
    void processCommand(QTcpSocket *socket, const QByteArray &rawCmd);
    void writeToClient(QTcpSocket *socket, const QByteArray &data);

    QByteArray handleCommand(const QByteArray &cmd, const QByteArray &value);

    QTcpServer *m_server;
    QHash<QTcpSocket *, QByteArray> m_buffers;

    double m_currentRaDeg;
    double m_currentDecDeg;

    double m_targetRaDeg;
    double m_targetDecDeg;
    bool m_haveTargetRa;
    bool m_haveTargetDec;

    double m_latitudeDeg;
    double m_longitudeDeg;

    bool m_highPrecision;
};

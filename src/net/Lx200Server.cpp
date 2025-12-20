#include "Lx200Server.h"

#include <QAbstractSocket>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>

#include <cmath>

#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QTimeZone>
#include <QtMath>

namespace {

static double normalizeDegrees360(double deg) {
  double v = std::fmod(deg, 360.0);
  if (v < 0.0) {
    v += 360.0;
  }
  return v;
}

static double clampDec(double decDeg) {
  if (decDeg > 90.0)
    return 90.0;
  if (decDeg < -90.0)
    return -90.0;
  return decDeg;
}

static bool parseRaToDegrees(const QByteArray &value, bool highPrecision,
                             double *outRaDeg) {
  // Accept HH:MM.T or HH:MM:SS (also tolerate leading space)
  QByteArray v = value.trimmed();
  QList<QByteArray> parts = v.split(':');
  if (parts.size() < 2) {
    return false;
  }

  bool okH = false;
  int hh = parts[0].toInt(&okH);
  if (!okH || hh < 0 || hh > 23) {
    return false;
  }

  double mm = 0.0;
  double ss = 0.0;

  if (parts.size() == 2) {
    // MM.T
    bool okM = false;
    mm = parts[1].toDouble(&okM);
    if (!okM || mm < 0.0 || mm >= 60.0) {
      return false;
    }
    // tenths of minute are already part of mm
  } else {
    bool okM = false;
    int mmi = parts[1].toInt(&okM);
    if (!okM || mmi < 0 || mmi >= 60) {
      return false;
    }
    mm = mmi;

    bool okS = false;
    ss = parts[2].toDouble(&okS);
    if (!okS || ss < 0.0 || ss >= 60.0) {
      return false;
    }
  }

  double hours = static_cast<double>(hh) + (mm / 60.0) + (ss / 3600.0);
  double raDeg = normalizeDegrees360(hours * 15.0);

  Q_UNUSED(highPrecision);
  *outRaDeg = raDeg;
  return true;
}

static bool parseDecToDegrees(const QByteArray &value, bool highPrecision,
                              double *outDecDeg) {
  // Accept sDD*MM or sDD*MM:SS. Also tolerate degree symbol 0xDF in place of
  // '*'.
  QByteArray v = value.trimmed();
  v.replace(static_cast<char>(0xDF), '*');

  if (v.size() < 6) {
    return false;
  }

  char signCh = v[0];
  if (signCh != '+' && signCh != '-') {
    return false;
  }
  int sign = (signCh == '-') ? -1 : 1;

  int starIdx = v.indexOf('*');
  if (starIdx < 0) {
    return false;
  }

  bool okDeg = false;
  int deg = v.mid(1, starIdx - 1).toInt(&okDeg);
  if (!okDeg || deg < 0 || deg > 90) {
    return false;
  }

  QByteArray rest = v.mid(starIdx + 1);
  QList<QByteArray> parts = rest.split(':');

  bool okMin = false;
  int min = parts[0].toInt(&okMin);
  if (!okMin || min < 0 || min >= 60) {
    return false;
  }

  int sec = 0;
  if (parts.size() >= 2) {
    bool okSec = false;
    sec = parts[1].toInt(&okSec);
    if (!okSec || sec < 0 || sec >= 60) {
      return false;
    }
  }

  double decDeg =
      sign * (static_cast<double>(deg) + (static_cast<double>(min) / 60.0) +
              (static_cast<double>(sec) / 3600.0));
  decDeg = clampDec(decDeg);

  Q_UNUSED(highPrecision);
  *outDecDeg = decDeg;
  return true;
}

static QByteArray formatRa(double raDeg, bool highPrecision) {
  // LX200 classic expects HH:MM.T or HH:MM:SS, terminated with '#'
  double ra = normalizeDegrees360(raDeg);
  double totalHours = ra / 15.0;

  int hh = static_cast<int>(std::floor(totalHours));
  double remHours = totalHours - hh;

  double totalMinutes = remHours * 60.0;
  int mm = static_cast<int>(std::floor(totalMinutes));
  double remMinutes = totalMinutes - mm;

  if (highPrecision) {
    double totalSeconds = remMinutes * 60.0;
    int ss = static_cast<int>(std::round(totalSeconds));
    if (ss >= 60) {
      ss = 0;
      mm += 1;
    }
    if (mm >= 60) {
      mm = 0;
      hh = (hh + 1) % 24;
    }
    return QString::asprintf("%02d:%02d:%02d#", hh, mm, ss).toLatin1();
  }

  // Tenths of minute
  int tenth = static_cast<int>(std::round(remMinutes * 10.0));
  if (tenth >= 10) {
    tenth = 0;
    mm += 1;
  }
  if (mm >= 60) {
    mm = 0;
    hh = (hh + 1) % 24;
  }

  return QString::asprintf("%02d:%02d.%1d#", hh, mm, tenth).toLatin1();
}

static QByteArray formatDec(double decDeg, bool highPrecision) {
  double dec = clampDec(decDeg);
  char sign = dec < 0.0 ? '-' : '+';
  dec = std::abs(dec);

  int dd = static_cast<int>(std::floor(dec));
  double remDeg = dec - dd;

  double totalMinutes = remDeg * 60.0;
  int mm = static_cast<int>(std::floor(totalMinutes));
  double remMinutes = totalMinutes - mm;

  if (highPrecision) {
    double totalSeconds = remMinutes * 60.0;
    int ss = static_cast<int>(std::round(totalSeconds));
    if (ss >= 60) {
      ss = 0;
      mm += 1;
    }
    if (mm >= 60) {
      mm = 0;
      dd += 1;
    }
    if (dd > 90) {
      dd = 90;
      mm = 0;
      ss = 0;
    }
    return QString::asprintf("%c%02d*%02d:%02d#", sign, dd, mm, ss).toLatin1();
  }

  int mmRound = static_cast<int>(std::round(totalMinutes));
  if (mmRound >= 60) {
    mmRound = 0;
    dd += 1;
  }
  if (dd > 90) {
    dd = 90;
    mmRound = 0;
  }

  return QString::asprintf("%c%02d*%02d#", sign, dd, mmRound).toLatin1();
}

} // namespace

Lx200Server::Lx200Server(QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this)), m_currentRaDeg(0.0),
      m_currentDecDeg(0.0), m_targetRaDeg(0.0), m_targetDecDeg(0.0),
      m_haveTargetRa(false), m_haveTargetDec(false), m_latitudeDeg(0.0),
      m_longitudeDeg(0.0), m_highPrecision(false) {
  connect(m_server, &QTcpServer::newConnection, this,
          &Lx200Server::onNewConnection);
}

bool Lx200Server::start(quint16 port, const QHostAddress &address) {
  if (m_server->isListening()) {
    return true;
  }

  if (!m_server->listen(address, port)) {
    emit errorOccurred(m_server->errorString());
    return false;
  }

  emit runningChanged(true);
  qDebug() << "[LX200] Listening on" << m_server->serverAddress().toString()
           << ":" << m_server->serverPort();
  return true;
}

void Lx200Server::stop() {
  if (!m_server->isListening()) {
    return;
  }

  for (QTcpSocket *sock : m_buffers.keys()) {
    if (!sock) {
      continue;
    }
    sock->disconnect(this);
    sock->disconnectFromHost();
    sock->deleteLater();
  }
  m_buffers.clear();

  m_server->close();
  emit runningChanged(false);
  qDebug() << "[LX200] Server stopped";
}

bool Lx200Server::isRunning() const { return m_server->isListening(); }

quint16 Lx200Server::listeningPort() const { return m_server->serverPort(); }

QHostAddress Lx200Server::listeningAddress() const {
  return m_server->serverAddress();
}

int Lx200Server::clientCount() const { return m_buffers.size(); }

void Lx200Server::setCurrentPosition(double raDeg, double decDeg) {
  m_currentRaDeg = normalizeDegrees360(raDeg);
  m_currentDecDeg = clampDec(decDeg);
}

void Lx200Server::setLocation(double latitudeDeg, double longitudeDeg) {
  m_latitudeDeg = latitudeDeg;
  m_longitudeDeg = longitudeDeg;
}

void Lx200Server::onNewConnection() {
  while (m_server->hasPendingConnections()) {
    QTcpSocket *sock = m_server->nextPendingConnection();
    if (!sock) {
      continue;
    }

    m_buffers.insert(sock, QByteArray());

    connect(sock, &QTcpSocket::readyRead, this,
            &Lx200Server::onClientReadyRead);
    connect(sock, &QTcpSocket::disconnected, this,
            &Lx200Server::onClientDisconnected);
    connect(sock, &QTcpSocket::errorOccurred, this,
            [this, sock](QAbstractSocket::SocketError err) {
              // RemoteHostClosedError is normal when client disconnects - don't
              // treat as error
              if (err != QAbstractSocket::RemoteHostClosedError) {
                qWarning() << "[LX200] Client socket error:"
                           << sock->errorString();
              }
            });

    emit clientConnected(sock->peerAddress(), sock->peerPort());
    qDebug() << "[LX200] Client connected" << sock->peerAddress().toString()
             << ":" << sock->peerPort();
  }
}

void Lx200Server::onClientReadyRead() {
  QTcpSocket *sock = qobject_cast<QTcpSocket *>(sender());
  if (!sock) {
    return;
  }

  QByteArray data = sock->readAll();
  if (data.isEmpty()) {
    return;
  }

  m_buffers[sock].append(data);
  processBuffer(sock);
}

void Lx200Server::onClientDisconnected() {
  QTcpSocket *sock = qobject_cast<QTcpSocket *>(sender());
  if (!sock) {
    return;
  }

  emit clientDisconnected(sock->peerAddress(), sock->peerPort());
  qDebug() << "[LX200] Client disconnected" << sock->peerAddress().toString()
           << ":" << sock->peerPort();

  m_buffers.remove(sock);
  sock->deleteLater();
}

void Lx200Server::processBuffer(QTcpSocket *socket) {
  QByteArray &buffer = m_buffers[socket];

  // Some clients prefix commands with stray '#', ignore leading terminators.
  while (!buffer.isEmpty() && buffer.startsWith('#')) {
    buffer.remove(0, 1);
  }

  // Handle ACK (single 0x06) queries, which are not terminated with '#'
  while (!buffer.isEmpty() &&
         static_cast<unsigned char>(buffer.at(0)) == 0x06) {
    buffer.remove(0, 1);
    // Alignment status: A=AltAz (good enough)
    writeToClient(socket, QByteArray("A"));
  }

  // Parse '#' terminated LX200 commands.
  int idx = -1;
  while ((idx = buffer.indexOf('#')) >= 0) {
    QByteArray rawCmd = buffer.left(idx);
    buffer.remove(0, idx + 1);

    // Drop additional stray '#'
    while (!buffer.isEmpty() && buffer.startsWith('#')) {
      buffer.remove(0, 1);
    }

    if (rawCmd.isEmpty()) {
      continue;
    }

    processCommand(socket, rawCmd);
  }
}

void Lx200Server::processCommand(QTcpSocket *socket, const QByteArray &rawCmd) {
  QByteArray cmd;
  QByteArray value;

  static const QList<QByteArray> knownCommands = {
      QByteArray(":GVP"), QByteArray(":GVN"), QByteArray(":Qe"),
      QByteArray(":Qn"),  QByteArray(":Qs"),  QByteArray(":Qw"),
      QByteArray(":Sr"),  QByteArray(":Sd"),  QByteArray(":MS"),
      QByteArray(":CM"),  QByteArray(":GR"),  QByteArray(":GD"),
      QByteArray(":Gg"),  QByteArray(":Gt"),  QByteArray(":Sg"),
      QByteArray(":St"),  QByteArray(":GL"),  QByteArray(":GS"),
      QByteArray(":GC"),  QByteArray(":GZ"),  QByteArray(":GA"),
      QByteArray(":GG"),  QByteArray(":Mn"),  QByteArray(":Ms"),
      QByteArray(":Me"),  QByteArray(":Mw"),  QByteArray(":RC"),
      QByteArray(":RG"),  QByteArray(":RM"),  QByteArray(":RS"),
      QByteArray(":U"),   QByteArray(":D"),   QByteArray(":Q")};

  for (const QByteArray &candidate : knownCommands) {
    if (rawCmd.startsWith(candidate)) {
      cmd = candidate;
      value = rawCmd.mid(candidate.size());
      break;
    }
  }

  if (cmd.isEmpty()) {
    if (rawCmd.size() >= 3) {
      cmd = rawCmd.left(3);
      value = rawCmd.mid(3);
    } else {
      cmd = rawCmd;
      value = QByteArray();
    }
  }

  QByteArray resp = handleCommand(cmd, value);
  if (!resp.isNull() && !resp.isEmpty()) {
    writeToClient(socket, resp);
  }
}

void Lx200Server::writeToClient(QTcpSocket *socket, const QByteArray &data) {
  if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
    return;
  }

  socket->write(data);
}

QByteArray Lx200Server::handleCommand(const QByteArray &cmd,
                                      const QByteArray &value) {
  // Note: Return strings for LX200 are generally terminated with '#'.
  // Some commands return raw strings without '#', but clients generally accept
  // with '#'.
  if (cmd == ":GR") {
    return formatRa(m_currentRaDeg, m_highPrecision);
  }

  if (cmd == ":GD") {
    return formatDec(m_currentDecDeg, m_highPrecision);
  }

  if (cmd == ":Sr") {
    double raDeg = 0.0;
    if (parseRaToDegrees(value, m_highPrecision, &raDeg)) {
      m_targetRaDeg = raDeg;
      m_haveTargetRa = true;
      return QByteArray("1");
    }
    return QByteArray("0");
  }

  if (cmd == ":Sd") {
    double decDeg = 0.0;
    if (parseDecToDegrees(value, m_highPrecision, &decDeg)) {
      m_targetDecDeg = decDeg;
      m_haveTargetDec = true;
      return QByteArray("1");
    }
    return QByteArray("0");
  }

  if (cmd == ":MS") {
    if (m_haveTargetRa && m_haveTargetDec) {
      emit gotoRequested(m_targetRaDeg, m_targetDecDeg);
      // 0 means slew is possible
      return QByteArray("0");
    }
    // Missing target, report not possible
    return QByteArray("2");
  }

  if (cmd == ":CM") {
    // Sync/align at current position to target coords.
    if (m_haveTargetRa && m_haveTargetDec) {
      emit syncRequested(m_targetRaDeg, m_targetDecDeg);
      // Typical LX200 returns an object name string, no '#'
      return QByteArray("M31 EX GAL MAG 3.5 SZ178.0'");
    }
    return QByteArray("?");
  }

  if (cmd == ":Q" || cmd == ":Qn" || cmd == ":Qs" || cmd == ":Qe" ||
      cmd == ":Qw") {
    emit stopRequested();
    return QByteArray();
  }

  if (cmd == ":U") {
    m_highPrecision = !m_highPrecision;
    return QByteArray();
  }

  if (cmd == ":Gg") {
    // Get longitude DDD*MM# (unsigned). LX200 Classic: West is positive.
    // Our internal m_longitudeDeg is East positive.
    double lonWest = normalizeDegrees360(-m_longitudeDeg);
    int ddd = static_cast<int>(std::floor(lonWest));
    int mm = static_cast<int>(std::round((lonWest - ddd) * 60.0));
    if (mm >= 60) {
      mm = 0;
      ddd = (ddd + 1) % 360;
    }
    return QString::asprintf("%03d*%02d#", ddd, mm).toLatin1();
  }

  if (cmd == ":Gt") {
    // Get latitude sDD*MM#
    double lat = clampDec(m_latitudeDeg);
    char sign = lat < 0.0 ? '-' : '+';
    lat = std::abs(lat);
    int dd = static_cast<int>(std::floor(lat));
    int mm = static_cast<int>(std::round((lat - dd) * 60.0));
    if (mm >= 60) {
      mm = 0;
      dd += 1;
    }
    if (dd > 90) {
      dd = 90;
      mm = 0;
    }
    return QString::asprintf("%c%02d*%02d#", sign, dd, mm).toLatin1();
  }

  if (cmd == ":St") {
    // Set latitude
    double lat = 0.0;
    if (parseDecToDegrees(value, m_highPrecision, &lat)) {
      m_latitudeDeg = lat;
      return QByteArray("1");
    }
    return QByteArray("0");
  }

  if (cmd == ":Sg") {
    // Set longitude DDD*MM
    QByteArray v = value.trimmed();
    v.replace(static_cast<char>(0xDF), '*');
    int starIdx = v.indexOf('*');
    if (starIdx < 0) {
      return QByteArray("0");
    }
    bool okDeg = false;
    int ddd = v.left(starIdx).toInt(&okDeg);
    if (!okDeg || ddd < 0 || ddd > 359) {
      return QByteArray("0");
    }
    bool okMin = false;
    int mm = v.mid(starIdx + 1).toInt(&okMin);
    if (!okMin || mm < 0 || mm >= 60) {
      return QByteArray("0");
    }
    m_longitudeDeg =
        -(static_cast<double>(ddd) + (static_cast<double>(mm) / 60.0));
    return QByteArray("1");
  }

  if (cmd == ":GVP") {
    // Product name
    return QByteArray("DWARFII#");
  }

  if (cmd == ":GVN") {
    // Firmware / version
    return QByteArray("0.1#");
  }

  if (cmd == ":GL") {
    // Get local time HH:MM:SS#
    return QTime::currentTime().toString("HH:mm:ss#").toLatin1();
  }

  if (cmd == ":GS") {
    // Get sidereal time HH:MM:SS#
    QDateTime now = QDateTime::currentDateTimeUtc();
    double jd = now.toMSecsSinceEpoch() / 86400000.0 + 2440587.5;
    double d = jd - 2451545.0;
    // GMST in hours
    double gmst = 18.697374558 + 24.06570982441908 * d;
    gmst = std::fmod(gmst, 24.0);
    if (gmst < 0)
      gmst += 24.0;

    // LMST = GMST + longitudeEastHours
    double longitudeHours = m_longitudeDeg / 15.0;
    double lmst = gmst + longitudeHours;
    lmst = std::fmod(lmst, 24.0);
    if (lmst < 0)
      lmst += 24.0;

    int hh = static_cast<int>(std::floor(lmst));
    double remHours = lmst - hh;
    int mm = static_cast<int>(std::floor(remHours * 60.0));
    int ss = static_cast<int>(std::round((remHours * 60.0 - mm) * 60.0));
    if (ss >= 60) {
      ss = 0;
      mm++;
    }
    if (mm >= 60) {
      mm = 0;
      hh = (hh + 1) % 24;
    }

    return QString::asprintf("%02d:%02d:%02d#", hh, mm, ss).toLatin1();
  }

  if (cmd == ":GC") {
    // Get calendar date MM/DD/YY#
    return QDate::currentDate().toString("MM/dd/yy#").toLatin1();
  }

  if (cmd == ":GZ") {
    // Get Azimuth DDD*MM#
    double az = 0.0; // Current azimuth
    int ddd = static_cast<int>(std::floor(az));
    int mm = static_cast<int>(std::round((az - ddd) * 60.0));
    return QString::asprintf("%03d*%02d#", ddd, mm).toLatin1();
  }

  if (cmd == ":GA") {
    // Get Altitude sDD*MM#
    double alt = 0.0; // Current altitude
    char sign = alt < 0 ? '-' : '+';
    alt = std::abs(alt);
    int dd = static_cast<int>(std::floor(alt));
    int mm = static_cast<int>(std::round((alt - dd) * 60.0));
    return QString::asprintf("%c%02d*%02d#", sign, dd, mm).toLatin1();
  }

  if (cmd == ":Mn" || cmd == ":Ms" || cmd == ":Me" || cmd == ":Mw") {
    // Start movement - effectively ignored for now but acknowledged
    return QByteArray();
  }

  if (cmd == ":RC" || cmd == ":RG" || cmd == ":RM" || cmd == ":RS") {
    // Set movement rates - ignored
    return QByteArray();
  }

  if (cmd == ":GG") {
    // Get UTC offset sHH# (West is positive)
    int offsetSeconds = QDateTime::currentDateTime().offsetFromUtc();
    int offsetHours = -offsetSeconds / 3600;
    return QString::asprintf("%+03d#", offsetHours).toLatin1();
  }

  if (cmd == ":D") {
    // Distance bars - return empty (not supported)
    return QByteArray("#");
  }

  // Unknown: no response (many clients tolerate)
  qDebug() << "[LX200] Unknown command" << cmd << "value" << value;
  return QByteArray();
}

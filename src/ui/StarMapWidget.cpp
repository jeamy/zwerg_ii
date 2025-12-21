#include "StarMapWidget.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QPen>
#include <QPainterPath>
#include <QRadialGradient>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>
#include <QtMath>
#include <QDebug>
#include <QRegularExpression>

namespace {
// Astronomical constants
constexpr double DEG_TO_RAD = M_PI / 180.0;
constexpr double RAD_TO_DEG = 180.0 / M_PI;
constexpr double HOURS_TO_DEG = 15.0;

double bvFromSpect(const QString &spect) {
    if (spect.isEmpty())
        return 0.5;

    const QChar c = spect.trimmed().toUpper().at(0);
    // Rough typical B-V per spectral class
    if (c == 'O') return -0.32;
    if (c == 'B') return -0.20;
    if (c == 'A') return 0.00;
    if (c == 'F') return 0.30;
    if (c == 'G') return 0.58;
    if (c == 'K') return 0.90;
    if (c == 'M') return 1.40;
    return 0.5;
}

// Star colors based on spectral type (simplified)
QColor starColor(double bv) {
    // B-V color index to RGB
    if (bv < -0.3) return QColor(155, 176, 255);      // O/B - Blue
    if (bv < 0.0) return QColor(170, 191, 255);       // A - Blue-white
    if (bv < 0.3) return QColor(202, 215, 255);       // F - White
    if (bv < 0.6) return QColor(255, 244, 234);       // G - Yellow-white
    if (bv < 1.0) return QColor(255, 210, 161);       // K - Orange
    return QColor(255, 189, 111);                      // M - Red-orange
}

// Star size based on magnitude
double starRadius(double mag, double magLimit) {
    // Brighter stars (lower mag) are larger
    double normalized = (magLimit - mag) / magLimit;
    return qMax(1.0, 1.0 + normalized * 4.0);
}

double labelMagLimit(double zoomLevel) {
    // More labels when zooming in.
    // zoom 0.5 -> ~1.8, zoom 1.0 -> ~2.2, zoom 2.0 -> ~3.5, zoom 5.0 -> ~5.5
    return qBound(1.5, 2.2 + (zoomLevel - 1.0) * 1.3, 6.0);
}

int labelFontSize(double zoomLevel, double mag) {
    // Slightly bigger font for brighter stars and when zoomed in.
    const int base = (mag < 1.0) ? 9 : (mag < 2.0 ? 8 : 7);
    const int zoomBoost = (zoomLevel >= 3.0) ? 2 : (zoomLevel >= 1.8 ? 1 : 0);
    return qBound(7, base + zoomBoost, 11);
}

QColor labelColorForMag(double mag) {
    // Fade labels with magnitude
    const int alpha = qBound(60, static_cast<int>(220 - mag * 35.0), 230);
    return QColor(210, 210, 235, alpha);
}

QString normalizeMessierDisplay(QString s) {
    // Normalize Messier display: M031 / M 031 -> M31
    s = s.trimmed();
    // Also accept digit-only values like "031".
    static const QRegularExpression re1(QStringLiteral("\\bm\\s*0*([0-9]{1,3})\\b"),
                                        QRegularExpression::CaseInsensitiveOption);
    s.replace(re1, QStringLiteral("M\\1"));

    static const QRegularExpression re2(QStringLiteral("^0*([0-9]{1,3})$"));
    const auto m2 = re2.match(s);
    if (m2.hasMatch())
        return QStringLiteral("M%1").arg(m2.captured(1).toInt());

    // Normalize any remaining "M0xx" patterns without word boundaries
    static const QRegularExpression re3(QStringLiteral("^m\\s*0*([0-9]{1,3})$"),
                                        QRegularExpression::CaseInsensitiveOption);
    const auto m3 = re3.match(s);
    if (m3.hasMatch())
        return QStringLiteral("M%1").arg(m3.captured(1).toInt());
    return s;
}

QString primaryCommonName(QString s) {
    // OpenNGC common_names often contains multiple names separated by commas.
    // Keep only the first for concise labeling.
    s = s.trimmed();

    // Strip a leading Messier token from common names, e.g. "M031 Andromeda Galaxy" -> "Andromeda Galaxy"
    static const QRegularExpression leadingMessier(
        QStringLiteral("^m\\s*0*([0-9]{1,3})(?:\\b|[\t\r\n ,;:\\-])\\s*"),
        QRegularExpression::CaseInsensitiveOption);
    s.remove(leadingMessier);

    const int comma = s.indexOf(',');
    if (comma >= 0)
        s = s.left(comma);
    return s.trimmed();
}
} // namespace

StarMapWidget::StarMapWidget(QWidget *parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_updateTimer(new QTimer(this))
    , m_latitude(52.52)      // Default: Berlin
    , m_longitude(13.405)
    , m_dateTime(QDateTime::currentDateTimeUtc())
    , m_realTimeMode(true)
    , m_magnitudeLimit(6.0)  // Naked eye limit
    , m_showConstellations(true)
    , m_showGrid(true)
    , m_showLabels(true)
    , m_zoomLevel(1.0)
    , m_fovWidth(60.0)       // 1 degree default FOV
    , m_fovHeight(40.0)
    , m_telescopeRA(0.0)
    , m_telescopeDec(0.0)
    , m_showTelescopeFOV(false)
    , m_fovItem(nullptr)
{
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setBackgroundBrush(QBrush(QColor(10, 10, 30)));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::ScrollHandDrag);
    
    initializeScene();
    
    connect(m_updateTimer, &QTimer::timeout, this, &StarMapWidget::updateSky);
    m_updateTimer->start(1000); // Update every second in real-time mode
}

StarMapWidget::~StarMapWidget() = default;

void StarMapWidget::initializeScene() {
    // Initial scene rect will be adjusted dynamically in updateSky() based on zoom.
    m_scene->setSceneRect(-500, -500, 1000, 1000);
    drawHorizon();
}

void StarMapWidget::setLocation(double latitude, double longitude) {
    m_latitude = latitude;
    m_longitude = longitude;
    updateSky();
}

void StarMapWidget::setDateTime(const QDateTime &dt) {
    m_dateTime = dt;
    updateSky();
}

void StarMapWidget::setRealTimeMode(bool enabled) {
    m_realTimeMode = enabled;
    if (enabled) {
        m_dateTime = QDateTime::currentDateTimeUtc();
        m_updateTimer->start(1000);
    } else {
        m_updateTimer->stop();
    }
    updateSky();
}

void StarMapWidget::setMagnitudeLimit(double mag) {
    m_magnitudeLimit = mag;
    updateSky();
}

void StarMapWidget::setShowConstellations(bool show) {
    m_showConstellations = show;
    updateSky();
}

void StarMapWidget::setShowGrid(bool show) {
    m_showGrid = show;
    updateSky();
}

void StarMapWidget::setShowLabels(bool show) {
    m_showLabels = show;
    updateSky();
}

void StarMapWidget::setTelescopeFOV(double widthArcmin, double heightArcmin) {
    m_fovWidth = widthArcmin;
    m_fovHeight = heightArcmin;
    m_showTelescopeFOV = true;
    updateSky();
}

void StarMapWidget::setTelescopePointing(double ra, double dec) {
    m_telescopeRA = ra;
    m_telescopeDec = dec;
    updateSky();
}

bool StarMapWidget::loadCatalog(const QString &dbPath) {
    m_dbPath = dbPath;
    
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "starmap");
    db.setDatabaseName(dbPath);
    
    if (!db.open()) {
        qWarning() << "Failed to open star catalog:" << db.lastError().text();
        return false;
    }
    
    loadStarsFromDatabase();
    loadDeepSkyFromDatabase();
    loadConstellationLinesFromFile();
    
    db.close();
    QSqlDatabase::removeDatabase("starmap");
    
    updateSky();
    return true;
}

void StarMapWidget::loadStarsFromDatabase() {
    QSqlDatabase db = QSqlDatabase::database("starmap");
    QSqlQuery query(db);
    
    // Load bright stars from HYG catalog
    query.prepare(R"(
        SELECT id, proper, ra, dec, mag, spect, con, hip, bayer, flam
        FROM stars
        WHERE mag <= :magLimit
        ORDER BY mag ASC
        LIMIT 5000
    )");
    query.bindValue(":magLimit", m_magnitudeLimit + 2.0);
    
    if (!query.exec()) {
        qWarning() << "Failed to load stars:" << query.lastError().text();
        return;
    }
    
    m_stars.clear();
    while (query.next()) {
        CelestialObject star;
        star.id = query.value(0).toInt();
        const QString proper = query.value(1).toString();
        const int hip = query.value(7).toInt();
        const QString bayer = query.value(8).toString();
        const QString flam = query.value(9).toString();
        const QString spect = query.value(5).toString();

        if (!proper.isEmpty()) {
            star.name = proper;
        } else if (!bayer.isEmpty()) {
            star.name = flam.isEmpty() ? bayer : (bayer + " " + flam);
        } else if (hip > 0) {
            star.name = QString("HIP %1").arg(hip);
        } else {
            star.name = QString("Star %1").arg(star.id);
        }

        star.commonName = star.name;
        star.type = "star";
        star.hip = hip;
        star.ra = query.value(2).toDouble() * HOURS_TO_DEG; // Convert hours to degrees
        star.dec = query.value(3).toDouble();
        star.magnitude = query.value(4).toDouble();
        star.bv = bvFromSpect(spect);
        star.constellation = query.value(6).toString();
        m_stars.append(star);
    }

    m_hipToStarIndex.clear();
    for (int i = 0; i < m_stars.size(); ++i) {
        if (m_stars[i].hip > 0)
            m_hipToStarIndex.insert(m_stars[i].hip, i);
    }
    
    qDebug() << "Loaded" << m_stars.size() << "stars";
}

void StarMapWidget::loadConstellationLinesFromFile() {
    m_constellationSegments.clear();

    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/../data/constellationship.fab",
        QCoreApplication::applicationDirPath() + "/data/constellationship.fab",
        QDir::currentPath() + "/data/constellationship.fab",
        "/usr/share/zwergii/constellationship.fab"
    };

    QString path;
    for (const auto &p : searchPaths) {
        if (QFile::exists(p)) {
            path = p;
            break;
        }
    }

    if (path.isEmpty()) {
        qWarning() << "Constellation file not found. Searched paths:" << searchPaths;
        return;
    }

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open constellation file:" << path;
        return;
    }

    QTextStream in(&f);
    int segmentCount = 0;
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        // Stellarium format: <abbr> <n> <hip1> <hip2> ... (2*n hip values)
        const QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() < 3)
            continue;

        bool ok = false;
        const int n = parts[1].toInt(&ok);
        if (!ok || n <= 0)
            continue;

        const int needed = 2 * n;
        if (parts.size() < 2 + needed)
            continue;

        for (int i = 0; i < needed; i += 2) {
            bool ok1 = false;
            bool ok2 = false;
            const int hip1 = parts[2 + i].toInt(&ok1);
            const int hip2 = parts[2 + i + 1].toInt(&ok2);
            if (ok1 && ok2 && hip1 > 0 && hip2 > 0) {
                m_constellationSegments.append(qMakePair(hip1, hip2));
                ++segmentCount;
            }
        }
    }

    qDebug() << "Loaded" << segmentCount << "constellation segments from" << path;
}

void StarMapWidget::loadDeepSkyFromDatabase() {
    QSqlDatabase db = QSqlDatabase::database("starmap");
    QSqlQuery query(db);
    
    // Load deep sky objects from OpenNGC
    query.prepare(R"(
        SELECT name, ra, dec, mag, type, const, common_names, messier
        FROM dso
        WHERE mag <= :magLimit OR mag IS NULL
        ORDER BY CASE WHEN mag IS NULL THEN 1 ELSE 0 END, mag ASC
        LIMIT 2000
    )");
    query.bindValue(":magLimit", m_magnitudeLimit + 4.0);
    
    if (!query.exec()) {
        qWarning() << "Failed to load DSOs:" << query.lastError().text();
        return;
    }
    
    m_deepSkyObjects.clear();
    while (query.next()) {
        CelestialObject dso;
        dso.name = query.value(0).toString();
        dso.ra = query.value(1).toDouble();
        dso.dec = query.value(2).toDouble();
        dso.magnitude = query.value(3).isNull() ? 99.0 : query.value(3).toDouble();
        dso.type = query.value(4).toString();
        dso.constellation = query.value(5).toString();
        const QString commonNames = query.value(6).toString();
        const QString messier = query.value(7).toString();

        const QString messierNorm = normalizeMessierDisplay(messier);
        QString commonPrimary = primaryCommonName(commonNames);

        // Extra safety: if the first common name still starts with a Messier token (e.g. "M031"), strip it.
        if (!messierNorm.isEmpty() && commonPrimary.startsWith('M', Qt::CaseInsensitive)) {
            QString cp = commonPrimary;
            // Normalize leading messier in the common name and remove if it matches.
            static const QRegularExpression leadingAnyMessier(
                QStringLiteral("^m\\s*0*([0-9]{1,3})(?:\\b|[\t\r\n ,;:\\-])\\s*"),
                QRegularExpression::CaseInsensitiveOption);
            const auto m = leadingAnyMessier.match(cp);
            if (m.hasMatch()) {
                const QString leadNorm = QStringLiteral("M%1").arg(m.captured(1).toInt());
                if (leadNorm.compare(messierNorm, Qt::CaseInsensitive) == 0) {
                    cp.remove(0, m.capturedLength(0));
                    commonPrimary = cp.trimmed();
                }
            }
        }

        if (!messierNorm.isEmpty()) {
            if (!commonPrimary.isEmpty())
                dso.commonName = messierNorm + " " + commonPrimary;
            else
                dso.commonName = messierNorm;
        } else {
            dso.commonName = commonPrimary;
        }
        if (dso.commonName.isEmpty())
            dso.commonName = dso.name;
        m_deepSkyObjects.append(dso);
    }
    
    qDebug() << "Loaded" << m_deepSkyObjects.size() << "deep sky objects";
}

QVector<CelestialObject> StarMapWidget::searchObjects(const QString &query, int limit) {
    QVector<CelestialObject> results;

    auto normalizeMessier = [](QString s) {
        s = s.toLower();
        // Normalize Messier: m031 / m 031 -> m31
        static const QRegularExpression re(QStringLiteral("\\bm\\s*0*([0-9]{1,3})\\b"));
        s.replace(re, QStringLiteral("m\\1"));
        return s;
    };

    const QString lowerQuery = query.toLower();
    const QString normalizedQuery = normalizeMessier(query);
    
    // Search in loaded objects first
    for (const auto &star : m_stars) {
        const QString name = star.name.toLower();
        const QString common = star.commonName.toLower();
        if (name.contains(lowerQuery) || common.contains(lowerQuery) ||
            normalizeMessier(name).contains(normalizedQuery) ||
            normalizeMessier(common).contains(normalizedQuery)) {
            results.append(star);
            if (results.size() >= limit) return results;
        }
    }
    
    for (const auto &dso : m_deepSkyObjects) {
        const QString name = dso.name.toLower();
        const QString common = dso.commonName.toLower();
        if (name.contains(lowerQuery) || common.contains(lowerQuery) ||
            normalizeMessier(name).contains(normalizedQuery) ||
            normalizeMessier(common).contains(normalizedQuery)) {
            results.append(dso);
            if (results.size() >= limit) return results;
        }
    }
    
    return results;
}

QVector<CelestialObject> StarMapWidget::getVisibleObjects(double minAltitude) {
    QVector<CelestialObject> visible;
    
    for (auto &obj : m_stars) {
        QPointF altAz = raDecToAltAz(obj.ra, obj.dec);
        obj.altitude = altAz.x();
        obj.azimuth = altAz.y();
        obj.isVisible = obj.altitude >= minAltitude;
        if (obj.isVisible && obj.magnitude <= m_magnitudeLimit) {
            visible.append(obj);
        }
    }
    
    for (auto &obj : m_deepSkyObjects) {
        QPointF altAz = raDecToAltAz(obj.ra, obj.dec);
        obj.altitude = altAz.x();
        obj.azimuth = altAz.y();
        obj.isVisible = obj.altitude >= minAltitude;
        if (obj.isVisible) {
            visible.append(obj);
        }
    }
    
    return visible;
}

void StarMapWidget::selectObjectByName(const QString &name) {
    for (const auto &star : m_stars) {
        if (star.name == name || star.commonName == name) {
            m_selectedObject = star;
            emit objectSelected(m_selectedObject);
            centerOn(raDecToScreen(star.ra, star.dec));
            updateSky();
            return;
        }
    }
    
    for (const auto &dso : m_deepSkyObjects) {
        if (dso.name == name || dso.commonName == name) {
            m_selectedObject = dso;
            emit objectSelected(m_selectedObject);
            centerOn(raDecToScreen(dso.ra, dso.dec));
            updateSky();
            return;
        }
    }
}

double StarMapWidget::localSiderealTime() const {
    // Calculate Local Sidereal Time
    QDateTime utc = m_realTimeMode ? QDateTime::currentDateTimeUtc() : m_dateTime;
    
    // Julian Date
    int y = utc.date().year();
    int m = utc.date().month();
    int d = utc.date().day();
    double h = utc.time().hour() + utc.time().minute() / 60.0 + utc.time().second() / 3600.0;
    
    if (m <= 2) {
        y -= 1;
        m += 12;
    }
    
    double jd = qFloor(365.25 * (y + 4716)) + qFloor(30.6001 * (m + 1)) + d + h / 24.0 - 1524.5;
    double t = (jd - 2451545.0) / 36525.0;
    
    // Greenwich Mean Sidereal Time
    double gmst = 280.46061837 + 360.98564736629 * (jd - 2451545.0) + 0.000387933 * t * t;
    gmst = fmod(gmst, 360.0);
    if (gmst < 0) gmst += 360.0;
    
    // Local Sidereal Time
    double lst = gmst + m_longitude;
    lst = fmod(lst, 360.0);
    if (lst < 0) lst += 360.0;
    
    return lst;
}

QPointF StarMapWidget::raDecToAltAz(double ra, double dec) const {
    double lst = localSiderealTime();
    double ha = lst - ra; // Hour angle in degrees
    
    double haRad = ha * DEG_TO_RAD;
    double decRad = dec * DEG_TO_RAD;
    double latRad = m_latitude * DEG_TO_RAD;
    
    // Calculate altitude
    double sinAlt = qSin(decRad) * qSin(latRad) + qCos(decRad) * qCos(latRad) * qCos(haRad);
    double alt = qAsin(sinAlt) * RAD_TO_DEG;
    
    // Calculate azimuth
    double cosAz = (qSin(decRad) - qSin(latRad) * sinAlt) / (qCos(latRad) * qCos(alt * DEG_TO_RAD));
    cosAz = qBound(-1.0, cosAz, 1.0);
    double az = qAcos(cosAz) * RAD_TO_DEG;
    
    if (qSin(haRad) > 0) {
        az = 360.0 - az;
    }
    
    return QPointF(alt, az);
}

QPointF StarMapWidget::altAzToScreen(double alt, double az) const {
    // Stereographic projection centered on zenith
    double r = (90.0 - alt) * 5.0 * m_zoomLevel; // Scale factor
    double azRad = az * DEG_TO_RAD;
    
    double x = r * qSin(azRad);
    double y = -r * qCos(azRad); // North is up
    
    return QPointF(x, y);
}

QPointF StarMapWidget::raDecToScreen(double ra, double dec) const {
    QPointF altAz = raDecToAltAz(ra, dec);
    return altAzToScreen(altAz.x(), altAz.y());
}

void StarMapWidget::screenToRaDec(const QPointF &screen, double &ra, double &dec) const {
    // Inverse of altAzToScreen + raDecToAltAz.
    // 1) Screen -> Alt/Az
    const double scale = 5.0 * m_zoomLevel;
    const double r = std::hypot(screen.x(), screen.y());
    const double alt = 90.0 - (r / scale);

    double az = std::atan2(screen.x(), -screen.y()) * RAD_TO_DEG;
    if (az < 0.0)
        az += 360.0;

    // 2) Alt/Az -> RA/Dec
    const double latRad = m_latitude * DEG_TO_RAD;
    const double altRad = alt * DEG_TO_RAD;
    const double azRad = az * DEG_TO_RAD;

    const double sinAlt = std::sin(altRad);
    const double cosAlt = std::cos(altRad);
    const double sinLat = std::sin(latRad);
    const double cosLat = std::cos(latRad);

    const double sinDec = sinAlt * sinLat + cosAlt * cosLat * std::cos(azRad);
    const double decRad = std::asin(qBound(-1.0, sinDec, 1.0));
    const double cosDec = std::cos(decRad);

    double haDeg = 0.0;
    if (std::abs(cosDec) < 1e-12) {
        haDeg = 0.0;
    } else {
        const double sinH = (-std::sin(azRad) * cosAlt) / cosDec;
        const double cosH = (sinAlt - sinLat * std::sin(decRad)) / (cosLat * cosDec);
        const double haRad = std::atan2(sinH, cosH);
        haDeg = haRad * RAD_TO_DEG;
        if (haDeg < 0.0)
            haDeg += 360.0;
    }

    const double lst = localSiderealTime();
    double raDeg = lst - haDeg;
    raDeg = std::fmod(raDeg, 360.0);
    if (raDeg < 0.0)
        raDeg += 360.0;

    ra = raDeg;
    dec = decRad * RAD_TO_DEG;
}

void StarMapWidget::updateSky() {
    if (m_realTimeMode) {
        m_dateTime = QDateTime::currentDateTimeUtc();
    }

    // Preserve current view center across redraws. This is important because we clear the scene.
    const QPointF prevCenter = mapToScene(viewport()->rect().center());

    m_scene->clear();
    m_starItems.clear();
    m_fovItem = nullptr;

    // Ensure scene rect grows with zoom so panning and centerOn work at high zoom levels.
    // Use the horizon radius (alt=0) as baseline.
    const double horizonR = (90.0 - 0.0) * 5.0 * m_zoomLevel;
    const double margin = 120.0;
    const double extent = horizonR + margin;
    m_scene->setSceneRect(-extent, -extent, extent * 2.0, extent * 2.0);

    drawHorizon();
    
    if (m_showGrid) {
        drawCoordinateGrid();
    }
    
    if (m_showConstellations) {
        drawConstellationLines();
    }
    
    drawStars();
    drawDeepSkyObjects();

    drawSelectionMarker();
    
    if (m_showTelescopeFOV) {
        drawTelescopeFOV();
    }

    // Recenter to where the user was looking (unless we are at default origin and have nothing yet).
    centerOn(prevCenter);
}

void StarMapWidget::drawSelectionMarker() {
    if (m_selectedObject.name.isEmpty() && m_selectedObject.commonName.isEmpty() && m_selectedObject.id == 0)
        return;

    QPointF altAz = raDecToAltAz(m_selectedObject.ra, m_selectedObject.dec);
    const double alt = altAz.x();
    const double az = altAz.y();
    const QPointF pos = altAzToScreen(alt, az);

    const bool isDso = (m_selectedObject.type != "star");
    const double r = isDso ? 10.0 : 8.0;

    QColor c(39, 174, 96, 220);
    QPen pen(c, 2);
    m_scene->addEllipse(pos.x() - r, pos.y() - r, r * 2, r * 2, pen, Qt::NoBrush);

    QColor outer = c;
    outer.setAlpha(90);
    QPen outerPen(outer, 1);
    m_scene->addEllipse(pos.x() - (r + 3.0), pos.y() - (r + 3.0), (r + 3.0) * 2,
                        (r + 3.0) * 2, outerPen, Qt::NoBrush);
}

void StarMapWidget::drawHorizon() {
    // Draw horizon circle (altitude = 0°) using the same projection scaling as objects
    const double r = (90.0 - 0.0) * 5.0 * m_zoomLevel;
    QPen horizonPen(QColor(50, 100, 50), 2);
    m_scene->addEllipse(-r, -r, 2 * r, 2 * r, horizonPen, Qt::NoBrush);
    
    // Cardinal directions
    QFont font("Arial", 10, QFont::Bold);
    auto addCardinal = [this, &font](const QString &text, double az) {
        QPointF pos = altAzToScreen(0, az);
        auto *item = m_scene->addText(text, font);
        item->setDefaultTextColor(QColor(100, 150, 100));
        item->setPos(pos.x() - 10, pos.y() - 10);
    };
    
    addCardinal("N", 0);
    addCardinal("E", 90);
    addCardinal("S", 180);
    addCardinal("W", 270);
}

void StarMapWidget::drawCoordinateGrid() {
    // Equatorial RA/Dec grid projected into current Alt/Az view
    QPen gridPen(QColor(60, 60, 110, 140), 1, Qt::DotLine);

    // Declination lines (-60..+60)
    for (int dec = -60; dec <= 60; dec += 30) {
        QPainterPath path;
        bool started = false;
        for (int ra = 0; ra <= 360; ra += 4) {
            QPointF p = raDecToScreen(static_cast<double>(ra), static_cast<double>(dec));
            if (!started) {
                path.moveTo(p);
                started = true;
            } else {
                path.lineTo(p);
            }
        }
        m_scene->addPath(path, gridPen);
    }

    // Right Ascension lines (every 2h = 30deg)
    for (int ra = 0; ra < 360; ra += 30) {
        QPainterPath path;
        bool started = false;
        for (int dec = -75; dec <= 85; dec += 3) {
            QPointF p = raDecToScreen(static_cast<double>(ra), static_cast<double>(dec));
            if (!started) {
                path.moveTo(p);
                started = true;
            } else {
                path.lineTo(p);
            }
        }
        m_scene->addPath(path, gridPen);
    }

    // Alt/Az grid (horizon coordinates) - subtle silver-grey
    QPen altAzPen(QColor(190, 190, 200, 70), 1, Qt::DotLine);

    // Altitude circles
    for (int alt = 15; alt <= 75; alt += 15) {
        const double r = (90.0 - static_cast<double>(alt)) * 5.0 * m_zoomLevel;
        m_scene->addEllipse(-r, -r, 2 * r, 2 * r, altAzPen, Qt::NoBrush);
    }

    // Azimuth lines
    for (int az = 0; az < 360; az += 30) {
        const QPointF p1 = altAzToScreen(0.0, static_cast<double>(az));
        const QPointF p2 = altAzToScreen(90.0, static_cast<double>(az));
        m_scene->addLine(QLineF(p1, p2), altAzPen);
    }
}

void StarMapWidget::drawConstellationLines() {
    if (m_constellationSegments.isEmpty() || m_hipToStarIndex.isEmpty())
        return;

    QPen pen(QColor(120, 140, 210, 90), 1);

    for (const auto &seg : m_constellationSegments) {
        const auto it1 = m_hipToStarIndex.constFind(seg.first);
        const auto it2 = m_hipToStarIndex.constFind(seg.second);
        if (it1 == m_hipToStarIndex.constEnd() || it2 == m_hipToStarIndex.constEnd())
            continue;

        const CelestialObject &s1 = m_stars[*it1];
        const CelestialObject &s2 = m_stars[*it2];

        QPointF aa1 = raDecToAltAz(s1.ra, s1.dec);
        QPointF aa2 = raDecToAltAz(s2.ra, s2.dec);
        if (aa1.x() <= 0.0 || aa2.x() <= 0.0)
            continue;

        const QPointF p1 = altAzToScreen(aa1.x(), aa1.y());
        const QPointF p2 = altAzToScreen(aa2.x(), aa2.y());
        m_scene->addLine(QLineF(p1, p2), pen);
    }
}

void StarMapWidget::drawStars() {
    for (auto &star : m_stars) {
        QPointF altAz = raDecToAltAz(star.ra, star.dec);
        star.altitude = altAz.x();
        star.azimuth = altAz.y();
        star.isVisible = star.altitude > 0;
        
        if (!star.isVisible || star.magnitude > m_magnitudeLimit) continue;
        
        QPointF pos = altAzToScreen(star.altitude, star.azimuth);
        double radius = starRadius(star.magnitude, m_magnitudeLimit);

        QColor color = starColor(star.bv);
        QBrush brush(color);
        QPen pen(Qt::NoPen);

        // Give very bright stars a subtle halo (looks more like a star map)
        if (star.magnitude <= 1.0) {
            QRadialGradient g(pos, radius * 5.0);
            QColor core = color.lighter(140);
            core.setAlpha(230);
            QColor outer = color;
            outer.setAlpha(0);
            g.setColorAt(0.0, core);
            g.setColorAt(1.0, outer);
            brush = QBrush(g);
        }
        
        auto *item = m_scene->addEllipse(pos.x() - radius, pos.y() - radius,
                                          radius * 2, radius * 2, pen, brush);
        item->setData(0, QVariant::fromValue(star.id));
        m_starItems.append(item);

        // Labels: depend on zoom + magnitude
        if (m_showLabels && !star.name.isEmpty()) {
            const double magLimit = labelMagLimit(m_zoomLevel);

            // Avoid spamming generic HIP labels unless very zoomed
            const bool isGenericHip = star.name.startsWith("HIP ");
            const bool allowGeneric = (m_zoomLevel >= 3.0);

            if (star.magnitude <= magLimit && (!isGenericHip || allowGeneric)) {
                const int fontSize = labelFontSize(m_zoomLevel, star.magnitude);
                auto *label = m_scene->addText(star.name, QFont("Arial", fontSize));
                label->setDefaultTextColor(labelColorForMag(star.magnitude));
                label->setPos(pos.x() + radius + 3, pos.y() - (fontSize));

                // Make the brightest stars always stand out
                if (star.magnitude <= 1.0) {
                    QFont f = label->font();
                    f.setBold(true);
                    label->setFont(f);
                }
            }
        }
    }
}

void StarMapWidget::drawDeepSkyObjects() {
    for (auto &dso : m_deepSkyObjects) {
        QPointF altAz = raDecToAltAz(dso.ra, dso.dec);
        dso.altitude = altAz.x();
        dso.azimuth = altAz.y();
        dso.isVisible = dso.altitude > 0;
        
        if (!dso.isVisible) continue;
        
        QPointF pos = altAzToScreen(dso.altitude, dso.azimuth);

        const bool isMessier = dso.commonName.startsWith('M', Qt::CaseInsensitive);

        // DSOs should be subtle compared to stars; keep them small and only slightly scale with zoom.
        // Messier objects get a small highlight.
        const double baseSize = qBound(2.0, 2.4 + (m_zoomLevel - 1.0) * 0.6, 4.0);
        const double size = isMessier ? (baseSize + 0.8) : baseSize;
        
        // Different symbols for different types
        QColor color;
        
        if (dso.type.contains("Gx") || dso.type.contains("galaxy", Qt::CaseInsensitive)) {
            color = QColor(255, 200, 100);  // Yellow for galaxies
        } else if (dso.type.contains("Nb") || dso.type.contains("nebula", Qt::CaseInsensitive)) {
            color = QColor(100, 200, 255);  // Cyan for nebulae
        } else if (dso.type.contains("Cl") || dso.type.contains("cluster", Qt::CaseInsensitive)) {
            color = QColor(200, 255, 200);  // Green for clusters
        } else {
            color = QColor(200, 200, 200);  // Gray for others
        }
        
        QPen pen(color, isMessier ? 2 : 1);

        // Add subtle glow ring for Messier objects
        if (isMessier) {
            QColor ring = color.lighter(140);
            ring.setAlpha(110);
            QPen ringPen(ring, 1);
            const double ringR = size + 2.0;
            m_scene->addEllipse(pos.x() - ringR, pos.y() - ringR, ringR * 2, ringR * 2, ringPen, Qt::NoBrush);
        }

        m_scene->addEllipse(pos.x() - size, pos.y() - size, size * 2, size * 2, pen, Qt::NoBrush);
        
        // Labels: depend on zoom + magnitude (like stars)
        if (m_showLabels && !dso.commonName.isEmpty()) {
            const double magLimit = qMin(10.0, labelMagLimit(m_zoomLevel) + 2.5);
            if (dso.magnitude <= magLimit) {
                const int fontSize = qBound(7, static_cast<int>(7 + (m_zoomLevel - 1.0) * 0.8), 10);
                auto *label = m_scene->addText(dso.commonName, QFont("Arial", fontSize));
                QColor c = color.lighter(120);
                c.setAlpha(qBound(70, static_cast<int>(220 - dso.magnitude * 18.0), 220));
                label->setDefaultTextColor(c);
                label->setPos(pos.x() + size + 3, pos.y() - (fontSize));

                if (isMessier) {
                    QFont f = label->font();
                    f.setBold(true);
                    label->setFont(f);
                }
            }
        }
    }
}

void StarMapWidget::drawTelescopeFOV() {
    QPointF center = raDecToScreen(m_telescopeRA, m_telescopeDec);
    
    // Convert arcminutes to screen pixels (approximate)
    double scaleX = m_fovWidth / 60.0 * 5.0 * m_zoomLevel;
    double scaleY = m_fovHeight / 60.0 * 5.0 * m_zoomLevel;
    
    QPen fovPen(QColor(255, 100, 100), 2);
    m_fovItem = m_scene->addRect(center.x() - scaleX/2, center.y() - scaleY/2,
                                  scaleX, scaleY, fovPen, Qt::NoBrush);
}

void StarMapWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPointF scenePos = mapToScene(event->pos());

        double ra = 0.0;
        double dec = 0.0;
        screenToRaDec(scenePos, ra, dec);
        emit coordinatesClicked(ra, dec);
        
        // Find nearest object
        double minDist = 20.0; // Click tolerance
        CelestialObject *nearest = nullptr;
        
        for (auto &star : m_stars) {
            if (!star.isVisible) continue;
            QPointF objPos = altAzToScreen(star.altitude, star.azimuth);
            double dist = QLineF(scenePos, objPos).length();
            if (dist < minDist) {
                minDist = dist;
                nearest = &star;
            }
        }
        
        for (auto &dso : m_deepSkyObjects) {
            if (!dso.isVisible) continue;
            QPointF objPos = altAzToScreen(dso.altitude, dso.azimuth);
            double dist = QLineF(scenePos, objPos).length();
            if (dist < minDist) {
                minDist = dist;
                nearest = &dso;
            }
        }
        
        if (nearest) {
            m_selectedObject = *nearest;
            emit objectSelected(m_selectedObject);
        }
    }
    
    QGraphicsView::mousePressEvent(event);
}

void StarMapWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_selectedObject.id != 0) {
        emit objectDoubleClicked(m_selectedObject);
        emit gotoRequested(m_selectedObject.ra, m_selectedObject.dec);
    }
    
    QGraphicsView::mouseDoubleClickEvent(event);
}

void StarMapWidget::wheelEvent(QWheelEvent *event) {
    double factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;
    m_zoomLevel *= factor;
    m_zoomLevel = qBound(0.5, m_zoomLevel, 10.0);

    // We handle zoom purely via m_zoomLevel in the projection (altAzToScreen),
    // to avoid double-scaling with QGraphicsView transforms.
    updateSky();
    event->accept();
}

void StarMapWidget::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    // apparent movement of the horizon/grid when zooming.
}

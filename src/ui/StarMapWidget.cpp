#include "StarMapWidget.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QPen>
#include <QtMath>
#include <QDebug>

namespace {
// Astronomical constants
constexpr double DEG_TO_RAD = M_PI / 180.0;
constexpr double RAD_TO_DEG = 180.0 / M_PI;
constexpr double HOURS_TO_DEG = 15.0;

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
    , m_showGrid(false)
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
        SELECT id, proper, ra, dec, mag, spect, con
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
        star.name = query.value(1).toString();
        star.commonName = star.name;
        star.type = "star";
        star.ra = query.value(2).toDouble() * HOURS_TO_DEG; // Convert hours to degrees
        star.dec = query.value(3).toDouble();
        star.magnitude = query.value(4).toDouble();
        star.constellation = query.value(6).toString();
        m_stars.append(star);
    }
    
    qDebug() << "Loaded" << m_stars.size() << "stars";
}

void StarMapWidget::loadDeepSkyFromDatabase() {
    QSqlDatabase db = QSqlDatabase::database("starmap");
    QSqlQuery query(db);
    
    // Load deep sky objects from OpenNGC
    query.prepare(R"(
        SELECT name, ra, dec, mag, type, const, common_names
        FROM dso
        WHERE mag <= :magLimit OR mag IS NULL
        ORDER BY mag ASC NULLS LAST
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
        dso.commonName = query.value(6).toString();
        if (dso.commonName.isEmpty()) dso.commonName = dso.name;
        m_deepSkyObjects.append(dso);
    }
    
    qDebug() << "Loaded" << m_deepSkyObjects.size() << "deep sky objects";
}

QVector<CelestialObject> StarMapWidget::searchObjects(const QString &query, int limit) {
    QVector<CelestialObject> results;
    QString lowerQuery = query.toLower();
    
    // Search in loaded objects first
    for (const auto &star : m_stars) {
        if (star.name.toLower().contains(lowerQuery) ||
            star.commonName.toLower().contains(lowerQuery)) {
            results.append(star);
            if (results.size() >= limit) return results;
        }
    }
    
    for (const auto &dso : m_deepSkyObjects) {
        if (dso.name.toLower().contains(lowerQuery) ||
            dso.commonName.toLower().contains(lowerQuery)) {
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
            return;
        }
    }
    
    for (const auto &dso : m_deepSkyObjects) {
        if (dso.name == name || dso.commonName == name) {
            m_selectedObject = dso;
            emit objectSelected(m_selectedObject);
            centerOn(raDecToScreen(dso.ra, dso.dec));
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

void StarMapWidget::updateSky() {
    if (m_realTimeMode) {
        m_dateTime = QDateTime::currentDateTimeUtc();
    }
    
    m_scene->clear();
    m_starItems.clear();
    m_fovItem = nullptr;
    
    drawHorizon();
    
    if (m_showGrid) {
        drawCoordinateGrid();
    }
    
    if (m_showConstellations) {
        drawConstellationLines();
    }
    
    drawStars();
    drawDeepSkyObjects();
    
    if (m_showTelescopeFOV) {
        drawTelescopeFOV();
    }
}

void StarMapWidget::drawHorizon() {
    // Draw horizon circle
    QPen horizonPen(QColor(50, 100, 50), 2);
    m_scene->addEllipse(-450, -450, 900, 900, horizonPen, Qt::NoBrush);
    
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
    QPen gridPen(QColor(40, 40, 80), 1, Qt::DotLine);
    
    // Altitude circles
    for (int alt = 15; alt <= 75; alt += 15) {
        double r = (90.0 - alt) * 5.0 * m_zoomLevel;
        m_scene->addEllipse(-r, -r, 2*r, 2*r, gridPen, Qt::NoBrush);
    }
    
    // Azimuth lines
    for (int az = 0; az < 360; az += 30) {
        QPointF p1 = altAzToScreen(0, az);
        QPointF p2 = altAzToScreen(90, az);
        m_scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), gridPen);
    }
}

void StarMapWidget::drawConstellationLines() {
    // Simplified constellation lines - would need constellation data
    // For now, just a placeholder
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
        
        QColor color = starColor(0.5); // Simplified - would need B-V from catalog
        QBrush brush(color);
        QPen pen(Qt::NoPen);
        
        auto *item = m_scene->addEllipse(pos.x() - radius, pos.y() - radius,
                                          radius * 2, radius * 2, pen, brush);
        item->setData(0, QVariant::fromValue(star.id));
        m_starItems.append(item);
        
        // Add label for bright stars
        if (m_showLabels && star.magnitude < 2.0 && !star.name.isEmpty()) {
            auto *label = m_scene->addText(star.name, QFont("Arial", 8));
            label->setDefaultTextColor(QColor(180, 180, 200));
            label->setPos(pos.x() + radius + 2, pos.y() - 6);
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
        
        // Different symbols for different types
        QColor color;
        double size = 6.0;
        
        if (dso.type.contains("Gx") || dso.type.contains("galaxy", Qt::CaseInsensitive)) {
            color = QColor(255, 200, 100);  // Yellow for galaxies
        } else if (dso.type.contains("Nb") || dso.type.contains("nebula", Qt::CaseInsensitive)) {
            color = QColor(100, 200, 255);  // Cyan for nebulae
        } else if (dso.type.contains("Cl") || dso.type.contains("cluster", Qt::CaseInsensitive)) {
            color = QColor(200, 255, 200);  // Green for clusters
        } else {
            color = QColor(200, 200, 200);  // Gray for others
        }
        
        QPen pen(color, 1);
        m_scene->addEllipse(pos.x() - size, pos.y() - size, size * 2, size * 2, pen, Qt::NoBrush);
        
        // Add label for named objects
        if (m_showLabels && !dso.commonName.isEmpty()) {
            auto *label = m_scene->addText(dso.commonName, QFont("Arial", 7));
            label->setDefaultTextColor(color.lighter(120));
            label->setPos(pos.x() + size + 2, pos.y() - 5);
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
    m_zoomLevel = qBound(0.5, m_zoomLevel, 5.0);
    
    scale(factor, factor);
    event->accept();
}

void StarMapWidget::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

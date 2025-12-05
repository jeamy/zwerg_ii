#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QDateTime>
#include <QPointF>
#include <QTimer>
#include <QVector>

class QSqlDatabase;

/**
 * @brief Celestial object data structure
 */
struct CelestialObject {
    int id;
    QString name;
    QString commonName;
    QString type;           // "star", "galaxy", "nebula", "cluster", "planet"
    double ra;              // Right Ascension in degrees (0-360)
    double dec;             // Declination in degrees (-90 to +90)
    double magnitude;       // Visual magnitude (lower = brighter)
    QString constellation;
    QString description;
    
    // Computed values
    double altitude;        // Current altitude above horizon
    double azimuth;         // Current azimuth
    bool isVisible;         // Currently above horizon
};

/**
 * @brief Interactive star map widget showing the night sky
 * 
 * Displays stars, deep sky objects, and planets based on observer location
 * and current time. Supports panning, zooming, and object selection.
 */
class StarMapWidget : public QGraphicsView {
    Q_OBJECT

public:
    explicit StarMapWidget(QWidget *parent = nullptr);
    ~StarMapWidget();

    // Observer location
    void setLocation(double latitude, double longitude);
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }

    // Time settings
    void setDateTime(const QDateTime &dt);
    QDateTime dateTime() const { return m_dateTime; }
    void setRealTimeMode(bool enabled);
    bool isRealTimeMode() const { return m_realTimeMode; }

    // Display settings
    void setMagnitudeLimit(double mag);
    double magnitudeLimit() const { return m_magnitudeLimit; }
    void setShowConstellations(bool show);
    void setShowGrid(bool show);
    void setShowLabels(bool show);

    // Telescope FOV overlay
    void setTelescopeFOV(double widthArcmin, double heightArcmin);
    void setTelescopePointing(double ra, double dec);

    // Object selection
    CelestialObject selectedObject() const { return m_selectedObject; }
    void selectObjectByName(const QString &name);

    // Database
    bool loadCatalog(const QString &dbPath);
    QVector<CelestialObject> searchObjects(const QString &query, int limit = 20);
    QVector<CelestialObject> getVisibleObjects(double minAltitude = 10.0);

signals:
    void objectSelected(const CelestialObject &obj);
    void objectDoubleClicked(const CelestialObject &obj);
    void gotoRequested(double ra, double dec);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateSky();

private:
    void initializeScene();
    void loadStarsFromDatabase();
    void loadDeepSkyFromDatabase();
    void drawStars();
    void drawDeepSkyObjects();
    void drawConstellationLines();
    void drawCoordinateGrid();
    void drawHorizon();
    void drawTelescopeFOV();
    
    // Coordinate conversions
    QPointF raDecToAltAz(double ra, double dec) const;
    QPointF altAzToScreen(double alt, double az) const;
    QPointF raDecToScreen(double ra, double dec) const;
    void screenToRaDec(const QPointF &screen, double &ra, double &dec) const;
    
    // Astronomical calculations
    double localSiderealTime() const;
    void updateObjectPositions();
    
    // Find object at screen position
    CelestialObject* objectAtPosition(const QPointF &pos);

    QGraphicsScene *m_scene;
    QTimer *m_updateTimer;
    
    // Observer location
    double m_latitude;
    double m_longitude;
    
    // Time
    QDateTime m_dateTime;
    bool m_realTimeMode;
    
    // Display settings
    double m_magnitudeLimit;
    bool m_showConstellations;
    bool m_showGrid;
    bool m_showLabels;
    double m_zoomLevel;
    
    // Telescope FOV
    double m_fovWidth;
    double m_fovHeight;
    double m_telescopeRA;
    double m_telescopeDec;
    bool m_showTelescopeFOV;
    
    // Data
    QVector<CelestialObject> m_stars;
    QVector<CelestialObject> m_deepSkyObjects;
    CelestialObject m_selectedObject;
    
    // Graphics items for efficient updates
    QVector<QGraphicsEllipseItem*> m_starItems;
    QGraphicsRectItem *m_fovItem;
    
    // Database
    QString m_dbPath;
};

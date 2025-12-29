#include "HelpWindow.h"
#include <QLabel>
#include <QScrollArea>

HelpWindow::HelpWindow(QWidget *parent) : QWidget(parent, Qt::Window) {
    setWindowTitle(tr("zwergII - Help / Hilfe"));
    resize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *topLayout = new QHBoxLayout();
    m_languageCombo = new QComboBox();
    m_languageCombo->addItem("English", "en");
    m_languageCombo->addItem("Deutsch", "de");
    topLayout->addWidget(new QLabel(tr("Language / Sprache:")));
    topLayout->addWidget(m_languageCombo);
    topLayout->addStretch();
    
    mainLayout->addLayout(topLayout);

    m_browser = new QTextBrowser();
    m_browser->setOpenExternalLinks(true);
    mainLayout->addWidget(m_browser);

    connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HelpWindow::updateContent);

    updateContent();
}

void HelpWindow::onLanguageChanged(int index) {
    updateContent();
}

void HelpWindow::updateContent() {
    QString lang = m_languageCombo->currentData().toString();
    QString html;

    if (lang == "de") {
        html = R"(
            <style>
                h1 { color: #27ae60; border-bottom: 1px solid #333; padding-bottom: 5px; }
                h3 { color: #2ecc71; margin-top: 20px; }
                b { color: #eee; }
                ul { margin-left: 20px; }
                li { margin-bottom: 8px; }
                .note { color: #aaa; font-style: italic; font-size: 0.9em; }
            </style>
            <h1>zwergII - Detaillierte Anleitung</h1>
            
            <h3>1. Verbindung (SCAN & SET)</h3>
            <p>Um das DWARF II zu steuern, ist eine aktive Netzwerkverbindung erforderlich.</p>
            <ul>
                <li><b>Netzwerk-Scan:</b> Klicke auf 'Scan Network', um das Teleskop automatisch im lokalen WLAN zu finden. Stelle sicher, dass dein PC mit dem DWARF-WLAN (Dwarf_XXXX) verbunden ist.</li>
                <li><b>Manuelle IP:</b> Falls der Scan fehlschlägt, gib die IP-Adresse (Standard meist 192.168.8.223 oder 10.1.1.102) direkt ein.</li>
                <li><b>Verbindungsstatus:</b> Die Statusleiste unten zeigt den aktuellen Ping und die Verbindungsqualität an.</li>
            </ul>

            <h3>2. Kamera & Live-Stream (CAM)</h3>
            <p>Zentrale Steuerung für beide Sensoren des DWARF II.</p>
            <ul>
                <li><b>Tele / Wide:</b> Die Tele-Kamera (Astro) und die Weitwinkel-Kamera (Orientierung) können gleichzeitig genutzt werden.</li>
                <li><b>Picture-in-Picture (PiP):</b> Klicke doppelt auf das kleine Fenster, um die Kameras zu tauschen. Du kannst das PiP-Fenster beliebig verschieben.</li>
                <li><b>Belichtung (Exposure):</b>
                    <ul>
                        <li><i>Auto:</i> Das System regelt Helligkeit automatisch (gut für Tag).</li>
                        <li><i>Manuell:</i> Shutter (Zeit) und Gain (ISO) können für Astro-Aufnahmen fest eingestellt werden.</li>
                    </ul>
                </li>
                <li><b>IR-Cut Filter:</b>
                    <ul>
                        <li><i>CUT:</i> Blockiert Infrarot (natürliche Farben, für Tag).</li>
                        <li><i>PASS:</i> Lässt Infrarot durch (für Astrofotografie erforderlich).</li>
                    </ul>
                </li>
                <li><b>Bildparameter:</b> Kontrast, Sättigung, Schärfe und Weißabgleich können über das schwebende Overlay-Panel in Echtzeit angepasst werden.</li>
            </ul>

            <h3>3. Astronomie & Navigation (ASTRO)</h3>
            <p>Die Hauptfunktion für Deep-Sky Beobachtungen.</p>
            <ul>
                <li><b>Sternkarte:</b> Eine interaktive OpenGL-Karte des Himmels. Nutze das Mausrad zum Zoomen und ziehe mit der Maus zum Navigieren.</li>
                <li><b>Objektsuche:</b> Über den Suchen-Tab findest du Deep-Sky-Objekte (Messier, NGC) und Sterne.</li>
                <li><b>GOTO:</b> Wähle ein Objekt aus und klicke auf 'GOTO'. Das Teleskop fährt das Ziel automatisch an.
                    <br><span class='note'>Hinweis: Das Teleskop muss zuvor kalibriert sein!</span></li>
                <li><b>Kalibrierung:</b> Führe eine 3-Punkt-Kalibrierung durch, damit das Teleskop seine Orientierung am Himmel kennt.</li>
                <li><b>LX200 Server:</b> Aktiviere diesen im Unter-Reiter 'Settings' der Astronomie-Seite.
                    <ul>
                        <li>Ermöglicht die Fernsteuerung durch Apps wie <b>SkySafari</b> oder <b>Stellarium</b>.</li>
                        <li>Verwende die IP deines PCs und Port 4030 in der externen App.</li>
                    </ul>
                </li>
                <li><b>Standort & Anzeige:</b> Ebenfalls im Astro-Settings-Tab kannst du Koordinaten setzen oder das Himmelsgitter einblenden.</li>
                <li><b>Live Stacking:</b> Startet die Aufnahme von mehreren Bildern hintereinander. Diese werden direkt im Gerät kombiniert, um Rauschen zu minimieren. Die App zeigt den Fortschritt und die Anzahl der akzeptierten Frames an.</li>
            </ul>

            <h3>4. Panorama (PANO)</h3>
            <p>Erstellung von Mosaik-Aufnahmen mit der Weitwinkel-Kamera.</p>
            <ul>
                <li><b>Raster-Einstellung:</b> Wähle wie viele Bilder horizontal und vertikal aufgenommen werden sollen (z.B. 3x3 oder 5x5).</li>
                <li><b>Vorschau:</b> Das Raster wird auf dem Live-Stream eingeblendet, um den Bildausschnitt zu kontrollieren.</li>
                <li><b>Ablauf:</b> Das Teleskop fährt alle Positionen automatisch ab und speichert das Ergebnis auf der SD-Karte.</li>
            </ul>

            <h3>5. Motor & Fokus (Overlays)</h3>
            <p>Feinsteuerung der Hardware-Komponenten.</p>
            <ul>
                <li><b>Virtueller Joystick:</b> Über das schwebende Overlay kannst du das Teleskop in alle Richtungen bewegen. Die Geschwindigkeit (deg/s) ist stufenlos einstellbar.</li>
                <li><b>Fokus:</b>
                    <ul>
                        <li><i>Auto-Fokus:</i> Das System sucht automatisch die optimale Schärfe.</li>
                        <li><i>Manuelle Schritte:</i> Nutze die Pfeiltasten für extrem feine Justierung (wichtig für Planeten und Sterne).</li>
                    </ul>
                </li>
            </ul>

            <h3>6. Galerie & Medien (GAL)</h3>
            <p>Verwaltung der Aufnahmen auf dem Teleskop.</p>
            <ul>
                <li><b>Browser:</b> Durchsuche Fotos, Videos, Astro-Stacks und Panoramen direkt auf der SD-Karte.</li>
                <li><b>Download:</b> Lade Bilder direkt auf deinen PC herunter. Den Zielordner kannst du im SET-Tab festlegen.</li>
                <li><b>Lightbox:</b> Klicke auf ein Bild für eine Vollbild-Vorschau.</li>
            </ul>

            <h3>7. Einstellungen (SET)</h3>
            <p>Allgemeine Konfiguration der Anwendung.</p>
            <ul>
                <li><b>Download-Ordner:</b> Zielverzeichnis für Medien-Downloads.</li>
                <li><b>Sprache:</b> Wechsel zwischen DE/EN (Neustart erforderlich).</li>
                <li><b>System-Info:</b> Live-Daten (Akku, Firmware, SD-Karte).</li>
                <li><span class='note'>Wichtig: Spezifische Astro-Einstellungen (LX200, Standort) befinden sich direkt im ASTRO-Tab unter 'Settings'.</span></li>
            </ul>

            <hr>
            <p><i>Vibe coding mit Unterstützung von Windsurf und verschiedenen KI-Modellen.</i></p>
        )";
    } else {
        html = R"(
            <style>
                h1 { color: #27ae60; border-bottom: 1px solid #333; padding-bottom: 5px; }
                h3 { color: #2ecc71; margin-top: 20px; }
                b { color: #eee; }
                ul { margin-left: 20px; }
                li { margin-bottom: 8px; }
                .note { color: #aaa; font-style: italic; font-size: 0.9em; }
            </style>
            <h1>zwergII - Detailed Instructions</h1>
            
            <h3>1. Connection (SCAN & SET)</h3>
            <p>Control of the DWARF II requires an active network connection.</p>
            <ul>
                <li><b>Network Scan:</b> Click 'Scan Network' to automatically find the telescope on your local Wi-Fi. Ensure your PC is connected to the DWARF Wi-Fi (Dwarf_XXXX).</li>
                <li><b>Manual IP:</b> If the scan fails, enter the IP address (default is usually 192.168.8.223 or 10.1.1.102) manually.</li>
                <li><b>Connection Status:</b> The bottom status bar shows current ping and connection quality.</li>
            </ul>

            <h3>2. Camera & Live-Stream (CAM)</h3>
            <p>Central control for both sensors of the DWARF II.</p>
            <ul>
                <li><b>Tele / Wide:</b> Use the Tele camera (Astro) and the Wide-angle camera (Orientation) simultaneously.</li>
                <li><b>Picture-in-Picture (PiP):</b> Double-click the small window to swap cameras. You can drag the PiP window to any position.</li>
                <li><b>Exposure Control:</b>
                    <ul>
                        <li><i>Auto:</i> The system adjusts brightness automatically (good for daytime).</li>
                        <li><i>Manual:</i> Set shutter speed and Gain (ISO) fixed for astrophotography.</li>
                    </ul>
                </li>
                <li><b>IR-Cut Filter:</b>
                    <ul>
                        <li><i>CUT:</i> Blocks infrared (natural colors, for daytime).</li>
                        <li><i>PASS:</i> Passes infrared (required for astrophotography).</li>
                    </ul>
                </li>
                <li><b>Image Parameters:</b> Fine-tune contrast, saturation, sharpness, and white balance in real-time using the floating overlay panel.</li>
            </ul>

            <h3>3. Astronomy & Navigation (ASTRO)</h3>
            <p>Main features for deep-sky observations.</p>
            <ul>
                <li><b>Star Map:</b> An interactive OpenGL sky map. Use the scroll wheel to zoom and drag with the mouse to navigate.</li>
                <li><b>Object Search:</b> Find deep-sky objects (Messier, NGC) and stars in the search tab.</li>
                <li><b>GOTO:</b> Select an object and click 'GOTO'. The telescope will automatically slew to the target.
                    <br><span class='note'>Note: The telescope must be calibrated first!</span></li>
                <li><b>Calibration:</b> Perform a 3-point calibration so the telescope knows its orientation in the sky.</li>
                <li><b>LX200 Server:</b> Enable this in the 'Settings' sub-tab of the Astronomy page.
                    <ul>
                        <li>Allows control via apps like <b>SkySafari</b> or <b>Stellarium</b>.</li>
                        <li>Use your PC's IP and port 4030 in the external app.</li>
                    </ul>
                </li>
                <li><b>Location & Display:</b> Also found in Astro Settings. Set coordinates or toggle the celestial grid.</li>
                <li><b>Live Stacking:</b> Starts capturing multiple images. They are combined directly in the device to minimize noise. The app shows progress and the number of accepted frames.</li>
            </ul>

            <h3>4. Panorama (PANO)</h3>
            <p>Create mosaic shots using the Wide-angle camera.</p>
            <ul>
                <li><b>Grid Setup:</b> Choose how many images to take horizontally and vertically (e.g., 3x3 or 5x5).</li>
                <li><b>Preview:</b> The grid is overlaid on the live stream to help check the field of view.</li>
                <li><b>Process:</b> The telescope automatically scans all positions and saves the result to the SD card.</li>
            </ul>

            <h3>5. Motor & Focus (Overlays)</h3>
            <p>Fine control over hardware components.</p>
            <ul>
                <li><b>Virtual Joystick:</b> Use the floating overlay to move the telescope in all directions. Speed (deg/s) is adjustable.</li>
                <li><b>Focus:</b>
                    <ul>
                        <li><i>Auto-Focus:</i> The system automatically finds the optimal sharpness.</li>
                        <li><i>Manual Steps:</i> Use arrow keys for extremely fine adjustment (essential for planets and stars).</li>
                    </ul>
                </li>
            </ul>

            <h3>6. Gallery & Media (GAL)</h3>
            <p>Manage recordings stored on the telescope.</p>
            <ul>
                <li><b>Browser:</b> Browse photos, videos, astro-stacks, and panoramas directly on the SD card.</li>
                <li><b>Download:</b> Download images directly to your PC. Set the destination folder in the SET tab.</li>
                <li><b>Lightbox:</b> Click an image for a full-screen preview.</li>
            </ul>

            <h3>7. Settings (SET)</h3>
            <p>General application configuration.</p>
            <ul>
                <li><b>Download Folder:</b> Destination for media downloads.</li>
                <li><b>Language:</b> Switch between EN/DE (restart required).</li>
                <li><b>System Info:</b> Live data (Battery, Firmware, SD Space).</li>
                <li><span class='note'>Note: Specific Astro settings (LX200, Location) are located directly within the ASTRO tab under 'Settings'.</span></li>
            </ul>

            <hr>
            <p><i>Vibe coding supported by Windsurf and various AI models.</i></p>
        )";
    }

    m_browser->setHtml(html);
}

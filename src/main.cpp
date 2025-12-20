#include "MainWindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QLocale>
#include <QLoggingCategory>
#include <QTextStream>
#include <QTranslator>
#include <iostream>

int main(int argc, char *argv[]) {
  // Disable Qt's verbose debug output, only show warnings and critical
  QLoggingCategory::setFilterRules("qt.*=false\n*.debug=false");
  qSetMessagePattern("[%{if-category}%{category}: %{endif}%{message}]");

  fprintf(stderr, "=== DWARF II Controller starting ===\n");
  fflush(stderr);

  // Some systems/drivers fail EGL/DRI2 init and can crash early in Qt's GL setup.
  // Force software OpenGL as a stable fallback.
  qputenv("QT_XCB_GL_INTEGRATION", "none");
  qputenv("QT_OPENGL", "software");
  qputenv("LIBGL_ALWAYS_SOFTWARE", "1");
  QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);

  QApplication app(argc, argv);

  QTranslator translator;
  const QString languageCode = QLocale::system().name().left(2);
  if (languageCode == "de") {
    const QString baseName = QStringLiteral("DwarfController_de");
    const QString i18nDir = QCoreApplication::applicationDirPath() + "/i18n";
    if (translator.load(baseName, i18nDir)) {
      app.installTranslator(&translator);
    }
  }

  // Load global application style sheet
  QFile styleFile(QCoreApplication::applicationDirPath() + "/styles/app.qss");
  if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&styleFile);
    app.setStyleSheet(in.readAll());
  }

  // Apply Dark Theme
  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
  darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);

  app.setPalette(darkPalette);
  app.setStyle("Fusion");

  MainWindow w;
  w.show();

  return app.exec();
}

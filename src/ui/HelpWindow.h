#pragma once

#include <QWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>

class HelpWindow : public QWidget {
    Q_OBJECT
public:
    explicit HelpWindow(QWidget *parent = nullptr);

private slots:
    void updateContent();
    void onLanguageChanged(int index);

private:
    QTextBrowser *m_browser;
    QComboBox *m_languageCombo;
    void setupHelpContent();
};

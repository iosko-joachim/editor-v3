#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    // Helle Palette erzwingen (verhindert Dark-Theme-Übernahme)
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor("#f0f0f0"));
    lightPalette.setColor(QPalette::WindowText, QColor("#000000"));
    lightPalette.setColor(QPalette::Base, QColor("#ffffff"));
    lightPalette.setColor(QPalette::AlternateBase, QColor("#f5f5f5"));
    lightPalette.setColor(QPalette::Text, QColor("#000000"));
    lightPalette.setColor(QPalette::Button, QColor("#f0f0f0"));
    lightPalette.setColor(QPalette::ButtonText, QColor("#000000"));
    lightPalette.setColor(QPalette::ToolTipBase, QColor("#ffffdc"));
    lightPalette.setColor(QPalette::ToolTipText, QColor("#000000"));
    lightPalette.setColor(QPalette::PlaceholderText, QColor("#808080"));
    lightPalette.setColor(QPalette::Highlight, QColor("#5b7fb5"));
    lightPalette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    app.setPalette(lightPalette);

    // Globales Stylesheet — eigenständiges Farbkonzept (Slate-Blau)
    app.setStyleSheet(
        "QToolBar { "
        "    background-color: #f5f6fa; "
        "    border-bottom: 1px solid #d1d5db; "
        "    spacing: 2px; "
        "    padding: 3px 6px; "
        "} "
        "QToolBar QToolButton { "
        "    background: transparent; "
        "    border: 1px solid transparent; "
        "    border-radius: 3px; "
        "    padding: 3px; "
        "    margin: 1px; "
        "    min-width: 28px; "
        "    min-height: 28px; "
        "} "
        "QToolBar QToolButton:hover { "
        "    background-color: #e2e6ed; "
        "    border: 1px solid #b0b8c9; "
        "} "
        "QToolBar QToolButton:pressed { "
        "    background-color: #cbd2de; "
        "} "
        "QToolBar QToolButton:checked { "
        "    background-color: #dbe1ec; "
        "    border: 1px solid #98a4ba; "
        "} "
    );

    MainWindow window;
    window.resize(1100, 750);
    window.show();

    return app.exec();
}

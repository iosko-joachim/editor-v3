#include "richtexteditor.h"
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QScrollBar>

RichTextEditor::RichTextEditor(QWidget *parent)
    : QTextEdit(parent)
{
    setAcceptRichText(true);

    // Standardschrift
    QFont editorFont("Segoe UI", 12);
    setFont(editorFont);
    document()->setDefaultFont(editorFont);

    // Textfarbe explizit schwarz
    QTextCharFormat defaultFmt;
    defaultFmt.setForeground(QColor("#000000"));
    QTextCursor c(document());
    c.select(QTextCursor::Document);
    c.mergeCharFormat(defaultFmt);
    setCurrentCharFormat(defaultFmt);

    // Tab-Stopp
    setTabStopDistance(40.0);

    // Zeilenumbruch am Widget-Rand
    setLineWrapMode(QTextEdit::WidgetWidth);
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    // Grauer Hintergrund — die "Seite" wird in paintEvent gezeichnet
    setStyleSheet(
        "QTextEdit {"
        "    background-color: #e8eaed;"
        "    color: #000000;"
        "    border: none;"
        "    selection-background-color: #5b7fb5;"
        "    selection-color: #ffffff;"
        "}"
    );

    // Dokumentenränder für den Papier-Innenraum
    document()->setDocumentMargin(20);

    // Keystroke-Timer starten
    m_elapsed.start();
}

NgramAnalyzer &RichTextEditor::analyzer()
{
    return m_analyzer;
}

const NgramAnalyzer &RichTextEditor::analyzer() const
{
    return m_analyzer;
}

void RichTextEditor::resetTracking()
{
    m_analyzer.clear();
    m_elapsed.restart();
}

void RichTextEditor::keyPressEvent(QKeyEvent *event)
{
    qint64 now = m_elapsed.elapsed();

    if (event->key() == Qt::Key_Backspace) {
        m_analyzer.addBackspace(now);
    } else {
        QChar ch = event->text().isEmpty() ? QChar() : event->text().at(0);
        if (ch.isPrint()) {
            m_analyzer.addKeystroke(ch, now, -1);
        }
    }

    // Normales QTextEdit-Verhalten (Enter, Buchstaben, etc.)
    QTextEdit::keyPressEvent(event);
}

void RichTextEditor::keyReleaseEvent(QKeyEvent *event)
{
    if (!event->isAutoRepeat()) {
        QChar ch = event->text().isEmpty() ? QChar() : event->text().at(0);
        if (ch.isPrint()) {
            qint64 now = m_elapsed.elapsed();
            m_analyzer.patchLastUpMs(ch, now);
        }
    }
    QTextEdit::keyReleaseEvent(event);
}

void RichTextEditor::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);
    updatePageLayout();
}

void RichTextEditor::updatePageLayout()
{
    // Guard: verhindert Endlosrekursion
    if (m_inLayoutUpdate)
        return;
    m_inLayoutUpdate = true;

    // Seitenbreite: 700px oder Viewport-Breite (was kleiner ist)
    // Die Viewport-Margins zentrieren die "Seite"
    int pageWidth = qMin(700, viewport()->width() - 20);
    int marginH = qMax(10, (viewport()->width() - pageWidth) / 2);

    // Viewport-Margins setzen den sichtbaren "Papier"-Bereich
    setViewportMargins(marginH, 30, marginH, 30);

    // Dokument-Breite an den verfügbaren Platz anpassen
    // So weiß QTextDocument wo es umbrechen soll
    document()->setTextWidth(pageWidth - 2 * document()->documentMargin());

    m_inLayoutUpdate = false;
}

void RichTextEditor::paintEvent(QPaintEvent *event)
{
    // 1) Weißes Rechteck als "Papier-Seite" hinter dem Text zeichnen
    QPainter bgPainter(viewport());
    bgPainter.fillRect(viewport()->rect(), QColor("#ffffff"));

    // Dünner Rahmen um die Seite
    bgPainter.setPen(QPen(QColor("#d0d0d0"), 1));
    bgPainter.drawRect(viewport()->rect().adjusted(0, 0, -1, -1));
    bgPainter.end();

    // 2) Normales QTextEdit-Rendering (Text, Cursor, Selektion)
    QTextEdit::paintEvent(event);
}

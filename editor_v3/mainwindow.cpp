#include "mainwindow.h"
#include "specialchardialog.h"
#include "tabledialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextCursor>
#include <QTextList>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QFontDatabase>
#include <QPainter>
#include <QIcon>
#include <QPixmap>
#include <QApplication>
#include <QStyle>
#include <QColorDialog>
#include <QPushButton>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QMessageBox>
#include <QDialog>
#include <QTextEdit>
#include <algorithm>
#include <utility>

// QtCharts
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

// ─── Hilfsfunktion: Icon aus Unicode-Text erzeugen ───────────────────────────
static QIcon textIcon(const QString &text, int size = 24,
                      const QColor &color = QColor(50, 50, 50),
                      bool bold = false)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QFont f("Segoe UI", size * 0.52);
    if (bold) f.setBold(true);
    p.setFont(f);
    p.setPen(color);
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, text);
    return QIcon(pix);
}

// ─── Farbiges Rechteck-Icon (für Highlight) ─────────────────────────────────
static QIcon colorRectIcon(const QColor &color, int size = 24)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(60, 60, 60), 1.5));
    p.drawLine(size / 2, 2, size / 2, size - 8);
    p.drawLine(size / 2 - 3, size - 8, size / 2 + 3, size - 8);
    p.setBrush(color);
    p.setPen(Qt::NoPen);
    p.drawRect(2, size - 6, size - 4, 5);
    return QIcon(pix);
}

// ─── Ausrichtungs-Icon (Linien-Symbol) ──────────────────────────────────────
static QIcon alignIcon(const QString &type, int size = 24)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(50, 50, 50), 1.8));

    int y = 4, step = 5, margin = 3;
    int fullW = size - 2 * margin;

    for (int i = 0; i < 4; ++i) {
        int lineW, x = margin;
        if (type == "left") {
            lineW = (i % 2 == 0) ? fullW : fullW * 2 / 3;
            x = margin;
        } else if (type == "center") {
            lineW = (i % 2 == 0) ? fullW : fullW * 2 / 3;
            x = margin + (fullW - lineW) / 2;
        } else if (type == "right") {
            lineW = (i % 2 == 0) ? fullW : fullW * 2 / 3;
            x = margin + fullW - lineW;
        } else {
            lineW = fullW;
            x = margin;
        }
        p.drawLine(x, y, x + lineW, y);
        y += step;
    }
    return QIcon(pix);
}

// ═════════════════════════════════════════════════════════════════════════════
//  MainWindow
// ═════════════════════════════════════════════════════════════════════════════

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , currentHighlightColor(Qt::yellow)
    , remainingSeconds(7 * 60 * 60 - 60)
{
    setupHeaderBar();
    setupToolBar();
    setupEditor();

    examTimer = new QTimer(this);
    connect(examTimer, &QTimer::timeout, this, &MainWindow::updateTimer);
    examTimer->start(1000);
    updateTimer();
}

MainWindow::~MainWindow() = default;

// ─── Header-Leiste ──────────────────────────────────────────────────────────
void MainWindow::setupHeaderBar()
{
    auto *headerWidget = new QFrame;
    headerWidget->setFixedHeight(48);
    headerWidget->setStyleSheet(
        "QFrame { background-color: #3b4d6b; }"
        "QLabel { color: #f0f2f5; }"
    );

    auto *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(12, 0, 12, 0);

    // Dokumenten-Icon
    auto *docIcon = new QLabel;
    QPixmap iconPix(28, 28);
    iconPix.fill(Qt::transparent);
    {
        QPainter p(&iconPix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor("#c8d0dc"), 1.5));
        p.setBrush(QColor("#4a6080"));
        p.drawRoundedRect(4, 2, 20, 24, 2, 2);
        p.setPen(QColor("#a0b0c0"));
        for (int i = 0; i < 3; ++i)
            p.drawLine(9, 9 + i * 5, 19, 9 + i * 5);
    }
    docIcon->setPixmap(iconPix);
    headerLayout->addWidget(docIcon);

    // Kennziffer
    auto *kennzifferBlock = new QVBoxLayout;
    kennzifferBlock->setSpacing(0);
    auto *lblKennLabel = new QLabel("Kennziffer");
    lblKennLabel->setFont(QFont("Segoe UI", 8));
    lblKennLabel->setStyleSheet("color: #a0b0cc;");
    kennzifferBlock->addWidget(lblKennLabel);
    lblKennziffer = new QLabel("NRW_810774");
    lblKennziffer->setFont(QFont("Segoe UI", 11, QFont::Bold));
    kennzifferBlock->addWidget(lblKennziffer);
    headerLayout->addLayout(kennzifferBlock);
    headerLayout->addSpacing(16);

    // Titel
    auto *lblTitle = new QLabel("Demoprüfung");
    lblTitle->setFont(QFont("Segoe UI", 15, QFont::Bold));
    headerLayout->addWidget(lblTitle);

    headerLayout->addStretch();

    // ── Auswertung-Menü ──
    auto *btnAuswertung = new QToolButton;
    btnAuswertung->setText("📊 Auswertung");
    btnAuswertung->setFont(QFont("Segoe UI", 10, QFont::Bold));
    btnAuswertung->setPopupMode(QToolButton::InstantPopup);
    btnAuswertung->setStyleSheet(
        "QToolButton {"
        "    color: #d0daea;"
        "    background: transparent;"
        "    border: 1px solid #5a6d8a;"
        "    border-radius: 4px;"
        "    padding: 5px 12px;"
        "}"
        "QToolButton:hover {"
        "    background-color: #4a5f7d;"
        "    border-color: #7a8da8;"
        "}"
        "QToolButton::menu-indicator { image: none; width: 0; }"
    );

    auto *auswertungMenu = new QMenu(btnAuswertung);
    auswertungMenu->setStyleSheet(
        "QMenu { background-color: #ffffff; color: #000000; border: 1px solid #ccc; }"
        "QMenu::item { padding: 6px 20px; }"
        "QMenu::item:selected { background-color: #e0e8f0; }"
    );

    QAction *actDashboard = auswertungMenu->addAction("📊 Performance Dashboard");
    connect(actDashboard, &QAction::triggered, this, &MainWindow::showPerformanceDashboard);

    QAction *actStats = auswertungMenu->addAction("📋 Detaillierte Statistiken");
    connect(actStats, &QAction::triggered, this, &MainWindow::showStatistics);

    QAction *actPie = auswertungMenu->addAction("🔍 Fehler-Kreisdiagramm");
    connect(actPie, &QAction::triggered, this, &MainWindow::showErrorPieChart);

    QAction *actHeatmap = auswertungMenu->addAction("⌨️ Tastatur-Heatmap");
    connect(actHeatmap, &QAction::triggered, this, &MainWindow::showKeyboardHeatmap);

    QAction *actWPM = auswertungMenu->addAction("📈 WPM-Verlauf");
    connect(actWPM, &QAction::triggered, this, &MainWindow::showWPMChart);

    auswertungMenu->addSeparator();

    QAction *actReset = auswertungMenu->addAction("🔄 Tracking zurücksetzen");
    connect(actReset, &QAction::triggered, this, &MainWindow::resetTracking);

    btnAuswertung->setMenu(auswertungMenu);
    headerLayout->addWidget(btnAuswertung);

    headerLayout->addSpacing(12);

    // Keystroke-Zähler
    lblKeystrokeCount = new QLabel("Tasten: 0");
    lblKeystrokeCount->setFont(QFont("Segoe UI", 9));
    lblKeystrokeCount->setStyleSheet("color: #a0b0cc;");
    headerLayout->addWidget(lblKeystrokeCount);

    headerLayout->addSpacing(12);

    // Timer
    auto *timerIcon = new QLabel("⏱");
    timerIcon->setFont(QFont("Segoe UI", 14));
    timerIcon->setStyleSheet("color: #d0daea;");
    headerLayout->addWidget(timerIcon);

    lblTimer = new QLabel("06:59:00");
    lblTimer->setFont(QFont("Segoe UI", 12, QFont::Bold));
    lblTimer->setStyleSheet("color: #e8ecf2;");
    headerLayout->addWidget(lblTimer);

    headerLayout->addSpacing(12);

    auto *btnEnd = new QPushButton("Prüfung beenden");
    btnEnd->setFont(QFont("Segoe UI", 10));
    btnEnd->setStyleSheet(
        "QPushButton {"
        "    background-color: #f0f2f5; color: #3b4d6b;"
        "    border: none; border-radius: 4px; padding: 6px 14px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #ffffff; }"
    );
    connect(btnEnd, &QPushButton::clicked, this, [this]() { close(); });
    headerLayout->addWidget(btnEnd);

    setMenuWidget(headerWidget);
}

// ─── Toolbar ────────────────────────────────────────────────────────────────
void MainWindow::setupToolBar()
{
    auto *toolbar = new QToolBar("Formatierung");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(22, 22));
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    addToolBar(Qt::TopToolBarArea, toolbar);

    // 1) Rückgängig
    actUndo = toolbar->addAction(
        QApplication::style()->standardIcon(QStyle::SP_ArrowBack), "");
    actUndo->setToolTip("Rückgängig (Strg+Z)");
    actUndo->setShortcut(QKeySequence::Undo);

    // 2) Wiederholen
    actRedo = toolbar->addAction(
        QApplication::style()->standardIcon(QStyle::SP_ArrowForward), "");
    actRedo->setToolTip("Wiederholen (Strg+Y)");
    actRedo->setShortcut(QKeySequence::Redo);

    toolbar->addSeparator();

    // 3) Ausschneiden
    actCut = toolbar->addAction(textIcon("✂"), "");
    actCut->setToolTip("Ausschneiden (Strg+X)");
    actCut->setShortcut(QKeySequence::Cut);

    // 4) Kopieren
    actCopy = toolbar->addAction(textIcon("⧉"), "");
    actCopy->setToolTip("Kopieren (Strg+C)");
    actCopy->setShortcut(QKeySequence::Copy);

    // 5) Einfügen
    actPaste = toolbar->addAction(textIcon("📋", 24, QColor(50, 50, 50)), "");
    actPaste->setToolTip("Einfügen (Strg+V)");
    actPaste->setShortcut(QKeySequence::Paste);

    toolbar->addSeparator();

    // 6) Seitenumbruch
    actPageBreak = toolbar->addAction(textIcon("⏎", 24, QColor(50, 50, 50)), "");
    actPageBreak->setToolTip("Seitenumbruch einfügen");

    // 7) Hintergrundfarbe
    actHighlight = toolbar->addAction(colorRectIcon(currentHighlightColor), "");
    actHighlight->setToolTip("Hintergrundfarbe");

    toolbar->addSeparator();

    // 8) Fett
    actBold = toolbar->addAction(textIcon("B", 24, QColor(50, 50, 50), true), "");
    actBold->setCheckable(true);
    actBold->setToolTip("Fett (Strg+B)");
    actBold->setShortcut(QKeySequence::Bold);

    // 9) Kursiv
    {
        QPixmap pix(24, 24);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        QFont f("Segoe UI", 14);
        f.setItalic(true);
        p.setFont(f);
        p.setPen(QColor(50, 50, 50));
        p.drawText(QRect(0, 0, 24, 24), Qt::AlignCenter, "I");
        actItalic = toolbar->addAction(QIcon(pix), "");
    }
    actItalic->setCheckable(true);
    actItalic->setToolTip("Kursiv (Strg+I)");
    actItalic->setShortcut(QKeySequence::Italic);

    // 10) Unterstrichen
    {
        QPixmap pix(24, 24);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        QFont f("Segoe UI", 13);
        f.setUnderline(true);
        p.setFont(f);
        p.setPen(QColor(50, 50, 50));
        p.drawText(QRect(0, 0, 24, 22), Qt::AlignCenter, "U");
        p.setPen(QPen(QColor(50, 50, 50), 1.5));
        p.drawLine(6, 21, 18, 21);
        actUnderline = toolbar->addAction(QIcon(pix), "");
    }
    actUnderline->setCheckable(true);
    actUnderline->setToolTip("Unterstrichen (Strg+U)");
    actUnderline->setShortcut(QKeySequence::Underline);

    // 11) Hochgestellt
    {
        QPixmap pix(24, 24);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setPen(QColor(50, 50, 50));
        p.setFont(QFont("Segoe UI", 12));
        p.drawText(QRect(0, 4, 16, 20), Qt::AlignCenter, "X");
        p.setFont(QFont("Segoe UI", 8));
        p.drawText(QRect(13, 0, 11, 12), Qt::AlignCenter, "²");
        actSuperscript = toolbar->addAction(QIcon(pix), "");
    }
    actSuperscript->setCheckable(true);
    actSuperscript->setToolTip("Hochgestellt");

    toolbar->addSeparator();

    // 12–15) Ausrichtung
    actAlignLeft   = toolbar->addAction(alignIcon("left"), "");
    actAlignLeft->setCheckable(true);
    actAlignLeft->setToolTip("Linksbündig");

    actAlignCenter = toolbar->addAction(alignIcon("center"), "");
    actAlignCenter->setCheckable(true);
    actAlignCenter->setToolTip("Zentriert");

    actAlignRight  = toolbar->addAction(alignIcon("right"), "");
    actAlignRight->setCheckable(true);
    actAlignRight->setToolTip("Rechtsbündig");

    actJustify     = toolbar->addAction(alignIcon("justify"), "");
    actJustify->setCheckable(true);
    actJustify->setToolTip("Blocksatz");

    toolbar->addSeparator();

    // 16) Einzug vergrößern
    {
        QPixmap pix(24, 24);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setPen(QPen(QColor(50, 50, 50), 1.5));
        p.drawLine(3, 12, 10, 12);
        p.drawLine(7, 9, 10, 12);
        p.drawLine(7, 15, 10, 12);
        p.drawLine(14, 5, 21, 5);
        p.drawLine(14, 10, 21, 10);
        p.drawLine(14, 15, 21, 15);
        p.drawLine(14, 20, 21, 20);
        actIndent = toolbar->addAction(QIcon(pix), "");
    }
    actIndent->setToolTip("Einzug vergrößern");

    // 17) Geschütztes Leerzeichen
    actNbsp = toolbar->addAction(textIcon("⍽", 24, QColor(50, 50, 50)), "");
    actNbsp->setToolTip("Geschütztes Leerzeichen");

    // 18) Sonderzeichen
    actSpecialChar = toolbar->addAction(textIcon("Ω", 24, QColor(50, 50, 50)), "");
    actSpecialChar->setToolTip("Sonderzeichen einfügen");

    toolbar->addSeparator();

    // 19) Tabelle (Dropdown)
    btnTable = new QToolButton;
    btnTable->setIcon(textIcon("⊞", 24, QColor(50, 50, 50)));
    btnTable->setToolTip("Tabelle");
    btnTable->setPopupMode(QToolButton::InstantPopup);
    btnTable->setStyleSheet("QToolButton::menu-indicator { image: none; width: 0; }");

    auto *tableMenu = new QMenu(btnTable);
    QAction *actInsertTable = tableMenu->addAction("Tabelle einfügen...");
    connect(actInsertTable, &QAction::triggered, this, &MainWindow::insertTable);
    tableMenu->addSeparator();
    tableMenu->addAction("Zeile hinzufügen")->setEnabled(false);
    tableMenu->addAction("Spalte hinzufügen")->setEnabled(false);
    tableMenu->addAction("Zeile löschen")->setEnabled(false);
    tableMenu->addAction("Spalte löschen")->setEnabled(false);
    tableMenu->addSeparator();
    tableMenu->addAction("Zellen verbinden")->setEnabled(false);
    btnTable->setMenu(tableMenu);
    toolbar->addWidget(btnTable);

    toolbar->addSeparator();

    // 20) PDF-Gesamtvorschau
    actPdfPreview = toolbar->addAction(textIcon("👁", 24, QColor(50, 50, 50)), "");
    actPdfPreview->setToolTip("PDF-Gesamtvorschau");
}

// ─── Editor ─────────────────────────────────────────────────────────────────
void MainWindow::setupEditor()
{
    editor = new RichTextEditor(this);
    setCentralWidget(editor);

    connect(actUndo,   &QAction::triggered, editor, &QTextEdit::undo);
    connect(actRedo,   &QAction::triggered, editor, &QTextEdit::redo);
    connect(actCut,    &QAction::triggered, editor, &QTextEdit::cut);
    connect(actCopy,   &QAction::triggered, editor, &QTextEdit::copy);
    connect(actPaste,  &QAction::triggered, editor, &QTextEdit::paste);

    connect(actPageBreak,   &QAction::triggered, this, &MainWindow::insertPageBreak);
    connect(actHighlight,   &QAction::triggered, this, &MainWindow::setHighlightColor);
    connect(actBold,        &QAction::triggered, this, &MainWindow::toggleBold);
    connect(actItalic,      &QAction::triggered, this, &MainWindow::toggleItalic);
    connect(actUnderline,   &QAction::triggered, this, &MainWindow::toggleUnderline);
    connect(actSuperscript, &QAction::triggered, this, &MainWindow::toggleSuperscript);

    connect(actAlignLeft,   &QAction::triggered, this, &MainWindow::alignLeft);
    connect(actAlignCenter, &QAction::triggered, this, &MainWindow::alignCenter);
    connect(actAlignRight,  &QAction::triggered, this, &MainWindow::alignRight);
    connect(actJustify,     &QAction::triggered, this, &MainWindow::alignJustify);

    connect(actIndent,      &QAction::triggered, this, &MainWindow::increaseIndent);
    connect(actNbsp,        &QAction::triggered, this, &MainWindow::insertNonBreakingSpace);
    connect(actSpecialChar, &QAction::triggered, this, &MainWindow::insertSpecialChar);
    connect(actPdfPreview,  &QAction::triggered, this, &MainWindow::showPdfPreview);

    connect(editor, &QTextEdit::undoAvailable, actUndo, &QAction::setEnabled);
    connect(editor, &QTextEdit::redoAvailable, actRedo, &QAction::setEnabled);
    actUndo->setEnabled(false);
    actRedo->setEnabled(false);

    connect(editor, &QTextEdit::copyAvailable, actCut,  &QAction::setEnabled);
    connect(editor, &QTextEdit::copyAvailable, actCopy, &QAction::setEnabled);
    actCut->setEnabled(false);
    actCopy->setEnabled(false);

    connect(editor, &QTextEdit::cursorPositionChanged,
            this, &MainWindow::onCursorPositionChanged);
    connect(editor, &QTextEdit::currentCharFormatChanged,
            this, &MainWindow::onCurrentCharFormatChanged);

    editor->setFocus();
}

// ═════════════════════════════════════════════════════════════════════════════
//  Formatting Slots
// ═════════════════════════════════════════════════════════════════════════════

void MainWindow::toggleBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actBold->isChecked() ? QFont::Bold : QFont::Normal);
    editor->mergeCurrentCharFormat(fmt);
}

void MainWindow::toggleItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actItalic->isChecked());
    editor->mergeCurrentCharFormat(fmt);
}

void MainWindow::toggleUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actUnderline->isChecked());
    editor->mergeCurrentCharFormat(fmt);
}

void MainWindow::toggleSuperscript()
{
    QTextCharFormat fmt;
    fmt.setVerticalAlignment(
        actSuperscript->isChecked()
            ? QTextCharFormat::AlignSuperScript
            : QTextCharFormat::AlignNormal
    );
    editor->mergeCurrentCharFormat(fmt);
}

void MainWindow::alignLeft()   { editor->setAlignment(Qt::AlignLeft);    updateAlignmentActions(); }
void MainWindow::alignCenter() { editor->setAlignment(Qt::AlignHCenter); updateAlignmentActions(); }
void MainWindow::alignRight()  { editor->setAlignment(Qt::AlignRight);   updateAlignmentActions(); }
void MainWindow::alignJustify(){ editor->setAlignment(Qt::AlignJustify); updateAlignmentActions(); }

void MainWindow::updateAlignmentActions()
{
    Qt::Alignment a = editor->alignment();
    actAlignLeft->setChecked(a & Qt::AlignLeft);
    actAlignCenter->setChecked(a & Qt::AlignHCenter);
    actAlignRight->setChecked(a & Qt::AlignRight);
    actJustify->setChecked(a & Qt::AlignJustify);
}

void MainWindow::increaseIndent()
{
    QTextCursor cursor = editor->textCursor();
    QTextBlockFormat blockFmt = cursor.blockFormat();
    blockFmt.setIndent(blockFmt.indent() + 1);
    cursor.setBlockFormat(blockFmt);
}

void MainWindow::insertPageBreak()
{
    QTextCursor cursor = editor->textCursor();
    cursor.insertText("\n");
    QTextBlockFormat breakFmt;
    breakFmt.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
    cursor.insertBlock(breakFmt);
}

void MainWindow::insertNonBreakingSpace()
{
    editor->textCursor().insertText(QString(QChar(0x00A0)));
}

void MainWindow::insertSpecialChar()
{
    SpecialCharDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted && !dlg.selectedChar().isEmpty())
        editor->textCursor().insertText(dlg.selectedChar());
}

void MainWindow::insertTable()
{
    TableDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QTextCursor cursor = editor->textCursor();
        QTextTableFormat tableFmt;
        tableFmt.setBorder(1);
        tableFmt.setBorderStyle(QTextTableFormat::BorderStyle_Solid);
        tableFmt.setCellPadding(6);
        tableFmt.setCellSpacing(0);
        tableFmt.setBorderBrush(QColor("#999999"));
        cursor.insertTable(dlg.rows(), dlg.columns(), tableFmt);
    }
}

void MainWindow::setHighlightColor()
{
    QColor color = QColorDialog::getColor(currentHighlightColor, this, "Hintergrundfarbe wählen");
    if (!color.isValid()) return;
    currentHighlightColor = color;
    actHighlight->setIcon(colorRectIcon(color));
    QTextCharFormat fmt;
    fmt.setBackground(color);
    editor->mergeCurrentCharFormat(fmt);
}

void MainWindow::showPdfPreview()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowTitle("PDF-Gesamtvorschau");
    preview.resize(800, 600);
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &MainWindow::printPreview);
    preview.exec();
}

void MainWindow::printPreview(QPrinter *printer)
{
    editor->document()->print(printer);
}

void MainWindow::onCursorPositionChanged() { updateAlignmentActions(); }

void MainWindow::onCurrentCharFormatChanged(const QTextCharFormat &fmt)
{
    actBold->setChecked(fmt.fontWeight() >= QFont::Bold);
    actItalic->setChecked(fmt.fontItalic());
    actUnderline->setChecked(fmt.fontUnderline());
    actSuperscript->setChecked(fmt.verticalAlignment() == QTextCharFormat::AlignSuperScript);
}

void MainWindow::updateTimer()
{
    if (remainingSeconds < 0) remainingSeconds = 0;
    int h = remainingSeconds / 3600;
    int m = (remainingSeconds % 3600) / 60;
    int s = remainingSeconds % 60;
    lblTimer->setText(QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0')));
    if (remainingSeconds <= 1800 && remainingSeconds > 0)
        lblTimer->setStyleSheet("color: #ff8888; font-weight: bold;");
    remainingSeconds--;

    // Keystroke-Zähler aktualisieren
    lblKeystrokeCount->setText(
        QString("Tasten: %1").arg(editor->analyzer().keystrokeCount()));
}

void MainWindow::resetTracking()
{
    editor->resetTracking();
    lblKeystrokeCount->setText("Tasten: 0");
    QMessageBox::information(this, "Tracking zurückgesetzt",
        "Alle Keystroke-Daten wurden gelöscht.\nDas Tracking beginnt jetzt von vorne.");
}

// ═════════════════════════════════════════════════════════════════════════════
//  Auswertung: Statistik-Dialog
// ═════════════════════════════════════════════════════════════════════════════

void MainWindow::showStatistics()
{
    const NgramAnalyzer &a = editor->analyzer();

    int backspaces = a.backspaceCount();
    double backspaceRate = a.backspaceRate();
    double accuracy = a.accuracy();
    double grossWPM = a.grossWPM();
    double netWPM = a.netWPM();
    double kspc = a.kspc();
    int motoricBS = a.motoricBackspaces();
    int autoBS = a.automatizationBackspaces();
    int contentBS = a.contentBackspaces();
    double motoricRate = a.motoricBackspaceRate();
    double autoRate = a.automatizationBackspaceRate();
    double contentRate = a.contentBackspaceRate();
    auto pauses = a.findPauses(500);
    auto legalChars = a.analyzeLegalChars();
    auto wpmList = a.calculateWPMOverTime(60);
    double sessionTime = a.totalSessionTimeSeconds();

    QString statsText;
    statsText += "=== SESSION STATISTIKEN ===\n\n";
    statsText += QString("Session-Dauer: %1 Sekunden (%2 Min)\n\n")
        .arg(sessionTime, 0, 'f', 1)
        .arg(sessionTime / 60.0, 0, 'f', 2);

    statsText += "=== PERFORMANCE ===\n";
    statsText += QString("Gross WPM: %1\n").arg(grossWPM, 0, 'f', 1);
    statsText += QString("Net WPM: %1\n").arg(netWPM, 0, 'f', 1);
    statsText += QString("Accuracy: %1%\n").arg(accuracy * 100.0, 0, 'f', 1);
    statsText += QString("KSPC: %1 ").arg(kspc, 0, 'f', 3);
    if (kspc <= 1.05) statsText += "(Exzellent!)";
    else if (kspc <= 1.15) statsText += "(Gut)";
    else if (kspc <= 1.30) statsText += "(Normal)";
    else statsText += "(Verbesserungsbedarf)";
    statsText += "\n\n";

    statsText += "=== FEHLERANALYSE (HEURISTIK) ===\n";
    statsText += QString("Backspace Gesamt: %1 (Rate: %2%)\n\n")
        .arg(backspaces).arg(backspaceRate, 0, 'f', 1);

    if (backspaces > 0) {
        statsText += QString("Motorisch (Tippfehler):      %1 (%2%)  < 200ms\n")
            .arg(motoricBS).arg(motoricRate, 0, 'f', 0);
        statsText += QString("Automatisierung (Tastsuche): %1 (%2%)  200-1000ms\n")
            .arg(autoBS).arg(autoRate, 0, 'f', 0);
        statsText += QString("Inhaltlich (Unsicherheit):   %1 (%2%)  > 1000ms\n\n")
            .arg(contentBS).arg(contentRate, 0, 'f', 0);

        statsText += "HINWEIS: Diese Kategorisierung ist eine Schätzung\n";
        statsText += "basierend auf der Reaktionszeit (ca. 60-70% Genauigkeit).\n\n";
    }

    statsText += "=== PAUSEN (>500ms) ===\n";
    statsText += QString("Anzahl: %1\n").arg(pauses.size());
    for (int i = 0; i < qMin(5, pauses.size()); ++i) {
        statsText += QString("  %1ms bei: \"%2|%3\"\n")
            .arg(pauses[i].durationMs)
            .arg(pauses[i].contextBefore, pauses[i].contextAfter);
    }
    statsText += "\n";

    statsText += "=== LEGAL ZEICHEN ===\n";
    if (legalChars.isEmpty()) {
        statsText += "Keine Legal-Zeichen verwendet\n";
    } else {
        for (const auto &stat : std::as_const(legalChars)) {
            statsText += QString("  '%1': %2x (Ø %3ms)\n")
                .arg(stat.character).arg(stat.count).arg(stat.avgTimeMs, 0, 'f', 1);
        }
    }
    statsText += "\n";

    statsText += "=== WPM ÜBER ZEIT ===\n";
    if (wpmList.isEmpty()) {
        statsText += "Nicht genug Daten\n";
    } else {
        for (int i = 0; i < wpmList.size(); ++i)
            statsText += QString("  Minute %1: %2 WPM\n").arg(i + 1).arg(wpmList[i], 0, 'f', 1);
    }

    statsText += "\n\n";
    statsText += generateRecommendations();

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Detaillierte Statistiken");
    dialog->resize(800, 650);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QTextEdit *textEdit = new QTextEdit(dialog);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(statsText);
    textEdit->setStyleSheet("background-color: #ffffff; color: #000000; font-family: 'Consolas'; font-size: 10pt;");
    layout->addWidget(textEdit);

    QPushButton *closeBtn = new QPushButton("Schließen", dialog);
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn);

    dialog->exec();
}

// ═════════════════════════════════════════════════════════════════════════════
//  Auswertung: Handlungsempfehlungen
// ═════════════════════════════════════════════════════════════════════════════

QString MainWindow::generateRecommendations()
{
    const NgramAnalyzer &a = editor->analyzer();
    QString text;
    text += "=== DEINE NÄCHSTEN SCHRITTE ===\n\n";

    int rec = 1;

    double autoRate = a.automatizationBackspaceRate();
    if (autoRate > 50.0) {
        text += QString("%1. 10-Finger-Training\n").arg(rec++);
        text += QString("   %1% deiner Fehler sind Tastsuche\n").arg(autoRate, 0, 'f', 0);
        text += "   30 Minuten täglich, Grundreihe (ASDF JKL;)\n\n";
    }

    double contentRate = a.contentBackspaceRate();
    if (contentRate > 5.0) {
        text += QString("%1. Jura-Fachbegriffe wiederholen\n").arg(rec++);
        text += QString("   %1% inhaltliche Unsicherheiten\n").arg(contentRate, 0, 'f', 1);
        text += "   15 Minuten täglich, Rechtschreibung festigen\n\n";
    }

    double accuracy = a.accuracy() * 100.0;
    if (accuracy < 90.0) {
        text += QString("%1. Genauigkeit steigern\n").arg(rec++);
        text += QString("   Aktuell: %1% (Ziel: 95%)\n").arg(accuracy, 0, 'f', 1);
        text += "   Langsamer tippen, Präzision vor Geschwindigkeit\n\n";
    }

    double grossWPM = a.grossWPM();
    if (accuracy >= 90.0 && grossWPM < 50.0) {
        text += QString("%1. Geschwindigkeit erhöhen\n").arg(rec++);
        text += QString("   Aktuell: %1 WPM (Ziel: 60 WPM)\n").arg(grossWPM, 0, 'f', 1);
        text += "   Häufige Wörter automatisieren\n\n";
    }

    if (rec == 1) {
        text += "HERVORRAGEND!\n\n";
        text += QString("• %1 WPM (schnell)\n").arg(grossWPM, 0, 'f', 1);
        text += QString("• %1% Genauigkeit (präzise)\n").arg(accuracy, 0, 'f', 1);
        text += QString("• %1 KSPC (effizient)\n\n").arg(a.kspc(), 0, 'f', 2);
        text += "Empfehlung: Halte dieses Niveau!\n";
    }

    text += "\nTIPP: 15-30 Minuten täglich. Konstanz schlägt Intensität!\n";
    return text;
}

// ═════════════════════════════════════════════════════════════════════════════
//  Auswertung: Performance Dashboard (Cards)
// ═════════════════════════════════════════════════════════════════════════════

void MainWindow::showPerformanceDashboard()
{
    const NgramAnalyzer &a = editor->analyzer();
    double grossWPM = a.grossWPM();
    double accuracy = a.accuracy() * 100.0;
    double kspc = a.kspc();

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Performance Dashboard");
    dialog->resize(900, 420);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    QLabel *title = new QLabel("DEINE TIPP-PERFORMANCE", dialog);
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("padding: 10px; color: #333;");
    mainLayout->addWidget(title);

    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(20);

    cardsLayout->addWidget(createPerformanceCard(
        "GESCHWINDIGKEIT", QString::number(grossWPM, 'f', 1),
        "WPM", getWPMRating(grossWPM), getWPMColor(grossWPM)));

    cardsLayout->addWidget(createPerformanceCard(
        "GENAUIGKEIT", QString::number(accuracy, 'f', 1),
        "%", getAccuracyRating(accuracy), getAccuracyColor(accuracy)));

    cardsLayout->addWidget(createPerformanceCard(
        "EFFIZIENZ", QString::number(kspc, 'f', 2),
        "KSPC", getKSPCRating(kspc), getKSPCColor(kspc)));

    mainLayout->addLayout(cardsLayout);

    QLabel *legend = new QLabel(
        "WPM = Words Per Minute  |  Accuracy = Tasten ohne Fehler (Ziel: 95%)  |  KSPC = 1.0 perfekt, >1.3 viele Korrekturen");
    legend->setAlignment(Qt::AlignCenter);
    legend->setStyleSheet("padding: 10px; color: #666; font-size: 10pt;");
    legend->setWordWrap(true);
    mainLayout->addWidget(legend);

    QPushButton *closeBtn = new QPushButton("Schließen", dialog);
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    mainLayout->addWidget(closeBtn);

    dialog->exec();
}

QFrame *MainWindow::createPerformanceCard(const QString &title, const QString &value,
    const QString &unit, const QString &rating, const QColor &color)
{
    QFrame *card = new QFrame;
    card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet(QString(
        "QFrame { background-color: white; border: 2px solid %1; border-radius: 10px; padding: 20px; }"
    ).arg(color.name()));

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setSpacing(10);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setFont(QFont("Segoe UI", 14, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #555;");
    layout->addWidget(titleLabel);

    QLabel *valueLabel = new QLabel(value);
    valueLabel->setFont(QFont("Segoe UI", 36, QFont::Bold));
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setStyleSheet(QString("color: %1;").arg(color.name()));
    layout->addWidget(valueLabel);

    QLabel *unitLabel = new QLabel(unit);
    unitLabel->setFont(QFont("Segoe UI", 12));
    unitLabel->setAlignment(Qt::AlignCenter);
    unitLabel->setStyleSheet("color: #888;");
    layout->addWidget(unitLabel);

    QLabel *ratingLabel = new QLabel(rating);
    ratingLabel->setFont(QFont("Segoe UI", 11));
    ratingLabel->setAlignment(Qt::AlignCenter);
    ratingLabel->setStyleSheet("color: #666; padding-top: 10px;");
    ratingLabel->setWordWrap(true);
    layout->addWidget(ratingLabel);

    return card;
}

QString MainWindow::getWPMRating(double wpm) const
{
    if (wpm >= 60) return "Sehr schnell!";
    if (wpm >= 50) return "Gut";
    if (wpm >= 40) return "Durchschnitt";
    return "Anfänger - Üben!";
}
QColor MainWindow::getWPMColor(double wpm) const
{
    if (wpm >= 60) return QColor(76, 175, 80);
    if (wpm >= 40) return QColor(255, 193, 7);
    return QColor(244, 67, 54);
}
QString MainWindow::getAccuracyRating(double acc) const
{
    if (acc >= 95) return "Exzellent!";
    if (acc >= 90) return "Gut";
    if (acc >= 85) return "OK";
    return "Üben nötig";
}
QColor MainWindow::getAccuracyColor(double acc) const
{
    if (acc >= 95) return QColor(76, 175, 80);
    if (acc >= 85) return QColor(255, 193, 7);
    return QColor(244, 67, 54);
}
QString MainWindow::getKSPCRating(double k) const
{
    if (k <= 1.1) return "Sehr effizient!";
    if (k <= 1.2) return "Gut";
    if (k <= 1.3) return "Durchschnitt";
    return "Zu viele Korrekturen";
}
QColor MainWindow::getKSPCColor(double k) const
{
    if (k <= 1.1) return QColor(76, 175, 80);
    if (k <= 1.3) return QColor(255, 193, 7);
    return QColor(244, 67, 54);
}

// ═════════════════════════════════════════════════════════════════════════════
//  Auswertung: Fehler-Kreisdiagramm
// ═════════════════════════════════════════════════════════════════════════════

void MainWindow::showErrorPieChart()
{
    const NgramAnalyzer &a = editor->analyzer();
    int motoricErrors = a.motoricBackspaces();
    int autoErrors = a.automatizationBackspaces();
    int contentErrors = a.contentBackspaces();
    int total = a.backspaceCount();

    if (total == 0) {
        QMessageBox::information(this, "Keine Fehler", "Keine Fehler gefunden!");
        return;
    }

    QPieSeries *series = new QPieSeries();
    series->append("Automatisierung (Tastsuche)", autoErrors);
    series->append("Motorisch (Tippfehler)", motoricErrors);
    series->append("Inhaltlich (Rechtschreibung)", contentErrors);

    series->slices().at(0)->setColor(QColor(255, 193, 7));
    series->slices().at(0)->setBorderColor(Qt::white);
    series->slices().at(0)->setBorderWidth(2);

    series->slices().at(1)->setColor(QColor(76, 175, 80));
    series->slices().at(1)->setBorderColor(Qt::white);
    series->slices().at(1)->setBorderWidth(2);

    series->slices().at(2)->setColor(QColor(33, 150, 243));
    series->slices().at(2)->setBorderColor(Qt::white);
    series->slices().at(2)->setBorderWidth(2);

    for (auto slice : series->slices()) {
        double pct = (slice->value() / total) * 100.0;
        slice->setLabel(QString("%1\n%2 (%3%)")
            .arg(slice->label())
            .arg(static_cast<int>(slice->value()))
            .arg(pct, 0, 'f', 0));
        slice->setLabelVisible(true);
        slice->setLabelColor(Qt::black);
    }

    QPieSlice *maxSlice = nullptr;
    double maxVal = 0;
    for (auto s : series->slices()) {
        if (s->value() > maxVal) { maxVal = s->value(); maxSlice = s; }
    }
    if (maxSlice) {
        maxSlice->setExploded(true);
        maxSlice->setExplodeDistanceFactor(0.1);
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Deine Fehlerquellen");
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QFont titleFont = chart->titleFont();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    chart->setTitleFont(titleFont);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Fehleranalyse");
    dialog->resize(700, 550);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(chartView);

    QLabel *explainLabel = new QLabel(dialog);
    if (autoErrors > motoricErrors && autoErrors > contentErrors)
        explainLabel->setText("Hauptproblem: Automatisierung — 10-Finger-Grundübungen empfohlen");
    else if (motoricErrors > autoErrors && motoricErrors > contentErrors)
        explainLabel->setText("Hauptproblem: Motorische Fehler — Langsamer und präziser tippen");
    else
        explainLabel->setText("Hauptproblem: Inhaltliche Unsicherheit — Fachbegriffe wiederholen");
    explainLabel->setAlignment(Qt::AlignCenter);
    explainLabel->setStyleSheet("padding: 8px; color: #444; font-size: 11pt;");
    explainLabel->setWordWrap(true);
    layout->addWidget(explainLabel);

    QPushButton *closeBtn = new QPushButton("Schließen", dialog);
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn);

    dialog->exec();
}

// ═════════════════════════════════════════════════════════════════════════════
//  Auswertung: WPM-Verlauf
// ═════════════════════════════════════════════════════════════════════════════

void MainWindow::showWPMChart()
{
    auto wpmList = editor->analyzer().calculateWPMOverTime(60);

    if (wpmList.isEmpty()) {
        QMessageBox::information(this, "WPM Chart",
            "Nicht genug Daten.\nTippe mindestens 1 Minute!");
        return;
    }

    QChart *chart = new QChart();
    chart->setTitle("WPM über Zeit");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QLineSeries *series = new QLineSeries();
    series->setName("WPM");
    for (int i = 0; i < wpmList.size(); ++i)
        series->append(i + 1, wpmList[i]);
    chart->addSeries(series);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Minute");
    axisX->setLabelFormat("%d");
    axisX->setTickCount(qMin(static_cast<int>(wpmList.size()) + 1, 12));
    axisX->setRange(0, wpmList.size() + 1);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("WPM");
    axisY->setLabelFormat("%d");
    double minWPM = *std::min_element(wpmList.begin(), wpmList.end());
    double maxWPM = *std::max_element(wpmList.begin(), wpmList.end());
    double range = maxWPM - minWPM;
    axisY->setRange(qMax(0.0, minWPM - range * 0.1), maxWPM + range * 0.1);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("WPM Verlauf");
    dialog->resize(800, 500);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(chartView);

    double avgWPM = 0;
    for (double w : std::as_const(wpmList)) avgWPM += w;
    avgWPM /= wpmList.size();

    QLabel *statsLabel = new QLabel(
        QString("Session: %1 Min  |  Durchschnitt: %2 WPM  |  Peak: %3 WPM")
            .arg(editor->analyzer().totalSessionTimeSeconds() / 60.0, 0, 'f', 1)
            .arg(avgWPM, 0, 'f', 1)
            .arg(maxWPM, 0, 'f', 1));
    statsLabel->setAlignment(Qt::AlignCenter);
    statsLabel->setFont(QFont("Segoe UI", 11, QFont::Bold));
    layout->addWidget(statsLabel);

    QPushButton *closeBtn = new QPushButton("Schließen", dialog);
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::close);
    layout->addWidget(closeBtn);

    dialog->exec();
}

// ═════════════════════════════════════════════════════════════════════════════
//  Auswertung: Tastatur-Heatmap
// ═════════════════════════════════════════════════════════════════════════════

void MainWindow::showKeyboardHeatmap()
{
    QMap<QChar, int> keyErrors = analyzeKeyErrors();

    if (keyErrors.isEmpty()) {
        QMessageBox::information(this, "Keine Fehler", "Keine Fehler analysiert!");
        return;
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Tastatur-Heatmap");
    dialog->resize(900, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    QLabel *title = new QLabel("DEINE PROBLEM-TASTEN", dialog);
    title->setFont(QFont("Segoe UI", 18, QFont::Bold));
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("padding: 10px; color: #333;");
    mainLayout->addWidget(title);

    // QWERTZ Tastatur
    QWidget *keyboardWidget = new QWidget(dialog);
    QVBoxLayout *kbLayout = new QVBoxLayout(keyboardWidget);
    kbLayout->setSpacing(5);

    // Reihe 1
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->setSpacing(3);
    QString row1Keys = QString::fromUtf8("QWERTZUIOPÜ");
    for (const QChar &key : std::as_const(row1Keys))
        row1->addWidget(createKeyButton(key, keyErrors.value(key.toLower(), 0)));
    kbLayout->addLayout(row1);

    // Reihe 2
    QHBoxLayout *row2 = new QHBoxLayout();
    row2->setSpacing(3);
    row2->addSpacing(20);
    QString row2Keys = QString::fromUtf8("ASDFGHJKLÖÄ");
    for (const QChar &key : std::as_const(row2Keys))
        row2->addWidget(createKeyButton(key, keyErrors.value(key.toLower(), 0)));
    kbLayout->addLayout(row2);

    // Reihe 3
    QHBoxLayout *row3 = new QHBoxLayout();
    row3->setSpacing(3);
    row3->addSpacing(40);
    QString row3Keys = "YXCVBNM";
    for (const QChar &key : std::as_const(row3Keys))
        row3->addWidget(createKeyButton(key, keyErrors.value(key.toLower(), 0)));
    row3->addWidget(createKeyButton(QChar(0xDF), keyErrors.value(QChar(0xDF), 0)));
    kbLayout->addLayout(row3);

    // Leertaste
    QHBoxLayout *row4 = new QHBoxLayout();
    row4->setSpacing(3);
    row4->addSpacing(80);
    QPushButton *spaceKey = createKeyButton(' ', keyErrors.value(' ', 0));
    spaceKey->setMinimumWidth(300);
    spaceKey->setText("LEERTASTE");
    row4->addWidget(spaceKey);
    kbLayout->addLayout(row4);

    mainLayout->addWidget(keyboardWidget);

    // Top 5 Problem-Tasten
    QLabel *topLabel = new QLabel(dialog);
    QList<QPair<QChar, int>> sorted;
    for (auto it = keyErrors.begin(); it != keyErrors.end(); ++it)
        if (it.value() > 0) sorted.append(qMakePair(it.key(), it.value()));
    std::sort(sorted.begin(), sorted.end(),
        [](const QPair<QChar, int> &a, const QPair<QChar, int> &b) { return a.second > b.second; });

    QString topText = "Top Problem-Tasten: ";
    for (int i = 0; i < qMin(5, sorted.size()); ++i) {
        QString kd = (sorted[i].first == ' ') ? "LEER" : QString(sorted[i].first).toUpper();
        topText += QString("%1 (%2x)  ").arg(kd).arg(sorted[i].second);
    }
    topLabel->setText(topText);
    topLabel->setAlignment(Qt::AlignCenter);
    topLabel->setStyleSheet("padding: 8px; color: #444; font-size: 11pt; font-weight: bold;");
    topLabel->setWordWrap(true);
    mainLayout->addWidget(topLabel);

    // Legende
    QLabel *legend = new QLabel("Rot: >10 Fehler  |  Orange: 5-10  |  Gelb: 1-5  |  Grau: 0");
    legend->setAlignment(Qt::AlignCenter);
    legend->setStyleSheet("padding: 6px; color: #666; font-size: 10pt;");
    mainLayout->addWidget(legend);

    QPushButton *closeBtn = new QPushButton("Schließen", dialog);
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    mainLayout->addWidget(closeBtn);

    dialog->exec();
}

QMap<QChar, int> MainWindow::analyzeKeyErrors()
{
    QMap<QChar, int> keyErrors;
    auto events = editor->analyzer().getBackspaceEvents();
    for (const auto &ev : std::as_const(events)) {
        QChar ch = ev.deletedChar.toLower();
        if (!ch.isNull() && ch != '\b')
            keyErrors[ch]++;
    }
    return keyErrors;
}

QPushButton *MainWindow::createKeyButton(QChar key, int errorCount)
{
    QPushButton *btn = new QPushButton(QString(key).toUpper());
    btn->setMinimumSize(50, 50);
    btn->setMaximumSize(50, 50);

    QString color, textColor = "black";
    if (errorCount > 10) { color = "rgb(244, 67, 54)"; textColor = "white"; }
    else if (errorCount > 5) { color = "rgb(255, 152, 0)"; textColor = "white"; }
    else if (errorCount > 0) { color = "rgb(255, 193, 7)"; }
    else { color = "rgb(224, 224, 224)"; }

    btn->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: 1px solid #999;"
        "    border-radius: 5px; font-size: 14pt; font-weight: bold; }"
        "QPushButton:hover { border: 2px solid #333; }"
    ).arg(color, textColor));

    btn->setToolTip(errorCount > 0 ? QString("%1x Fehler").arg(errorCount) : "Keine Fehler");
    return btn;
}

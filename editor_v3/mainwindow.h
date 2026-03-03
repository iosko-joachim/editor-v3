#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QColorDialog>
#include <QTextEdit>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QToolButton>
#include <QMenu>
#include <QTimer>
#include <QFrame>
#include <QMap>

#include "richtexteditor.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Formatierung
    void toggleBold();
    void toggleItalic();
    void toggleUnderline();
    void toggleSuperscript();

    // Ausrichtung
    void alignLeft();
    void alignCenter();
    void alignRight();
    void alignJustify();

    // Einzug
    void increaseIndent();

    // Einfügen
    void insertPageBreak();
    void insertNonBreakingSpace();
    void insertSpecialChar();
    void insertTable();

    // Hintergrundfarbe
    void setHighlightColor();

    // Vorschau
    void showPdfPreview();
    void printPreview(QPrinter *printer);

    // Cursor-/Formatänderung
    void onCursorPositionChanged();
    void onCurrentCharFormatChanged(const QTextCharFormat &fmt);

    // Timer
    void updateTimer();

    // ── Auswertung ──
    void showStatistics();
    void showPerformanceDashboard();
    void showErrorPieChart();
    void showKeyboardHeatmap();
    void showWPMChart();
    void resetTracking();

private:
    void setupHeaderBar();
    void setupToolBar();
    void setupEditor();
    void updateAlignmentActions();

    // Auswertung: Hilfsfunktionen
    QString generateRecommendations();

    // Dashboard Cards
    QFrame *createPerformanceCard(const QString &title, const QString &value,
                                  const QString &unit, const QString &rating,
                                  const QColor &color);
    QString getWPMRating(double wpm) const;
    QColor  getWPMColor(double wpm) const;
    QString getAccuracyRating(double accuracy) const;
    QColor  getAccuracyColor(double accuracy) const;
    QString getKSPCRating(double kspc) const;
    QColor  getKSPCColor(double kspc) const;

    // Heatmap
    QMap<QChar, int> analyzeKeyErrors();
    QPushButton *createKeyButton(QChar key, int errorCount);

    RichTextEditor *editor;

    // Toolbar-Aktionen
    QAction *actUndo;
    QAction *actRedo;
    QAction *actCut;
    QAction *actCopy;
    QAction *actPaste;
    QAction *actPageBreak;
    QAction *actHighlight;
    QAction *actBold;
    QAction *actItalic;
    QAction *actUnderline;
    QAction *actSuperscript;
    QAction *actAlignLeft;
    QAction *actAlignCenter;
    QAction *actAlignRight;
    QAction *actJustify;
    QAction *actIndent;
    QAction *actNbsp;
    QAction *actSpecialChar;
    QToolButton *btnTable;
    QAction *actPdfPreview;

    // Header-Elemente
    QLabel *lblKennziffer;
    QLabel *lblTimer;
    QLabel *lblKeystrokeCount;
    QTimer *examTimer;
    int remainingSeconds;

    QColor currentHighlightColor;
};

#endif // MAINWINDOW_H

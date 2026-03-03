#ifndef RICHTEXTEDITOR_H
#define RICHTEXTEDITOR_H

#include <QTextEdit>
#include <QElapsedTimer>

#include "ngramanalyzer.h"

/// Rich-Text-Editor-Widget mit Papierstil-Darstellung und Keystroke-Tracking.
class RichTextEditor : public QTextEdit
{
    Q_OBJECT

public:
    explicit RichTextEditor(QWidget *parent = nullptr);

    NgramAnalyzer &analyzer();
    const NgramAnalyzer &analyzer() const;

    void resetTracking();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void updatePageLayout();

    NgramAnalyzer m_analyzer;
    QElapsedTimer m_elapsed;
    bool m_inLayoutUpdate = false;  // Guard gegen Rekursion
};

#endif // RICHTEXTEDITOR_H

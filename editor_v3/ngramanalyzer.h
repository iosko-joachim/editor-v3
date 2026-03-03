#ifndef NGRAMANALYZER_H
#define NGRAMANALYZER_H

#include <QChar>
#include <QHash>
#include <QList>
#include <QString>

struct NgramStat {
    QString ngram;
    int occurrences = 0;
    double meanMs = 0.0;
    double stddevMs = 0.0;
    double coeffOfVariation = 0.0;
};

struct Keystroke {
    QChar character;
    qint64 downMs = 0;
    qint64 upMs = 0;
};

enum class TimingMode { KeyDownOnly, KeyDownAndKeyUp };

class NgramAnalyzer {
public:
    NgramAnalyzer();

    void addKeystroke(QChar ch, qint64 downMs, qint64 upMs = -1);
    void addBackspace(qint64 timestamp);
    void clear();

    void setNgramSize(int size);
    int ngramSize() const;

    void setTimingMode(TimingMode mode);
    TimingMode timingMode() const;

    QList<NgramStat> computeStats(int minOccurrences = 2) const;

    int keystrokeCount() const;

    void patchLastUpMs(QChar ch, qint64 upMs);

           // Backspace-Analyse
    int backspaceCount() const;
    double backspaceRate() const;  // Prozent

           // Pausen-Analyse (IKI > threshold)
    struct PauseInfo {
        int position;
        qint64 durationMs;
        QString contextBefore;  // 5 Zeichen davor
        QString contextAfter;   // 5 Zeichen danach
    };
    QList<PauseInfo> findPauses(qint64 thresholdMs = 500) const;

           // Legal-Zeichen-Analyse (§, ä, ö, ü, ...)
    struct LegalCharStat {
        QChar character;
        int count;
        double avgTimeMs;  // Durchschnittliche Zeit für dieses Zeichen
    };
    QList<LegalCharStat> analyzeLegalChars() const;

           // WPM über Zeit (pro Minute)
    QList<double> calculateWPMOverTime(int intervalSeconds = 60) const;

           // Gesamtzeit der Session in Sekunden
    double totalSessionTimeSeconds() const;

           // ===== NEUE METRIKEN =====

    // Backspace Event mit detailliertem Timing
    struct BackspaceEvent {
        qint64 timestamp;
        QChar deletedChar;
        qint64 timeSinceLastKeystroke;
    };

           // WPM-Metriken
    double grossWPM() const;  // Rohe WPM (alle Zeichen)
    double netWPM() const;    // Netto WPM (abzgl. Fehler)

           // Genauigkeit
    double accuracy() const;  // 0.0 - 1.0 (z.B. 0.87 = 87%)

           // Effizienz (Keystrokes per Character)
    double kspc() const;

           // Backspace-Kategorisierung (Heuristik)
    int motoricBackspaces() const;        // < 200ms (reflexartig)
    int automatizationBackspaces() const; // 200-1000ms (Tastsuche)
    int contentBackspaces() const;        // > 1000ms (Nachdenken)

    double motoricBackspaceRate() const;        // Prozent
    double automatizationBackspaceRate() const; // Prozent
    double contentBackspaceRate() const;        // Prozent

           // Alle Backspace-Events mit Timing
    QList<BackspaceEvent> getBackspaceEvents() const;

           // N-Gramm Statistiken (Wrapper für computeStats mit flexibler n-Gramm-Größe)
    QList<NgramStat> ngramStatistics(int ngramSize = 2, int minOccurrences = 2);

private:
    QList<Keystroke> m_keystrokes;
    int m_ngramSize = 5;
    TimingMode m_timingMode = TimingMode::KeyDownOnly;

    // Backspace-Tracking mit Kontext
    QList<BackspaceEvent> m_backspaceEvents;

    // Speichere Backspace mit gelöschtem Zeichen
    void addBackspaceWithContext(qint64 timestamp, QChar deletedChar);
};

#endif // NGRAMANALYZER_H

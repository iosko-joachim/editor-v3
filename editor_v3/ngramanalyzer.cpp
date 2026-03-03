#include "ngramanalyzer.h"

#include <QtMath>
#include <algorithm>
#include <utility>  // für std::as_const

NgramAnalyzer::NgramAnalyzer() { }

void NgramAnalyzer::addKeystroke(QChar ch, qint64 downMs, qint64 upMs)
{
    if (!ch.isPrint())
        return;

    Keystroke ks;
    ks.character = ch;
    ks.downMs = downMs;
    ks.upMs = upMs;
    m_keystrokes.append(ks);
}

void NgramAnalyzer::clear()
{
    m_keystrokes.clear();
    m_backspaceEvents.clear();  // NEU: Auch Backspace-Events löschen
}

void NgramAnalyzer::addBackspace(qint64 timestamp)
{
    Keystroke ks;
    ks.character = '\b';
    ks.downMs = timestamp;
    ks.upMs = timestamp;
    m_keystrokes.append(ks);
}

void NgramAnalyzer::setNgramSize(int size)
{
    if (size >= 2 && size <= 7)
        m_ngramSize = size;
}

int NgramAnalyzer::ngramSize() const { return m_ngramSize; }

void NgramAnalyzer::setTimingMode(TimingMode mode) { m_timingMode = mode; }

TimingMode NgramAnalyzer::timingMode() const { return m_timingMode; }

int NgramAnalyzer::keystrokeCount() const { return m_keystrokes.size(); }

void NgramAnalyzer::patchLastUpMs(QChar ch, qint64 upMs)
{
    // Walk backwards to find the most recent keystroke for this character with upMs == -1
    for (int i = m_keystrokes.size() - 1; i >= 0; --i) {
        if (m_keystrokes[i].character == ch && m_keystrokes[i].upMs == -1) {
            m_keystrokes[i].upMs = upMs;
            return;
        }
    }
}

QList<NgramStat> NgramAnalyzer::computeStats(int minOccurrences) const
{
    if (m_keystrokes.size() < m_ngramSize)
        return {};

           // Collect timing intervals for each n-gram
    QHash<QString, QList<double>> ngramTimings;

    for (int i = 0; i <= m_keystrokes.size() - m_ngramSize; ++i) {
        QString ngram;
        double totalInterval = 0.0;
        bool valid = true;

        for (int j = 0; j < m_ngramSize; ++j) {
            ngram.append(m_keystrokes[i + j].character);
        }

               // Calculate timing for this n-gram occurrence
        if (m_timingMode == TimingMode::KeyDownOnly) {
            // IKI: sum of key-down to key-down intervals within the n-gram
            for (int j = 0; j < m_ngramSize - 1; ++j) {
                qint64 interval
                    = m_keystrokes[i + j + 1].downMs - m_keystrokes[i + j].downMs;
                if (interval <= 0) {
                    valid = false;
                    break;
                }
                totalInterval += static_cast<double>(interval);
            }
        } else {
            // KeyDown+KeyUp: sum of hold times + flight times
            for (int j = 0; j < m_ngramSize; ++j) {
                const auto& ks = m_keystrokes[i + j];
                if (ks.upMs < ks.downMs) {
                    valid = false;
                    break;
                }
                // Hold time
                totalInterval += static_cast<double>(ks.upMs - ks.downMs);
            }
            if (valid) {
                // Flight times between consecutive keys
                for (int j = 0; j < m_ngramSize - 1; ++j) {
                    qint64 flight
                        = m_keystrokes[i + j + 1].downMs - m_keystrokes[i + j].upMs;
                    totalInterval += static_cast<double>(flight);
                }
            }
        }

        if (valid && totalInterval > 0.0) {
            ngramTimings[ngram].append(totalInterval);
        }
    }

           // Compute statistics
    QList<NgramStat> results;

    for (auto it = ngramTimings.constBegin(); it != ngramTimings.constEnd(); ++it) {
        const QList<double>& timings = it.value();
        if (timings.size() < minOccurrences)
            continue;

        NgramStat stat;
        stat.ngram = it.key();
        stat.occurrences = timings.size();

               // Mean
        double sum = 0.0;
        for (double t : timings)
            sum += t;
        stat.meanMs = sum / timings.size();

               // Standard deviation
        double sqSum = 0.0;
        for (double t : timings) {
            double diff = t - stat.meanMs;
            sqSum += diff * diff;
        }
        stat.stddevMs = qSqrt(sqSum / timings.size());

               // Coefficient of variation
        stat.coeffOfVariation
            = (stat.meanMs > 0.0) ? (stat.stddevMs / stat.meanMs) : 0.0;

        results.append(stat);
    }

           // Sort by CV descending (most inconsistent first)
    std::sort(results.begin(), results.end(),
        [](const NgramStat& a, const NgramStat& b) {
            return a.coeffOfVariation > b.coeffOfVariation;
        });

    return results;
}

// Backspace-Zähler
int NgramAnalyzer::backspaceCount() const
{
    int count = 0;
    for (const auto& ks : m_keystrokes) {
        // Backspace ist '\b' oder QChar(0x08)
        if (ks.character == '\b' || ks.character == QChar(0x08)) {
            count++;
        }
    }
    return count;
}

// Backspace-Rate in Prozent
double NgramAnalyzer::backspaceRate() const
{
    int total = m_keystrokes.size();
    if (total == 0) return 0.0;

    int backspaces = backspaceCount();
    return (static_cast<double>(backspaces) / total) * 100.0;
}

// Pausen-Analyse
QList<NgramAnalyzer::PauseInfo> NgramAnalyzer::findPauses(qint64 thresholdMs) const
{
    QList<PauseInfo> pauses;

    if (m_keystrokes.size() < 2) {
        return pauses;
    }

    for (int i = 1; i < m_keystrokes.size(); ++i) {
        // IKI = Inter-Keystroke Interval
        qint64 iki = m_keystrokes[i].downMs - m_keystrokes[i-1].downMs;

        if (iki > thresholdMs) {
            PauseInfo pause;
            pause.position = i;
            pause.durationMs = iki;

                   // Kontext: 5 Zeichen davor
            QString before;
            for (int j = qMax(0, i - 5); j < i; ++j) {
                before += m_keystrokes[j].character;
            }
            pause.contextBefore = before;

                   // Kontext: 5 Zeichen danach
            QString after;
            for (int j = i; j < qMin(m_keystrokes.size(), i + 5); ++j) {
                after += m_keystrokes[j].character;
            }
            pause.contextAfter = after;

            pauses.append(pause);
        }
    }

    return pauses;
}

// Legal-Zeichen-Analyse
QList<NgramAnalyzer::LegalCharStat> NgramAnalyzer::analyzeLegalChars() const
{
    // Deutsche Legal-Zeichen
    static const QList<QChar> legalChars = {
        u'§', u'ä', u'ö', u'ü', u'Ä', u'Ö', u'Ü', u'ß'
    };

           // Zähle jeden Legal-Char
    QHash<QChar, QList<qint64>> charTimings;

    for (int i = 1; i < m_keystrokes.size(); ++i) {
        QChar ch = m_keystrokes[i].character;
        if (legalChars.contains(ch)) {
            // Zeit seit letztem Tastendruck
            qint64 iki = m_keystrokes[i].downMs - m_keystrokes[i-1].downMs;
            charTimings[ch].append(iki);
        }
    }

           // Erstelle Statistiken
    QList<LegalCharStat> results;

    for (const auto& ch : legalChars) {
        if (charTimings.contains(ch) && !charTimings[ch].isEmpty()) {
            LegalCharStat stat;
            stat.character = ch;
            stat.count = charTimings[ch].size();

                   // Durchschnitt
            qint64 sum = 0;
            for (qint64 t : charTimings[ch]) {
                sum += t;
            }
            stat.avgTimeMs = static_cast<double>(sum) / stat.count;

            results.append(stat);
        }
    }

           // Sortiere nach Häufigkeit (absteigend)
    std::sort(results.begin(), results.end(),
        [](const LegalCharStat& a, const LegalCharStat& b) {
            return a.count > b.count;
        });

    return results;
}

// WPM über Zeit
QList<double> NgramAnalyzer::calculateWPMOverTime(int intervalSeconds) const
{
    QList<double> wpmList;

    if (m_keystrokes.size() < 2) {
        return wpmList;
    }

    qint64 startTime = m_keystrokes.first().downMs;
    qint64 endTime = m_keystrokes.last().downMs;
    qint64 duration = endTime - startTime;

    if (duration <= 0) {
        return wpmList;
    }

    qint64 intervalMs = intervalSeconds * 1000;
    int numIntervals = (duration / intervalMs) + 1;

    for (int i = 0; i < numIntervals; ++i) {
        qint64 intervalStart = startTime + (i * intervalMs);
        qint64 intervalEnd = intervalStart + intervalMs;

               // Zähle Zeichen in diesem Interval
        int charCount = 0;
        for (const auto& ks : m_keystrokes) {
            if (ks.downMs >= intervalStart && ks.downMs < intervalEnd) {
                charCount++;
            }
        }

               // WPM berechnen (5 Zeichen = 1 Wort)
               // Normalisiere auf 60 Sekunden
        double actualSeconds = qMin(intervalMs, duration - (i * intervalMs)) / 1000.0;
        double wordsPerMinute = (charCount / 5.0) * (60.0 / actualSeconds);

        wpmList.append(wordsPerMinute);
    }

    return wpmList;
}

// Gesamtzeit der Session
double NgramAnalyzer::totalSessionTimeSeconds() const
{
    if (m_keystrokes.size() < 2) {
        return 0.0;
    }

    qint64 start = m_keystrokes.first().downMs;
    qint64 end = m_keystrokes.last().downMs;

    return (end - start) / 1000.0;  // ms zu Sekunden
}

// ===== NEUE IMPLEMENTIERUNGEN =====

// Backspace mit Kontext hinzufügen (NEUE METHODE)
void NgramAnalyzer::addBackspaceWithContext(qint64 timestamp, QChar deletedChar)
{
    // Füge Backspace zu Keystroke-Liste hinzu
    addBackspace(timestamp);

    // Erstelle BackspaceEvent mit Kontext
    BackspaceEvent event;
    event.timestamp = timestamp;
    event.deletedChar = deletedChar;

    // Berechne Zeit seit letztem Nicht-Backspace-Tastendruck
    event.timeSinceLastKeystroke = 0;
    for (int i = m_keystrokes.size() - 2; i >= 0; --i) {
        if (m_keystrokes[i].character != '\b' && m_keystrokes[i].character != QChar(0x08)) {
            event.timeSinceLastKeystroke = timestamp - m_keystrokes[i].downMs;
            break;
        }
    }

    m_backspaceEvents.append(event);
}

// Gross WPM (rohe WPM, alle Zeichen) - UMBENANNT von averageWPM()
double NgramAnalyzer::grossWPM() const
{
    double sessionSeconds = totalSessionTimeSeconds();
    if (sessionSeconds == 0.0) return 0.0;

    // Zähle alle Zeichen außer Backspaces
    int charCount = 0;
    for (const auto& ks : m_keystrokes) {
        if (ks.character != '\b' && ks.character != QChar(0x08)) {
            charCount++;
        }
    }

    // 5 Zeichen = 1 Wort, normalisiert auf Minuten
    return (charCount / 5.0) * (60.0 / sessionSeconds);
}

// Net WPM (netto WPM, abzüglich Fehler) - ANGEPASST
double NgramAnalyzer::netWPM() const
{
    double sessionSeconds = totalSessionTimeSeconds();
    if (sessionSeconds == 0.0) return 0.0;

    // Zähle alle Zeichen außer Backspaces
    int charCount = 0;
    for (const auto& ks : m_keystrokes) {
        if (ks.character != '\b' && ks.character != QChar(0x08)) {
            charCount++;
        }
    }

    // Ziehe Fehler ab (jeder Backspace = 1 Fehler)
    int errors = backspaceCount();
    int netChars = qMax(0, charCount - errors);

    // 5 Zeichen = 1 Wort, normalisiert auf Minuten
    return (netChars / 5.0) * (60.0 / sessionSeconds);
}

// Accuracy (Genauigkeit) - ANGEPASST
double NgramAnalyzer::accuracy() const
{
    int total = keystrokeCount();
    if (total == 0) return 1.0;

    int errors = backspaceCount();
    int correct = total - errors;

    return static_cast<double>(correct) / total;
}

// KSPC (Keystrokes per Character) - KOMPLETT NEU
double NgramAnalyzer::kspc() const
{
    // Zähle Nicht-Backspace Zeichen
    int charCount = 0;
    for (const auto& ks : m_keystrokes) {
        if (ks.character != '\b' && ks.character != QChar(0x08)) {
            charCount++;
        }
    }

    if (charCount == 0) return 0.0;

    // Gesamte Tastendrücke / Finale Zeichen
    int totalKeystrokes = keystrokeCount();
    return static_cast<double>(totalKeystrokes) / charCount;
}

// ===== BACKSPACE-FAKTORISIERUNG (KOMPLETT NEU) =====

// Alle Backspace-Events mit neuem Format
QList<NgramAnalyzer::BackspaceEvent> NgramAnalyzer::getBackspaceEvents() const
{
    // Wenn wir bereits Events haben (über addBackspaceWithContext), gib diese zurück
    if (!m_backspaceEvents.isEmpty()) {
        return m_backspaceEvents;
    }

    // Fallback: Erstelle Events aus Keystroke-Liste
    QList<BackspaceEvent> events;

    for (int i = 0; i < m_keystrokes.size(); ++i) {
        const auto& ks = m_keystrokes[i];

        // Ist das ein Backspace?
        if (ks.character == '\b' || ks.character == QChar(0x08)) {
            BackspaceEvent event;
            event.timestamp = ks.downMs;

            // Finde letzten echten Tastendruck (kein Backspace)
            event.timeSinceLastKeystroke = 0;
            event.deletedChar = QChar();

            for (int j = i - 1; j >= 0; --j) {
                if (m_keystrokes[j].character != '\b' &&
                    m_keystrokes[j].character != QChar(0x08)) {
                    event.timeSinceLastKeystroke = ks.downMs - m_keystrokes[j].downMs;
                    event.deletedChar = m_keystrokes[j].character;
                    break;
                }
            }

            events.append(event);
        }
    }

    return events;
}

// Motorische Backspaces (< 200ms)
int NgramAnalyzer::motoricBackspaces() const
{
    int count = 0;
    auto events = getBackspaceEvents();

    for (const auto& event : std::as_const(events)) {
        if (event.timeSinceLastKeystroke > 0 && event.timeSinceLastKeystroke < 200) {
            count++;
        }
    }
    return count;
}

// Automatisierungs-Backspaces (200-1000ms)
int NgramAnalyzer::automatizationBackspaces() const
{
    int count = 0;
    auto events = getBackspaceEvents();

    for (const auto& event : std::as_const(events)) {
        if (event.timeSinceLastKeystroke >= 200 && event.timeSinceLastKeystroke <= 1000) {
            count++;
        }
    }
    return count;
}

// Inhaltliche Backspaces (> 1000ms)
int NgramAnalyzer::contentBackspaces() const
{
    int count = 0;
    auto events = getBackspaceEvents();

    for (const auto& event : std::as_const(events)) {
        if (event.timeSinceLastKeystroke > 1000) {
            count++;
        }
    }
    return count;
}

// Motorische Backspace-Rate
double NgramAnalyzer::motoricBackspaceRate() const
{
    int total = backspaceCount();
    if (total == 0) return 0.0;

    return (static_cast<double>(motoricBackspaces()) / total) * 100.0;
}

// Automatisierungs-Backspace-Rate
double NgramAnalyzer::automatizationBackspaceRate() const
{
    int total = backspaceCount();
    if (total == 0) return 0.0;

    return (static_cast<double>(automatizationBackspaces()) / total) * 100.0;
}

// Inhaltliche Backspace-Rate
double NgramAnalyzer::contentBackspaceRate() const
{
    int total = backspaceCount();
    if (total == 0) return 0.0;

    return (static_cast<double>(contentBackspaces()) / total) * 100.0;
}

// N-Gramm Statistiken (Wrapper für computeStats mit flexibler n-Gramm-Größe)
QList<NgramStat> NgramAnalyzer::ngramStatistics(int ngramSize, int minOccurrences)
{
    int oldSize = m_ngramSize;
    setNgramSize(ngramSize);
    QList<NgramStat> stats = computeStats(minOccurrences);
    setNgramSize(oldSize);
    return stats;
}

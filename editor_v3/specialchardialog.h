#ifndef SPECIALCHARDIALOG_H
#define SPECIALCHARDIALOG_H

#include <QDialog>
#include <QString>

/// Dialog zur Auswahl von Sonderzeichen (§, €, ©, etc.).
class SpecialCharDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SpecialCharDialog(QWidget *parent = nullptr);

    /// Gibt das ausgewählte Zeichen zurück (leer bei Abbruch).
    QString selectedChar() const;

private:
    QString m_selected;
};

#endif // SPECIALCHARDIALOG_H

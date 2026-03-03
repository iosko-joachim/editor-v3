#ifndef TABLEDIALOG_H
#define TABLEDIALOG_H

#include <QDialog>
#include <QSpinBox>

/// Dialog zum Einfügen einer Tabelle (Zeilen × Spalten).
class TableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TableDialog(QWidget *parent = nullptr);

    int rows() const;
    int columns() const;

private:
    QSpinBox *m_rows;
    QSpinBox *m_cols;
};

#endif // TABLEDIALOG_H

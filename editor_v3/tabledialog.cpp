#include "tabledialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

TableDialog::TableDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Tabelle einfügen");
    setFixedSize(280, 160);

    auto *mainLayout = new QVBoxLayout(this);

    auto *form = new QFormLayout;

    m_rows = new QSpinBox;
    m_rows->setRange(1, 50);
    m_rows->setValue(3);
    form->addRow("Zeilen:", m_rows);

    m_cols = new QSpinBox;
    m_cols->setRange(1, 20);
    m_cols->setValue(3);
    form->addRow("Spalten:", m_cols);

    mainLayout->addLayout(form);
    mainLayout->addSpacing(10);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
    );
    buttons->button(QDialogButtonBox::Ok)->setText("Einfügen");
    buttons->button(QDialogButtonBox::Cancel)->setText("Abbrechen");

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttons);
}

int TableDialog::rows() const
{
    return m_rows->value();
}

int TableDialog::columns() const
{
    return m_cols->value();
}

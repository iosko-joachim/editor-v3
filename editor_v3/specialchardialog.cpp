#include "specialchardialog.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFont>

SpecialCharDialog::SpecialCharDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Sonderzeichen einfügen");
    setMinimumSize(420, 320);
    setStyleSheet(
        "QDialog { background-color: #f8f8f8; color: #000000; }"
        "QLabel { color: #000000; }"
    );

    // Juristisch relevante und allgemein nützliche Sonderzeichen
    QStringList chars = {
        "§", "€", "£", "¥", "¢",
        "©", "®", "™", "°", "±",
        "µ", "¶", "†", "‡", "•",
        "–", "—", "…", "‰", "‹",
        "›", "«", "»", "¡", "¿",
        "Ä", "Ö", "Ü", "ä", "ö",
        "ü", "ß", "á", "é", "í",
        "ó", "ú", "à", "è", "ì",
        "ò", "ù", "â", "ê", "î",
        "ô", "û", "ñ", "ç", "ø",
        "æ", "å", "×", "÷", "≤",
        "≥", "≠", "≈", "∞", "∑",
        "√", "∆", "π", "Ω", "α",
        "β", "γ", "δ", "ε", "λ"
    };

    auto *mainLayout = new QVBoxLayout(this);

    auto *label = new QLabel("Zeichen auswählen:");
    label->setStyleSheet("font-weight: bold; margin-bottom: 4px;");
    mainLayout->addWidget(label);

    auto *grid = new QGridLayout;
    grid->setSpacing(3);

    const int cols = 10;
    for (int i = 0; i < chars.size(); ++i) {
        const QString &ch = chars[i];
        auto *btn = new QPushButton(ch);
        btn->setFixedSize(36, 36);
        btn->setFont(QFont("Segoe UI", 14));
        btn->setStyleSheet(
            "QPushButton { border: 1px solid #ccc; border-radius: 3px; background: #fafafa; color: #000000; }"
            "QPushButton:hover { background: #e0e8f0; border-color: #8899aa; color: #000000; }"
        );

        // Qt 6.10 Fix: static_cast<int> für char16_t → int Konversion
        btn->setToolTip(
            QString("U+%1")
                .arg(static_cast<int>(ch.at(0).unicode()), 4, 16, QChar('0'))
                .toUpper()
        );

        connect(btn, &QPushButton::clicked, this, [this, ch]() {
            m_selected = ch;
            accept();
        });

        grid->addWidget(btn, i / cols, i % cols);
    }

    mainLayout->addLayout(grid);

    // Abbrechen-Button
    auto *cancelBtn = new QPushButton("Abbrechen");
    cancelBtn->setFixedWidth(120);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto *bottomLayout = new QGridLayout;
    bottomLayout->addWidget(cancelBtn, 0, 0, Qt::AlignRight);
    mainLayout->addLayout(bottomLayout);
}

QString SpecialCharDialog::selectedChar() const
{
    return m_selected;
}

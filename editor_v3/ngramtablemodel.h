#ifndef NGRAMTABLEMODEL_H
#define NGRAMTABLEMODEL_H

#include <QAbstractTableModel>

#include "ngramanalyzer.h"

class NgramTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column { Rank = 0, Ngram, Occurrences, MeanMs, StdDev, CV, ColumnCount };

    explicit NgramTableModel(QObject* parent = nullptr);

    void setStats(const QList<NgramStat>& stats);
    void clear();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(
        int section, Qt::Orientation orientation, int role) const override;

private:
    QList<NgramStat> m_stats;
};

#endif // NGRAMTABLEMODEL_H

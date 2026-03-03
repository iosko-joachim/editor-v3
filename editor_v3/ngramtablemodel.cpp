#include "ngramtablemodel.h"

NgramTableModel::NgramTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

void NgramTableModel::setStats(const QList<NgramStat>& stats)
{
    beginResetModel();
    m_stats = stats;
    endResetModel();
}

void NgramTableModel::clear()
{
    beginResetModel();
    m_stats.clear();
    endResetModel();
}

int NgramTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_stats.size();
}

int NgramTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant NgramTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_stats.size())
        return {};

    const NgramStat& stat = m_stats.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case Rank:
            return index.row() + 1;
        case Ngram:
            return QString("\"%1\"").arg(stat.ngram);
        case Occurrences:
            return stat.occurrences;
        case MeanMs:
            return QString::number(stat.meanMs, 'f', 1);
        case StdDev:
            return QString::number(stat.stddevMs, 'f', 1);
        case CV:
            return QString::number(stat.coeffOfVariation, 'f', 3);
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == Ngram)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);
    }

    // For sorting: return raw numeric values
    if (role == Qt::UserRole) {
        switch (index.column()) {
        case Rank:
            return index.row() + 1;
        case Ngram:
            return stat.ngram;
        case Occurrences:
            return stat.occurrences;
        case MeanMs:
            return stat.meanMs;
        case StdDev:
            return stat.stddevMs;
        case CV:
            return stat.coeffOfVariation;
        }
    }

    return {};
}

QVariant NgramTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch (section) {
    case Rank:
        return tr("Rank");
    case Ngram:
        return tr("N-Gram");
    case Occurrences:
        return tr("Count");
    case MeanMs:
        return tr("Mean (ms)");
    case StdDev:
        return tr("StdDev");
    case CV:
        return tr("CV");
    }

    return {};
}

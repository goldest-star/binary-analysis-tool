#ifndef COLORMAPMODEL_H
#define COLORMAPMODEL_H

#include <QAbstractTableModel>
#include <QColor>

class ColorMapModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ColorMapModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QList<QPair<QString, QColor>> getMappings();
    void setMappings(QList<QPair<QString, QColor>> mappings);

public slots:
    void setRemapLength(int length);

private:
    void initializeMappings();

    int m_remapLength;
    QList<QPair<QString, QColor>> m_mappings;
    QList<QColor> m_defaultColors;

};


#endif // COLORMAPMODEL_H

#ifndef BITCONTAINERTREEMODEL_H
#define BITCONTAINERTREEMODEL_H

#include "bitcontainer.h"
#include <QAbstractItemModel>

#include "hobbits-widgets_global.h"

class HOBBITSWIDGETSSHARED_EXPORT BitContainerTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit BitContainerTreeModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;
    QModelIndex index(
            int row,
            int column,
            const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;


    QModelIndex addContainer(QSharedPointer<BitContainer> bitContainer);
    void removeContainer(const QModelIndex &index);
    void removeAllContainers();
    QSharedPointer<BitContainer> getContainer(const QModelIndex &index) const;
    QModelIndex getContainerIndex(const QUuid &id) const;

    QList<QSharedPointer<BitContainer>> getContainers() const;
    QSharedPointer<BitContainer> getContainerById(QUuid id) const;

Q_SIGNALS:
    void containerAdded(QSharedPointer<BitContainer> bitContainer);

public Q_SLOTS:
    void updateAll();

private:
    QUuid getIndexId(const QModelIndex &index) const;
    int getContainerRow(BitContainer *bitContainer) const;
    QModelIndex getContainerParentIndex(BitContainer *bitContainer) const;
    QModelIndex addContainerImpl(QSharedPointer<BitContainer> bitContainer);

    QUuid m_rootUuid;

    QMap<QUuid, QSharedPointer<BitContainer>> m_containerMap;
    QMap<QUuid, QList<QSharedPointer<BitContainer>>> m_containerGroups;
};

#endif // BITCONTAINERTREEMODEL_H

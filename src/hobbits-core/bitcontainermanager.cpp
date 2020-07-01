#include "bitcontainermanager.h"

BitContainerManager::BitContainerManager(QObject *parent) :
    QObject(parent),
    m_bitContainerTreeModel(QSharedPointer<BitContainerTreeModel>(new BitContainerTreeModel())),
    m_currSelectionModel(QSharedPointer<QItemSelectionModel>(new QItemSelectionModel))
{
    m_currSelectionModel->setModel(m_bitContainerTreeModel.data());
    connect(
            m_currSelectionModel.data(),
            SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
            this,
            SLOT(manageSelectionChanged(const QItemSelection&,const QItemSelection&)));

    connect(
                m_bitContainerTreeModel.data(),
                SIGNAL(containerAdded(QSharedPointer<BitContainer>)),
                this,
                SIGNAL(containerAdded(QSharedPointer<BitContainer>)));
}

void BitContainerManager::manageSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    auto newBits = QSharedPointer<BitContainer>();
    auto oldBits = QSharedPointer<BitContainer>();

    if (selected.indexes().size() == 1 && selected.indexes().at(0).isValid()) {
        newBits = m_bitContainerTreeModel->getContainer(selected.indexes().first());
    }
    if (deselected.indexes().size() == 1 && deselected.indexes().at(0).isValid()) {
        oldBits = m_bitContainerTreeModel->getContainer(deselected.indexes().first());
    }

    emit currSelectionChanged(newBits, oldBits);
}

QSharedPointer<BitContainer> BitContainerManager::getCurrentContainer()
{
    if (m_currSelectionModel->selection().isEmpty()) {
        return QSharedPointer<BitContainer>();
    }
    return m_bitContainerTreeModel->getContainer(m_currSelectionModel->selection().indexes().first());
}

QSharedPointer<BitContainerTreeModel> BitContainerManager::getTreeModel()
{
    return m_bitContainerTreeModel;
}

QSharedPointer<BitContainer> BitContainerManager::getContainerById(QUuid id)
{
    return m_bitContainerTreeModel->getContainerById(id);
}

QSharedPointer<QItemSelectionModel> BitContainerManager::getCurrSelectionModel()
{
    return m_currSelectionModel;
}

void BitContainerManager::deleteCurrentContainer()
{
    if (m_currSelectionModel->selection().indexes().isEmpty()) {
        return;
    }
    QModelIndex index = m_currSelectionModel->selection().indexes().first();
    if (!index.isValid()) {
        return;
    }
    m_bitContainerTreeModel->removeContainer(index);
}

void BitContainerManager::deleteAllContainers()
{
    m_bitContainerTreeModel->removeAllContainers();
}

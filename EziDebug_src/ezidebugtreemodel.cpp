#include <QString>
#include <QList>
#include <QDebug>
#include <QFileIconProvider>
#include "ezidebuginstancetreeitem.h"
#include "ezidebugtreemodel.h"


EziDebugTreeModel::EziDebugTreeModel(EziDebugInstanceTreeItem* item, QObject *parent)\
    :QAbstractItemModel(parent)
{
    m_pinstanceTopItem = item ;
    m_pheadItem = new EziDebugInstanceTreeItem("module","instance");
    m_pheadItem->setItemParent(NULL);
    m_pheadItem->appendChild(m_pinstanceTopItem);

    m_pinstanceTopItem->setItemParent(m_pheadItem);

}


EziDebugTreeModel::~EziDebugTreeModel()
{
    if(m_pheadItem)
        delete m_pheadItem ;
    m_pheadItem = NULL ;
}

QVariant EziDebugTreeModel::data(const QModelIndex &index, int role) const
{

    QIcon iexistChainIcon(tr(":/images/yeschain.png"));
    QIcon inoChainIcon(tr(":/images/nochain.png"));


    EziDebugInstanceTreeItem* pparentItem = static_cast<EziDebugInstanceTreeItem*>(index.internalPointer());

    // index ��Ч �򷵻� QVariant()
    if (!index.isValid())
        return QVariant();
    //
    if (role == Qt::DisplayRole)
    {
        return (pparentItem->getNameData());
    }

    if (role == Qt::DecorationRole)
    {
        if (index.column() == 0)
        {
            if(pparentItem->getScanChainInfo())
                return iexistChainIcon ;
            else
                return inoChainIcon ;
        }
            //return iconProvider.icon(QFileIconProvider::Folder);
       // return iconProvider.icon(QFileIconProvider::File);
    }
    return QVariant();
}

Qt::ItemFlags EziDebugTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant EziDebugTreeModel::headerData(int section, Qt::Orientation orientation, \
                    int role) const
{
    QString iheadData = "Node Tree" ;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return iheadData ;

    return QVariant();
}

// ��� parent �� row �� column �� �� modelIndex
QModelIndex EziDebugTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    //�������������Χ֮��
    if(!hasIndex(row, column, parent))
    {
      return QModelIndex();
    }

    EziDebugInstanceTreeItem * pparentItem;

    if (!parent.isValid())
        //return QModelIndex();
        pparentItem = m_pheadItem ;
    else
        pparentItem = static_cast<EziDebugInstanceTreeItem*>(parent.internalPointer());

    EziDebugInstanceTreeItem *pchildItem = pparentItem->child(row);
    // qDebug() << pchildItem << "first" << column ;
    //qDebug() << pchildItem << row << column << pparentItem;
    //qDebug() << QObject::tr("����index") << QObject::tr("����") << row;

    //qDebug() << "index" << pchildItem->getModuleName() << pchildItem->getInstanceName()  ;
    if (pchildItem)
        return createIndex(row, column, pchildItem);
    else
        return QModelIndex();

}

// ���� child �� modelIndex ��ȡ  parent �� modelIndex
QModelIndex EziDebugTreeModel::parent(const QModelIndex &child) const
{
    //qDebug() << "parent" << __LINE__  ;
    if(child.isValid())
    {
        EziDebugInstanceTreeItem *pchildItem = static_cast<EziDebugInstanceTreeItem*>(child.internalPointer());

        EziDebugInstanceTreeItem *pparentItem = pchildItem->parent();
        // qDebug() << pchildItem << "result" << child.row() << child.column();

        if (pparentItem == m_pheadItem)
        {
            return QModelIndex();
        }
        else
        {
            //  return createIndex(0, 0, pparentItem);
            //qDebug() << "parent" <<  pparentItem->row() ;
            return createIndex(pparentItem->row(), 0, pparentItem);
        }
    }
    return QModelIndex();
}

// ���ڵ������ж�����
int EziDebugTreeModel::rowCount(const QModelIndex &parent) const
{
    //qDebug() << "rowCount" << __LINE__  ;
    EziDebugInstanceTreeItem *pparentItem;
    if (parent.column() > 1)
        return 0 ;

    if (!parent.isValid())
        pparentItem = m_pheadItem ;
    else
        pparentItem = static_cast<EziDebugInstanceTreeItem*>(parent.internalPointer());
    //qDebug() << QObject::tr("��ȡ�ڵ����������") << pparentItem->getModuleName() << pparentItem->childCount() ;
    return pparentItem->childCount();
}

int EziDebugTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 1 ;
    else
        return 1 ;

}

EziDebugInstanceTreeItem* EziDebugTreeModel::getInstanceTreeRootItem(void)
{
    return m_pinstanceTopItem ;
}

void  EziDebugTreeModel::setInstanceTreeRootItem(EziDebugInstanceTreeItem* item)
{
    m_pheadItem->removeChild(m_pinstanceTopItem);

    m_pinstanceTopItem = item ;

    m_pheadItem->appendChild(m_pinstanceTopItem);

    m_pinstanceTopItem->setItemParent(m_pheadItem);

    return ;
}



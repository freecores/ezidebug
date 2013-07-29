#include <QString>
#include <QStringList>
#include <QDir>
#include <QMap>
#include <QDebug>
#include <QObject>

#include "ezidebuginstancetreeitem.h"

#include "ezidebugmodule.h"
#include "ezidebugvlgfile.h"
#include "ezidebugvhdlfile.h"
#include "ezidebugscanchain.h"


EziDebugPrj* EziDebugInstanceTreeItem::sm_pprj = 0 ;

EziDebugInstanceTreeItem::EziDebugInstanceTreeItem(const QString instance, const QString module)
    :m_iinstanceName(instance),m_imoduleName(module)
{
    m_pChainInfo = NULL ;
}

EziDebugInstanceTreeItem:: ~EziDebugInstanceTreeItem()
{
    EziDebugInstanceTreeItem *pitem = NULL  ;

    for(int i = 0 ; i < m_ichildModules.count() ; i++)
    {
        pitem = m_ichildModules.at(i) ;
        if(pitem)
            delete pitem ;
        pitem = NULL ;
    }

}

EziDebugInstanceTreeItem* EziDebugInstanceTreeItem::parent(void) const
{
    return m_pparentInstance ;
}

EziDebugInstanceTreeItem* EziDebugInstanceTreeItem::child(int num) const
{
    if((num >= 0)&&(num < m_ichildModules.count()))
    {   
        //qDebug() <<  m_imoduleName <<  m_iinstanceName ;
        return m_ichildModules[num] ;
    }
    else
    {
        qDebug() << " the num have exceed the scope ! " ;
        return NULL ;
    }
}

void EziDebugInstanceTreeItem::appendChild(EziDebugInstanceTreeItem *child)
{
    m_ichildModules.append(child);
}

void EziDebugInstanceTreeItem::removeChild(EziDebugInstanceTreeItem *child)
{
    for(int i = 0 ; i < m_ichildModules.count() ;i++)
    {
        if(m_ichildModules.at(i) == child)
        {
            m_ichildModules.removeAt(i);
            break ;
        }
    }
    return ;
}

void EziDebugInstanceTreeItem::setScanChainInfo(EziDebugScanChain * chaininfo)
{   
    m_pChainInfo = chaininfo ;
    return ;
}

EziDebugScanChain* EziDebugInstanceTreeItem::getScanChainInfo()
{
    return m_pChainInfo ;
}



QString EziDebugInstanceTreeItem::getNameData() const
{
    return  (m_imoduleName + QObject::tr(":") + m_iinstanceName) ;
}

const QString &EziDebugInstanceTreeItem::getModuleName() const
{
    return m_imoduleName ;
}

const QString &EziDebugInstanceTreeItem::getInstanceName() const
{
    return m_iinstanceName ;
}

QMap<QString,QString> EziDebugInstanceTreeItem::getInstancePortMapTbl(const QString &instancename) const
{
    return m_iinstancePortMap.value(instancename) ;
}

//void EziDebugInstanceTreeItem::traverseChainTreeItem(void)
//{
//    for(int i = 0 ;i < m_ichildModules.count(); i++)
//    {
//        m_ichildModules.at(i)->traverseChainTreeItem();
//        int ntimes = sm_pprj->getPrjModuleMap().value(getModuleName())->getInstanceTimes();
//        ntimes += 1 ;
//        sm_pprj->getPrjModuleMap().value(getModuleName())->setInstanceTimes(ntimes);
//    }
//    return ;
//}

int EziDebugInstanceTreeItem::getAllRegNum(const QString &clock,QString chainname ,int &regbitcount , int &bitwidth ,const QStringList &instancelist)    // �ӱ��ڵ㿪ʼ���������ӽڵ�
{
    EziDebugModule *pmodule = NULL ;
    QVector<EziDebugModule::RegStructure*> sregVec ;
    QVector<EziDebugModule::RegStructure*> vregVec ;
    int nregCount = 0 ;
    int nlistNum = 0 ;
    int nmaxRegBitNum = sm_pprj->getMaxRegNumPerChain() ;
    int nsRegNumSum = 0 ;
    int nvRegNumSum = 0 ;
    int nresult = 0 ;

    // �������� �� module �� �� clock
    // �����ڵ�ʱӦ ���� ���ڵ��clockmap �ҵ�����module����֮��Ӧ�� clock

    for( nlistNum = 0 ; nlistNum < m_ichildModules.count() ;nlistNum++)
    {
        pmodule = sm_pprj->getPrjModuleMap().value(m_ichildModules.at(nlistNum)->getModuleName());
        if(pmodule->isLibaryCore())
        {
            continue ;
        }
        QMap<QString,QString> iclockMap = getModuleClockMap(m_ichildModules.at(nlistNum)->getInstanceName()) ;
        QString iclock = iclockMap.value(clock);  // ��ǰmodule�е�clock
        if(!iclock.isEmpty())
        {
            nresult = m_ichildModules.at(nlistNum)->getAllRegNum(clock , chainname ,regbitcount , bitwidth , instancelist);
            if(nresult)
            {
                qDebug() << "EziDebug Error: travereall reg error!" ;
                return 1 ;
            }
        }
    }

    pmodule  = sm_pprj->getPrjModuleMap().value(this->getModuleName());

    pmodule->getAllRegMap(clock,sregVec,vregVec);

    /*
         �жϵ�ǰmodule�Ƿ񵥶�������ֻ�о�̬�Ĵ�����
         YES
         {
            �ж�ʣ���ɨ�������� �Ƿ��ã��������ھɵ�ɨ�����ϼ�����ӣ�
            �������´���һ��ɨ�����������
           ������������µ�ɨ���� ����������Ϊֹ�����������ʣ��ļĴ���������
         }
         NO (����ͬʱ���� ��̬�붯̬�ļĴ��� ��ӷ����� ����Ӿ�̬�ļĴ��� ����Ӷ�̬�ļĴ���)
         {
            ԭ�����ڶ���������Ĵ���������Ϊ���ı仯 �ڲ�ͬ�������� �����˲�ͬ��������
                if(��̬+��̬ < ʣ��Ĵ���������)
                {
                    if(������ɨ����ʱ����һ�ε�����ͬ )
                    {
                       ��ֻ��¼�Ĵ���λ����Ϣ ������κ�
                    }
                    // ���е������  ����ʣ��� ɨ�����Ĵ�������
                }
                else
                {
                    // ���µ�ɨ���� ��ʼ���
                }
         }
         // ����Ŀ���Ǳ�֤�����Ψһ��
    */
    for(;nregCount < sregVec.count() ;nregCount++)
    {
        EziDebugModule::RegStructure* preg = sregVec.at(nregCount) ;
        nsRegNumSum += preg->m_unMaxBitWidth ;
    }

    nregCount = 0 ;

    for(;nregCount < vregVec.count();nregCount++)
    {
        EziDebugModule::RegStructure* preg = sregVec.at(nregCount) ;
        nvRegNumSum += preg->m_unMaxBitWidth ;
    }


    if(pmodule->getConstInstacedTimesPerChain(chainname) == 1)
    {
        for( nregCount = 0 ; nregCount < sregVec.count();nregCount++)
        {
            EziDebugModule::RegStructure* preg = sregVec.at(nregCount);
            int nRegNumCount = 0 ;
            for( ; nRegNumCount < preg->m_unRegNum ; nRegNumCount++)
            {

                if((regbitcount + preg->m_unRegBitWidth) >= nmaxRegBitNum)
                {
                    bitwidth += ((preg->m_unRegBitWidth -(nmaxRegBitNum - regbitcount))/nmaxRegBitNum + 1 );

                    regbitcount = (preg->m_unRegBitWidth -(nmaxRegBitNum - regbitcount))%nmaxRegBitNum ;
                }
                else
                {
                    regbitcount += preg->m_unRegBitWidth ;
                }
            }
        }
    }
    else if(pmodule->getConstInstacedTimesPerChain(chainname) > 1)
    {
        if((nsRegNumSum + nvRegNumSum) <= (nmaxRegBitNum - regbitcount))
        {
            regbitcount += (nvRegNumSum + nsRegNumSum) ;
        }
        else
        {
            bitwidth++ ;
            // �µ�ɨ������ʼ���
            for(nregCount = 0 ; nregCount < sregVec.count();nregCount++)
            {
                EziDebugModule::RegStructure* preg = sregVec.at(nregCount);
                int nRegNumCount = 0 ;
                for( ; nRegNumCount < preg->m_unRegNum ; nRegNumCount++)
                {
                    regbitcount += preg->m_unRegBitWidth ;

                    if(regbitcount >= nmaxRegBitNum)
                    {
                        bitwidth += ((preg->m_unRegBitWidth -(nmaxRegBitNum - regbitcount))/nmaxRegBitNum + 1 );

                        regbitcount = (preg->m_unRegBitWidth -(nmaxRegBitNum - regbitcount))%nmaxRegBitNum ;
                    }
                }
            }

            for(nregCount = 0 ; nregCount < vregVec.count() ;nregCount++)
            {
                EziDebugModule::RegStructure* preg = vregVec.at(nregCount) ;

                if((nmaxRegBitNum - regbitcount) >= preg->m_unMaxBitWidth)
                {
                   regbitcount += preg->m_unMaxBitWidth ;
                }
                else
                {
                    bitwidth++ ;
                    regbitcount = nmaxRegBitNum - preg->m_unMaxBitWidth ;
                }
            }
        }
    }
    else
    {
        // do nothting
        qDebug() << "EziDebug Error: instance times error!";
        return 1 ;
    }
    return 0 ;
}

int  EziDebugInstanceTreeItem::insertScanChain(QMap<QString,SCAN_CHAIN_STRUCTURE*> &chainStructuremap ,EziDebugScanChain* pchain ,QString topinstance)
{
    /*��ȡ���ڵ㡡�ҵ���Ӧ��module���ļ������붨ʱ��ģ�������������Ҫ����ɨ������������������Ӧ�Ķ˿ڡ������Ӷ�ʱ���Լ���������Ӧ�Ķ˿ڡ�*/
    QString iinstanceName =  getInstanceName() ;
    EziDebugInstanceTreeItem* pparent = this->parent();
    EziDebugModule *pparentModule = sm_pprj->getPrjModuleMap().value(pparent->getModuleName()) ;
    EziDebugModule *pmodule = sm_pprj->getPrjModuleMap().value(this->getModuleName()) ;
    QMap<QString,SCAN_CHAIN_STRUCTURE*> ichildChainStructureMap ;


    if(iinstanceName == topinstance)
    {
       /*���붨ʱ��ģ�� �� ���� ���Ӷ�ʱ�� ��Ӧ�Ķ˿�*/      

       //EziDebugModule *pchildModule = sm_pprj->getPrjModuleMap().value(this->getModuleName());
       if(pparentModule->getLocatedFileName().endsWith(".v"))
       {
           //pchain->addToInstanceItemList(pparent->getModuleName(),pparent->getInstanceName());

           /*�����ļ����ҵ� ��Ӧ���ļ�����ָ��*/
           EziDebugVlgFile *pfile = sm_pprj->getPrjVlgFileMap().value(pparentModule->getLocatedFileName()) ;

           if(pfile->addScanChain(EziDebugVlgFile::InsertTimer,chainStructuremap,pchain,this))
           {
               qDebug() << "EziDebug Error: Insert Chain \" InsertTimer \" Error  ";
               return 1 ;
           }
       }
       else if(pparentModule->getLocatedFileName().endsWith(".vhd"))
       {

       }
       else
       {

       }

       pchain->addToClockSetMap(this);
       /*����Ӧ���ļ�*/
       /*ѭ������ ����ɨ���� ����*/
       for(int i = 0 ; i < this->childCount() ; i++)
       {
           // ���ϵͳcore �ļ� ����
           EziDebugModule *pchild = sm_pprj->getPrjModuleMap().value(m_ichildModules.at(i)->getModuleName());
           if(pchild->isLibaryCore())
           {
               // pchain->addToInstanceItemList(m_ichildModules.at(i)->getModuleName(),m_ichildModules.at(i)->getInstanceName());
               continue ;
           }

           if(!pchild->getClockSignal().count())
           {
               continue ;
           }

           /*�޸� key - value ��ʱ�ӻ��� instance �� ʱ�� - value*/
           QMap<QString,QString> inewMap = m_imoduleClockMap.value(m_ichildModules.at(i)->getInstanceName());
           QMap<QString,QString>::const_iterator m = inewMap.constBegin() ;
           while(m != inewMap.constEnd())
           {
               struct SCAN_CHAIN_STRUCTURE* pchainSt = chainStructuremap.value(m.key(),NULL) ;
               if(!pchainSt)
               {
                   return 1 ;
               }
               ichildChainStructureMap.insert(m.value(),pchainSt);
               ++m ;
           }

           if(!ichildChainStructureMap.count())
           {
               qDebug() << "EziDebug Info: There is no corresponding clock  in the module:" << pchild->getModuleName() << "!" ;
               continue ;
           }


//         QString iname =  hiberarchyname ;
//         iname.append(QObject::tr("%1:%2|").arg(m_ichildModules.at(i)->getModuleName()).arg(m_ichildModules.at(i)->getInstanceName()));
           if(m_ichildModules.at(i)->insertScanChain(ichildChainStructureMap,pchain,topinstance))
           {
               return 1 ;
           }
       }

       //pchain->addToInstanceItemList(this->getModuleName(),this->getInstanceName());
       EziDebugVlgFile *pfile = sm_pprj->getPrjVlgFileMap().value(pmodule->getLocatedFileName()) ;
       if(pfile->addScanChain(EziDebugVlgFile::InsertUserCore,chainStructuremap,pchain,this))
       {
           qDebug() << "EziDebug Error: InsertChain \" InsertUserCore \" Error " << "In the node " << m_imoduleName << ":" << m_iinstanceName ;
           return 1 ;
       }
    }
    else
    {
        pchain->addToClockSetMap(this);
        /*ѭ������ ����ɨ���� ����*/
        for(int i = 0 ; i < this->childCount() ; i++)
        {
            // ���ϵͳcore �ļ� ����
            EziDebugModule *pchild = sm_pprj->getPrjModuleMap().value(m_ichildModules.at(i)->getModuleName());
            if(pchild->isLibaryCore())
            {
                // pchain->addToInstanceItemList(m_ichildModules.at(i)->getModuleName(),m_ichildModules.at(i)->getInstanceName());
                continue ;
            }

            if(!pchild->getClockSignal().count())
            {
                continue ;
            }

            /*�޸� key - value ��ʱ�ӻ��� instance �� ʱ�� - value*/
            QMap<QString,QString> inewMap = m_imoduleClockMap.value(m_ichildModules.at(i)->getInstanceName());
            QMap<QString,QString>::const_iterator m = inewMap.constBegin() ;
            while(m != inewMap.constEnd())
            {
                // chainmap �� parent �� clockmap ���뵽ɨ����
                struct SCAN_CHAIN_STRUCTURE* pchainSt = chainStructuremap.value(m.key(),NULL) ;
                if(!pchainSt)
                {
                    return 1 ;
                }
                ichildChainStructureMap.insert(m.value(),pchainSt);
                ++m ;
            }

            //pchain->addToInstanceItemList(m_ichildModules.at(i)->getModuleName(),m_ichildModules.at(i)->getInstanceName());

            if(!ichildChainStructureMap.count())
            {
                qDebug() << "EziDebug Info: There is no corresponding clock  in the module:" << pchild->getModuleName() << "!" ;
                continue ;
            }

            if(m_ichildModules.at(i)->insertScanChain(ichildChainStructureMap,pchain,topinstance))
            {
                return 1 ;
            }
        }

        // pchain->addToInstanceItemList(this->getModuleName(),this->getInstanceName());
        EziDebugVlgFile *pfile = sm_pprj->getPrjVlgFileMap().value(pmodule->getLocatedFileName()) ;
        if(pfile->addScanChain(EziDebugVlgFile::InsertUserCore,chainStructuremap,pchain,this))
        {
            qDebug() << "EziDebug Error: InsertChain \" InsertUserCore \" Error " << "In the node " << m_imoduleName << ":" << m_iinstanceName ;
            return 1 ;
        }
    }
    return 0 ;
}



int EziDebugInstanceTreeItem::deleteScanChain(EziDebugPrj::OPERATE_TYPE type)
{
    EziDebugScanChain* pchainInfo = getScanChainInfo() ;
    EziDebugVlgFile*   pvlgFile = NULL ;
    EziDebugVhdlFile*   pvhdlFile = NULL ;

    if(!pchainInfo)
    {
        /*�򴰿ڴ�����Ϣ  ɾ����ʧ��*/
        return 1 ;
    }


    QMap<QString,QStringList> ichainCodeBlock = pchainInfo->getLineCode() ;
    QMap<QString,QStringList>::const_iterator i = ichainCodeBlock.constBegin() ;

    while( i!= ichainCodeBlock.constEnd())
    {
        EziDebugModule *pmodule = sm_pprj->getPrjModuleMap().value(i.key(),NULL);
        if(!pmodule)
        {
            /*�򴰿ڴ�����Ϣ  ɾ����ʧ�� ʧ��ԭ��*/
            return 1 ;
        }
        // ��ռ��������� module�������Ϣ,��ֹӰ���´μ��������ñ��ݣ�����֮�����ô�
        pmodule->clearChainInfo(pchainInfo->getChainName());
        // ���·��
        QString irelativeFileName = pmodule->getLocatedFileName();

        if(irelativeFileName.endsWith(QObject::tr(".v")))
        {
            pvlgFile = sm_pprj->getPrjVlgFileMap().value(irelativeFileName) ;
            QStringList ilineCodeList = pchainInfo->getLineCode().value(i.key()) ;
            if(pvlgFile->deleteScanChain(ilineCodeList,pchainInfo->getBlockCode().value(i.key()),pchainInfo,type))
            {
               qDebug() << "Error:Delete The Scan Chain Failed!";
               return 1 ;
            }
            else
            {
               // do nothing
            }
        }
        else if(irelativeFileName.endsWith(QObject::tr(".vhd")))
        {
        }
        else
        {
            /*�򴰿ڴ�����Ϣ  ɾ����ʧ�� ʧ��ԭ��*/
            return 1 ;
        }

        ++i ;
    }

    return 0 ;



//    EziDebugInstanceTreeItem* pparent = this->parent();
//    EziDebugModule *pparentModule = sm_pprj->getPrjModuleMap().value(pparent->getModuleName()) ;
//    EziDebugVlgFile *pfile = sm_pprj->getPrjVlgFileMap().value(pparentModule->getLocatedFileName()) ;


}

QString EziDebugInstanceTreeItem::findCorrespondClock(QString hiberarchyname, QString clock ,EziDebugInstanceTreeItem * headitem)
{
    int i = 0 ;
    int j = 0 ;
    QString iclock = clock;
    QStringList ihiberarchyNameList =  hiberarchyname.split(QObject::tr("|"));
    if((headitem->getNameData()) != ihiberarchyNameList.at(0))
    {
        return QString();
    }

    /*0 Ϊ�۲�ģ��� module:instance*/
    for(j = 1 ; j < ihiberarchyNameList.count() ; j++)
    {
        for(i = 0 ; i < headitem->childCount();i++)
        {
            if(headitem->child(i)->getNameData() == ihiberarchyNameList.at(j))
            {
                headitem = headitem->child(i) ;
                iclock = headitem->getModuleClockMap(headitem->child(i)->getInstanceName()).value(iclock,QString());
                if(iclock.isEmpty())
                {
                    return iclock ;
                }
                j++ ;
                break ;
            }
        }
        if(i == headitem->childCount())
        {
            return QString();
        }
    }
    return iclock ;
}




void EziDebugInstanceTreeItem::setModuleClockMap(const QString &instancename,const QMap<QString,QString> &clockmap)
{
    m_imoduleClockMap.insert(instancename,clockmap);
    return ;
}

QMap<QString,QString> EziDebugInstanceTreeItem::getModuleClockMap(const QString& instancename) const
{
    return m_imoduleClockMap.value(instancename) ;
}

//void EziDebugInstanceTreeItem::traverseAllNode(TRANVERSE_TYPE type)
//{
//    if(type == TranvereModule)
//    {
//        if(!m_ichildModules.count())
//        {
//            return ;
//        }
//        else
//        {
//            for(int i = 0 ; i < m_ichildModules.count() ;i++)
//            {
//                int count = 0 ;
//                EziDebugModule *pmodule = sm_pprj->getPrjModuleMap().value(m_ichildModules.at(i)->getModuleName());
//                count += pmodule->getRegNumber();
//                //traverseAllNode(type);

//            }
//        }
//    }
//    else if(type == TranvereFile)
//    {
//        return ;
//    }
//    else
//    {
//        return ;
//    }

//}

const QString & EziDebugInstanceTreeItem::getItemHierarchyName(void) const
{
    return m_ihierarchyName ;
}

void EziDebugInstanceTreeItem::setItemParent(EziDebugInstanceTreeItem* parentitem)
{
    m_pparentInstance = parentitem ;
    return ;
}

void EziDebugInstanceTreeItem::settItemHierarchyName(const QString& name)
{
    m_ihierarchyName = name ;
}

void EziDebugInstanceTreeItem::setProject(EziDebugPrj * prj)
{
   sm_pprj = prj ;
   return ;
}

const EziDebugPrj* EziDebugInstanceTreeItem::getProject(void)
{
    return sm_pprj ;
}

int EziDebugInstanceTreeItem::childCount() const
{
    return  m_ichildModules.size();
}

int EziDebugInstanceTreeItem::row() const
{

    EziDebugInstanceTreeItem* pparentItem = parent();
    

    for(int i = 0 ; i < pparentItem->m_ichildModules.count();i++)
    {
        if(pparentItem->m_ichildModules.at(i) == this)
        {
            return i ;
        }
    }
    return 0 ;
}






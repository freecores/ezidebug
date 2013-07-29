#ifndef EZIDEBUGPRJ_H
#define EZIDEBUGPRJ_H
#include <QMap>
#include <QDir>
#include <QStack>
#include <QXmlStreamReader>

#include "updatedetectthread.h"






#define Parse_by_QDom

#define DEFAULT_MAX_REGNUM  512
#define READ_FILE_INFO      0x01
#define READ_MODULE_INFO    0x02
#define READ_CHAIN_INFO     0x04

#define EZIDEBUG_STRUCTURE_FILE        0x01
#define EZIDEBUG_STRUCTURE_MODULE      0x02
#define EZIDEBUG_STRUCTURE_SCAN_CHAIN  0x04
#define EZIDEBUG_STRUCTURE_ALL         0x07

#define MODULE_STRUCTURE_CLOCK_DESCRIPTION   0x01
#define MODULE_STRUCTURE_REG_DESCRIPTION     0x02
#define MODULE_STRUCTURE_PORT_DESCRIPTION    0x04
#define MODULE_STRUCTURE_INSTANCE_PORT_MAP_DESCRIPTION 0x08
#define MODULE_STRUCTURE_ALL_DESCRIPTION     0x0F

#define SCAN_CHAIN_STRUCTURE_REGLIST_DESCRIPTION   0x01
#define SCAN_CHAIN_STRUCTURE_CODE_DESCRIPTION     0x02
#define SCAN_CHAIN_STRUCTURE_ALL_DESCRIPTION     0x03
#define MAX_SIGNAL_NUM     256

class QString ;
class QDir ;
class EziDebugVlgFile ;
class EziDebugVhdlFile ;
class EziDebugModule ;
class EziDebugInstanceTreeItem ;
class EziDebugScanChain ;
class QStringList ;
class QXmlStreamReader ;
class QDomElement ;
class QDomDocument ;






template <class Key, class T> class QMap;


class EziDebugPrj:public QObject
{
    Q_OBJECT
public: 
    enum TOOL
    {
        ToolQuartus = 0 ,
        ToolIse ,
        ToolOther
    };
    enum OPERATE_TYPE
    {

        OperateTypeAddScanChain = 0 ,
        OperateTypeDelSingleScanChain ,
        OperateTypeDelAllScanChain ,
        OperateTypeNone ,
        OperateTypeOther
    };

    enum INFO_TYPE
    {
        infoTypeScanChainStructure = 0 ,
        infoTypeModuleStructure,
        infoTypeTreeStructure ,
        infoTypeFileInfo ,
        infoTypeOther
    };

    struct LOG_FILE_INFO
    {
        char ainfoName[128] ;
        void *pinfo ;
        INFO_TYPE etype ;
    };

    enum FILE_UPDATE_TYPE
    {
        addedUpdateFileType = 0 ,
        deletedUpdateFileType ,
        changedUpdateFileType ,
        otherUpdateFileType
    };

    enum SCAN_TYPE
    {
        partScanType = 0 ,
        fullScanType ,
        otherScabType
    };


    EziDebugPrj(int maxregnum , QString projectdir , TOOL tool = ToolQuartus ,QObject * parent = 0);
    ~EziDebugPrj();

    const TOOL &getToolType(void) const;
    const int  &getMaxRegNumPerChain(void) const;
    const QDir &getCurrentDir(void) const ;
    bool getSoftwareXilinxErrCheckedFlag(void) ;
    QString getTopModule(void);
    UpdateDetectThread * getThread(void) const ;
    bool getLogFileExistFlag(void) ;
    const QMap<QString,EziDebugScanChain*> &getScanChainInfo(void) const ;
    const QMap<QString,EziDebugInstanceTreeItem*> &getChainTreeItemMap(void) const ;
    const QMap<QString,EziDebugInstanceTreeItem*> &getBackupChainTreeItemMap(void) const ;
    const QMap<QString,EziDebugScanChain*> &getBackupChainMap(void) const ;

    const QStringList &getPrjCodeFileNameList(void) const ;
    const QStringList &getUpdateFileList(FILE_UPDATE_TYPE fileupdatetype) const;
    const QStringList &getFileNameList(void) const ;
    const QString &getPrjName(void) const ;
    OPERATE_TYPE  getLastOperation(void) ;
    int   getPermittedRegNum(void) ;
    EziDebugScanChain* getLastOperateChain(void) ;
    EziDebugInstanceTreeItem* getLastOperateTreeItem(void) ;

    const QMap<QString,EziDebugModule*> &getPrjModuleMap(void) const ;
    const QMap<QString,EziDebugVlgFile*> &getPrjVlgFileMap(void) const ;
    const QMap<QString,EziDebugVhdlFile*> &getPrjVhdlFileMap(void) const ;


    int eliminateLastOperation(void) ;
    int  eliminateFile(const QString &filename,QList<LOG_FILE_INFO*> &infolist) ;
    int  addFile(const QString &filename,SCAN_TYPE type,QList<LOG_FILE_INFO*> &infolist);

    void  addToModuleMap(const QString &modoule,EziDebugModule *pmodule);
    void  addToDestroyedChainList(const QString& chainname);
    void  addToCheckedChainList(const QString& chainname) ;
    QStringList  checkChainExist(void) ;      // EziDebug ��ӵĴ���λ�ñ䶯 ��¼
    const QStringList &getDestroyedChainList(void) const ;
    const QStringList &getCheckedChainList(void) const ;
    void  clearupDestroyedChainList(void) ;
    void  clearupCheckedChainList(void) ;
    QStringList   deleteDestroyedChain(QList<LOG_FILE_INFO*> &addedinfoList,QList<LOG_FILE_INFO*> &deletedinfoList);


    int  setToolType(TOOL tool);
    int  setMaxRegNumPerChain(int num);
    void setLogFileExistFlag(bool flag);
    void setXilinxErrCheckedFlag(bool flag) ;
    void setLogFileName(const QString& filename);
    void setMaxRegWidth(int width) ;
    void setLogfileDestroyedFlag(bool flag) ;
    bool getLogfileDestroyedFlag(void) ;

    bool isPrjFileExist(void) ;
    void preModifyPrjFile(void);
    int  parsePrjFile(QMap<QString,EziDebugVlgFile*> &vlgFileMap , QMap<QString,EziDebugVhdlFile*> &vhdlFileMap) ;
    void checkDelFile(QMap<QString,EziDebugVlgFile*> &vlgFileMap , QMap<QString,EziDebugVhdlFile*> &vhdlFileMap,QList<LOG_FILE_INFO*> &deleteinfolist);
    int  traverseAllCodeFile(EziDebugPrj::SCAN_TYPE type , const QMap<QString,EziDebugVlgFile*> &vlgFileMap ,const QMap<QString,EziDebugVhdlFile*> &vhdlFileMap ,QList<LOG_FILE_INFO*> &addedinfoList,QList<LOG_FILE_INFO*> &deletedinfoList);
    void updateFileMap(const QMap<QString,EziDebugVlgFile*> &vlgFileMap ,const QMap<QString,EziDebugVhdlFile*> &vhdlFileMap);
    int  detectLogFile(char nreadFlag);
    int  updatePrjAllFile(const QStringList& addfilelist,const QStringList& delfilelist,const QStringList& chgfilelist ,QList<LOG_FILE_INFO*>& addinfolist , QList<LOG_FILE_INFO*> &deleteinfolist ,bool updateFlag);
    void preupdateProgressBar(bool updateflag ,int value);
    void compareFileList(const QStringList& newfilelist , QStringList& addFileList , QStringList &delFileList , QStringList &chgFileList);


    void addToChainMap(EziDebugScanChain* chain);
    void addToTreeItemMap(const QString &chain ,EziDebugInstanceTreeItem* item);
    void addToQueryItemMap(const QString &name ,EziDebugInstanceTreeItem* item);
    void addToMacroMap(void) ;
    const QMap<QString,QString> &getMacroMap(void) const;

    void addToDefparameterMap(void);
    QMap<QString,QString> getdefparam(const QString &instancename);
    EziDebugInstanceTreeItem* getQueryItem(const QString &name);
    void updateTreeItem(EziDebugInstanceTreeItem* item) ;

    void eliminateChainFromMap(const QString &chain);
    void eliminateTreeItemFromMap(const QString &chain);
    void eliminateTreeItemFromQueryMap(const QString &combinedname) ;

    void backupChainQueryTreeItemMap(void);
    void backupChainTreeItemMap(void);
    void backupChainMap(void);

    void cleanupChainMap(void);

    void cleanupBakChainTreeItemMap(void) ;
    void cleanupChainTreeItemMap(void) ;
    void cleanupChainQueryTreeItemMap(void) ;
    void cleanupBakChainQueryTreeItemMap(void) ;



    void resumeChainMap(void);
    void resumeChainTreeItemMap(void) ;
    void resumeChainQueryTreeItemMap(void) ;


    int domParseModuleInfoElement(const QDomElement &element, char readflag) ;
    int domParseModuleStructure(const QDomElement &element, char readflag, EziDebugModule* module) ;
    int domParseClockDescriptionElement(const QDomElement &element, char readflag, EziDebugModule* module) ;
    int domParseRegDescriptionElement(const QDomElement &element,char readflag, EziDebugModule* module) ;
    int domParsePortDescriptionElement(const QDomElement &element,char readflag, EziDebugModule* module) ;
    int domParseInstancePortMapDescriptionElement(const QDomElement &element,char readflag, EziDebugModule* module) ;

    int domParseClockStructure(const QDomElement &element, char readflag , EziDebugModule* module);
    int domParseRegStructure(const QDomElement &element, char readflag , EziDebugModule* module);
    int domParsePortStructure(const QDomElement &element, char readflag , EziDebugModule* module);


    int domParseScanChainInfoElement(const QDomElement &element,char readflag) ;
    int domParseScanChainStructure(const QDomElement &element,char readflag, EziDebugScanChain *chain);
    int domParseReglistDescriptionElement(const QDomElement &element,char readflag, EziDebugScanChain *chain);
//  int domParseReglistStructure(const QDomElement &element,char readflag, EziDebugScanChain *chain);

    int domParseCodeDescriptionElement(const QDomElement &element,char readflag, EziDebugScanChain *chain);
//  int domParseCodeStructure(const QDomElement &element,char readflag, EziDebugScanChain *chain) ;

    int readModuleStructure(char readflag) ;
    int readClockStructure(char readflag , EziDebugModule* module);
    int readRegStructure(char readflag , EziDebugModule* module);
    int readPortStructure(char readflag , EziDebugModule* module);

    int  generateTreeView(void) ;
    int  traverseModuleTree(const QString &module,EziDebugInstanceTreeItem* item);

    EziDebugInstanceTreeItem * getInstanceTreeHeadItem(void);
    void setInstanceTreeHeadItem(EziDebugInstanceTreeItem *item);

    int  resumeDataFromFile(void);

    void updateOperation(OPERATE_TYPE type, EziDebugScanChain* chain,EziDebugInstanceTreeItem* item);

    int  changedLogFile(const QList<LOG_FILE_INFO*>& addlist, const QList<LOG_FILE_INFO*> &deletelist);
    int  saveInfoToLogFile(QDomDocument &idoc, LOG_FILE_INFO* loginfo);
    int  deleteLogFileElement(QDomDocument &idoc ,LOG_FILE_INFO* loginfo);

    int  createLogFile(void);

    int  createCfgFile(EziDebugInstanceTreeItem * item) ;

    int chkEziDebugFileInvolved() ;
    QString constructCfgInstanceString(EziDebugInstanceTreeItem * item) ;
    //void constructIlaunitString(int &unitnumber,int &regwidth, QString &triggerChannelString ,QString &ilaString ,bool forceflag) ;


    int  parseQuartusPrjFile(QMap<QString,EziDebugVlgFile*> &vlgFileMap ,QMap<QString,EziDebugVhdlFile*> &vhdlFileMap);
    int  parseQuartusPrjFile(QStringList &filelist);
    int  parseIsePrjFile(QMap<QString,EziDebugVlgFile*> &vlgFileMap ,QMap<QString,EziDebugVhdlFile*> &vhdlFileMap);
    int  parseIsePrjFile(QStringList &filelist);

    void  deleteAllEziDebugCode(const QMap<QString,EziDebugVlgFile*> &vlgFileMap ,const QMap<QString,EziDebugVhdlFile*> &vhdlFileMap);

    friend void UpdateDetectThread::update();
signals:
    void displayMessage(QStringList message);
    void updateProgressBar(int value) ;

private slots:
    int  updateCodeFile();

   // ʲôʱ����������߳̿�ʼ �������ļ�

   // 1��Ĭ��·�� ok ���� �ָ��� ���ݽṹ֮�� ��ʼ�����߳�
   // 2��ȫ������ ���֮�� ���� д�� log �ļ�֮�� ��ʼ�����߳� ������

   // ʲôʱ����ͣ �������ļ�
   // 1���̼߳�⵽�и��µ��ļ�֮�� �����ź� �����̣߳����߳� ��ʼɨ������ļ�ʱ ��ͣ��⣬ɨ����֮�󣬿���ɨ��
private:
    QStringList m_iCodefileNameList ;
    QString m_iprjName ;
    QDir   m_iprjPath ;
    int    m_nmaxRegNumInChain ;
    TOOL   m_eusedTool ;
    QString m_itoolSoftwareVersion ;
    QString m_icoreLangType ;
    bool   m_isLogFileExist ;
    int    m_ipermettedMaxRegNum ;
    int    m_imaxRegWidth ;
    bool   m_isDisXilinxErrChecked ;
    bool   m_isUpdated ;

    UpdateDetectThread* m_pthread ;
    QXmlStreamReader m_ireader ;
    QStack<QString> m_ielementStack ;
    QMap<QString,EziDebugVlgFile*> m_ivlgFileMap ;
    QMap<QString,EziDebugVhdlFile*> m_ivhdlFileMap ;
    QMap<QString,QString> m_iupdateTimeOfFile ;
    QString m_itopModule ;
    QStringList m_iwaveFileList ;
    QMap<QString,EziDebugModule*> m_imoduleMap ;
    QMap<QString,EziDebugInstanceTreeItem*> m_ichainTreeItemMap ;  // �����治�õĻ� ����ɾ��
    QMap<QString,EziDebugInstanceTreeItem*> m_ibackupChainTreeItemMap ;
    QMap<QString,EziDebugInstanceTreeItem*> m_iqueryTreeItemMap ;  // ���� ���� modue:instance ��ѯ ��������ͷ�ڵ�
    QMap<QString,EziDebugInstanceTreeItem*> m_ibackupQueryTreeItemMap ;  // ���� ���� modue:instance ��ѯ ��������ͷ�ڵ�

    QMap<QString,EziDebugScanChain*> m_ichainInfoMap ;
    QMap<QString,EziDebugScanChain*> m_ibackupChainInfoMap ;

    QString m_ilogFileName ;
    bool    m_isLogFileDestroyed ;
    EziDebugInstanceTreeItem* m_headItem ;
    QStringList m_iupdateDeletedFileList ;
    QStringList m_iupdateAddedFileList ;
    QStringList m_iupdateChangedFileList ;

    OPERATE_TYPE m_elastOperation ;
    EziDebugScanChain* m_pLastOperateChain ;
    EziDebugInstanceTreeItem* m_pLastOperteTreeItem ;
    QStringList m_idestroyedChain ; // ����ɨ���ļ�ʱ����¼���ƻ���ɨ����
    QStringList m_icheckedChain ;   // ����ɨ�跢���仯���ļ�ʱ����¼ɨ�赽����

    QMap<QString,QString>  m_imacro ; // ȫ�ֱ��� defparameter
    QMap<QString,QMap<QString,QString> > m_idefparameter ;  // ȫ�ֱ��� define



    int domParseEziDebugElement(const QDomElement &element, char readflag) ;
    int domParseFileInfoElement(const QDomElement &element, char readflag) ;
    int domParseFileStructure(const QDomElement &element, char readflag) ;


    int readFileInfo(char readflag) ;
    int readFileStructure(char readflag) ;

    int readModuleInfo(char readflag) ;

    int readClockDescription(char readflag, EziDebugModule* module) ;

    int readRegDescription(char readflag, EziDebugModule* module) ;

    int readPortDescription(char readflag, EziDebugModule* module) ;

    int readScanChainInfo(char readflag) ;


};

#endif // EZIDEBUGPRJ_H

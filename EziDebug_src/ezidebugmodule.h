#ifndef EZIDEBUGMODULE_H
#define EZIDEBUGMODULE_H


#include "ezidebugprj.h"

class QString ;
class QStringList ;
template <class Key, class T> class QMap ;
template <typename T> class QVector;

class EziDebugModule
{
public:
    enum DIRECTION_TYPE
    {
        dirctionNone  = 0 ,
        directionTypeInput,
        directionTypeOutput,
        directionTypeInoutput
    };

    enum SIGNAL_EDGE
    {
        signalPosEdge = 0 ,
        signalNegEdge,
        signalLow,
        signalHigh,
        signalOtherEdge
    };

    enum ENDIAN_TYPE
    {
        endianLittle = 0 ,
        endianBig ,
        endianOther
    };

    enum ATTRIBUTE_TYPE
    {
        attributeStatic = 0 ,
        attributeDynamic ,
        attributeOther
    };



    struct RegStructure
    {
        // ��̬����
        char m_pMouduleName[128] ;
        char m_pRegName[128] ;
        char m_pclockName[128] ;
        SIGNAL_EDGE  m_eedge ;

        char m_pregNum[64] ;
        char m_pregNumNoMacroString[128] ;

        char m_pExpString[64] ;             // ���ڸ��ݲ��� ����λ���ı��ʽ
        char m_pExpNoMacroString[128] ;     // ֻ���� ���滻 ������������֤ parameter ���� ��

        // �仯���� (��ɨ������ͬ���ܱ仯)
        unsigned int m_unMaxRegNum ;        // ���� defparameter �����Ĵ�������
        ENDIAN_TYPE m_eRegNumEndian ;
        ATTRIBUTE_TYPE m_eRegNumType ;
        unsigned int m_unMaxBitWidth ;      // λ����ܳ��ֵ������
        ENDIAN_TYPE m_eRegBitWidthEndian ;
        ATTRIBUTE_TYPE m_eRegBitWidthType ;


        // ���ھ�̬�Ĵ���ʹ��(ÿ�������ļĴ����Ǿ�̬��)
        unsigned int m_unStartNum ;
        unsigned int m_unEndNum ;
        unsigned int m_unRegNum ;

        unsigned int m_unStartBit ;
        unsigned int m_unEndBit ;
        unsigned int m_unRegBitWidth ;
    };



    struct PortStructure
    {
        char m_pPortName[128] ;
        char m_pModuleName[128] ;
        DIRECTION_TYPE eDirectionType ;
        char m_pBitWidth[64] ;
        unsigned int m_unStartBit ;
        unsigned int m_unEndBit ;
        unsigned int m_unBitwidth ;
        ENDIAN_TYPE m_eEndian ;
    };

    EziDebugModule(const QString modulename);
    ~EziDebugModule();


    friend int EziDebugPrj::readModuleStructure(char readflag) ;
    friend int EziDebugPrj::readClockStructure(char readflag ,EziDebugModule* module) ;
    friend int EziDebugPrj::readRegStructure(char readflag ,EziDebugModule* module);
    friend int EziDebugPrj::readPortStructure(char readflag ,EziDebugModule* module);



    friend int EziDebugPrj::domParseModuleInfoElement(const QDomElement &element, char readflag);


    friend int EziDebugPrj::domParseClockDescriptionElement(const QDomElement &element, char readflag, EziDebugModule* module) ;
    friend int EziDebugPrj::domParseRegDescriptionElement(const QDomElement &element,char readflag, EziDebugModule* module) ;
    friend int EziDebugPrj::domParsePortDescriptionElement(const QDomElement &element,char readflag, EziDebugModule* module) ;


    friend int EziDebugPrj::domParsePortStructure(const QDomElement &element, char readflag, EziDebugModule *module);
    friend int EziDebugPrj::domParseRegStructure(const QDomElement &element, char readflag, EziDebugModule *module);
    friend int EziDebugPrj::domParseClockStructure(const QDomElement &element, char readflag, EziDebugModule *module);
    friend int EziDebugPrj::domParseInstancePortMapDescriptionElement(const QDomElement &element,char readflag, EziDebugModule* module);


    friend int EziDebugPrj::saveInfoToLogFile(QDomDocument &idoc , EziDebugPrj::LOG_FILE_INFO* loginfo);
    friend class EziDebugVlgFile ;
    //friend int EziDebugVlgFile::scanFile(EziDebugPrj* prj,EziDebugPrj::SCAN_TYPE type,QList<EziDebugPrj::LOG_FILE_INFO*> &addedinfolist,QList<EziDebugPrj::LOG_FILE_INFO*> &deletedinfolist) ;



    QString getModuleName(void) const ;
    const QStringList &getInstanceList(void) const;
    QVector<RegStructure*> getReg(const QString &clock)  ;
    const QMap<QString,QVector<EziDebugModule::RegStructure*> > getRegMap(void) const ;
    const QMap<QString,QString> & getClockSignal(void) const ;
    const QMap<QString,QString> & getResetSignal(void) const ;
    const QVector<PortStructure*> & getPort(EziDebugPrj *prj ,const QString &instancename)  ;
    void  addToClockMap(const QString &clock);
    void  addToResetSignalMap(const QString &reset,const QString &edge);


    const QMap<QString,QMap<QString,QString> > &getInstancePortMap(void) const ;
    QMap<QString,QString> getInstancePortMap(const QString &instanceName) ;

    const QString  &getLocatedFileName(void) const;
    bool  getAddCodeFlag(void) ;
    int   getInstancedTimesPerChain(const QString &chainName);
    int   getConstInstacedTimesPerChain(const QString &chainName);
    // int   getRegNumber(const QString &clock) ;


    void  getBitRangeInChain(const QString& chainname, const QString &clockname, int* startbit,int * endbit);
    void  setBitRangeInChain(const QString& chainname, const QString &clockname, int startbit,int endbit);

    void  AddToRegMap(const QString &clock,RegStructure*preg);
    // void  addToVaribleRegMap(const QString &clock,RegStructure*preg);
    void  addToDefParameterMap(const QString &instancename, const QString &parametername ,const QString &value);
    void  addToParameterMap(const QString &parametername,const QString &value);
    int   getInstanceTimes(void) ;
    QString getChainClockWireNameMap(const QString& chainname ,const QString& clockname) ;
    bool  isLibaryCore(void) ;
    void  setLibaryCoreFlag(bool flag) ;
    //void  setInstanceTimes(int count) ;
    void  setInstancedTimesPerChain(const QString &chainame,int count);
    void  setConstInstanceTimesPerChain(const QString &chainname,int count);
    void  setEziDebugCoreCounts(const QString &chainname,int count);
    int   getEziDebugCoreCounts(const QString &chainname);
    void  setEziDebugWireCounts(const QString &chainname,int count);
    int   getEziDebugWireCounts(const QString &chainname);
    void  AddToClockWireNameMap(const QString& chainname,const QString& clockname,const QString& lastwirename);
    void  setAddCodeFlag(bool flag) ;
    void  setRegInfoToInitalState(void) ;
    bool  isChainCompleted(EziDebugScanChain *pchain);
    bool  isChainCompleted(EziDebugModule *pmodue);
    void  clearChainInfo(const QString& chainname);

    int   getAllRegMap(QString clock ,QVector<EziDebugModule::RegStructure*> &sregVec,QVector<EziDebugModule::RegStructure*> &vregVec);
    void  calInstanceRegData(EziDebugPrj*prj,const QString &instancename);
    RegStructure* getInstanceReg(QString instancename , QString clock , QString regname) ;
    void  getBitRange(const QString &widthStr ,int *startbit ,int *endbit) ;
    int   constructChainRegMap(EziDebugPrj * prj ,const QStringList &cominstanceList,QString instancename) ;


private:
    QString m_imoduleName ;
    QStringList m_iinstanceNameList ;
    //QStringList m_iclockNameList ;
    QMap<QString,QString>  m_iclockMap ;
    QMap<QString,QString>  m_iresetMap ;
    // <clock,reg>
    QMap<QString,QVector<RegStructure*> > m_iregMap ;  // ���мĴ���

#if 0
    QMap<QString,QVector<RegStructure*> > m_ivregMap ; // ���ɼ���  �仯λ��ļĴ�����
    QMap<QString,QVector<RegStructure*> > m_isregMap ; // �ɼ���   �̶�λ��ļĴ�����
#endif

    // ���㵱ǰ�����ļĴ����Ŀ����Ҫ  ��ǰ���������֡�ɨ�������ֵ�module�������������������ɽ�һЩ�仯λ��ļĴ����̶�������

    QMap<QString,QMap<QString ,QVector<RegStructure*> > > m_iinstanceRegMap ;  // module �� ÿ������ ��Ӧ����ֵ�� �Ĵ�����

    // ��ɨ��������������1�εĶ�Ϊ �̶�λ��ļĴ�����

    QMap<QString,QMap<QString,QString> > m_iinstancePortMap   ;  //������Ӧ ���������ϵĶ�Ӧ�Ķ˿�����
    QMap<QString,QMap<QString,QString> > m_iclockWireNameMap ;  // ���������ɨ����ʱʹ�ã���������log�ļ���(���⺯����������)
                                                                // ����ÿ���� ���һ����ϵͳcore ���� ������ wire_tdo

    QMap<QString,QMap<QString,int> > m_istartChainNum ;    // �� module �� ÿ�� clock�� �� ����    ��ʼ bit λ �������� �û�core �� module tdo
    QMap<QString,QMap<QString,int> > m_iendChainNum ;  // �� module ��ÿ�� clock �� �� ����  ��  ���� bit λ ���� ���� tdo
    QVector<PortStructure*> m_iportVec ;
    bool m_isSubstituted ;
    QString m_ilocatedFile ;
    int  m_ninstanceTimes ;
    int  m_nConstInstanceTimes ;
    bool m_isLibaryCore ;
    QMap<QString,int> m_iinstanceTimesPerChain ;
    QMap<QString,int> m_iconstInstanceTimesPerChain ;
    QMap<QString,int> m_ieziDebugCoreCounts ;
    QMap<QString,int> m_ieziDebugWireCounts ;
    QMap<QString,QString> m_iparameter ;  //  ( parameter : value )
    QMap<QString,QMap<QString,QString> > m_idefparameter ;  // instance (parameter: value)
    //QMap<QString,QMap<QString,QString> > m_ivarRegWidth ;
    // �Ĵ�����  ��ͬ�Ŀ���ַ���
    QMultiMap<QString,QString> m_ivarRegWidth ; // ���� 1���Ĵ��� 1��ɨ���� ��ͬ���� ��Ӧ �Ŀ���ַ���

    // �̶�λ��
    // 1����parameter ֻ�������� define �������
    // 2����Ȼ��parameter ȴ�� defparameter
    // 3����defparameter ȴɨ������ֻ����1������

    // �仯λ��
    // 1����parameter �� �� defparameter ��ɨ�������ڶ������  �����ö���defparameter ��
    // ����λ��ʱ   ��������Ӧ��defparamete ��
    // ɨ��������ʱ ��¼ÿ��module������������Щ
    // �ڸ���λ���ַ�������λ��ʱ
    // 1����define �ĺ�ȫ���滻
    // 2�������滻����ַ��� �����ݵ�ǰ���� ��defparameter ���뵽 parameter �� �����������λ��
    // 3�����λ�� �� ��ǰλ�� ��һ�� �� Ϊ�仯λ��
    bool m_isaddedCode ;
};

#endif // EZIDEBUGMODULE_H

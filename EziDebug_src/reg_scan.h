#ifndef REG_SCAN_H
#define REG_SCAN_H

#include <QWidget>
#include "reg_type.h"

#include <QStringList>
#include <QDIR>
#include <QMap>

namespace Ui {
    class reg_scan;
}

class reg_scan : public QWidget
{
    Q_OBJECT
public:
    char *prog;
    char *p_buf;
    char cToken[64];
    TOK_TYPES eTokenType;
    TOKEN_IREPS eTok;

public:
    explicit reg_scan(QWidget *parent = 0);

        ~reg_scan();
public:
/*caculate the value of expressions*/
    /////////////////////////////
    //���ڽ������ʽ
    void EvalExp(int &value);
    void EvalExp0(int &value);
    void EvalExp1(int &value);
    void EvalExp2(int &value);
    void EvalExp3(int &value);
    void EvalExp4(int &value);
    void EvalExp5(int &value);
    void Atom(int &value);
    /////////////////////////////
    void ClearModule();
    bool IsDelim(char c);
    bool DealError(char *str);
    bool FindModule(char* name);
    bool FindMacro(char *vname);
    bool IsVar(char *vname);
    bool LoadVeriFile(char *p,char *fname);
    bool SetRegAttr(char *reg,struct TempBuf* clk,struct TempBuf* rst,bool isRegValid);

    void PutBack();
    void ScanPre();
    void Interp();
    void PrintFile();
    void StoreMacro();
    void ExecInclude();
    void ExecModule();
    void ExecIO();
    void ExecReg();
    void ExecParam();
    void ExecAlways();
    void ExecAssign();
    void ExecInst();
    void ExecDefparam();
    void ExecDef();
    void ExecIfels(int &nElseFlag);
    void ExecElsend(int &nElseFlag);
    void SkipIfels(int &nElseFlag);
    void SkipElsend(int &nElseFlag);
    void InterpAlways(struct TempBuf* clk,struct TempBuf* rst,bool isRegValid);

    TOK_TYPES GetToken();
    TOKEN_IREPS LookUp(char *s);

    char *FindInstModu(char *name);

    unsigned int VarToUint(char *str);
    QString FindVar(char *vname);
    //void find_attr_reg();
    //void assign_var(char *vname,int value);
private:
    //Ui::reg_scan *ui;
};


struct commands
{
    char cCmd[20];
    TOKEN_IREPS eTok;
};


//����ģ��ṹ�壬ÿ���ļ��п����ж��ģ��
struct ModuleMem{ //store the attribute of  module and inst_modle
    char cModuleName[MAX_NAME_LEN];

    //����ģ��ṹ��
    struct InstModuType
    {
        char cInstName[MAX_NAME_LEN];
        char cModuName[MAX_NAME_LEN];
        unsigned int unSize;
    }InstModuTab[MAX_T_LEN];  //record the location of inst_module

    //IO�ӿڽṹ��
    struct IOPort{
        char cIOName[MAX_NAME_LEN];
        IO_ATTRI eIOAttri;
        QString  iIOWidth; // Ϊ�˱����滻��ģ���ⲿ��������ģ��������������������ַ���
    }IOTab[MAX_T_LEN];

    //Register�ṹ��
    struct RegMem{ //store the found reg
        char cRegName[MAX_NAME_LEN];
        QString  iRegWidth ;
        QString  iRegCnt ;
        struct RegClk{
            char  cClkName[MAX_NAME_LEN];
            EDGE_ATTRI eClkEdge;
        }ClkAttri;
        struct RegRst{
            char cRstName[MAX_NAME_LEN];
            EDGE_ATTRI eRstEdge;
        }RstAttri;

        //��־�Ƿ�����ʱ���ر仯���ź�
        bool IsFlag;   /* 0: invalid,1: valid ;*/
    }RegTab[MAX_T_LEN];

    //Parameter�ṹ��
    struct ParaMem{ //store the local variable
        char cParaName[MAX_NAME_LEN];
        QString   iParaVal;
    }ParaTab[MAX_T_LEN];


    //����ͳ�Ʊ���
    unsigned int unIOCnt ;
    unsigned int unParaCnt;
    unsigned int unRegCnt ;
    unsigned int unInstCnt ;

    //��־�Ƿ���IPCore
    int nIPCore;

};


//�洢����Ϣ
struct MacroMem{ //store the macro
    char cMacroName[MAX_NAME_LEN];
    QString   iMacroVal;
    int nMacroFlag;
};

#endif // REG_SCAN_H

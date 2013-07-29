#include "ezidebugfile.h"
#include "ezidebugvlgfile.h"
#include "ezidebugprj.h"
#include "ezidebugscanchain.h"
#include "ezidebugmodule.h"
#include "reg_scan.h"
#include "string.h"
#include <QDebug>
#include <QMessageBox>

extern unsigned int unModuCnt ;
extern unsigned int unMacroCnt ;

extern ModuleMem ModuleTab[MAX_T_LEN];
extern MacroMem MacroTab[MAX_T_LEN];

extern QMap<QString,QString> def_map ;  // ʵ����.�˿�������ֵ
extern QMap<QString ,QMap<QString,QString> >inst_map ;
extern QStringList iinstNameList ;

char buffer[200000];


const char *g_pscanRegModuleName =
"" ;
const char *g_pScanRegfileContentFirst =
"(\n"
"\tclock,\n"
"\tresetn,\n"
"\tTDI_reg,\n"
"\tTDO_reg,\n"
"\tTOUT_reg,\n"
"\tshift_reg\n"
"\t);\n"
"\tparameter shift_width = 100;\n"
"\n"
"\tinput   clock;\n"
"\tinput   resetn;\n"
"\tinput   TDI_reg;\n"
"\toutput  TDO_reg;\n"
"\tinput   TOUT_reg;\n"
"\tinput   [shift_width-1:0] shift_reg;\n"
"\n"
"\treg     [shift_width-1:0] shift_reg_r;\n" ;



const char *g_pScanRegfileContentSecond =
"\talways@(posedge clock or negedge resetn)\n"
"\t\tif(!resetn)\n"
"\t\t\tshift_reg_r <= ~shift_reg_r ;\n"
"\t\telse if(TOUT_reg)\n"
"\t\t\tshift_reg_r <=  shift_reg;\n"
"\t\telse\n"
"\t\t\tshift_reg_r <= {shift_reg_r[shift_width-2:0],TDI_reg};\n"
"\n"
"\tassign TDO_reg = shift_reg_r[shift_width-1] ;\n"
"\n"
"endmodule\n"
;



//  "\talways@(posedge clock or negedge resetn)\n"


const char *g_pscanIoModuleName =
"module _EziDebugScanChainIo(\n" ;
const char *g_pScanIofileContentFirst =
"\tclock,\n"
"\tresetn,\n"
"\tTDI_reg,\n"
"\tTDO_reg,\n"
"\tTOUT_reg,\n"
"\tshift_reg\n"
"\t);\n"
"\tparameter shift_width = 100;\n"
"\n"
"\tinput   clock;\n"
"\tinput   resetn;\n"
"\tinput   TDI_reg;\n"
"\toutput  TDO_reg;\n"
"\tinput   TOUT_reg;\n"
"\tinput   [shift_width-1:0] shift_io;\n"
"\n"
"\treg     [shift_width-1:0] shift_io_r;\n" ;




const char*g_pScanIoFileContentSecond =
"\talways@(posedge clock or negedge resetn)\n"
"\t\tif(!resetn)\n"
"\t\t\tshift_io_r <= 0;\n"
"\t\telse if(TOUT_reg)\n"
"\t\t\tshift_io_r <=  shift_io;\n"
"\t\telse\n"
"\t\t\tshift_io_r <= {shift_io_r[shift_width-2:0],TDI_reg};\n"
"\n"
"\tassign TDO_reg = shift_io_r[shift_width-1] ;\n"
"\n"
"endmodule\n"
;


const char* g_ptoutfileContentFirst =
"(\n"
"\t""clock ,\n"
"\t""reset ,\n"
"\t""rstn_out ,\n"
"\t""TOUT_reg \n"
"\t"") ;\n"
"\n"
"\t""input   clock ;\n"
"\t""input   reset ;\n"
"\t""output  rstn_out ;\n"
"\t""output  TOUT_reg ;\n"
"\t""reg[31:0] cnt ;\n"
"\t""reg[31:0] counter ;\n"
"\t""parameter CNT_MAX = 32'd";


const char* g_ptoutfileContentSecond =
"\n\n\t""always@(posedge clock or posedge reset)\n"
"\t\t""if(reset)\n"
"\t\t\t""cnt <= 32'h0 ;\n"
"\t\t""else if(cnt != CNT_MAX)\n"
"\t\t\t""cnt <= cnt + 32'h1 ;\n"
"\n\t""assign rstn_out = (cnt != CNT_MAX ) ? 1'b0 : 1'b1 ;"
"\n\n\t""always@(posedge clock or posedge reset)\n"
"\t\t""if(reset)\n"
"\t\t\t""counter <= 32'h0 ;\n"
"\t\t""else if(counter >= CNT_MAX)\n"
"\t\t\t""counter <= 32'h0 ;\n"
"\t\t""else\n"
"\t\t\t""counter <= counter + 32'h1 ;\n"
"\n"
"\t""assign TOUT_reg = (counter == 32'h0)? 1'b1 : 1'b0 ;\n"
"\n\t""endmodule\n";




EziDebugVlgFile::EziDebugVlgFile(const QString &filename):EziDebugFile(filename)
{

}

EziDebugVlgFile::EziDebugVlgFile(const QString &filename,const QDateTime &datetime,const QStringList &modulelist)\
    :EziDebugFile(filename,datetime,modulelist)
{

}


EziDebugVlgFile::~EziDebugVlgFile()
{

}

int EziDebugVlgFile::deleteScanChain(QStringList &ideletelinecodelist,const QStringList &ideleteblockcodelist,EziDebugScanChain *pchain,EziDebugPrj::OPERATE_TYPE type)
{
    int npos = 0 ;
    int ndeletePos = 0 ;
    int nresultPos = 0 ;
    QList<int> iposList ;
    QMap<int,int> ideleteCodePosMap ;
    QDateTime ilastModifedTime ;

    qDebug() << fileName() ;

    if(fileName().endsWith("SspApbifX.v"))
    {
        qDebug() << "SspApbifX.v";
    }

    // ��
    // ��ȡ �ļ�
    if(!open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // ���û����  �ļ��򲻿�
        qDebug() << errorString() << fileName() ;
        return 1 ;
    }

    QTextStream iin(this);
    QString ifileContent = iin.readAll();

    // �ر�
    close();
    // �� д �ļ�

    /*����ɨ������ļ�*/
    QString ieziDebugFileSuffix ;
    if(type == EziDebugPrj::OperateTypeDelSingleScanChain)
    {
        ieziDebugFileSuffix.append(tr(".delete.%1").arg(pchain->getChainName()));
    }
    else if(type == EziDebugPrj::OperateTypeDelAllScanChain)
    {
        ieziDebugFileSuffix.append(tr(".deleteall"));
    }
    else
    {
        return 1 ;
    }

    EziDebugPrj *pprj = const_cast<EziDebugPrj *>(EziDebugInstanceTreeItem::getProject());
    QFileInfo ifileInfo(fileName());

    QString idir = EziDebugScanChain::getUserDir() ;
    QString ibackupFileName = pprj->getCurrentDir().absolutePath()\
            + idir + tr("/") + ifileInfo.fileName() \
            + ieziDebugFileSuffix ;
    copy(ibackupFileName);
    pchain->addToScanedFileList(fileName());


    // ��ע�� �滻�� �ո�
    QString iblankString = replaceCommentaryByBlank(ifileContent);

#if 0
    QFile itestfile("d:/test.txt");
    if(!itestfile.open(QIODevice::Text|QIODevice::WriteOnly))
    {
        qDebug()<<"aaaaaaaa";
    }
    QTextStream itestout(&itestfile);
    itestout << iblankString ;
    itestfile.close();
#endif

    //QString icaptureString ;
    // ɾ���д���
    for(int i = 0 ; i < ideletelinecodelist.count() ; i++)
    {
        // �� stringlist ��ȡ �ַ��� ���� QRegExp
        QRegExp ifindExp(ideletelinecodelist.at(i));
        ifindExp.setMinimal(true);
        QRegExp ifindExpOther(tr("\\s*") + ideletelinecodelist.at(i) );
        ifindExpOther.setMinimal(true);
        // ���� ��ɾ�����ַ���
        if((nresultPos = ifindExp.indexIn(iblankString,npos)) == -1)
        {
            // ���û����  ���Ҳ��� �ַ���
            qDebug() << "EziDebug info: Can't find the string:" <<ideletelinecodelist.at(i) ;
            continue ;
        }
        else
        {
            // ɾ���� ��һ���ǿհ��ַ�
            ndeletePos = nresultPos ;
            int nlastNoBlankChar = ifileContent.lastIndexOf(QRegExp("\\S"),ndeletePos-1) ;

            npos = ndeletePos + ifindExp.matchedLength() ;

            if(-1 == nlastNoBlankChar)
            {
                ndeletePos = ndeletePos ;
            }
            else
            {
                ndeletePos = nlastNoBlankChar + 1 ;
            }

            //QString icatchStr = ifindExp.capturedTexts().at(0) ;
            //QString itest = ifileContent.mid(ndeletePos,nresultPos - ndeletePos +ifindExp.matchedLength()) ;
            //qDebug() << itest << icatchStr ;
            iposList.append(ndeletePos);
            ideleteCodePosMap.insert(ndeletePos,(nresultPos - ndeletePos + ifindExp.matchedLength()));

            // �滻 ��ɾ�����ַ��� Ϊ���ַ���
            //ifileContent.replace(icaptureString,tr(""));
        }
    }

    // ɾ�������
    npos = 0 ;
    for(int i = 0 ; i < ideleteblockcodelist.count() ; i++)
    {
        // �� stringlist ��ȡ �ַ��� ���� QRegExp
        QRegExp ifindExp(ideleteblockcodelist.at(i));


        // ���� ��ɾ�����ַ���
        if((npos = ifindExp.indexIn(iblankString,npos)) == -1)
        {
            // ���û����  ���Ҳ��� �ַ���
            qDebug() << ideletelinecodelist.at(i) ;
        }
        else
        {
            // always �� ;

            // Ѱ�� always
            int nalwaysPos = iblankString.lastIndexOf(QRegExp(QObject::tr("\\balways\\b")),npos);
            // ����ķǿհ��ַ�, �ӵ�һ���հ��ַ���ʼ �� always ���� �滻Ϊ��
            if(-1 != ifileContent.lastIndexOf(QRegExp(QObject::tr("\\S")),nalwaysPos-1))
            {
                nalwaysPos = ifileContent.lastIndexOf(QRegExp(QObject::tr("\\S")),nalwaysPos-1) + 1;
            }

            int nfirstBeginPos = iblankString.indexOf(QRegExp(QObject::tr("\\bbegin\\b")),nalwaysPos);
            /*����ƥ���  end*/
            QRegExp iwordsExp(QObject::tr("\\b\\w+\\b"));
            iwordsExp.setMinimal(true);
            int nmatch = 1 ;
            int nendPos = 0 ;
            int nbeginPos = nfirstBeginPos + 5 ;

            while((nbeginPos = iwordsExp.indexIn(iblankString,nbeginPos)) != -1)
            {
                if(iwordsExp.capturedTexts().at(0) == "begin")
                {
                    nmatch++ ;

                }
                else if(iwordsExp.capturedTexts().at(0) == "end")
                {
                    nmatch-- ;
                    if(0 == nmatch)
                    {
                        nendPos = nbeginPos ;
                        break ;
                    }

                }
                else
                {
                    //
                }
                nbeginPos += iwordsExp.matchedLength();
            }

            if(nmatch != 0)
            {
                qDebug() << "no matching end string!";
                close();
                return 1 ;
            }

            if(nendPos != 0)
            {
                //qDebug() << ifileContent.mid(nalwaysPos,nendPos - nalwaysPos + 3);
                //ifileContent.replace(nalwaysPos , nendPos - nalwaysPos + 3 ,"");
                iposList.append(nalwaysPos);
                ideleteCodePosMap.insert(nalwaysPos , nendPos - nalwaysPos + 3);
                npos = nendPos + 3 ;
            }
            else
            {
                qDebug() << "no finding end string!";
            }
        }
    }

    qSort(iposList.begin(), iposList.end(), qGreater<int>());

    for(int j = 0 ; j < iposList.count() ; j++)
    {
        int nstartPos = iposList.at(j) ;
        int nlength = ideleteCodePosMap.value(nstartPos , -1) ;
        ifileContent.replace(nstartPos , nlength ,"");
    }


    if(!open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        // ���û����  �ļ��򲻿�
        qDebug() << errorString() << fileName() ;
        return 1 ;
    }

    QTextStream iout(this);
    iout << ifileContent ;

    //�ر�
    close();

    ilastModifedTime = ifileInfo.lastModified() ;
    modifyStoredTime(ilastModifedTime);

    return 0 ;
}

int EziDebugVlgFile::addScanChain(INSERT_TYPE type,QMap<QString,EziDebugInstanceTreeItem::SCAN_CHAIN_STRUCTURE*> &chainStructuremap ,EziDebugScanChain* pchain, EziDebugInstanceTreeItem *pitem)
{

    QString ifileData ;
    QString ilastString ;
    QString ichainClock ;
    QString inoSynCode ;
    int noffSet = 0 ;
    EziDebugPrj * iprj = const_cast<EziDebugPrj *>(EziDebugInstanceTreeItem::getProject());
    EziDebugModule *pmodule =  iprj->getPrjModuleMap().value(pitem->getModuleName()) ;

    EziDebugModule *pparentMoudle = iprj->getPrjModuleMap().value(pitem->parent()->getModuleName()) ;
    QMap<QString,QString> iclockMap = pmodule->getClockSignal() ;
    //QMap<QString,QString> iresetMap = pmodule->getResetSignal() ;

    qDebug() << "Add chain in file:" << fileName();
    if(!open(QIODevice::ReadOnly|QIODevice::Text))
    {
        qDebug() << "Cannot Open file for reading:" << qPrintable(this->errorString());
        return 1 ;
    }

#if 0
    if(fileName().endsWith("ifft_airif_rdctrl.v"))
    {
        qDebug("add chain in iifft_airif_rdctrl.v");
    }
#endif

    /*�����ڵ�ʱ���� һ���Ĵ����� ������ϵ �ַ���  Ҫ���� verilog �ļ��� addScanChain ������ �����������֮��  ���浽��Ӧ�Ķ�����*/
    /*��ȡ�ļ������ַ�  ����һ��ע�͵Ľ����� �� ��һ��ע�͵Ŀ�ʼ�� ֮�� ������Ҫ���ַ���*/
    ifileData = this->readAll();
    close();



    /*��¼�� ɨ������ļ�  ������  */
    QFileInfo ifileInfo(fileName());
    pchain->addToScanedFileList(fileName());

    QString idir = EziDebugScanChain::getUserDir() ;
    QString ibackupFileName = iprj->getCurrentDir().absolutePath()\
            + idir + tr("/")+ ifileInfo.fileName() \
            + tr(".add") + tr(".%1").arg(pchain->getChainName());
    copy(ibackupFileName);

    QDateTime ilastModifedTime ;

    if(iprj->getToolType() == EziDebugPrj::ToolQuartus)
    {
        inoSynCode = "/*synthesis noprune*/" ;
    }
    else if(iprj->getToolType() == EziDebugPrj::ToolIse)
    {
        inoSynCode = "/* synthesis syn_keep = 1 xc_props = \"x\" */" ;
    }

    /*���ݲ���������� �ֱ������� */
    // ���뺯�� �ж� �¼���� �����Ƿ��ظ� ������ظ� ���ں����������
    if(InsertTimer == type)
    {
        /*����Ƕ�ʱ�� ��ش���  */
		
        struct SEARCH_STRING_STRUCTURE iModuleKeyWordSt ;
        iModuleKeyWordSt.m_etype = SearchModuleKeyWordPos ;
        iModuleKeyWordSt.m_icontent.m_imodulest.m_eportAnnounceFormat = NonAnsicFormat ;
        // ���ڵ�� �� module name
        
        strcpy(iModuleKeyWordSt.m_icontent.m_imodulest.m_amoduleName,pitem->parent()->getModuleName().toAscii().data());
        //  module ��
        int nmoduleKeyWordStartPos = 0 ;
        /*�ҵ�  �Ҽ�����ɨ������ module ��Ӧ���� �� module */
        if(skipCommentaryFind(ifileData,0,iModuleKeyWordSt,nmoduleKeyWordStartPos))
        {
            close();
            return 1 ;
        }

        struct SEARCH_MODULE_POS_STRUCTURE imodulePos ;
        strcpy(imodulePos.m_amoduleName,iModuleKeyWordSt.m_icontent.m_imodulest.m_amoduleName);
        /*���ù� �˿�������  ��ʽ*/
        imodulePos.m_nendModuleKeyWordPos = -1 ;
        /*��Ϊ��׼�ģ�����ɨ�� port */
        imodulePos.m_eportFormat = AnsicFormat ;
        imodulePos.m_nlastRegKeyWordPos = -1 ;
        imodulePos.m_nlastPortKeyWordPos = -1 ;
        imodulePos.m_nlastWireKeyWordPos = -1 ;
        imodulePos.m_nnextRightBracketPos = nmoduleKeyWordStartPos ;


        struct SEARCH_INSTANCE_POS_STRUCTURE *pinstanceSt = (struct SEARCH_INSTANCE_POS_STRUCTURE *)operator new(sizeof(struct SEARCH_INSTANCE_POS_STRUCTURE)) ;
        strcpy(pinstanceSt->m_amoduleName,pitem->getModuleName().toAscii().data());
        strcpy(pinstanceSt->m_ainstanceName,pitem->getInstanceName().toAscii().data());
        pinstanceSt->m_einstanceFormat = NonStardardFormat ;
        pinstanceSt->m_nnextRightBracketPos = -1 ;

        QMap<QString,SEARCH_INSTANCE_POS_STRUCTURE*> iinstancePosMap ;
        iinstancePosMap.insert(pitem->getInstanceName(),pinstanceSt);
        QString iinstanceCode ;
        QString iresetnWireName ;
        QString iresetnWireCode ;
        QString itoutWireName ;
        QString itoutRegName ;
        QString itdoWireCode ;
        QString itdoWireName ;
        QString itoutRegCode;
        QString itdoRegCode ;
        QString itdoRegName ;
        //QString itdoRegEvaluateCode ;
        QString itoutWireCode ;
        QString iblockCode ;
        QString ianncounceCode ;
        QString iaddedCode ;
        QStringList iaddedCodeList ;
        QStringList iaddedBlockCodeList ;
        // �ҵ� lastreg ��lastwire���Ҽ��ڵ������λ��
        if(!matchingTargetString(ifileData,imodulePos,iinstancePosMap))
        {

            // Ŀǰ�� ��� �˿��� �Ƿ� �ظ�
            //5������ ���ļĴ������� ������ �˿� ʱ�Ӹ��� �жϼ��뼸��

            //��ô����˿�ʱ��

            // ���� ���� �� tout_wire �Ĵ���
            itoutWireName.append(tr("_EziDebug_%1_tout_w").arg(pchain->getChainName()));
            itoutWireCode.append(tr("\n\t""wire _EziDebug_%1_tout_w ;").arg(pchain->getChainName()));
            iaddedCodeList.append(tr("\\bwire\\s+_EziDebug_%1_tout_w\\s*;").arg(pchain->getChainName()));



            itoutRegName.append(tr("_EziDebug_%1_tout_r").arg(pchain->getChainName()));
            itoutRegCode.append(tr("\n\t""reg %1 %2 ;").arg(itoutRegName).arg(inoSynCode));
            iaddedCodeList.append(tr("\\breg\\s*%1\\s*.*;").arg(itoutRegName));

            // �������ڿ��Ʒ�ת�� reset �ź�
            iresetnWireName.append(tr("_EziDebug_%1_rstn_w").arg(pchain->getChainName()));
            iresetnWireCode.append(tr("\n\t""wire _EziDebug_%1_rstn_w ;").arg(pchain->getChainName()));
            iaddedCodeList.append(tr("\\bwire\\s+_EziDebug_%1_rstn_w\\s*;").arg(pchain->getChainName()));

            // iaddedCode.append(itoutWireCode) ;
            ianncounceCode.append(itoutWireCode);
            ianncounceCode.append(itoutRegCode);
            ianncounceCode.append(iresetnWireCode);

            SEARCH_INSTANCE_POS_STRUCTURE *pchainInstanceSt = iinstancePosMap.value(pitem->getInstanceName()) ;
            //clockname Ϊ module ����� clock ����
            int num = 0 ;
            iinstanceCode.append("\n\t\t") ;
            QMap<QString,QString>::const_iterator i = iclockMap.constBegin();
            while(i != iclockMap.constEnd())
            {
                QString iparentClock ;
                QMap<QString,QString> iparentClockMap = pparentMoudle->getClockSignal();
                QMap<QString,QString>::const_iterator iparentClockIter = iparentClockMap.constBegin() ;
                while(iparentClockIter != iparentClockMap.constEnd())
                {
                    iparentClock = iparentClockIter.key();
                    ++iparentClockIter ;
                }
                /*���ݵ�ǰ�ڵ��ʱ�� ��� ɨ��������  TDI ����*/
                QString iconstBit(tr("%1'b").arg(chainStructuremap.value(i.key())->m_untotalChainNumber));
                for(int j = 0 ;j < chainStructuremap.value(i.key())->m_untotalChainNumber ; j++)
                {
                    iconstBit.append("1");
                }
                //6������ �Ҽ��������� instance �Ĵ��� ������ ʱ�Ӹ��� ��������ٸ� 2*TDI TDO ��λ��chainStructuremap ���ж��������ı���
                // ���� module ������ٸ� ʱ��, ���� ���ٸ� ��
                //   _EziDebug_clockname_TDI_reg
                //   _EziDebug_clockname_TDO_reg
                //   _EziDebug_TOUT_reg
                //  1�� ,\n\t._EziDebug_clockname_TDI_reg(1),\n\t_EziDebug_clockname_TDO_reg(wire_tdo ����),\n\t_EziDebug_TOUT_reg(wire_tout����)\n
                //  2�� , 1 , wire_tdo ���� , wire_tout����

                //  ���붨�� ��  tdo_wire �Ĵ���  // λ�����ٸ� wire(clock ����)
                // wire [number:0] _EziDebug_chainName_tdo_wire��� ;\n

                itdoWireName.append(tr("_EziDebug_%1_%2_tdo_w").arg(pchain->getChainName()).arg(i.key()));
                itdoRegName.append(tr("_EziDebug_%1_%2_tdo_r").arg(pchain->getChainName()).arg(i.key()));

                if((chainStructuremap.value(i.key())->m_untotalChainNumber) > 1)
                {

                    itdoWireCode.append(tr("\n\t""wire [%1:0] %2 ;").arg(chainStructuremap.value(i.key())->m_untotalChainNumber-1).arg(itdoWireName));
                    //wire [ %1 : 0 ] _EziDebug_%2_tdo_wire%3 ;
                    iaddedCodeList.append(tr("\\b")+tr("wire\\s+\\[")+tr("\\s*%1")\
                                          .arg(chainStructuremap.value(i.key())->m_untotalChainNumber - 1)\
                                          + tr("\\s*:\\s*0\\s*\\]\\s*") + tr("%1\\s*;")\
                                          .arg(itdoWireName));

                    itdoRegCode.append(tr("\n\t""reg [%1:0] %2 %3 ;").arg(chainStructuremap.value(i.key())->m_untotalChainNumber-1).arg(itdoRegName).arg(inoSynCode));
                    iaddedCodeList.append(tr("\\breg\\s*\\[\\s*%1\\s*:\\s*0\\s*\\]\\s*%2\\s*.*;").arg(chainStructuremap.value(i.key())->m_untotalChainNumber-1).arg(itdoRegName));
                }
                else
                {
                    itdoWireCode.append(tr("\n\t""wire %1 ;").arg(itdoWireName));
                    iaddedCodeList.append(tr("\\b")+tr("wire\\s+")+tr("%1\\s*;").arg(itdoWireName));

                    itdoRegCode.append(tr("\n\t""reg %1 %2 ;").arg(itdoRegName).arg(inoSynCode));
                    iaddedCodeList.append(tr("\\breg\\s*%1\\s*.*;").arg(itdoRegName));
                }

                //iaddedCode.append(itdoWireCode);
                //iaddedCode.append(itdoRegCode);

                ianncounceCode.append(itdoWireCode);
                ianncounceCode.append(itdoRegCode);


                // tdo always ��ֵ��� �޸�λ�ź�

                iblockCode.append(tr("\n\n\t""always@(posedge %1)").arg(iparentClock));
                iblockCode.append(tr("\n\t\tbegin"));
                iblockCode.append(tr("\n\t\t""%1 <= %2 ;").arg(itdoRegName).arg(itdoWireName));
                iblockCode.append(tr("\n\t\t""%1 <= %2 ;").arg(itoutRegName).arg(itoutWireName));
                iblockCode.append(tr("\n\t\tend"));



                iaddedBlockCodeList.append(tr("%1\\s*<=\\s*%2\\s*;").arg(itdoRegName).arg(itdoWireName));
                iaddedCode.append(iblockCode);

                /*
                    always@(posedge clk or negedge rstn)
                        if(!rstn)
                            TOut_reg <= 1'b0;
                        else if( addr[8:0] == 9'h0 )
                            TOut_reg <= 1'b1;
                        else
                            TOut_reg <= 1'b0;
                */
                if(StardardForamt == pchainInstanceSt->m_einstanceFormat)
                {
                    QString iportName = tr(",\n\t\t""._EziDebug_%1_%2_TDI_reg(%3),\n\t\t""._EziDebug_%4_%5_TDO_reg(%6)")\
                            .arg(pchain->getChainName()).arg(i.key()).arg(iconstBit).arg(pchain->getChainName()).arg(i.key()).arg(itdoWireName);
                    iinstanceCode.append(iportName);
                    //iportName.replace("\n\t","\\s*");
                    iaddedCodeList.append(tr(",\\s*\\._EziDebug_%1_%2_TDI_reg\\s*\\(\\s*%3\\s*\\)").arg(pchain->getChainName()).arg(i.key()).arg(iconstBit));
                    iaddedCodeList.append(tr(",\\s*\\._EziDebug_%1_%2_TDO_reg\\s*\\(\\s*%3\\s*\\)").arg(pchain->getChainName()).arg(i.key()).arg(itdoWireName));
                }
                else if(NonStardardFormat == pchainInstanceSt->m_einstanceFormat)
                {
                    QString iportName = tr(", %1 , %2 ,").arg(iconstBit).arg(itdoWireName) ;

                    iinstanceCode.append(iportName);
                    //iportName.replace(" ","\\s*");
                    iaddedCodeList.append(tr(",\\s*%1").arg(iconstBit));
                    iaddedCodeList.append(tr(",\\s*%1").arg(itdoWireName));
                }
                else
                {
                    // �ͷ��ڴ�
                    iinstancePosMap.remove(pitem->getInstanceName());
                    delete pinstanceSt ;
                    close();
                    return 1 ;
                }
                /*�����ַ���*/

                ++i ;
                num++;
            }

            ifileData.insert(pinstanceSt->m_nstartPos,ianncounceCode);
            noffSet += ianncounceCode.size() ;
            // ._EziDebug_chn_rstn(_EziDebug_chn_rstn_w)
            if(StardardForamt == pchainInstanceSt->m_einstanceFormat)
            {
                iinstanceCode.append(tr(",\n\t\t""._EziDebug_%1_rstn(%2)"",\n\t\t""._EziDebug_%3_TOUT_reg(%4)").arg(pchain->getChainName()).arg(iresetnWireName).arg(pchain->getChainName()).arg(itoutWireName)) ;

                iaddedCodeList.append(tr(",\\s*\\._EziDebug_%1_rstn\\s*\\(\\s*%2\\s*\\)").arg(pchain->getChainName()).arg(iresetnWireName));
                iaddedCodeList.append(tr(",\\s*\\._EziDebug_%1_TOUT_reg\\s*\\(\\s*%2\\s*\\)").arg(pchain->getChainName()).arg(itoutWireName));
            }
            else
            {
                iinstanceCode.append(tr(", %1 , %2").arg(iresetnWireName).arg(itoutWireName)) ;
                iaddedCodeList.append(tr(",\\s*%1\\s*,\\s*%2").arg(iresetnWireName).arg(itoutWireName));
            }

            // ����λ��ƫ�� iinstanceCode.size()
            ifileData.insert(pinstanceSt->m_nnextRightBracketPos + noffSet , iinstanceCode);
            noffSet += iinstanceCode.size() ;

            // �˿ڶ�Ӧ ���мĴ��� ����
            iaddedCode.append(tr("\n"));
            QVector<EziDebugModule::PortStructure*> iportVec = pmodule->getPort(iprj,pitem->getInstanceName()) ;
            QString iportRegCode ;
            QString iportRegName ;
            QStringList iportRegEvaluationStrList ;
            QStringList iportRegRevereStrList ;
            QStringList iportRegResetStrList ;
            for(int i = 0 ; i < iportVec.count() ;i++)
            {
                QString iportWireName = pparentMoudle->getInstancePortMap(pitem->getInstanceName()).value(QString::fromAscii(iportVec.at(i)->m_pPortName));
                iportRegName.clear();
                iportRegCode.clear();
                iportRegName.append(tr("_EziDebug_%1_%2_r").arg(pchain->getChainName()).arg(QString::fromAscii(iportVec.at(i)->m_pPortName)));

                if(iportVec.at(i)->m_unBitwidth == 1)
                {
                    iportRegCode.append(tr("\n\t""reg %1 %2 ;").arg(iportRegName).arg(inoSynCode));
                    iaddedCodeList.append(tr("\\breg\\s+%1\\s*.*;").arg(iportRegName));
                }
                else
                {
                    iportRegCode.append(tr("\n\t""reg [%1:%2] %3 %4 ;").arg(iportVec.at(i)->m_unStartBit).arg(iportVec.at(i)->m_unEndBit).arg(iportRegName).arg(inoSynCode));
                    iaddedCodeList.append(tr("\\breg\\s*\\[\\s*%1\\s*:\\s*%2\\s*\\]\\s*%3\\s*.*;").arg(iportVec.at(i)->m_unStartBit).arg(iportVec.at(i)->m_unEndBit).arg(iportRegName));
                }
                iportRegEvaluationStrList.append(tr("%1 <= %2 ;").arg(iportRegName).arg(iportWireName));
                iportRegRevereStrList.append(tr("%1 <= ~%2 ;").arg(iportRegName).arg(iportRegName));
                iportRegResetStrList.append(tr("%1 <= 0 ;").arg(iportRegName));
                iaddedCode.append(iportRegCode);

                if(i == 0)
                {
                    iaddedBlockCodeList.append(tr("%1\\s*<=\\s*%2\\s*;").arg(iportRegName).arg(iportWireName));
                }
            }

            iblockCode.clear() ;
            if(pchain->getscaningPortClock().isEmpty())
            {
                if(pmodule->getClockSignal().count() > 1)
                {
                    qDebug() << "Error: There is two or more Clock Signal";
                    close();
                    return 1 ;
                }

                QString iscanningClock ;
                QMap<QString,QString> iparentClockMap = pparentMoudle->getClockSignal();
                QMap<QString,QString>::const_iterator iparentClockIter = iparentClockMap.constBegin() ;
                while(iparentClockIter != iparentClockMap.constEnd())
                {
                    iscanningClock = iparentClockIter.key();
                    ++iparentClockIter ;
                }

                iblockCode.append(tr("\n\n\t""always@(posedge %1 or negedge %2)").arg(iscanningClock).arg(iresetnWireName));
                iblockCode.append(tr("\n\tbegin"));
                iblockCode.append(tr("\n\t\t""if(!%1)").arg(iresetnWireName));
                iblockCode.append(tr("\n\t\t\t""begin"));
                iblockCode.append(tr("\n\t\t\t%1").arg(iportRegRevereStrList.join("\n\t\t\t"))) ;
                iblockCode.append(tr("\n\t\t\t""end"));
                iblockCode.append(tr("\n\t\t""else"));
                iblockCode.append(tr("\n\t\t\t""begin"));
                iblockCode.append(tr("\n\t\t\t%1").arg(iportRegEvaluationStrList.join("\n\t\t\t")));
                iblockCode.append(tr("\n\t\t\t""end"));
                iblockCode.append(tr("\n\tend"));

            }
            else
            {

                iblockCode.append(tr("\n\n\t""always@(posedge %1)").arg(pchain->getscaningPortClock()));
                iblockCode.append(tr("\n\t""begin"));
                iblockCode.append(tr("\n\t\t%1").arg(iportRegEvaluationStrList.join("\n\t\t")));
                iblockCode.append(tr("\n\t""end"));

            }

            iaddedCode.append(iblockCode);


            //7������ �� TDO ���� reg �Ĵ���
            // �ҵ�ʱ�Ӷ�Ӧ��ϵ , �����й� tdo ��Ҫ�� wire �� ���� ��ؼĴ��������
            // ���ҵ�clock �� reset �ź� �� ʱ���ر仯��ϵ ������صļĴ���
            /*
              reg ......
              always@(posedge clock or negedge restn)
                if(!restn)
                    reg_n <= 0;
                else
                    reg_n <= wire tdo;

              always@(posedge clock or negedge restn)
                if(!restn)
                    reg_n <= 0;
                else
                    reg_n <= wire tdo;
            */

            //8�����뽫�˿ڱ���Ĵ���  �ڼ���ɨ�����Ľ��� �� �ṩ���ж˿� �� clock �Ķ�Ӧ��ϵ
            // ���� �˿�����ʱ�Ӹ��� ����  ��������صļĴ���  reg_n
            /*always@(posedge clock or negedge restn)
              if(!restn)
                  reg_n <= 0;
              else
                  reg_n <= wire tdo;
            */


            //9���� endmodule λ��֮ǰ���� timer �����Ĵ��� ;  ������ �������� �����¸�д������ظ� ֱ�ӱ��� ��Ӧ�ø��ʳ���С
            // itoutCore
            /*
            itoutCore itoutCore_chainName_inst(
                    .clock(��ʱ������),
                    .reset(module�и�λ�ź�����),
                    .Tout_reg(wire_tout��)
                    );
            */

            QString itoutInstanceCode ;
            itoutInstanceCode.append("\n\n\t");

            itoutInstanceCode.append(EziDebugScanChain::getChainToutCore());
            QString itoutInstanceName(EziDebugScanChain::getChainToutCore() + tr("_%1_inst").arg(pchain->getChainName()));

            QString ireset = "1'b0";
            QString iclock ;

            QMap<QString,QString> iparentClockMap = pparentMoudle->getClockSignal();
            QMap<QString,QString>::const_iterator iparentClockIter = iparentClockMap.constBegin() ;
            while(iparentClockIter != iparentClockMap.constEnd())
            {
                iclock = iparentClockIter.key();
                ++iparentClockIter ;
            }


            if(pchain->getSlowClock().isEmpty())
            {
                if(iclockMap.count() > 1)
                {

                    qDebug() << "Error: There is two or more Clock Signal,Please Input the Tout Clock!";
                    close();
                    return 1 ;
                }
                else
                {
                    // .rstn_out (_EziDebug_chn_rstn_w ),
                    if(iclockMap.count() == 1)
                    {

                        itoutInstanceCode.append(tr(" ")+itoutInstanceName+tr("(\n\t\t"".clock(%1),\n\t\t"".reset(%2),\n\t\t"".rstn_out(%3),\n\t\t"".TOUT_reg(%4)\n\t);")\
                                             .arg(iclock).arg(ireset).arg(iresetnWireName).arg(itoutWireName));


                    }
                    else
                    {
                        qDebug() << "Error: There is no Clock Signal In moudle:" << pitem->getModuleName();
                        close();
                        return 1 ;
                    }
                }
            }
            else
            {
                itoutInstanceCode.append(tr(" ")+itoutInstanceName+tr("(\n\t\t"".clock(%1),\n\t\t"".reset(%2),\n\t\t"".rstn_out(%3),\n\t\t"".TOUT_reg(%4)\n\t);")\
                                         .arg(pchain->getSlowClock()).arg(ireset).arg(iresetnWireName).arg(itoutWireName));
            }

            iaddedCode.append(itoutInstanceCode) ;

            ifileData.insert(imodulePos.m_nendModuleKeyWordPos + noffSet ,iaddedCode);
            /*ɾ�����뷽�� itoutCore /s+ itoutInstanceName \s+ ( �����ַ� ) /s+ ;*/
            iaddedCodeList.append(EziDebugScanChain::getChainToutCore()+tr("\\s+")+itoutInstanceName+tr("\\(")+tr(".+")+tr("\\)")+tr("\\s*;"));

        }
        else
        {
            iinstancePosMap.remove(pitem->getInstanceName());
            delete pinstanceSt ;
            close();
            return 1 ;
        }

        //10��д���ļ�

        pchain->addToLineCodeMap(pitem->parent()->getModuleName(),iaddedCodeList);
        pchain->addToBlockCodeMap(pitem->parent()->getModuleName(),iaddedBlockCodeList);

        if(!open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
        {
            qDebug() << "Cannot Open file for writing:" << qPrintable(this->errorString());
            return 1 ;
        }

        QTextStream iout(this);

        iout << ifileData  ;
        close();

        ilastModifedTime = ifileInfo.lastModified() ;
        modifyStoredTime(ilastModifedTime);
        // �ͷ��ڴ�
        iinstancePosMap.remove(pitem->getInstanceName());
        delete pinstanceSt ;
        return 0 ;

    }
    else if(InsertUserCore == type)
    {
        int nwireCount = pmodule->getEziDebugWireCounts(pchain->getChainName()) ;
        QStringList iaddedCodeList ;
        QStringList iaddedBlockCodeList ;
        QString iparentToutPort(tr("_EziDebug_%1_TOUT_reg").arg(pchain->getChainName())) ;

        struct SEARCH_STRING_STRUCTURE iModuleKeyWordSt ;
        iModuleKeyWordSt.m_etype = SearchModuleKeyWordPos ;
        iModuleKeyWordSt.m_icontent.m_imodulest.m_eportAnnounceFormat = NonAnsicFormat ;
        // ���ڵ�� �� module name
        strcpy(iModuleKeyWordSt.m_icontent.m_imodulest.m_amoduleName,pitem->getModuleName().toAscii().data());
        //  module ��
        int nmoduleKeyWordStartPos = 0 ;
        /*�ҵ�  ���屾 module �Ŀ�ʼλ��  */
        if(skipCommentaryFind(ifileData,0,iModuleKeyWordSt,nmoduleKeyWordStartPos))
        {
            close();
            return 1 ;
        }

        QString imodulePortCode ;
        // �ж����  module �ڱ���ɨ���� �� �Ƿ� ���������
        // ����ӹ��ˣ���ֱ������ ���Զ��� core �ĵط� �Ӵ���
        if(!pmodule->getAddCodeFlag())
        {
            //  ���ݱ�׼���߷Ǳ�׼ ���� �� moudle �� �����˿�  ÿ��clock��Ӧ��(TDI TDO) �� TOUT
            //clockname Ϊ module ����� clock ����
            QMap<QString,QString>::const_iterator clockMapIterator = iclockMap.constBegin();
            imodulePortCode.append("\n\t\t") ;
            while(clockMapIterator != iclockMap.constEnd())
            {
                // ���� module ������ٸ� ʱ��, ���� ���ٸ� ��
                //  [bitwidth-1:0] _EziDebug_chainname_clockname_TDI_reg or _EziDebug_chainname_clockname_TDI_reg
                //  [bitwidth-1:0] _EziDebug_chainname_clockname_TDO_reg or _EziDebug_chainname_clockname_TDO_reg
                //  _EziDebug_chainname_TOUT_reg

                //  ���붨�� ��  tdo_wire �Ĵ���  // λ�����ٸ� wire(clock ����)
                // wire _EziDebug_chainName_tdo_wire���[number:0] ;\n

                if(AnsicFormat == iModuleKeyWordSt.m_icontent.m_imodulest.m_eportAnnounceFormat)
                {
                    if((chainStructuremap.value(clockMapIterator.key())->m_untotalChainNumber) > 1)
                    {
                        QString iportName = tr(", input [%1:0] _EziDebug_%2_%3_TDI_reg ,\n\t""output [%4:0] _EziDebug_%5_%6_TDO_reg")\
                                .arg(chainStructuremap.value(clockMapIterator.key())->m_untotalChainNumber - 1).arg(pchain->getChainName()).arg(clockMapIterator.key())\
                                .arg(chainStructuremap.value(clockMapIterator.key())->m_untotalChainNumber - 1).arg(pchain->getChainName()).arg(clockMapIterator.key());

                        imodulePortCode.append(iportName);
                        //[ %1 : 0 ] _EziDebug_%2_%3_TDI_reg
                        iaddedCodeList.append(tr(",\\s*input\\s*\\[\\s*%1\\s*:\\s*0\\s*\\]\\s+_EziDebug_%2_%3_TDI_reg").arg(chainStructuremap.value(clockMapIterator.key())->m_untotalChainNumber - 1).arg(pchain->getChainName()).arg(clockMapIterator.key()));
                        //[ %1 : 0 ] _EziDebug_%2_%3_TDO_reg
                        iaddedCodeList.append(tr(",\\s*output\\s*\\[\\s*%1\\s*:\\s*0\\s*\\]\\s+_EziDebug_%2_%3_TDO_reg").arg(chainStructuremap.value(clockMapIterator.key())->m_untotalChainNumber - 1).arg(pchain->getChainName()).arg(clockMapIterator.key()));
                    }
                    else
                    {
                        imodulePortCode.append(tr(", input _EziDebug_%1_%2_TDI_reg ,\n\t""ouput_EziDebug_%3_%4_TDO_reg ,\n\t""input_EziDebug_%5_TOUT_reg")\
                                .arg(pchain->getChainName()).arg(clockMapIterator.key()).arg(pchain->getChainName()).arg(clockMapIterator.key()).arg(pchain->getChainName()));
                        iaddedCodeList.append(tr(",\\s*_EziDebug_%1_%2_TDI_reg\\s*").arg(pchain->getChainName()).arg(clockMapIterator.key()));
                        iaddedCodeList.append(tr(",\\s*_EziDebug_%1_%2_TDO_reg\\s*").arg(pchain->getChainName()).arg(clockMapIterator.key()));
                    }
                }
                else if(NonAnsicFormat == iModuleKeyWordSt.m_icontent.m_imodulest.m_eportAnnounceFormat)
                {
                    imodulePortCode.append(tr(",\n\t_EziDebug_%1_%2_TDI_reg ,\n\t""_EziDebug_%3_%4_TDO_reg")\
                            .arg(pchain->getChainName()).arg(clockMapIterator.key()).arg(pchain->getChainName()).arg(clockMapIterator.key()));
                    iaddedCodeList.append(tr(",\\s*_EziDebug_%1_%2_TDI_reg").arg(pchain->getChainName()).arg(clockMapIterator.key()));
                    iaddedCodeList.append(tr(",\\s*_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(clockMapIterator.key()));

                }
                else
                {
                    close();
                    return 1 ;
                }
                /*�����ַ���*/
                ++clockMapIterator ;
            }

            if(AnsicFormat == iModuleKeyWordSt.m_icontent.m_imodulest.m_eportAnnounceFormat)
            {
                imodulePortCode.append(tr(",\n\t""input _EziDebug_%1_rstn"",\n\t""input _EziDebug_%2_TOUT_reg").arg(pchain->getChainName()).arg(pchain->getChainName())) ;
                iaddedCodeList.append(tr(",\\s*input\\s+_EziDebug_%1_rstn").arg(pchain->getChainName()));
                iaddedCodeList.append(tr(",\\s*input\\s+_EziDebug_%2_TOUT_reg").arg(pchain->getChainName()));
            }
            else
            {
                imodulePortCode.append(tr(",\n\t_EziDebug_%1_rstn,\n\t""_EziDebug_%2_TOUT_reg").arg(pchain->getChainName()).arg(pchain->getChainName())) ;
                iaddedCodeList.append(tr(",\\s*_EziDebug_%1_rstn").arg(pchain->getChainName()));
                iaddedCodeList.append(tr(",\\s*_EziDebug_%1_TOUT_reg").arg(pchain->getChainName()));
            }

            ifileData.insert(nmoduleKeyWordStartPos,imodulePortCode);
        }

        /*1��END ����˿���������*/

        struct SEARCH_MODULE_POS_STRUCTURE imodulePos ;
        strcpy(imodulePos.m_amoduleName,iModuleKeyWordSt.m_icontent.m_imodulest.m_amoduleName);

        imodulePos.m_nendModuleKeyWordPos = -1 ;
        /*���� �˿������ķ�ʽ  �����Ƿ����� �˿�*/
        imodulePos.m_eportFormat = iModuleKeyWordSt.m_icontent.m_imodulest.m_eportAnnounceFormat ;
        imodulePos.m_nlastRegKeyWordPos = -1 ;
        imodulePos.m_nlastPortKeyWordPos = -1 ;
        imodulePos.m_nlastWireKeyWordPos = -1 ;
        imodulePos.m_nnextRightBracketPos = nmoduleKeyWordStartPos ;

        QMap<QString,SEARCH_INSTANCE_POS_STRUCTURE*> iinstancePosMap ;
        int  nnumberOfNoLibCore = 0 ;

        // ���� ���ڵ����� �������ӽڵ�
        for(int count = 0 ; count< pitem->childCount() ;count++ )
        {
            EziDebugInstanceTreeItem* pchildItem =  pitem->child(count) ;
            EziDebugPrj *prj = const_cast<EziDebugPrj*>(EziDebugInstanceTreeItem::getProject()) ;
            EziDebugModule *pchildModule = prj->getPrjModuleMap().value(pchildItem->getModuleName()) ;
            if(!pchildModule->isLibaryCore())
            {
                struct SEARCH_INSTANCE_POS_STRUCTURE *pinstanceSt = (struct SEARCH_INSTANCE_POS_STRUCTURE *)operator new(sizeof(struct SEARCH_INSTANCE_POS_STRUCTURE)) ;
                strcpy(pinstanceSt->m_amoduleName,pchildItem->getModuleName().toAscii().data());
                strcpy(pinstanceSt->m_ainstanceName,pchildItem->getInstanceName().toAscii().data());
                pinstanceSt->m_einstanceFormat = NonStardardFormat ;
                pinstanceSt->m_nnextRightBracketPos = -1 ;
                iinstancePosMap.insert(pchildItem->getInstanceName(),pinstanceSt);
                nnumberOfNoLibCore++ ;
            }
        }

        QString ilastTdoWire ; // ������һ�� tdo ���� ���� ���� instance ֮�� �� �˿�
        QString itdoWire ;
        imodulePortCode.clear();

        // �ҵ� lastreg ��lastwire
        if(matchingTargetString(ifileData,imodulePos,iinstancePosMap))
        {
            // �ͷ��ڴ�
            QMap<QString,SEARCH_INSTANCE_POS_STRUCTURE*>::iterator i = iinstancePosMap.begin();
            while(i != iinstancePosMap.end())
            {
                struct SEARCH_INSTANCE_POS_STRUCTURE *pinstanceSt = i.value();
                delete pinstanceSt ;
                ++i ;
            }
            iinstancePosMap.clear();
            close();
            return 1 ;
        }


        if(!pmodule->getAddCodeFlag())
        {
            /*������Ҫ�� ���� TDI��TOUT��TDO �˿ڶ��� */
            if(NonAnsicFormat == iModuleKeyWordSt.m_icontent.m_imodulest.m_eportAnnounceFormat)
            {
                /*����˿�*/
                QMap<QString,QString>::const_iterator i = iclockMap.constBegin();
                while(i != iclockMap.constEnd())
                {
                    // ���� module ������ٸ� ʱ��, ���� ���ٸ� ��
                    //  input  [bitwidth-1:0] _EziDebug_clockname_TDI_reg ; or input _EziDebug_clockname_TDI_reg ;
                    //  output [bitwidth-1:0] _EziDebug_clockname_TDO_reg ; or output _EziDebug_clockname_TDO_reg ;
                    //  input  _EziDebug_TOUT_reg ;

                    if((chainStructuremap.value(i.key())->m_untotalChainNumber) > 1)
                    {
                        QString iportName = tr("\n\t""input [%1:0] _EziDebug_%2_%3_TDI_reg ;\n\t""output [%4:0] _EziDebug_%5_%6_TDO_reg ;")\
                                .arg(chainStructuremap.value(i.key())->m_untotalChainNumber - 1).arg(pchain->getChainName()).arg(i.key())\
                                .arg(chainStructuremap.value(i.key())->m_untotalChainNumber - 1).arg(pchain->getChainName()).arg(i.key());

                        imodulePortCode.append(iportName);
                        //input [ %1 : 0 ] _EziDebug_%2_%3_TDI_reg
                        iaddedCodeList.append(tr("\\binput\\s*\\[\\s*%1\\s*:\\s*0\\s*\\]\\s+_EziDebug_%2_%3_TDI_reg\\s*;").arg(chainStructuremap.value(i.key())->m_untotalChainNumber - 1).arg(pchain->getChainName()).arg(i.key()));
                        //output [ %1 : 0 ] _EziDebug_%2_%3_TDO_reg
                        iaddedCodeList.append(tr("\\boutput\\s*\\[\\s*%1\\s*:\\s*0\\s*\\]\\s+_EziDebug_%2_%3_TDO_reg\\s*;").arg(chainStructuremap.value(i.key())->m_untotalChainNumber - 1).arg(pchain->getChainName()).arg(i.key()));
                    }
                    else
                    {
                        QString iportName = tr("\n\t""input _EziDebug_%1_%2_TDI_reg ;\n\t""output _EziDebug_%3_%4_TDO_reg ;")\
                                .arg(pchain->getChainName()).arg(i.key())\
                                .arg(pchain->getChainName()).arg(i.key());

                        imodulePortCode.append(iportName);
                        //input _EziDebug_%1_%2_TDI_reg
                        iaddedCodeList.append(tr("\\binput\\s+_EziDebug_%1_%2_TDI_reg\\s*;").arg(pchain->getChainName()).arg(i.key()));
                        //output _EziDebug_%1_%2_TDO_reg
                        iaddedCodeList.append(tr("\\boutput\\s+_EziDebug_%1_%2_TDO_reg\\s*;").arg(pchain->getChainName()).arg(i.key()));
                    }

                    /*�����ַ���*/
                    ++i ;
                }
                imodulePortCode.append(tr("\n\t""input _EziDebug_%1_TOUT_reg ;").arg(pchain->getChainName())) ;
                iaddedCodeList.append(tr("\\binput\\s+_EziDebug_%1_TOUT_reg\\s*;").arg(pchain->getChainName()));

                // _EziDebug_%1_rstn
                imodulePortCode.append(tr("\n\t""input _EziDebug_%1_rstn ;").arg(pchain->getChainName())) ;
                iaddedCodeList.append(tr("\\binput\\s+_EziDebug_%1_rstn\\s*;").arg(pchain->getChainName()));

                // �����һ���˿�����֮ǰ����  EziDebug �˿� TDI TDO TOUT ������
                ifileData.insert(imodulePos.m_nlastPortKeyWordPos ,imodulePortCode);
                noffSet += imodulePortCode.size() ;
            }

            QString itdoWireDefinitionCode ;
            QString iinstanceCode ;

            QString iclockString ;
            QString iresetString ;
            QString iresetRegStr ;
            QString ievaluateRegStr ;
            QString isysCoreCode ;
            QString iregDefinitionCode ;
            QString iinstanceExp ;

            int nnonCoreNum = 0 ;
            int nFirstnonCoreNum = 0 ;

            // ���� ���ڵ����� �������ӽڵ�
            for(int i = 0 ; i< pitem->childCount() ;i++ )
            {
                int m = 0 ;
                EziDebugInstanceTreeItem* pchildItem =  pitem->child(i) ;
                EziDebugPrj *prj = const_cast<EziDebugPrj *>(EziDebugInstanceTreeItem::getProject()) ;
                EziDebugModule *pchildModule = prj->getPrjModuleMap().value(pchildItem->getModuleName()) ;
                QMap<QString,QString> ichildModulePortMap = pmodule->getInstancePortMap(pchildItem->getInstanceName()) ;
                itdoWireDefinitionCode.clear();
                iinstanceCode.clear();
                itdoWire.clear();
                iinstanceExp.clear();
                if(!pchildModule->isLibaryCore())
                {
                    nnonCoreNum++ ;
                    iinstanceCode.append("\n\t\t") ;
                    /*����ģ����������� ������EziDebug�˿ڴ��� ��Ҫ�ӽڵ��븸ģ���ʱ����ͬ*/
                    QMap<QString,QString> ichildClockMap = pchildModule->getClockSignal();
                    if(!ichildClockMap.count())
                    {
                        continue ;
                    }
                    nFirstnonCoreNum++ ;
                    nwireCount++ ;
                    /*���� ��ϵͳcore ������ ������룺���� wire �Ĵ���  �������� �������� �˿ڡ�����ģ������ �˿ڲ������*/
                    struct SEARCH_INSTANCE_POS_STRUCTURE *pinstSt = iinstancePosMap.value(pchildItem->getInstanceName());

                    QMap<QString,QString>::const_iterator p = ichildClockMap.constBegin();
                    while(p != ichildClockMap.constEnd())
                    {
                        QString iparentTdiPort ;
                        QString iparentTdoPort ;

                        QString iparentClock = pitem->getModuleClockMap(pchildItem->getInstanceName()).key(p.key(),QString()) ;
                        if(iparentClock.isEmpty())
                        {
                            // �ͷ��ڴ�
                            QMap<QString,SEARCH_INSTANCE_POS_STRUCTURE*>::iterator i = iinstancePosMap.begin();
                            while(i != iinstancePosMap.end())
                            {
                                struct SEARCH_INSTANCE_POS_STRUCTURE *pinstanceSt = i.value();
                                delete pinstanceSt ;
                                ++i ;
                            }
                            iinstancePosMap.clear();
                            close();
                            return 1 ;
                        }
                        else
                        {
                            iparentTdiPort.append(tr("_EziDebug_%1_%2_TDI_reg").arg(pchain->getChainName()).arg(iparentClock));
                            iparentTdoPort.append(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iparentClock));
                        }

                        // ������������  ���������˿ڵ� wire
                        if((chainStructuremap.value(iparentClock)->m_untotalChainNumber) > 1)
                        {
                            /*1������ ���� wire TDO ���ݲ�ͬ�� clock ����Ĵ���*/
                            itdoWireDefinitionCode.append(tr("\n\t""wire [%1:0] _EziDebug_%2_%3_tdo%4 ;").arg(chainStructuremap.value(iparentClock)->m_untotalChainNumber-1).arg(pchain->getChainName()).arg(iparentClock).arg(nwireCount));
                            itdoWire.append(tr("_EziDebug_%1_%2_tdo%3").arg(pchain->getChainName()).arg(iparentClock).arg(nwireCount));
                            iaddedCodeList.append(tr("\\bwire\\s*") + tr("\\[\\s*%1\\s*:\\s*0\\s*\\]\\s*").arg(chainStructuremap.value(iparentClock)->m_untotalChainNumber-1) +tr("_EziDebug_%1_%2_tdo%3\\s*;").arg(pchain->getChainName()).arg(iparentClock).arg(nwireCount));
                        }
                        else
                        {
                            /*1������ ���� wire TDO ���ݲ�ͬ�� clock ����Ĵ���*/
                            itdoWireDefinitionCode.append(tr("\n\t""wire _EziDebug_%1_%2_tdo%3 ;").arg(pchain->getChainName()).arg(iparentClock).arg(nwireCount));
                            itdoWire.append(tr("_EziDebug_%1_%2_tdo%3").arg(pchain->getChainName()).arg(iparentClock).arg(nwireCount));
                            iaddedCodeList.append(tr("\\bwire\\s+") +tr("_EziDebug_%1_%2_tdo%3\\s*;").arg(pchain->getChainName()).arg(iparentClock).arg(nwireCount));
                        }

                        // �������� ��д��ʽ  ���������˿��Լ��˿����Ӵ���
                        if(StardardForamt == pinstSt->m_einstanceFormat)
                        {

                            /*�ж� ʱ�� �Ƿ�  ��Ӧ  �ҵ� ���� module �� clock ���Ǹ� TDI �� �˿� ���� */
                            // ��һ�� instance �� tdi �� module ��Ӧ clock �� tdi ���� , tdo �� ��һ�� instance ���� ����  ��module �Զ����  core ����
                            if(1 == nFirstnonCoreNum)
                            {
                                /*�����1�� no libcore instance*/
                                // ��һ�� instance �� tdi �� module ��Ӧ clock �� tdi ���� , tdo �� ��һ�� instance
                                QString iportName = tr(" ,\n\t""._EziDebug_%1_%2_TDI_reg(%3) ,\n\t""._EziDebug_%4_%5_TDO_reg(%6)")\
                                        .arg(pchain->getChainName()).arg(p.key()).arg(iparentTdiPort).arg(pchain->getChainName()).arg(p.key()).arg(itdoWire);
                                iinstanceCode.append(iportName);


                                iinstanceExp.append(tr("\\s*,\\s*._EziDebug_%1_%2_TDI_reg\\s*\\(\\s*%3\\s*\\)").arg(pchain->getChainName()).arg(p.key()).arg(iparentTdiPort));
                                iinstanceExp.append(tr("\\s*,\\s*._EziDebug_%1_%2_TDO_reg\\s*\\(\\s*%3\\s*\\)").arg(pchain->getChainName()).arg(p.key()).arg(itdoWire));
                            }
                            else
                            {
                                //��һ�� instance �� tdi �� module ��Ӧ clock �� tdi ����,��module �Զ����  core ����
                                // ����� instance �� tdi �� ��һ�� instance �� tdo ���� , tdo �� ��һ��  instacne ���� ���� �� module  �Զ���� core ����
                                QString iportName = tr(" ,\n\t""._EziDebug_%1_%2_TDI_reg(%3) ,\n\t""._EziDebug_%4_%5_TDO_reg(%6)")\
                                        .arg(pchain->getChainName()).arg(p.key()).arg(ilastTdoWire).arg(pchain->getChainName()).arg(p.key()).arg(itdoWire);
                                iinstanceCode.append(iportName);

                                // TDI ��  ��һ��  ������
                                iinstanceExp.append(tr("\\s*,\\s*._EziDebug_%1_%2_TDI_reg\\s*\\(\\s*%3\\s*\\)").arg(pchain->getChainName()).arg(p.key()).arg(ilastTdoWire));
                                iinstanceExp.append(tr("\\s*,\\s*._EziDebug_%1_%2_TDO_reg\\s*\\(\\s*%3\\s*\\)").arg(pchain->getChainName()).arg(p.key()).arg(itdoWire));

                            }

                        }
                        else if(NonStardardFormat == pinstSt->m_einstanceFormat)
                        {

                            // ��һ�� instance �� tdi �� module ��Ӧ clock �� tdi ���� , tdo �� ��һ�� instance ���� ����  ��module �Զ����  core ����
                            // ����� instance �� tdi �� ��һ�� instance �� tdo ���� , tdo �� ��һ��  instacne ���� ���� �� module  �Զ���� core ����
                            if(1 == nFirstnonCoreNum) // ԭ�� Ϊ i ,����Ϊ ��һ�� ��ϵͳ���� i = 0 ֻ�ܱ�֤�ǵ�һ�� ����(�п���Ϊ ��һ�� Ϊ ϵͳ����  �ڶ���Ϊ  ��ϵͳ���� �� �ڶ��� �� tdi ��Ҫ �� �˿� tdi ����)
                            {
                                QString iportName = tr(" , %1 , %2 ").arg(iparentTdiPort).arg(itdoWire) ;
                                iinstanceCode.append(iportName);
                                iportName.replace(" ","\\s*");
                                iinstanceExp.append(iportName);
                            }
                            else
                            {
                                QString iportName = tr(" , %1 , %2 ").arg(ilastTdoWire).arg(itdoWire) ;
                                iinstanceCode.append(iportName);
                                iportName.replace(" ","\\s*");
                                iinstanceExp.append(iportName);
                            }
                        }
                        else
                        {
                            // �ͷ��ڴ�
                            QMap<QString,SEARCH_INSTANCE_POS_STRUCTURE*>::iterator i = iinstancePosMap.begin();
                            while(i != iinstancePosMap.end())
                            {
                                struct SEARCH_INSTANCE_POS_STRUCTURE *pinstanceSt = i.value();
                                iinstancePosMap.remove(i.key());
                                delete pinstanceSt ;
                            }
                            close();
                            return 1 ;
                        }

                        /*���������1����ϵͳ����ʱ  ���浱ǰ�ĵ���� 1 �� tdo wire ,����Ҫ�������� EziDebug �Զ��������� tdi���� */
                        if(nnonCoreNum == nnumberOfNoLibCore)
                        {
                            /*�������� clock �� wire���� �Ķ�Ӧ��ϵ*/
                            pmodule->AddToClockWireNameMap(pchain->getChainName(),iparentClock,itdoWire);
                        }

                        ++p ;
                        //num++;
                    }

                    if(StardardForamt == pinstSt->m_einstanceFormat)
                    {
                        iinstanceCode.append(tr(",\n\t""._EziDebug_%1_rstn(_EziDebug_%2_rstn)").arg(pchain->getChainName()).arg(pchain->getChainName())) ;
                        iinstanceExp.append(tr("\\s*,\\s*._EziDebug_%1_rstn\\s*\\(\\s*_EziDebug_%2_rstn\\s*\\)").arg(pchain->getChainName()).arg(pchain->getChainName()));

                        iinstanceCode.append(tr(",\n\t""._EziDebug_%1_TOUT_reg(%2)").arg(pchain->getChainName()).arg(iparentToutPort)) ;
                        iinstanceExp.append(tr("\\s*,\\s*._EziDebug_%1_TOUT_reg\\s*\\(\\s*%2\\s*\\)").arg(pchain->getChainName()).arg(iparentToutPort));
                    }
                    else
                    {
                        QString iportName = tr(", _EziDebug_%1_rstn , %2").arg(pchain->getChainName()).arg(iparentToutPort) ;
                        iinstanceCode.append(iportName) ;
                        iinstanceExp.append(tr("\\s*,\\s*_EziDebug_%1_rstn\\s*,\\s*%2").arg(pchain->getChainName()).arg(iparentToutPort));
                    }


                    iaddedCodeList.append(iinstanceExp);

                    ifileData.insert(pinstSt->m_nstartPos + noffSet,itdoWireDefinitionCode);
                    noffSet += itdoWireDefinitionCode.size() ;
                    ifileData.insert(pinstSt->m_nnextRightBracketPos + noffSet,iinstanceCode);
                    noffSet += iinstanceCode.size() ;


                    /*�������һ�� Tdo �� �� ���� */


                    // ��¼��һ�ε�һ���ߵ�����,������һ�ε�����
                    ilastTdoWire = itdoWire ;
                }
                else
                {
                    QMap<QString,QString> iinstancePortMap = pmodule->getInstancePortMap(pchildItem->getInstanceName());

                    /*���� ϵͳcore ������  ������룺���� �Ĵ��� reg_n ���롢������˿��ź�����Ĵ�������  */
                    /*��module �� ��� ���е� ����˿�(λ����С��) ���� ʱ���� ��ʱ�����䷽��*/
                    QVector<EziDebugModule::PortStructure*> iportVec =  pchildModule->getPort(iprj,pchildItem->getInstanceName());
                    int nportCount = 0 ;
                    for(; nportCount < iportVec.count() ; nportCount++)
                    {
                        if((EziDebugModule::directionTypeOutput == iportVec.at(nportCount)->eDirectionType)||(EziDebugModule::directionTypeInoutput == iportVec.at(nportCount)->eDirectionType))
                        {
#if 0
                            QString iportName = iinstancePortMap.value(QString::fromAscii(iportVec.at(nportCount)->m_pPortName),QString());
                            if(iportName.isEmpty())
                            {
                                // �ͷ��ڴ�
                                QMap<QString,SEARCH_INSTANCE_POS_STRUCTURE*>::iterator j = iinstancePosMap.begin();
                                while(j != iinstancePosMap.end())
                                {
                                    struct SEARCH_INSTANCE_POS_STRUCTURE *pinstanceSt = j.value();
                                    iinstancePosMap.remove(j.key());
                                    delete pinstanceSt ;
                                }
                                goto ErrorHandle ;
                            }
#else
                            QString iportName = QString::fromAscii(iportVec.at(nportCount)->m_pPortName)   ;
                            QString icomPortName = pchildItem->getItemHierarchyName() + iportName ;
#endif

                            if(iportVec.at(nportCount)->m_unBitwidth > 1)
                            {
                                iregDefinitionCode.append(tr("\n\t""reg [%1:%2] _EziDebug_%3_%4_r %5 ;").arg(iportVec.at(nportCount)->m_unStartBit).arg(iportVec.at(nportCount)->m_unEndBit).arg(pchain->getChainName()).arg(QString::fromAscii(iportVec.at(nportCount)->m_pPortName)).arg(inoSynCode));
                                //isysCoreCode.append(iregDefinitionCode);
                                iaddedCodeList.append(tr("\\breg\\s*\\[\\s*%1\\s*:\\s*%2\\s*\\]\\s*_EziDebug_%3_%4_r.+;").arg(iportVec.at(nportCount)->m_unStartBit).arg(iportVec.at(nportCount)->m_unEndBit).arg(pchain->getChainName()).arg(QString::fromAscii(iportVec.at(nportCount)->m_pPortName)));
                            }
                            else
                            {
                                iregDefinitionCode.append(tr("\n\t""reg _EziDebug_%1_%2_r %3 ;").arg(pchain->getChainName()).arg(QString::fromAscii(iportVec.at(nportCount)->m_pPortName)).arg(inoSynCode));
                                //isysCoreCode.append(iregDefinitionCode);
                                iaddedCodeList.append(tr("\\breg\\s+_EziDebug_%1_%2_r\\s*.+;").arg(pchain->getChainName()).arg(QString::fromAscii(iportVec.at(nportCount)->m_pPortName)));
                            }


                            QString iregName(tr("_EziDebug_%1_%2_r").arg(pchain->getChainName()).arg(QString::fromAscii(iportVec.at(nportCount)->m_pPortName))) ;


                            QString iregWireName = ichildModulePortMap.value(QString::fromAscii(iportVec.at(nportCount)->m_pPortName),QString());

                            //qDebug() << "EziDebug info:sys core wire name:" << iregWireName ;
                            if((iregWireName.isEmpty()|(QRegExp(tr("^\\d+$")).exactMatch(iregWireName.toLower()))\
                                |(QRegExp(tr("^\\d+'[bhd][\\da-f]+$")).exactMatch(iregWireName.toLower()))\
                                |(QRegExp(tr("^`[ho][\\da-f]+$")).exactMatch(iregWireName.toLower()))
                                |(iregWireName.toLower()== "null")))
                            {
                                continue ;
                            }

                            QStringList iportList ;
                            iportList << pitem->getItemHierarchyName() << icomPortName  << iregName << QString::number(iportVec.at(nportCount)->m_unBitwidth)  ;

                            pchain->addToSyscoreOutputPortList(iportList.join(tr("#")));


                            //iresetRegStr.append(tr("\n\t\t%1 <= 0 ;").arg(iregName));
                            iresetRegStr.clear();
                            ievaluateRegStr.append(tr("\n\t\t%1 <= %2 ;").arg(iregName).arg(iregWireName));


                            if(m == 0)
                            {
                                iaddedBlockCodeList.append(tr("%1\\s*<=\\s*%2\\s*;").arg(iregName).arg(iregWireName));
                            }
                            m++ ;

                        }
                    }
                }
            }

            if(!ievaluateRegStr.isEmpty())
            {
                isysCoreCode.append(iregDefinitionCode);
                /*ͳһ���� �Ĵ��� ���� �����*/

                /*���� pchain �б����  clock list �ҵ� ��ɨ��˿ڵ�ʱ�� ��Ӧ�ı�ģ���ʱ�� */
                if(iclockMap.count() == 1)
                {
                    QMap<QString,QString>::const_iterator iclockIterator = iclockMap.constBegin();
                    while(iclockIterator != iclockMap.constEnd())
                    {
                        iclockString.append(tr("%1 %2").arg(QObject::tr("posedge")).arg(iclockIterator.key()));
                        ++iclockIterator ;
                    }
                }
                else
                {
                    close();
                    return 1 ;
                }

                QString iprocessCode ;

                iprocessCode.append((tr("\n\n\t""always@(%1)").arg(iclockString)));
                iprocessCode.append(tr("\n\t""begin"));
                iprocessCode.append(tr("%1").arg(ievaluateRegStr));
                iprocessCode.append(tr("\n\t""end"));


                /*����ɾ��������Ϣ*/
                isysCoreCode.append(iprocessCode);
            }
            /*д���ļ�*/
            ifileData.insert(imodulePos.m_nendModuleKeyWordPos + noffSet ,isysCoreCode);
            noffSet += isysCoreCode.size() ;
        }


        // �ͷ��ڴ�
        QMap<QString,SEARCH_INSTANCE_POS_STRUCTURE*>::iterator iinstanceIter = iinstancePosMap.begin();
        while(iinstanceIter != iinstancePosMap.end())
        {
            struct SEARCH_INSTANCE_POS_STRUCTURE *pinstanceSt = iinstanceIter.value();
            delete pinstanceSt ;
            ++iinstanceIter;
        }
        iinstancePosMap.clear();

#if 0
        if(fileName().endsWith("ifft.v"))
        {
            qDebug("add chain in ifft.v");
        }
#endif
        int nwireShiftRegNum = 0 ;
        int nlastChainStartNum = 0 ;
        int nlastChainEndNum = 0 ;
        int nchainStartNum = 0 ;
        int nchainEndNum = 0 ;
        int nusedNum = 0  ;
        QVector<EziDebugModule::RegStructure*> sregVec ;
        QVector<EziDebugModule::RegStructure*> vregVec ;
        QString ilastInput ;

        QString iusrCoreCode ;

        // ��������ʱ�� ȥ�����Ӧʱ�ӵ�ɨ����
        QMap<QString,QString>::const_iterator iclockIterator = iclockMap.constBegin();
        while(iclockIterator != iclockMap.constEnd())
        {
            EziDebugInstanceTreeItem::SCAN_CHAIN_STRUCTURE* pchainSt = chainStructuremap.value(iclockIterator.key()) ;

            int nSregNum = 0 ;
            int nVregNum = 0 ;

            // �Ѿ���ӵ� EziDebug core �ĸ���
            int ninstNum = pmodule->getEziDebugCoreCounts(pchain->getChainName()) ;
            int nleftRegNum = 0 ;
            int nregBitCount = 0 ;
            int nleftBit = -1 ;
            int nlastStopNum =  0 ;
            int nRegNum = 0 ;
            bool isNeedAdded = false ;
            ilastInput.clear();

            if(!pmodule->getAllRegMap(iclockIterator.key(),sregVec,vregVec))
            {
                qDebug() << "EeziDebug Error: Insert Chain Error!";
                return 1 ;
            }

            // û�мĴ��� ֱ�ӿ� next clock �Ƿ��� reg
            if((!sregVec.count())&&(!vregVec.count()))
            {
                // ���ü��κδ��� �����ü�¼��ǰ�� ����
                ++iclockIterator ;
                continue ;
            }

            // ����̶�λ�����мĴ�����bit�ܺ�
            int nregCount = 0 ;

            if(pchainSt->m_unleftRegNumber == 0)
            {
               pchainSt->m_unleftRegNumber =  iprj->getMaxRegNumPerChain() ;
               pchainSt->m_uncurrentChainNumber++ ;
            }

            // ��ʼbitλ
            nchainStartNum = pchainSt->m_uncurrentChainNumber ;

            // ��ȡ module�� chainxxx �� ������� ռ���� �� nlastChainStartNum �� nlastChainEndNum ����
            pmodule->getBitRangeInChain(pchain->getChainName(),iclockIterator.key(),&nlastChainStartNum ,&nlastChainEndNum);


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
            if(!pmodule->getConstInstacedTimesPerChain(pchain->getChainName()))
            {
                qDebug() << "EziDebug Error: the module instance times error!";
                return 1 ;
            }

            if(pmodule->getConstInstacedTimesPerChain(pchain->getChainName()) == 1)
            {
AddReg:
                // ֻ���ھ�̬�ļĴ���
                /*��λ�Ĵ��� ��ֵ ����*/
                QStringList iregNameList ;
                QVector<EziDebugModule::RegStructure*> iregVec = sregVec ;
                QStringList iregCombinationCode ;


                if(nleftBit != -1)
                {
                    int nlastLeftRegNum = qAbs(iregVec.at(nleftRegNum)->m_unEndBit - nleftBit) + 1 ;

                    QVector<EziDebugModule::RegStructure*> iregVec =  sregVec ;
                    // ʣ��ĸ���
                    if(nlastLeftRegNum >= iprj->getMaxRegNumPerChain())
                    {
                        while(nlastLeftRegNum >= iprj->getMaxRegNumPerChain())
                        {

                            ninstNum++ ;
                            nwireCount++ ;
                            iregCombinationCode.clear();
                            iregNameList.clear();

                            // ���� wire ���� tdo ����
                            QString iwireTdoName(tr("_EziDebug_%1_%2_tdo%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));
                            QString iwireTdoDefinitionCode(tr("\n\n\t""wire ")+ iwireTdoName + tr(" ;")) ;
                            iusrCoreCode.append(iwireTdoDefinitionCode);
                            iaddedCodeList.append(tr("\\bwire\\s+_EziDebug_%1_%2_tdo%3\\s*;")\
                                                  .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));

                            // ���� wire ���� ��λ�Ĵ��� ����
                            QString iwireShiftRegDefinitionCode(tr("\n\t""wire ") + tr("[%1:0]").arg(iprj->getMaxRegNumPerChain()-1)+ tr("_EziDebug_%1_%2_sr%3")\
                                                                .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(ninstNum) + tr(" ;"));
                            QString iwireSrName(tr("_EziDebug_%1_%2_sr%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(ninstNum)) ;

                            iaddedCodeList.append(tr("\\bwire\\s*\\[\\s*%1:\\s*0\\s*\\]\\s*").arg(iprj->getMaxRegNumPerChain()-1)+tr("_EziDebug_%1_%2_sr%3\\s*;")\
                                                  .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(ninstNum));

                            iusrCoreCode.append(iwireShiftRegDefinitionCode);
                            nwireShiftRegNum++ ;

                            /*��λ�Ĵ��� ��ֵ ����*/
                            // int nendBit = nleftBit + iprj->getMaxRegNumPerChain() -1;
                            int nendBit = 0 ;
                            if(iregVec.at(nleftRegNum)->m_unRegNum > 1)
                            {
                                if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                {
                                    /*��:��*/
                                    // iregVec.at(nleftRegNum)->m_unBitWidth-1
                                    nendBit = nleftBit + 1 - iprj->getMaxRegNumPerChain() ;
                                    iregNameList << constructChainRegString(iregVec.at(nleftRegNum) , nlastStopNum , nleftBit , nendBit , pitem);

                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum - nlastStopNum)+tr("[%1:%2]").arg(nleftBit).arg(nendBit));

                                    nleftBit = nendBit - 1 ;
                                }
                                else
                                {
                                    /*��:��*/
                                    nendBit = nleftBit -1 + iprj->getMaxRegNumPerChain() ;
                                    iregNameList << constructChainRegString(iregVec.at(nleftRegNum) , nlastStopNum , nleftBit , nendBit , pitem);

                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum + nlastStopNum)+tr("[%1:%2]").arg(nleftBit).arg(nendBit));

                                    nleftBit = nendBit + 1 ;
                                }
                            }
                            else
                            {
                                if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                {
                                    /*��:��*/
                                    nendBit = nleftBit + 1 - iprj->getMaxRegNumPerChain() ;
                                    iregNameList << constructChainRegString(iregVec.at(nleftRegNum) , nlastStopNum , nleftBit , nendBit , pitem);

                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1:%2]") \
                                                               .arg(nleftBit).arg(nendBit));
                                    nleftBit = nendBit - 1 ;
                                }
                                else
                                {
                                    /*��:��*/
                                    nendBit = nleftBit -1 + iprj->getMaxRegNumPerChain() ;
                                    iregNameList << constructChainRegString(iregVec.at(nleftRegNum) , nlastStopNum , nleftBit , nendBit , pitem);

                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1:%2]")\
                                                               .arg(nleftBit).arg(nendBit));

                                    nleftBit = nendBit + 1 ;
                                }
                            }


                            ichainClock = pchain->getChainClock(pitem->getInstanceName(),iclockIterator.key());
                            if(ichainClock.isEmpty())
                            {
                                qDebug() << "EziDebug Error: the top clock is not finded!" << __LINE__;
                                return 2 ;
                            }

                            pchain->addToRegChain(ichainClock ,pchainSt->m_uncurrentChainNumber ,iregNameList);

                            // ����ɨ��������
                            QString iwireShiftRegEvaluateString ;
                            iwireShiftRegEvaluateString.append(tr("\n\t""assign %1 = {\n\t\t\t\t\t\t\t\t\t""%2""\n\t\t\t\t\t\t\t\t\t};").arg(iwireSrName).arg(iregCombinationCode.join(" ,\n\t\t\t\t\t\t\t\t\t")));
                            iaddedCodeList.append(tr("\\bassign\\s+%1.*;").arg(iwireSrName));
                            iusrCoreCode.append(iwireShiftRegEvaluateString);

                            /*�Զ��� core �������� */
                            QString iusrCoreDefinitionCode ;
                            // QString iresetName = "1'b1";
                            // _EziDebug_chn_rstn
                            QString iresetName = tr("_EziDebug_%1_rstn").arg(pchain->getChainName());
                            QString iusrCoreTdi ;
                            if(nnumberOfNoLibCore != 0)
                            {
                                if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                                {
                                    iusrCoreTdi.append(tr("%1[%2]").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())).arg(pchainSt->m_uncurrentChainNumber));
                                }
                                else
                                {
                                    iusrCoreTdi.append(tr("%1").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())));
                                }
                            }
                            else
                            {
                                if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                                {
                                    iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg[%3]").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(pchainSt->m_uncurrentChainNumber));
                                }
                                else
                                {
                                    iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg").arg(pchain->getChainName()).arg(iclockIterator.key()));
                                }
                            }

                            iusrCoreDefinitionCode.append(tr("\n\t")+ EziDebugScanChain::getChainRegCore() + tr(" %1_%2_inst%3(\n").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));
                            iusrCoreDefinitionCode.append(  tr("\t"".clock""\t(%1) ,\n").arg(iclockIterator.key()) \
                                                            + tr("\t"".resetn""\t(%1) ,\n").arg(iresetName) \
                                                            + tr("\t"".TDI_reg""\t(%1) ,\n").arg(iusrCoreTdi) \
                                                            + tr("\t"".TDO_reg""\t(%1) ,\n").arg(iwireTdoName) \
                                                            + tr("\t"".TOUT_reg""\t(%1) ,\n").arg(iparentToutPort) \
                                                            + tr("\t"".shift_reg""\t(%1) \n\t) ;").arg(iwireSrName));

                            /*���� ���� userCore regWidth �޶��� ������*/
                            QString iparameterDefCode ;
                            iparameterDefCode.append(tr("\n\t""defparam %1_%2_inst%3.shift_width = %4 ;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum).arg(iprj->getMaxRegNumPerChain()));
                            iaddedCodeList.append(tr("\\bdefparam\\s+%1_%2_inst%3\\.shift_width\\s*=.*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));
                            iusrCoreCode.append(iparameterDefCode)  ;

                            iaddedCodeList.append(EziDebugScanChain::getChainRegCore() + tr("\\s+%1_%2_inst%3\\s*\\(.*\\)\\s*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));

                            iusrCoreCode.append(iusrCoreDefinitionCode);


                            /*module �˿����Ӵ���*/

                            QString iportConnectCode ;
                            // condition (chain number > 1) is true
                            iportConnectCode.append(tr("\n\t""assign %1[%2] = %3 ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                    .arg(pchainSt->m_uncurrentChainNumber).arg(iwireTdoName));
                            iaddedCodeList.append(tr("\\bassign\\s+%1\\s*\\[\\s*%2\\s*\\].*;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                  .arg(pchainSt->m_uncurrentChainNumber));
                            iusrCoreCode.append(iportConnectCode);


                            nlastLeftRegNum = nlastLeftRegNum - iprj->getMaxRegNumPerChain();
                            pchainSt->m_uncurrentChainNumber++ ;
                            pchainSt->m_unleftRegNumber = iprj->getMaxRegNumPerChain() ;
                        }
                    } // REG n ���� maxregnum �Ĳ��� ������

                    //  REG n ʣ�� �Ĳ���
                    iregCombinationCode.clear();
                    iregNameList.clear();

                    if(iregVec.at(nleftRegNum)->m_eRegBitWidthEndian == EziDebugModule::endianBig)
                    {
                        if((nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) > 0)
                        {
                            pchainSt->m_unleftRegNumber -= (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                            nregBitCount = (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                            iregNameList << constructChainRegString(iregVec.at(nleftRegNum),nlastStopNum,nleftBit,iregVec.at(nleftRegNum)->m_unEndBit ,pitem);

                            iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName) + QObject::tr("[%1][%2:%3]").arg(iregVec.at(nleftRegNum)->m_unStartNum - nlastStopNum).arg(nleftBit).arg(iregVec.at(nleftRegNum)->m_unEndBit));
                        }
                        else if((nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) == 0)
                        {
                            pchainSt->m_unleftRegNumber -= (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                            nregBitCount = (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                            iregNameList << constructChainRegString(iregVec.at(nleftRegNum),nlastStopNum,nleftBit,iregVec.at(nleftRegNum)->m_unEndBit ,pitem);

                            iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+QObject::tr("[%1][0]").arg(iregVec.at(nleftRegNum)->m_unStartNum - nlastStopNum));
                        }
                        else
                        {
                            // �����Ѿ�������
                        }

                    }
                    else
                    {
                        if((iregVec.at(nleftRegNum)->m_unEndBit - nleftBit) > 0) // ʣ����1bit
                        {
                            pchainSt->m_unleftRegNumber -= (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                            nregBitCount = (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                            iregNameList << constructChainRegString(iregVec.at(nleftRegNum),nlastStopNum,nleftBit,iregVec.at(nleftRegNum)->m_unEndBit ,pitem);

                            iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName) + QObject::tr("[%1][%2:%3]").arg(iregVec.at(nleftRegNum)->m_unStartNum + nlastStopNum).arg(nleftBit).arg(iregVec.at(nleftRegNum)->m_unEndBit));
                        }
                        else if((iregVec.at(nleftRegNum)->m_unEndBit - nleftBit) == 0) // ʣ1bit
                        {
                            pchainSt->m_unleftRegNumber -= (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                            nregBitCount = (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                            iregNameList << constructChainRegString(iregVec.at(nleftRegNum),nlastStopNum,nleftBit,iregVec.at(nleftRegNum)->m_unEndBit ,pitem);

                            iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1][0]").arg(iregVec.at(nleftRegNum)->m_unStartNum + nlastStopNum));
                        }
                        else
                        {
                            // �Ѿ�����
                        }
                    }



                    if(iregVec.at(nleftRegNum)->m_unRegNum > 1)
                    {
                        nlastStopNum++ ;
                    }
                    else
                    {
                        nleftRegNum++ ;
                        nlastStopNum = 0 ;
                    }

                    nleftBit = -1 ;
                }
                else
                {
                    nregBitCount = 0 ;
                }


                for(; nleftRegNum < iregVec.count() ; nleftRegNum++)
                {
                    for(int m = nlastStopNum ; m < iregVec.at(nleftRegNum)->m_unRegNum; m++)
                    {
                        nregBitCount += iregVec.at(nleftRegNum)->m_unRegBitWidth ;

                        if(nregBitCount < pchainSt->m_unleftRegNumber)
                        {
                            /*���������*/
                            // ���ü������
                            if(iregVec.at(nleftRegNum)->m_unRegNum > 1)
                            {
                                if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum - m));
                                }
                                else
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum + m));
                                }
                            }
                            else
                            {
                                iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName));
                            }

                            iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,iregVec.at(nleftRegNum)->m_unStartBit,iregVec.at(nleftRegNum)->m_unEndBit ,pitem);
                        }
                        else if(nregBitCount == pchainSt->m_unleftRegNumber)
                        {
                            // �պ���  ����  �´��µ�ɨ�������� ���
                            if(iregVec.at(nleftRegNum)->m_unRegNum > 1)
                            {
                                if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum - m));
                                }
                                else
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum + m));
                                }
                            }
                            else
                            {
                                iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName));
                            }

                            iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,iregVec.at(nleftRegNum)->m_unStartBit,iregVec.at(nleftRegNum)->m_unEndBit,pitem);

                            nlastStopNum = m ;
                            // nleftBit = -1 ;
                            nleftRegNum++ ;
                            isNeedAdded = true ;


                            goto WireShiftRegEvaluate0 ;
                        }
                        else
                        {
                            if(iregVec.at(nleftRegNum)->m_unRegNum > 1)
                            {

                                if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                {
                                    int nstartBit = iregVec.at(nleftRegNum)->m_unStartBit ;
                                    int nendBit = nregBitCount- pchainSt->m_unleftRegNumber + iregVec.at(nleftRegNum)->m_unStartBit + 1 - iregVec.at(nleftRegNum)->m_unRegBitWidth ;
                                    nleftBit = nendBit - 1 ;
                                    iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,nstartBit,nendBit,pitem);

                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum - m)+tr("[%1:%2]").arg(nstartBit).arg(nendBit));
                                }
                                else
                                {
                                    int nstartBit = iregVec.at(nleftRegNum)->m_unStartBit ;
                                    int nendBit = iregVec.at(nleftRegNum)->m_unRegBitWidth + iregVec.at(nleftRegNum)->m_unStartBit - 1 - (nregBitCount- pchainSt->m_unleftRegNumber) ;
                                    nleftBit = nendBit + 1 ;
                                    iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,nstartBit,nendBit,pitem);

                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum + m)+tr("[%1:%2]").arg(nstartBit).arg(nendBit));
                                }
                            }
                            else
                            {
                                if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                {
                                    int nstartBit = iregVec.at(nleftRegNum)->m_unStartBit ;
                                    int nendBit = nregBitCount- pchainSt->m_unleftRegNumber + iregVec.at(nleftRegNum)->m_unStartBit + 1 - iregVec.at(nleftRegNum)->m_unRegBitWidth ;
                                    nleftBit = nendBit - 1 ;
                                    iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,nstartBit,nendBit,pitem);

                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1:%2]") \
                                                               .arg(nstartBit).arg(nendBit));
                                }
                                else
                                {
                                    int nstartBit = iregVec.at(nleftRegNum)->m_unStartBit ;
                                    int nendBit = iregVec.at(nleftRegNum)->m_unRegBitWidth + iregVec.at(nleftRegNum)->m_unStartBit - 1 - (nregBitCount- pchainSt->m_unleftRegNumber) ;
                                    nleftBit = nendBit + 1 ;
                                    iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,nstartBit,nendBit,pitem);

                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1:%2]")\
                                                               .arg(nstartBit).arg(nendBit));
                                }
                            }

                            nlastStopNum = m ;
                            isNeedAdded = true ;
                            goto WireShiftRegEvaluate0 ;
                        }
                    }
                    nlastStopNum = 0 ;
                }
WireShiftRegEvaluate0:



                if(iregNameList.count())
                {

                    ichainClock = pchain->getChainClock(pitem->getInstanceName(),iclockIterator.key());

                    if(ichainClock.isEmpty())
                    {
                        return 2 ;
                    }

                    if(isNeedAdded == true)
                    {
                        nusedNum =  pchainSt->m_unleftRegNumber ;
                    }
                    else
                    {
                        nusedNum = nregBitCount ;
                    }

                    pchain->addToRegChain(ichainClock,pchainSt->m_uncurrentChainNumber,iregNameList);

                    nwireCount++ ;
                    // ���� wire ���� tdo ����
                    QString iwireTdoName(tr("_EziDebug_%1_%2_tdo%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));
                    QString iwireTdoDefinitionCode(tr("\n\n\t""wire ")+ iwireTdoName + tr(" ;")) ;
                    iusrCoreCode.append(iwireTdoDefinitionCode);
                    iaddedCodeList.append(tr("\\bwire\\s+_EziDebug_%1_%2_tdo%3\\s*;")\
                                          .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));

                    // ���� wire ���� ��λ�Ĵ��� ����
                    QString iwireShiftRegDefinitionCode(tr("\n\t""wire ") + tr("[%1:0] ").arg(nusedNum-1) + tr("_EziDebug_%1_%2_sr%3")\
                                                        .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum) + tr(" ;"));
                    QString iwireSrName(tr("_EziDebug_%1_%2_sr%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum)) ;

                    iaddedCodeList.append(tr("\\bwire\\s*\\[\\s*%1\\s*:\\s*0\\s*\\]\\s*").arg(nusedNum-1) + tr("_EziDebug_%1_%2_sr%3\\s*;")\
                                          .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum));

                    iusrCoreCode.append(iwireShiftRegDefinitionCode);
                    nwireShiftRegNum++ ;

                    //pchain->addToRegChain(ichainClock,pchainSt->m_uncurrentChainNumber,iregNameList);

                    QString iwireShiftRegEvaluateString ;
                    iwireShiftRegEvaluateString.append(tr("\n\t""assign %1 = {\n\t\t\t\t\t\t\t\t\t""%2""\n\t\t\t\t\t\t\t\t\t};").arg(iwireSrName).arg(iregCombinationCode.join(" ,\n\t\t\t\t\t\t\t\t\t")));
                    iaddedCodeList.append(tr("\\bassign\\s+%1.*;").arg(iwireSrName));
                    iusrCoreCode.append(iwireShiftRegEvaluateString);


                    /*�Զ��� core �������� */
                    QString iusrCoreDefinitionCode ;
                    //QString iresetName = "1'b1";
                    QString iresetName = tr("_EziDebug_%1_rstn").arg(pchain->getChainName());


                    QString iusrCoreTdi ;
                    if(nnumberOfNoLibCore != 0)
                    {
                        if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                        {
                            iusrCoreTdi.append(tr("%1[%2]").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())).arg(pchainSt->m_uncurrentChainNumber));
                        }
                        else
                        {
                            iusrCoreTdi.append(tr("%1").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())));
                        }
                    }
                    else
                    {
                        if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                        {
                            iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg[%3]").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(pchainSt->m_uncurrentChainNumber));
                        }
                        else
                        {
                            iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg").arg(pchain->getChainName()).arg(iclockIterator.key()));
                        }
                    }

                    iusrCoreDefinitionCode.append(tr("\n\t")+ EziDebugScanChain::getChainRegCore() + tr(" %1_%2_inst%3(\n").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));
                    iusrCoreDefinitionCode.append(  tr("\t"".clock""\t(%1) ,\n").arg(iclockIterator.key()) \
                                                    + tr("\t"".resetn""\t(%1) ,\n").arg(iresetName) \
                                                    + tr("\t"".TDI_reg""\t(%1) ,\n").arg(iusrCoreTdi) \
                                                    + tr("\t"".TDO_reg""\t(%1) ,\n").arg(iwireTdoName) \
                                                    + tr("\t"".TOUT_reg""\t(%1) ,\n").arg(iparentToutPort) \
                                                    + tr("\t"".shift_reg""\t(%1) \n\t) ;").arg(iwireSrName));


                    /*���� ���� userCore regWidth �޶��� ������*/
                    QString iparameterDefCode ;
                    iparameterDefCode.append(tr("\n\n\t""defparam %1_%2_inst%3.shift_width = %4 ;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum).arg(nusedNum));
                    iaddedCodeList.append(tr("\\bdefparam\\s+%1_%2_inst%3\\.shift_width\\s*=.*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));

                    iusrCoreCode.append(iparameterDefCode)  ;
                    iaddedCodeList.append(EziDebugScanChain::getChainRegCore() + tr("\\s+%1_%2_inst%3\\s*(.*)\\s*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));

                    iusrCoreCode.append(iusrCoreDefinitionCode);

                    /*module �˿����Ӵ���*/
                    QString iportConnectCode ;
                    if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                    {
                        iportConnectCode.append(tr("\n\t""assign %1[%2] = %3 ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                .arg(pchainSt->m_uncurrentChainNumber).arg(iwireTdoName));
                        iaddedCodeList.append(tr("\\bassign\\s+%1\\s*\\[\\s*%2\\s*\\].*;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                              .arg(pchainSt->m_uncurrentChainNumber));
                    }
                    else
                    {
                        iportConnectCode.append(tr("\n\t""assign %1 = %2 ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                .arg(iwireTdoName));
                        iaddedCodeList.append(tr("\\bassign\\s+%1\\s*=\\s*%2\\s*;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                              .arg(iwireTdoName));
                    }
                    iusrCoreCode.append(iportConnectCode);
                }


                if(isNeedAdded == true)
                {
                    pchainSt->m_uncurrentChainNumber++ ;
                    pchainSt->m_unleftRegNumber = iprj->getMaxRegNumPerChain() ;
                    isNeedAdded = false ;
                }
                else
                {
                    pchainSt->m_unleftRegNumber -= nregBitCount ;
                }

                if(nleftRegNum != iregVec.count())
                {
                    goto AddReg ;
                }

            }
            else
            {
                // �϶�Ϊ��������� ģ��
                for(;nregCount < sregVec.count(); nregCount++)
                {
                    EziDebugModule::RegStructure* preg =  sregVec.at(nregCount) ;
                    nSregNum += preg->m_unMaxBitWidth ;
                }

                nregCount = 0 ;

                for(;nregCount < vregVec.count();nregCount++)
                {
                    EziDebugModule::RegStructure* preg = vregVec.at(nregCount) ;
                    nVregNum += preg->m_unMaxBitWidth ;
                }


                //
                if((nSregNum + nVregNum) < pchainSt->m_unleftRegNumber)
                {
                    QStringList iregNameList ;
                    QStringList iregCombinationCode ;


                    QVector<EziDebugModule::RegStructure*> iregVec = sregVec ;
                    nleftRegNum = 0 ;
                    int nstaticRegBitSum = 0 ;
                    for(; nleftRegNum < iregVec.count() ; nleftRegNum++)
                    {
                        for(int m = 0 ; m < iregVec.at(nleftRegNum)->m_unRegNum ; m++)
                        {
                            EziDebugModule::RegStructure* preg = sregVec.at(nleftRegNum)  ;                    

                            nstaticRegBitSum += preg->m_unRegBitWidth ;
                            nRegNum += preg->m_unRegBitWidth ;

                            // reg pointer regnum  startbit  endbit  hiberarchyname
                            if(preg->m_eRegNumEndian)
                            {
                                iregNameList << constructChainRegString(preg,iregVec.at(nleftRegNum)->m_unStartNum - m , preg->m_unStartBit , preg->m_unEndBit ,pitem);
                                if(preg->m_unRegNum != 1)
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName) + tr("[%1]")\
                                                               .arg(iregVec.at(nleftRegNum)->m_unStartNum - m));
                                }
                                else
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName));
                                }
                            }
                            else
                            {
                                iregNameList << constructChainRegString(preg,iregVec.at(nleftRegNum)->m_unStartNum + m , preg->m_unStartBit , preg->m_unEndBit ,pitem);
                                if(preg->m_unRegNum != 1)
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]")\
                                                               .arg(iregVec.at(nleftRegNum)->m_unStartNum + m));
                                }
                                else
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName));
                                }
                            }                    

                        }
                    }

                    QString ivarRegBitSum ;
                    if(nstaticRegBitSum)
                    {
                        ivarRegBitSum =  QString::number(nstaticRegBitSum) ;
                    }
                    nleftRegNum = 0 ;

                    iregVec = vregVec ;

                    for(; nleftRegNum < iregVec.count() ; nleftRegNum++)
                    {
                        EziDebugModule::RegStructure* pvarReg = iregVec.at(nleftRegNum) ;
                        // ȷ�еض�ֵ �� instancereg �ṩ
                        EziDebugModule::RegStructure* pinstanceReg = pmodule->getInstanceReg(pitem->getInstanceName(),iclockIterator.key(),QString::fromAscii(pvarReg->m_pRegName));
                        nRegNum += pvarReg->m_unMaxBitWidth ;
                        // construct the bitwidth string
                        if(pinstanceReg->m_unRegNum == 1)
                        {
                            ivarRegBitSum.append(QObject::tr("+") + QString::fromAscii(pvarReg->m_pExpString));
                        }
                        else
                        {
                            ivarRegBitSum.append(QObject::tr("+") + QObject::tr("(%1)*%2").arg(QString::fromAscii(pvarReg->m_pExpString)).arg(pinstanceReg->m_unRegNum));
                        }
                         // iprefixStr
                        for(int m = 0 ; m < pinstanceReg->m_unRegNum ; m++)
                        {

                            if(pinstanceReg->m_eRegNumEndian == EziDebugModule::endianBig)
                            {
                                iregNameList << constructChainRegString(pinstanceReg,pinstanceReg->m_unStartNum - m , pinstanceReg->m_unStartBit , pinstanceReg->m_unEndBit ,pitem);
                                if(pinstanceReg->m_unRegNum != 1)
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName) + tr("[%1]")\
                                                               .arg(iregVec.at(nleftRegNum)->m_unStartNum - m));
                                }
                                else
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName));
                                }
                            }
                            else
                            {
                                iregNameList << constructChainRegString(pinstanceReg,pinstanceReg->m_unStartNum + m , pinstanceReg->m_unStartBit , pinstanceReg->m_unEndBit ,pitem);
                                if(pinstanceReg->m_unRegNum != 1)
                                {
                                    iregCombinationCode.append( QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName) + tr("[%1]")\
                                                               .arg(iregVec.at(nleftRegNum)->m_unStartNum - m));
                                }
                                else
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName));
                                }
                            }
                        }

                    }

                    ichainClock = pchain->getChainClock(pitem->getInstanceName(),iclockIterator.key());

                    if(ichainClock.isEmpty())
                    {
                        return 2 ;
                    }

                    pchain->addToRegChain(ichainClock,pchainSt->m_uncurrentChainNumber,iregNameList);


                    // ������� �µĴ���
                    pchainSt->m_unleftRegNumber = pchainSt->m_unleftRegNumber - nRegNum ;

                    if(nchainStartNum == nlastChainEndNum) // ������� �µĴ���,����ͬһ��ɨ�����С��ҡ�ʣ�����Ĵ�������
                    {
                        ++iclockIterator ;
                        continue ;
                    }

                    // defparameter �͡�������λ�Ĵ�����ȡ���Ϊ���ַ������͡�����ȫ����(������ڱ仯λ��ļĴ����Ļ�)
                    // ����ġ�ezidebug core �ġ�tdi ���ݡ�ǰ���Ƿ���ڡ���ϵͳcore ���ӡ�ģ��˿ڵġ�tdi ���ߡ������
                    // ��ϵͳcore �ġ����һ��tdo_wire
                    // ezidebug core ��tdo �����Ƿ�Ϊ��󣱴���Ӵ��롡�롡lastwire ���������ߡ��˿ڵġ�tdo ����
                    // ���ӡ����������ġ���ʼbit �롡��һ�����bit ֮��ġ����ӡ���assign

                    nwireCount++ ;
                    // ���� wire ���� tdo ����
                    QString iwireTdoName(tr("_EziDebug_%1_%2_tdo%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));
                    QString iwireTdoDefinitionCode(tr("\n\n\t""wire ")+ iwireTdoName + tr(" ;")) ;
                    iusrCoreCode.append(iwireTdoDefinitionCode);
                    iaddedCodeList.append(tr("\\bwire\\s+_EziDebug_%1_%2_tdo%3\\s*;")\
                                          .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));


                    // ���� wire ���� ��λ�Ĵ��� ����
                    QString iwireShiftRegDefinitionCode(tr("\n\t""wire ") + tr("[%1:0] ").arg(ivarRegBitSum + tr(" - 1")) + tr("_EziDebug_%1_%2_sr%3")\
                                                        .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum) + tr(" ;"));
                    QString iwireSrName(tr("_EziDebug_%1_%2_sr%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum)) ;

                    iaddedCodeList.append(tr("\\bwire\\s*\\[\\s*%1\\s*:\\s*0\\s*\\]\\s*").arg(ivarRegBitSum + tr(" - 1")) + tr("_EziDebug_%1_%2_sr%3\\s*;")\
                                          .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum));

                    iusrCoreCode.append(iwireShiftRegDefinitionCode);
                    nwireShiftRegNum++ ;

                    //pchain->addToRegChain(ichainClock,pchainSt->m_uncurrentChainNumber,iregNameList);

                    QString iwireShiftRegEvaluateString ;
                    iwireShiftRegEvaluateString.append(tr("\n\t""assign %1 = {\n\t\t\t\t\t\t\t\t\t""%2""\n\t\t\t\t\t\t\t\t\t};").arg(iwireSrName).arg(iregCombinationCode.join(" ,\n\t\t\t\t\t\t\t\t\t")));
                    iaddedCodeList.append(tr("\\bassign\\s+%1.*;").arg(iwireSrName));
                    iusrCoreCode.append(iwireShiftRegEvaluateString);


                    /*�Զ��� core �������� */
                    QString iusrCoreDefinitionCode ;
                    //QString iresetName = "1'b1";
                    QString iresetName = tr("_EziDebug_%1_rstn").arg(pchain->getChainName());


                    QString iusrCoreTdi ;
                    if(nnumberOfNoLibCore != 0)
                    {
                        if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                        {
                            iusrCoreTdi.append(tr("%1[%2]").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())).arg(pchainSt->m_uncurrentChainNumber));
                        }
                        else
                        {
                            iusrCoreTdi.append(tr("%1").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())));
                        }
                    }
                    else
                    {
                        if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                        {
                            iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg[%3]").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(pchainSt->m_uncurrentChainNumber));
                        }
                        else
                        {
                            iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg").arg(pchain->getChainName()).arg(iclockIterator.key()));
                        }
                    }

                    iusrCoreDefinitionCode.append(tr("\n\t")+ EziDebugScanChain::getChainRegCore() + tr(" %1_%2_inst%3(\n").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));
                    iusrCoreDefinitionCode.append(  tr("\t"".clock""\t(%1) ,\n").arg(iclockIterator.key()) \
                                                    + tr("\t"".resetn""\t(%1) ,\n").arg(iresetName) \
                                                    + tr("\t"".TDI_reg""\t(%1) ,\n").arg(iusrCoreTdi) \
                                                    + tr("\t"".TDO_reg""\t(%1) ,\n").arg(iwireTdoName) \
                                                    + tr("\t"".TOUT_reg""\t(%1) ,\n").arg(iparentToutPort) \
                                                    + tr("\t"".shift_reg""\t(%1) \n\t) ;").arg(iwireSrName));


                    /*���� ���� userCore regWidth �޶��� ������*/
                    QString iparameterDefCode ;
                    iparameterDefCode.append(tr("\n\n\t""defparam %1_%2_inst%3.shift_width = %4 ;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum).arg(ivarRegBitSum));
                    iaddedCodeList.append(tr("\\bdefparam\\s+%1_%2_inst%3\\.shift_width\\s*=.*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));

                    iusrCoreCode.append(iparameterDefCode)  ;
                    iaddedCodeList.append(EziDebugScanChain::getChainRegCore() + tr("\\s+%1_%2_inst%3\\s*(.*)\\s*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));

                    iusrCoreCode.append(iusrCoreDefinitionCode);

                    /*module �˿����Ӵ���*/
                    QString iportConnectCode ;
                    if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                    {
                        iportConnectCode.append(tr("\n\t""assign %1[%2] = %3 ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                .arg(pchainSt->m_uncurrentChainNumber).arg(iwireTdoName));
                        iaddedCodeList.append(tr("\\bassign\\s+%1\\s*\\[\\s*%2\\s*\\].*;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                              .arg(pchainSt->m_uncurrentChainNumber));
                    }
                    else
                    {
                        iportConnectCode.append(tr("\n\t""assign %1 = %2 ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                .arg(iwireTdoName));
                        iaddedCodeList.append(tr("\\bassign\\s+%1\\s*=\\s*%2\\s*;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                              .arg(iwireTdoName));
                    }
                    iusrCoreCode.append(iportConnectCode);
                }
                else
                {
                    // ���µ�ɨ���� ��ʼ���
                    pchainSt->m_uncurrentChainNumber++ ;
                    pchainSt->m_unleftRegNumber = iprj->getMaxRegNumPerChain() ;

AddReg1:
                    // ֻ���ھ�̬�ļĴ���
                    /*��λ�Ĵ��� ��ֵ ����*/
                    QStringList iregNameList ;
                    QVector<EziDebugModule::RegStructure*> iregVec = sregVec ;
                    QStringList iregCombinationCode ;


                    if(nleftBit != -1)
                    {
                        int nlastLeftRegNum = qAbs(iregVec.at(nleftRegNum)->m_unEndBit - nleftBit) + 1 ;

                        QVector<EziDebugModule::RegStructure*> iregVec =  sregVec ;
                        // ʣ��ĸ���
                        if(nlastLeftRegNum >= iprj->getMaxRegNumPerChain())
                        {
                            while(nlastLeftRegNum >= iprj->getMaxRegNumPerChain())
                            {
                                ninstNum++ ;
                                nwireCount++ ;
                                iregCombinationCode.clear();
                                iregNameList.clear();
                                // ���� wire ���� tdo ����
                                QString iwireTdoName(tr("_EziDebug_%1_%2_tdo%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));
                                QString iwireTdoDefinitionCode(tr("\n\n\t""wire ")+ iwireTdoName + tr(" ;")) ;
                                iusrCoreCode.append(iwireTdoDefinitionCode);
                                iaddedCodeList.append(tr("\\bwire\\s+_EziDebug_%1_%2_tdo%3\\s*;")\
                                                      .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));

                                // ���� wire ���� ��λ�Ĵ��� ����
                                QString iwireShiftRegDefinitionCode(tr("\n\t""wire ") + tr("[%1:0]").arg(iprj->getMaxRegNumPerChain()-1)+ tr("_EziDebug_%1_%2_sr%3")\
                                                                    .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(ninstNum) + tr(" ;"));
                                QString iwireSrName(tr("_EziDebug_%1_%2_sr%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(ninstNum)) ;

                                iaddedCodeList.append(tr("\\bwire\\s*\\[\\s*%1:\\s*0\\s*\\]\\s*").arg(iprj->getMaxRegNumPerChain()-1)+tr("_EziDebug_%1_%2_sr%3\\s*;")\
                                                      .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(ninstNum));

                                iusrCoreCode.append(iwireShiftRegDefinitionCode);
                                nwireShiftRegNum++ ;

                                /*��λ�Ĵ��� ��ֵ ����*/
                                // int nendBit = nleftBit + iprj->getMaxRegNumPerChain() -1;
                                int nendBit = 0 ;
                                if(iregVec.at(nleftRegNum)->m_unRegNum > 1)
                                {
                                    if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                    {
                                        /*��:��*/
                                        // iregVec.at(nleftRegNum)->m_unBitWidth-1
                                        nendBit = nleftBit + 1 - iprj->getMaxRegNumPerChain() ;
                                        iregNameList << constructChainRegString(iregVec.at(nleftRegNum) , nlastStopNum , nleftBit , nendBit , pitem);

                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum - nlastStopNum)+tr("[%1:%2]").arg(nleftBit).arg(nendBit));

                                        nleftBit = nendBit - 1 ;
                                    }
                                    else
                                    {
                                        /*��:��*/
                                        nendBit = nleftBit -1 + iprj->getMaxRegNumPerChain() ;
                                        iregNameList << constructChainRegString(iregVec.at(nleftRegNum) , nlastStopNum , nleftBit , nendBit , pitem);

                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum + nlastStopNum)+tr("[%1:%2]").arg(nleftBit).arg(nendBit));

                                        nleftBit = nendBit + 1 ;
                                    }
                                }
                                else
                                {
                                    if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                    {
                                        /*��:��*/
                                        nendBit = nleftBit + 1 - iprj->getMaxRegNumPerChain() ;
                                        iregNameList << constructChainRegString(iregVec.at(nleftRegNum) , nlastStopNum , nleftBit , nendBit , pitem);

                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1:%2]") \
                                                                   .arg(nleftBit).arg(nendBit));
                                        nleftBit = nendBit - 1 ;
                                    }
                                    else
                                    {
                                        /*��:��*/
                                        nendBit = nleftBit -1 + iprj->getMaxRegNumPerChain() ;
                                        iregNameList << constructChainRegString(iregVec.at(nleftRegNum) , nlastStopNum , nleftBit , nendBit , pitem);

                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1:%2]")\
                                                                   .arg(nleftBit).arg(nendBit));

                                        nleftBit = nendBit + 1 ;
                                    }
                                }

                                ichainClock = pchain->getChainClock(pitem->getInstanceName(),iclockIterator.key());
                                if(ichainClock.isEmpty())
                                {
                                     // ichainClock = iclockIterator.key() ;
                                    return 2 ;
                                }

                                pchain->addToRegChain(ichainClock ,pchainSt->m_uncurrentChainNumber ,iregNameList);

                                QString iwireShiftRegEvaluateString ;
                                iwireShiftRegEvaluateString.append(tr("\n\t""assign %1 = {\n\t\t\t\t\t\t\t\t\t""%2""\n\t\t\t\t\t\t\t\t\t};").arg(iwireSrName).arg(iregCombinationCode.join(" ,\n\t\t\t\t\t\t\t\t\t")));
                                iaddedCodeList.append(tr("\\bassign\\s+%1.*;").arg(iwireSrName));
                                iusrCoreCode.append(iwireShiftRegEvaluateString);

                                /*�Զ��� core �������� */
                                QString iusrCoreDefinitionCode ;
                                //QString iresetName = "1'b1";
                                QString iresetName = tr("_EziDebug_%1_rstn").arg(pchain->getChainName());

                                QString iusrCoreTdi ;
                                if(nnumberOfNoLibCore != 0)
                                {
                                    if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                                    {
                                        iusrCoreTdi.append(tr("%1[%2]").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())).arg(pchainSt->m_uncurrentChainNumber));
                                    }
                                    else
                                    {
                                        iusrCoreTdi.append(tr("%1").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())));
                                    }
                                }
                                else
                                {
                                    if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                                    {
                                        iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg[%3]").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(pchainSt->m_uncurrentChainNumber));
                                    }
                                    else
                                    {
                                        iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg").arg(pchain->getChainName()).arg(iclockIterator.key()));
                                    }
                                }

                                iusrCoreDefinitionCode.append(tr("\n\t")+ EziDebugScanChain::getChainRegCore() + tr(" %1_%2_inst%3(\n").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));
                                iusrCoreDefinitionCode.append(  tr("\t"".clock""\t(%1) ,\n").arg(iclockIterator.key()) \
                                                                + tr("\t"".resetn""\t(%1) ,\n").arg(iresetName) \
                                                                + tr("\t"".TDI_reg""\t(%1) ,\n").arg(iusrCoreTdi) \
                                                                + tr("\t"".TDO_reg""\t(%1) ,\n").arg(iwireTdoName) \
                                                                + tr("\t"".TOUT_reg""\t(%1) ,\n").arg(iparentToutPort) \
                                                                + tr("\t"".shift_reg""\t(%1) \n\t) ;").arg(iwireSrName));

                                /*���� ���� userCore regWidth �޶��� ������*/
                                QString iparameterDefCode ;
                                iparameterDefCode.append(tr("\n\n\t""defparam %1_%2_inst%3.shift_width = %4 ;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum).arg(iprj->getMaxRegNumPerChain()));
                                iaddedCodeList.append(tr("\\bdefparam\\s+%1_%2_inst%3\\.shift_width\\s*=.*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));
                                iusrCoreCode.append(iparameterDefCode)  ;

                                iaddedCodeList.append(EziDebugScanChain::getChainRegCore() + tr("\\s+%1_%2_inst%3\\s*\\(.*\\)\\s*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));

                                iusrCoreCode.append(iusrCoreDefinitionCode);


                                /*module �˿����Ӵ���*/
                                QString iportConnectCode ;
                                iportConnectCode.append(tr("\n\t""assign %1[%2] = %3 ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                        .arg(pchainSt->m_uncurrentChainNumber).arg(iwireTdoName));
                                iaddedCodeList.append(tr("\\bassign\\s+%1\\s*\\[\\s*%2\\s*\\].*;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                      .arg(pchainSt->m_uncurrentChainNumber));
                                iusrCoreCode.append(iportConnectCode);


                                nlastLeftRegNum = nlastLeftRegNum - iprj->getMaxRegNumPerChain();
                                pchainSt->m_uncurrentChainNumber++ ;
                                pchainSt->m_unleftRegNumber = iprj->getMaxRegNumPerChain() ;
                            }
                        }

                        // ��ʣ��ļĴ������뵽����
                        iregCombinationCode.clear();
                        iregNameList.clear();
#if 1
                        if(iregVec.at(nleftRegNum)->m_eRegBitWidthEndian == EziDebugModule::endianBig)
                        {
                            if((nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) > 0)
                            {
                                pchainSt->m_unleftRegNumber -= (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                                nregBitCount = (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                                iregNameList << constructChainRegString(iregVec.at(nleftRegNum),nlastStopNum,nleftBit,iregVec.at(nleftRegNum)->m_unEndBit ,pitem);

                                iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1][%2:%3]").arg(iregVec.at(nleftRegNum)->m_unStartNum - nlastStopNum).arg(nleftBit).arg(iregVec.at(nleftRegNum)->m_unEndBit));
                            }
                            else if((nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) == 0)
                            {
                                pchainSt->m_unleftRegNumber -= (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                                nregBitCount = (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                                iregNameList << constructChainRegString(iregVec.at(nleftRegNum),nlastStopNum,nleftBit,iregVec.at(nleftRegNum)->m_unEndBit ,pitem);

                                iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1][0]").arg(iregVec.at(nleftRegNum)->m_unStartNum - nlastStopNum));
                            }
                            else
                            {
                                // �����Ѿ�������
                            }

                        }
                        else
                        {
                            if((iregVec.at(nleftRegNum)->m_unEndBit - nleftBit) > 0)
                            {
                                pchainSt->m_unleftRegNumber -= (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                                nregBitCount = (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                                iregNameList << constructChainRegString(iregVec.at(nleftRegNum),nlastStopNum,nleftBit,iregVec.at(nleftRegNum)->m_unEndBit ,pitem);

                                iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1][%2:%3]").arg(iregVec.at(nleftRegNum)->m_unStartNum + nlastStopNum).arg(nleftBit).arg(iregVec.at(nleftRegNum)->m_unEndBit));
                            }
                            else if((iregVec.at(nleftRegNum)->m_unEndBit - nleftBit) == 0)
                            {
                                pchainSt->m_unleftRegNumber -= (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                                nregBitCount = (qAbs(nleftBit - iregVec.at(nleftRegNum)->m_unEndBit) + 1) ;
                                iregNameList << constructChainRegString(iregVec.at(nleftRegNum),nlastStopNum,nleftBit,iregVec.at(nleftRegNum)->m_unEndBit ,pitem);

                                iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1][0]").arg(iregVec.at(nleftRegNum)->m_unStartNum + nlastStopNum));
                            }
                            else
                            {
                                // �Ѿ�����
                            }
                        }
#endif


                        if(iregVec.at(nleftRegNum)->m_unRegNum > 1)
                        {
                            nlastStopNum++ ;
                        }
                        else
                        {
                            nleftRegNum++ ;
                            nlastStopNum = 0 ;
                        }

                        nleftBit = -1 ;
                    }
                    else
                    {
                        nregBitCount = 0 ;
                    }


                    for(; nleftRegNum < iregVec.count() ; nleftRegNum++)
                    {
                        for(int m = nlastStopNum ; m < iregVec.at(nleftRegNum)->m_unRegNum; m++)
                        {
                            nregBitCount += iregVec.at(nleftRegNum)->m_unRegBitWidth ;

                            if(nregBitCount < pchainSt->m_unleftRegNumber)
                            {
                                /*���������*/
                                // ���ü������
                                if(iregVec.at(nleftRegNum)->m_unRegNum > 1)
                                {
                                    if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                    {
                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum - m));
                                    }
                                    else
                                    {
                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum + m));
                                    }
                                }
                                else
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName));
                                }

                                iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,iregVec.at(nleftRegNum)->m_unStartBit,iregVec.at(nleftRegNum)->m_unEndBit,pitem);
                            }
                            else if(nregBitCount == pchainSt->m_unleftRegNumber)
                            {
                                // �պ���  ����  �´��µ�ɨ�������� ���
                                if(iregVec.at(nleftRegNum)->m_unRegNum > 1)
                                {
                                    if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                    {
                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum - m));
                                    }
                                    else
                                    {
                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum + m));
                                    }
                                }
                                else
                                {
                                    iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName));
                                }

                                iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,iregVec.at(nleftRegNum)->m_unStartBit,iregVec.at(nleftRegNum)->m_unEndBit,pitem);

                                nlastStopNum = m ;
                                // nleftBit = -1 ;
                                nleftRegNum++ ;
                                isNeedAdded = true ;

                                goto WireShiftRegEvaluate1 ;
                            }
                            else
                            {
                                if(iregVec.at(nleftRegNum)->m_unRegNum > 1)
                                {
                                    if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                    {
                                        int nstartBit = iregVec.at(nleftRegNum)->m_unStartBit ;
                                        int nendBit = nregBitCount- pchainSt->m_unleftRegNumber + iregVec.at(nleftRegNum)->m_unStartBit + 1 - iregVec.at(nleftRegNum)->m_unRegBitWidth ;
                                        nleftBit = nendBit - 1 ;
                                        iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,nstartBit,nendBit,pitem);

                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum - m)+tr("[%1:%2]").arg(nstartBit).arg(nendBit));
                                    }
                                    else
                                    {
                                        int nstartBit = iregVec.at(nleftRegNum)->m_unStartBit ;
                                        int nendBit = iregVec.at(nleftRegNum)->m_unRegBitWidth + iregVec.at(nleftRegNum)->m_unStartBit - 1 - (nregBitCount- pchainSt->m_unleftRegNumber) ;
                                        nleftBit = nendBit + 1 ;
                                        iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,nstartBit,nendBit,pitem);

                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1]").arg(iregVec.at(nleftRegNum)->m_unStartNum + m)+tr("[%1:%2]").arg(nstartBit).arg(nendBit));
                                    }
                                }
                                else
                                {
                                    if(iregVec.at(nleftRegNum)->m_eRegNumEndian == EziDebugModule::endianBig)
                                    {
                                        int nstartBit = iregVec.at(nleftRegNum)->m_unStartBit ;
                                        int nendBit = nregBitCount- pchainSt->m_unleftRegNumber + iregVec.at(nleftRegNum)->m_unStartBit + 1 - iregVec.at(nleftRegNum)->m_unRegBitWidth ;
                                        nleftBit = nendBit - 1 ;
                                        iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,nstartBit,nendBit,pitem);

                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1:%2]") \
                                                                   .arg(nstartBit).arg(nendBit));
                                    }
                                    else
                                    {
                                        int nstartBit = iregVec.at(nleftRegNum)->m_unStartBit ;
                                        int nendBit = iregVec.at(nleftRegNum)->m_unRegBitWidth + iregVec.at(nleftRegNum)->m_unStartBit - 1 - (nregBitCount- pchainSt->m_unleftRegNumber) ;
                                        nleftBit = nendBit + 1 ;
                                        iregNameList << constructChainRegString(iregVec.at(nleftRegNum),m,nstartBit,nendBit,pitem);

                                        iregCombinationCode.append(QString::fromAscii(iregVec.at(nleftRegNum)->m_pRegName)+tr("[%1:%2]")\
                                                                   .arg(nstartBit).arg(nendBit));
                                    }
                                }

                                nlastStopNum = m ;
                                isNeedAdded = true ;
                                goto WireShiftRegEvaluate1 ;
                            }
                        }
                        nlastStopNum = 0 ;
                    }
WireShiftRegEvaluate1:

                    if(isNeedAdded == true)
                    {
                        nusedNum =  pchainSt->m_unleftRegNumber ;
                    }
                    else
                    {
                        nusedNum = nregBitCount ;
                    }

                    // û���������
                    if(nleftRegNum != iregVec.count())
                    {
                        // �мĴ���
                        if(iregNameList.count())
                        {

                            ichainClock = pchain->getChainClock(pitem->getInstanceName(),iclockIterator.key());

                            if(ichainClock.isEmpty())
                            {
                                return 2 ;
                            }

                            pchain->addToRegChain(ichainClock,pchainSt->m_uncurrentChainNumber,iregNameList);

                            nwireCount++ ;
                            // ���� wire ���� tdo ����
                            QString iwireTdoName(tr("_EziDebug_%1_%2_tdo%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));
                            QString iwireTdoDefinitionCode(tr("\n\n\t""wire ")+ iwireTdoName + tr(" ;")) ;
                            iusrCoreCode.append(iwireTdoDefinitionCode);
                            iaddedCodeList.append(tr("\\bwire\\s+_EziDebug_%1_%2_tdo%3\\s*;")\
                                                  .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));


                            // ���� wire ���� ��λ�Ĵ��� ����
                            QString iwireShiftRegDefinitionCode(tr("\n\t""wire ") + tr("[%1:0] ").arg(nusedNum - 1) + tr("_EziDebug_%1_%2_sr%3")\
                                                                .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum) + tr(" ;"));
                            QString iwireSrName(tr("_EziDebug_%1_%2_sr%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum)) ;

                            iaddedCodeList.append(tr("\\bwire\\s*\\[\\s*%1\\s*:\\s*0\\s*\\]\\s*").arg(nusedNum - 1) + tr("_EziDebug_%1_%2_sr%3\\s*;")\
                                                  .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum));

                            iusrCoreCode.append(iwireShiftRegDefinitionCode);
                            nwireShiftRegNum++ ;

                            //pchain->addToRegChain(ichainClock,pchainSt->m_uncurrentChainNumber,iregNameList);

                            QString iwireShiftRegEvaluateString ;
                            iwireShiftRegEvaluateString.append(tr("\n\t""assign %1 = {\n\t\t\t\t\t\t\t\t\t""%2""\n\t\t\t\t\t\t\t\t\t};").arg(iwireSrName).arg(iregCombinationCode.join(" ,\n\t\t\t\t\t\t\t\t\t")));
                            iaddedCodeList.append(tr("\\bassign\\s+%1\\s*=.*;").arg(iwireSrName));
                            iusrCoreCode.append(iwireShiftRegEvaluateString);


                            /*�Զ��� core �������� */
                            QString iusrCoreDefinitionCode ;
                            //QString iresetName = "1'b1";
                            QString iresetName = tr("_EziDebug_%1_rstn").arg(pchain->getChainName());


                            QString iusrCoreTdi ;
                            if(nnumberOfNoLibCore != 0)
                            {
                                if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                                {
                                    iusrCoreTdi.append(tr("%1[%2]").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())).arg(pchainSt->m_uncurrentChainNumber));
                                }
                                else
                                {
                                    iusrCoreTdi.append(tr("%1").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())));
                                }
                            }
                            else
                            {
                                if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                                {
                                    iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg[%3]").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(pchainSt->m_uncurrentChainNumber));
                                }
                                else
                                {
                                    iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg").arg(pchain->getChainName()).arg(iclockIterator.key()));
                                }
                            }

                            iusrCoreDefinitionCode.append(tr("\n\n\t")+EziDebugScanChain::getChainRegCore() + tr(" %1_%2_inst%3(\n").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));
                            iusrCoreDefinitionCode.append(  tr("\t\t"".clock""\t(%1) ,\n").arg(iclockIterator.key()) \
                                                            + tr("\t\t"".resetn""\t(%1) ,\n").arg(iresetName) \
                                                            + tr("\t\t"".TDI_reg""\t(%1) ,\n").arg(iusrCoreTdi) \
                                                            + tr("\t\t"".TDO_reg""\t(%1) ,\n").arg(iwireTdoName) \
                                                            + tr("\t\t"".TOUT_reg""\t(%1) ,\n").arg(iparentToutPort) \
                                                            + tr("\t\t"".shift_reg""\t(%1) \n\t) ;").arg(iwireSrName));


                            /*���� ���� userCore regWidth �޶��� ������*/
                            QString iparameterDefCode ;
                            iparameterDefCode.append(tr("\n\n\t""defparam %1_%2_inst%3.shift_width = %4 ;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum).arg(nusedNum));
                            iaddedCodeList.append(tr("\\bdefparam\\s+%1_%2_inst%3\\.shift_width\\s*=.*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));

                            iusrCoreCode.append(iparameterDefCode)  ;
                            iaddedCodeList.append(EziDebugScanChain::getChainRegCore() + tr("\\s+%1_%2_inst%3\\s*(.*)\\s*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));

                            iusrCoreCode.append(iusrCoreDefinitionCode);

                            /*module �˿����Ӵ���*/
                            QString iportConnectCode ;
                            if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                            {
                                iportConnectCode.append(tr("\n\n\t""assign %1[%2] = %3 ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                        .arg(pchainSt->m_uncurrentChainNumber).arg(iwireTdoName));
                                iaddedCodeList.append(tr("\\bassign\\s+%1\\s*\\[\\s*%2\\s*\\].*;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                      .arg(pchainSt->m_uncurrentChainNumber));
                            }
                            else
                            {
                                iportConnectCode.append(tr("\n\n\t""assign %1 = %2 ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                        .arg(iwireTdoName));
                                iaddedCodeList.append(tr("\\bassign\\s+%1\\s*=\\s*%2\\s*;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                      .arg(iwireTdoName));
                            }
                            iusrCoreCode.append(iportConnectCode);
                        }

                        pchainSt->m_uncurrentChainNumber++ ;
                        pchainSt->m_unleftRegNumber = iprj->getMaxRegNumPerChain() ;
                        isNeedAdded = false ;
                        goto AddReg ;
                    }
                    else
                    {
                        pchainSt->m_unleftRegNumber -= nregBitCount ;
                    }
#if 0
                    if(!sregVec.count())
                    {
                        pchainSt->m_uncurrentChainNumber++ ;
                        pchainSt->m_unleftRegNumber = iprj->getMaxRegNumPerChain() ;
                    }
#endif

                    // ��Ӷ�̬�Ĵ���
                    QString ivarRegSumStr ;
                    int nregNumCount = 0 ;
                    nleftRegNum = 0 ;

AddVReg:
                    if(nregBitCount != 0)
                    {
                        ivarRegSumStr = QString::number(nregBitCount) ;
                    }
                    else
                    {
                        ivarRegSumStr.clear();
                        iregCombinationCode.clear();
                    }

                    for(;nleftRegNum < vregVec.count();nleftRegNum++)
                    {
                        // ��Ҫ���� regnum  ��¼��ӵ��� �ĸ� �Ĵ���
                        EziDebugModule::RegStructure* preg = vregVec.at(nleftRegNum) ;
                        EziDebugModule::RegStructure* pinstancereg = pmodule->getInstanceReg(pitem->getInstanceName(),iclockIterator.key(),QString::fromAscii(preg->m_pRegName));

                        if(preg->m_unMaxBitWidth < pchainSt->m_unleftRegNumber)
                        {
                            pchainSt->m_unleftRegNumber -= preg->m_unMaxBitWidth ;

                            if(pinstancereg->m_unRegNum == 1)
                            {
                                if(!ivarRegSumStr.isEmpty())
                                {
                                    ivarRegSumStr.append(QObject::tr("+") + QString::fromAscii(preg->m_pExpString));
                                }
                                else
                                {
                                    ivarRegSumStr.append(QString::fromAscii(preg->m_pExpString));
                                }

                                iregNameList << constructChainRegString(pinstancereg , 0 ,pinstancereg->m_unStartBit , pinstancereg->m_unEndBit , pitem);

                                iregCombinationCode.append(QString::fromAscii(preg->m_pRegName));
                            }
                            else if(pinstancereg->m_unRegNum > 1)
                            {
                                if(!ivarRegSumStr.isEmpty())
                                {
                                    ivarRegSumStr.append(QObject::tr("+") + QString::fromAscii(preg->m_pExpString) + QObject::tr("*") + QString::number(pinstancereg->m_unRegNum));
                                }
                                else
                                {
                                    ivarRegSumStr.append(QString::fromAscii(preg->m_pExpString) + QObject::tr("*") + QString::number(pinstancereg->m_unRegNum));
                                }

                                for( ; nregNumCount < pinstancereg->m_unRegNum ; nregNumCount++ )
                                {
                                    QString iregNumStr ;
                                    if(pinstancereg->m_eRegNumEndian == EziDebugModule::endianBig)
                                    {
                                        iregNumStr = QString::number(pinstancereg->m_unStartNum - nregNumCount) ;
                                    }
                                    else
                                    {
                                        iregNumStr = QString::number(pinstancereg->m_unStartNum + nregNumCount) ;
                                    }
                                    iregNameList << constructChainRegString(pinstancereg , nregNumCount ,pinstancereg->m_unStartBit , pinstancereg->m_unEndBit , pitem);

                                    iregCombinationCode.append(QObject::tr("%1[%2]").arg(QString::fromAscii(pinstancereg->m_pRegName)).arg(iregNumStr)) ;
                                }
                            }
                        }
                        else
                        {
                            // ���������
                            break ;
                        }
                    }

                    // ���ɨ����
                    ichainClock = pchain->getChainClock(pitem->getInstanceName(),iclockIterator.key());
                    if(ichainClock.isEmpty())
                    {
                        ichainClock = iclockIterator.key() ;
                    }

                    pchain->addToRegChain(ichainClock ,pchainSt->m_uncurrentChainNumber ,iregNameList);


                    nwireCount++ ;
                    // ���� wire ���� tdo ����
                    QString iwireTdoName(tr("_EziDebug_%1_%2_tdo%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));
                    QString iwireTdoDefinitionCode(tr("\n\n\t""wire ")+ iwireTdoName + tr(" ;")) ;
                    iusrCoreCode.append(iwireTdoDefinitionCode);
                    iaddedCodeList.append(tr("\\bwire\\s+_EziDebug_%1_%2_tdo%3\\s*;")\
                                          .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireCount));


                    // ���� wire ���� ��λ�Ĵ��� ����
                    QString iwireShiftRegDefinitionCode(tr("\n\t""wire ") + tr("[%1:0] ").arg(ivarRegSumStr + tr(" - 1")) + tr("_EziDebug_%1_%2_sr%3")\
                                                        .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum) + tr(" ;"));
                    QString iwireSrName(tr("_EziDebug_%1_%2_sr%3").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum)) ;

                    iaddedCodeList.append(tr("\\bwire\\s*\\[\\s*%1\\s*:\\s*0\\s*\\]\\s*").arg(ivarRegSumStr + tr(" - 1")) + tr("_EziDebug_%1_%2_sr%3\\s*;")\
                                          .arg(pchain->getChainName()).arg(iclockIterator.key()).arg(nwireShiftRegNum));

                    iusrCoreCode.append(iwireShiftRegDefinitionCode);
                    nwireShiftRegNum++ ;

                    QString iwireShiftRegEvaluateString ;
                    iwireShiftRegEvaluateString.append(tr("\n\t""assign %1 = {\n\t\t\t\t\t\t\t\t\t""%2""\n\t\t\t\t\t\t\t\t\t};").arg(iwireSrName).arg(iregCombinationCode.join(" ,\n\t\t\t\t\t\t\t\t\t")));
                    iaddedCodeList.append(tr("\\bassign\\s+%1.*;").arg(iwireSrName));
                    iusrCoreCode.append(iwireShiftRegEvaluateString);

                    /*�Զ��� core �������� */
                    QString iusrCoreDefinitionCode ;
                    //QString iresetName ;
                    QString iresetName = tr("_EziDebug_%1_rstn").arg(pchain->getChainName());

                    QString iusrCoreTdi ;
                    if(nnumberOfNoLibCore != 0)
                    {
                        if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                        {
                            iusrCoreTdi.append(tr("%1[%2]").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())).arg(pchainSt->m_uncurrentChainNumber));
                        }
                        else
                        {
                            iusrCoreTdi.append(tr("%1").arg(pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key())));
                        }
                    }
                    else
                    {
                        if((chainStructuremap.value(iclockIterator.key())->m_untotalChainNumber) > 1)
                        {
                            iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg[%3]").arg(pchain->getChainName()).arg(iclockIterator.key()).arg(pchainSt->m_uncurrentChainNumber));
                        }
                        else
                        {
                            iusrCoreTdi.append(tr("_EziDebug_%1_%2_TDI_reg").arg(pchain->getChainName()).arg(iclockIterator.key()));
                        }
                    }

                    iusrCoreDefinitionCode.append(tr("\n\t")+ EziDebugScanChain::getChainRegCore() + tr(" %1_%2_inst%3(\n").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));
                    iusrCoreDefinitionCode.append(  tr("\t"".clock""\t(%1) ,\n").arg(iclockIterator.key()) \
                                                    + tr("\t"".resetn""\t(%1) ,\n").arg(iresetName) \
                                                    + tr("\t"".TDI_reg""\t(%1) ,\n").arg(iusrCoreTdi) \
                                                    + tr("\t"".TDO_reg""\t(%1) ,\n").arg(iwireTdoName) \
                                                    + tr("\t"".TOUT_reg""\t(%1) ,\n").arg(iparentToutPort) \
                                                    + tr("\t"".shift_reg""\t(%1) \n\t) ;").arg(iwireSrName));

                    /*���� ���� userCore regWidth �޶��� ������*/
                    QString iparameterDefCode ;
                    iparameterDefCode.append(tr("\n\n\t""defparam %1_%2_inst%3.shift_width = %4 ;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum).arg(ivarRegSumStr));
                    iaddedCodeList.append(tr("\\bdefparam\\s+%1_%2_inst%3\\.shift_width\\s*=.*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));
                    iusrCoreCode.append(iparameterDefCode)  ;

                    iaddedCodeList.append(EziDebugScanChain::getChainRegCore() + tr("\\s+%1_%2_inst%3\\s*\\(.*\\)\\s*;").arg(EziDebugScanChain::getChainRegCore()).arg(pchain->getChainName()).arg(ninstNum));

                    iusrCoreCode.append(iusrCoreDefinitionCode);


                    /*module �˿����Ӵ���*/
                    QString iportConnectCode ;
                    iportConnectCode.append(tr("\n\t""assign %1[%2] = %3 ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                            .arg(pchainSt->m_uncurrentChainNumber).arg(iwireTdoName));
                    iaddedCodeList.append(tr("\\bassign\\s+%1\\s*\\[\\s*%2\\s*\\].*;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                          .arg(pchainSt->m_uncurrentChainNumber));
                    iusrCoreCode.append(iportConnectCode);

                    ivarRegSumStr.clear();

                    if(iregNameList.count())
                    {
                        // ����һ���ֵļĴ��� ���һ�ε� regbitcount ��¼����
                        // ת��Ϊ QString ����

                        if(nleftRegNum != vregVec.count())
                        {
                            // ���мĴ���
                            pchainSt->m_uncurrentChainNumber++ ;
                            pchainSt->m_unleftRegNumber = iprj->getMaxRegNumPerChain() ;
                            goto AddVReg ;
                        }
                    }
                }

            }
            // ���� Ҫ���� ��Ŀ ���� ���� ,������ wire_tdo �ź� ����λ�Ĵ��� ��
            if(fileName().endsWith("fft_ram_256x17.v"))
            {
                qDebug("add chain in fft_ram_256x17.v");
            }
            QString ilastPortConnect ;
            nchainEndNum = pchainSt->m_uncurrentChainNumber ;

            // ��һ�������� �� ��һ������ʼ �� �˿�����
            ilastInput.clear();
            if(nnumberOfNoLibCore != 0)
            {
                // �� lastwire
                ilastInput = pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key()) ;
            }
            else
            {
                // �� tdi
                ilastInput = tr("_EziDebug_%1_%2_TDI_reg").arg(pchain->getChainName()).arg(iclockIterator.key()) ;
            }

            // ����ͬһ�����ϣ��Ҳ��Ǵ�0������ʼ��
            if((nlastChainEndNum != nchainStartNum)&&(nchainStartNum != 0))
            {
                int nstartBit = 0 ;

                nstartBit = nlastChainEndNum + 1;


                if((nchainStartNum - nlastChainEndNum) > 2)
                {
                    ilastPortConnect.append(tr("\n\t""assign %1[%2:%3] = %4[%5:%6] ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                            .arg(nchainStartNum-1).arg(nstartBit).arg(ilastInput).arg(nchainStartNum-1).arg(nstartBit));
                    iusrCoreCode.append(ilastPortConnect);
                    iaddedCodeList.append(tr("\\bassign\\s+%1\\s*\\[\\s*%2\\s*:\\s*%3\\s*\\]\\s*=\\s*%4\\[\\s*%5\\s*:\\s*%6\\s*\\]\\s*;")
                                          .arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                                                      .arg(nchainStartNum-1).arg(nstartBit).arg(ilastInput).arg(nchainStartNum-1).arg(nstartBit));
                }
                else
                {
                    ilastPortConnect.append(tr("\n\t""assign %1[%2] = %3[%4] ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                            .arg(nstartBit)\
                                            .arg(ilastInput)\
                                            .arg(nstartBit));
                    iusrCoreCode.append(ilastPortConnect);
                    iaddedCodeList.append(tr("\\bassign\\s+%1\\s*\\[\\s*%2\\s*\\]\\s*=\\s*%3\\s*\\[\\s*%4\\s*\\]\\s*;")\
                                          .arg(tr("_EziDebug_%1_%2_TDO_reg")\
                                          .arg(pchain->getChainName())\
                                          .arg(iclockIterator.key()))\
                                          .arg(nstartBit)\
                                          .arg(ilastInput)\
                                          .arg(nstartBit));
                }
            }

            pmodule->setBitRangeInChain(pchain->getChainName(),iclockIterator.key(),nchainStartNum ,nchainEndNum);
            pmodule->setEziDebugCoreCounts(pchain->getChainName(),ninstNum);
            ++iclockIterator ;
        }

            pmodule->setEziDebugWireCounts(pchain->getChainName(),nwireCount);

        /*
                �Զ���core ����������
                1��  _EziDebugScanChainReg + ������(
                    .clock      (module�еĸ���clock)  ,
                    .resetn     (module�еĸ�λ�ź�)  ,
                    .TDI_reg    (��ϵͳcore �� ���һ�� wire_tdo)  ,
                    .TDO_reg    (�Զ���� wire_tdo  )  ,
                    .TOUT_reg   (module�˿ڵ� tout  )  ,
                    .shift_reg  (�Զ���� shift_reg )
                 );
            */
        int ntimesPerChain = pmodule->getInstancedTimesPerChain(pchain->getChainName()) ;
        if(1 == ntimesPerChain)
        {
            iclockIterator = iclockMap.constBegin();
            while(iclockIterator != iclockMap.constEnd())
            {
                EziDebugInstanceTreeItem::SCAN_CHAIN_STRUCTURE* pchainSt = chainStructuremap.value(iclockIterator.key()) ;
                QString ilastInput ;
                QString ilastPortConnect ;
                if(nnumberOfNoLibCore != 0)
                {
                    // �� lastwire
                    ilastInput = pmodule->getChainClockWireNameMap(pchain->getChainName(),iclockIterator.key()) ;

                }
                else
                {
                    // �� tdi
                    ilastInput = tr("_EziDebug_%1_%2_TDI_reg").arg(pchain->getChainName()).arg(iclockIterator.key()) ;
                }

                // ������1bit wire
                pmodule->getBitRangeInChain(pchain->getChainName(),iclockIterator.key(),&nlastChainStartNum ,&nlastChainEndNum);

                int nbitNum = 0 ;

                // �˿����Ӳ���ȫ
                if(nlastChainEndNum != (pchainSt->m_untotalChainNumber-1))
                {
                    if((pchainSt->m_untotalChainNumber - nlastChainEndNum) > 2)
                    {
                        if( -1 == nlastChainEndNum)
                        {
                            // û�ӹ��Զ���core
                            nbitNum  = 0 ;
                        }
                        else
                        {
                            nbitNum = nlastChainEndNum + 1 ;
                        }

                        ilastPortConnect.append(tr("\n\t""assign %1[%2:%3] = %4[%5:%6] ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                .arg(pchainSt->m_untotalChainNumber-1).arg(nbitNum).arg(ilastInput).arg(pchainSt->m_untotalChainNumber-1).arg(nbitNum));
                        iusrCoreCode.append(ilastPortConnect);
                        iaddedCodeList.append(tr("\\bassign\\s+")+tr("%1\\s*\\[\\s*%2\\s*:\\s*%3\\s*\\]\\s*=\\s*%4\\s*\\[\\s*%5\\s*:\\s*%6\\s*\\]\\s*;") \
                                              .arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                              .arg(pchainSt->m_untotalChainNumber-1)\
                                              .arg(nbitNum).arg(ilastInput).arg(pchainSt->m_untotalChainNumber-1).arg(nbitNum)
                                              );
                    }
                    else
                    {

                        if(nlastChainEndNum == -1) // module�޼Ĵ��� �޲���������  ֱ��͸��
                        {
                            ilastPortConnect.append(tr("\n\t""assign %1 = %2 ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                    .arg(ilastInput));
                            iusrCoreCode.append(ilastPortConnect);
                            iaddedCodeList.append(tr("\\bassign\\s+")+tr("%1\\s*=\\s*%2\\s*;") \
                                                  .arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                  .arg(ilastInput));
                        }
                        else
                        {
                            ilastPortConnect.append(tr("\n\t""assign %1[%2] = %3[%4] ;").arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key())).arg(pchainSt->m_untotalChainNumber-1)\
                                                    .arg(ilastInput).arg(pchainSt->m_untotalChainNumber-1));
                            iusrCoreCode.append(ilastPortConnect);
                            iaddedCodeList.append(tr("\\bassign\\s+")+tr("%1\\s*\\[\\s*%2\\s*\\]\\s*=\\s*%3\\s*\\[\\s*%4\\s*\\]\\s*;") \
                                                  .arg(tr("_EziDebug_%1_%2_TDO_reg").arg(pchain->getChainName()).arg(iclockIterator.key()))\
                                                  .arg(pchainSt->m_untotalChainNumber-1)\
                                                  .arg(ilastInput).arg(pchainSt->m_untotalChainNumber-1)
                                                  );
                        }
                    }
                }

                ++iclockIterator ;
            }
        }
        else
        {
            ntimesPerChain-- ;
            pmodule->setInstancedTimesPerChain(pchain->getChainName(),ntimesPerChain);
        }

        pchain->addToLineCodeMap(pmodule->getModuleName(),iaddedCodeList);
        pchain->addToBlockCodeMap(pmodule->getModuleName(),iaddedBlockCodeList);
        /*���뵽�ַ�����*/

        ifileData.insert(imodulePos.m_nendModuleKeyWordPos + noffSet ,iusrCoreCode);

        if(!open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
        {
            qDebug() << "Cannot Open file for writing:" << qPrintable(this->errorString());
            return 1 ;
        }

        QTextStream iout(this);
        /*д���ļ� */
        iout <<  ifileData ;

        // ��¼�Ѿ������� һϵ�� �û� core ����
        pmodule->setAddCodeFlag(true);      

        close();
        ilastModifedTime = ifileInfo.lastModified() ;
        modifyStoredTime(ilastModifedTime);
        return 0 ;
    }
    else
    {
        goto ErrorHandle ;
    }


ErrorHandle:

    /*�ر��ļ�*/
    close();
    return 1 ;

}

int  EziDebugVlgFile::skipCommentaryFind(const QString &rangestring,int startpos ,SEARCH_STRING_STRUCTURE &stringtype,int &targetpos)
{
    int ncommentaryEnd = startpos ;
    int isNoCommentary = 0 ;

    int  nfindPosOther = 0 ;
    int  nfindPos = 0 ;
    int  noffset = 0 ;
    int  nleftBracketPos = 0 ;
    int  nstartPos = 0 ;

    QString imoduleName(tr("No Name")) ;
    QString ikeyWord(tr("No Name"));
    QRegExp ifindExp(tr(" ")) ;
    QRegExp ifindExpOther(tr(" ")) ;


    QString ipartString ;

    if(stringtype.m_etype == SearchModuleKeyWordPos)
    {
        ikeyWord = tr("module");
        imoduleName = QString::fromAscii(stringtype.m_icontent.m_imodulest.m_amoduleName);
        ifindExpOther.setPattern(tr("\\b")+ikeyWord+tr("\\b"));
        ifindExp.setPattern(tr("\\b")+ikeyWord+(tr("\\s+"))+imoduleName);
    }
    else if(stringtype.m_etype == SearchLeftBracketPos)
    {
        ifindExp.setPattern(tr("\\("));
    }
    else if(stringtype.m_etype == SearchSemicolonPos)
    {
        ifindExp.setPattern(tr(";"));
    }
    else
    {
        return 1 ;
    }

    while(1)
    {
        nstartPos = ncommentaryEnd ;
        ipartString = getNoCommentaryString(rangestring,ncommentaryEnd,isNoCommentary) ;
        /*��װ��1������ ���ڵõ�   ����Ϊ ��һ��ע�͵Ľ�������һ��ע�͵Ŀ�ʼ  ���Ϊ һ����ע�͵��ַ���*/
        /* ��ɨ�� module + module �� �������Ӧλ�� */
        /*1�����û��ɨ��  ֱ���ַ������� ��������Ҳû�� �ͷ��ش��� ������ɹ��ͼ�����һ�� */
        // while ѭ��
        /*2������ɨ������ reg��port��wire��endmodule��instance*/

        /*3������ƥ��ľ� ������Ӧ���ַ�����λ�� ���� ���� "reg"  �ؼ���ʱ����reg ��һ���ַ� ��� ��ע���ַ���
          ���� �����ŵ� ; ������ֱ��  ���1����ע�ʹ��� Ҳû���� �˳������ش���
         �������� ��ƥ�䵽���ַ������� ���µõ� ��һ����ע���ַ���
        Ȼ�����ɨ���¸�����ƥ����ַ���ֱ������endmode �ؼ��� ���˳� ���� ���1����ע�ʹ��� ƥ�����֮���˳�
        ������ƥ�� instance ʱ �ֱ���������ʽ������ƥ�䣬���ƥ�䵽 instance �� �˳�����Ϊ���1����ע�ʹ���
        */

FindString:

        if(SearchModuleKeyWordPos == stringtype.m_etype)
        {
            nfindPosOther = ipartString.indexOf(ifindExpOther) ;
            nfindPos = ipartString.indexOf(ifindExp) ;
            noffset = nstartPos + nfindPos + ifindExp.capturedTexts().at(0).count() ;


            if(NO_STRING_FINDED == nfindPos)
            {
                if(NO_STRING_FINDED == nfindPosOther)
                {
                    /*����ע������²��ҵ�*/
                    if(NO_COMMENTARY == isNoCommentary)
                    {
                        return 1 ;
                    }
//                    isNeededFindFlag = true ;
                    /*������һ�ֲ���*/
                    continue ;
                }
                else
                {
                    noffset = nstartPos + nfindPosOther + ifindExpOther.capturedTexts().count();

                    struct SEARCH_STRING_STRUCTURE inextFindSemicolon ;
                    inextFindSemicolon.m_etype =  SearchSemicolonPos ;
                    inextFindSemicolon.m_icontent.m_nreserved = 0 ;
                    int nSemicolonPos = 0 ;
                    /*���� ��һ����Ч�� ";" �ַ� */
                    if(!skipCommentaryFind(rangestring,noffset,inextFindSemicolon,nSemicolonPos))
                    {
                        /*��ȡ�� module ���ֳ��ֵ�λ��  �� ;���е��ַ�*/
                        QString itruncateString  = rangestring.mid(nfindPosOther,nSemicolonPos-nfindPosOther-1);
                        PORT_ANNOUNCE_FORMAT iportAnnounceformat = NonAnsicFormat ;
                        int  nrelativeRightBracketPos = 0 ;
                        int  nresult = 0 ;
                        /*�ж�����ַ��Ƿ� ƥ�� module + module��  ���ж��Ƿ�Ϊ�淶��port����*/
                        nresult =isModuleDefinition(itruncateString,imoduleName,iportAnnounceformat,nrelativeRightBracketPos) ;
                        if(!nresult)
                        {
                            // ������Ӧ��λ��  �Լ�  ������Ϣ
                            /*�ҵ� module */
                            stringtype.m_icontent.m_imodulest.m_eportAnnounceFormat = iportAnnounceformat ;
                            /*���� ")" ����λ��*/
                            targetpos = nrelativeRightBracketPos + nfindPosOther ;
                            int nlastNoblankChar = rangestring.lastIndexOf(QRegExp("\\S"),(targetpos-1));
                            targetpos = nlastNoblankChar +1 ;
                            return 0 ;
                        }
                        else if(1 == nresult)
                        {
//                          isNeededFindFlag = true ;
                            /*������һ�ֲ���*/
                            continue;
                        }
                        else
                        {
                           return 1 ;
                        }

                    }
                    else
                    {
                        return 1 ;
                    }
                }
            }// if(NO_STRING_FINDED == nfindPos)

            SEARCH_STRING_STRUCTURE inextFind ;
            inextFind.m_etype = SearchLeftBracketPos ;
            inextFind.m_icontent.m_nreserved = 0 ;

            int nleftBracketPos = 0 ;
            if(!skipCommentaryFind(rangestring,noffset,inextFind,nleftBracketPos))
            {
                /*���� ��֮��Ӧ�� ��һ�� ")"*/
                struct  SEARCH_MODULE_STRUCTURE imoduleSt ;
                imoduleSt.m_eportAnnounceFormat = NonAnsicFormat ;
                strcpy(imoduleSt.m_amoduleName,imoduleName.toAscii().data());
                struct  SEARCH_STRING_STRUCTURE inextFindRightBracket ;
                inextFindRightBracket.m_etype = SearchModuleKeyWordPos ;
                inextFindRightBracket.m_icontent.m_imodulest = imoduleSt ;
                int nrightBracketPos = noffset ;
                qDebug() << rangestring.mid(nleftBracketPos+1,100);
                if(!findOppositeBracket(rangestring,nleftBracketPos+1,inextFindRightBracket,nrightBracketPos))
                {
                    targetpos = nrightBracketPos ;
                    int nlastNoblankChar = rangestring.lastIndexOf(QRegExp("\\S"),(targetpos-1));
                    targetpos = nlastNoblankChar + 1 ;
                    /*���ŷ��� module �˿ڵ���д���� �Ƿ�Ϊ��׼ */
                    stringtype.m_icontent.m_imodulest.m_eportAnnounceFormat =  \
                            inextFindRightBracket.m_icontent.m_imodulest.m_eportAnnounceFormat ;
                    // ������Ӧ��λ��  �Լ�  ������Ϣ
                   return 0 ;
                }
                else
                {
                    return 1 ;
                }
            }
            else
            {
                return 1 ;
            }
        } //if(SearchModuleKeyWordPos == stringtype.m_etype)
        else if(stringtype.m_etype == SearchLeftBracketPos)
        {
            nleftBracketPos = ipartString.indexOf(ifindExp) ;

            if(NO_STRING_FINDED == nleftBracketPos)
            {
                /*���ַ�����ע������²��ҵ�*/
                if(NO_COMMENTARY == isNoCommentary)
                {
                    return 1 ;
                }
                continue ;
            }
            else
            {
                nleftBracketPos += nstartPos ;
                targetpos = nleftBracketPos ;
                return 0 ;
            }
        }
        else if(stringtype.m_etype == SearchSemicolonPos)
        {
            int nBackToBackSemicolonPos = 0 ;
            nBackToBackSemicolonPos = ipartString.indexOf(ifindExp) ;
            if(NO_STRING_FINDED == nBackToBackSemicolonPos)
            {
                /*���ַ�����ע������²��ҵ�*/
                if(NO_COMMENTARY == isNoCommentary)
                {
                    return 1 ;
                }
                continue ;
            }
            else
            {
                nBackToBackSemicolonPos += nstartPos ;
                targetpos = nBackToBackSemicolonPos ;
                return 0 ;
            }
        }
        else if(stringtype.m_etype == SearchRightBracketPos)
        {
            // do nothing
        }
        else
        {
            return 1 ;
        }

        if(1 == isNoCommentary)
        {
            goto FindString ;
        }

      } // while(1)


      return 0 ;
    }

int EziDebugVlgFile::matchingTargetString(const QString &rangestring ,SEARCH_MODULE_POS_STRUCTURE &modulepos ,QMap<QString,SEARCH_INSTANCE_POS_STRUCTURE*> &instanceposmap)
{
    // ������� ��ע�͵��ַ���
    /*port(output input)*/
    QString ipartString ;
    QString imoduleName ;
    QString iinstanceName ;

    int ncommentaryEnd = modulepos.m_nnextRightBracketPos + 1;
    int isNoCommentary = 0 ;
    int nlastRegtKeyWordPos = -1 ;
    int nlastWireKeyWordPos = -1 ;
    int nendmoduleKeyWordPos = -1 ;

    int  nfindPosOther = 0 ;
    int  nfindPos = 0 ;
    int  noffset = 0 ;
    int  nsaveStartPos = 0 ;
    int  nendInsertPos = 0 ;
    int  nendLastChar = 0 ;
    int  nannounceInsertPos  = 0 ;

    bool ismoduleEnd = false ;
    QRegExp ifindOutputPortExp(tr("\\b") + tr("output") +tr("\\b")) ;
    QRegExp ifindInputPortPortExpOther(tr("\\b") + tr("input") +tr("\\b")) ;
    /*reg*/
    QRegExp ifindRegExp(tr("\\b") + tr("reg") +tr("\\b"));
    /*wire*/
    QRegExp ifindWireExp(tr("\\b") + tr("wire") +tr("\\b"));
    /*endmodule*/
    QRegExp ifindEndmoduleExp(tr("\\b") + tr("endmodule") +tr("\\b"));

    QRegExp ifindInstanceExp(tr(" "));
    QRegExp ifindInstanceExpOther(tr(" "));


    while(1)
    {
        nsaveStartPos = ncommentaryEnd ;
        ipartString = getNoCommentaryString(rangestring,ncommentaryEnd,isNoCommentary) ;

        nendmoduleKeyWordPos = ipartString.indexOf(ifindEndmoduleExp);
        if(nendmoduleKeyWordPos != -1)
        {
            /*������Ӧλ��*/
            // endmodule �ؼ��ֲ���Ϊ ��ͷ�ַ���,����� ������
            if(nendmoduleKeyWordPos)
            {
                if(ipartString.lastIndexOf(QRegExp(tr("\\S")),nendmoduleKeyWordPos -1) != -1)
                {
                    nendInsertPos = nsaveStartPos + ipartString.lastIndexOf(QRegExp(tr("\\S")),nendmoduleKeyWordPos -1) ;
                }
            }

            modulepos.m_nendModuleKeyWordPos = nendInsertPos + 1;

            ismoduleEnd = true ;
        }
        else
        {
            if(-1 != ipartString.lastIndexOf(QRegExp(tr("\\S"))))
            {
                qDebug() << ipartString ;
                nendInsertPos = nsaveStartPos + ipartString.lastIndexOf(QRegExp(tr("\\S"))) ;
            }
        }

        // �Ǳ�׼�����ɨ�� �����˿�λ��
        if(NonAnsicFormat == modulepos.m_eportFormat)
        {
            if(ipartString.contains(ifindOutputPortExp)||ipartString.contains(ifindInputPortPortExpOther))
            {
                int nlastPortKeyWordPos = 0 ;
                int noutputKeyWordPos = ipartString.lastIndexOf(ifindOutputPortExp,nendmoduleKeyWordPos) ;
                int ninputKeyWordPos = ipartString.lastIndexOf(ifindInputPortPortExpOther,nendmoduleKeyWordPos) ;


                if(-1 == ninputKeyWordPos)
                {
                    nlastPortKeyWordPos = nsaveStartPos + noutputKeyWordPos ;
                }
                else if(-1 == noutputKeyWordPos)
                {
                    nlastPortKeyWordPos = nsaveStartPos + ninputKeyWordPos ;
                }
                else
                {
                    if(noutputKeyWordPos > ninputKeyWordPos)
                    {
                        nlastPortKeyWordPos = nsaveStartPos + noutputKeyWordPos ;
                    }
                    else
                    {
                        nlastPortKeyWordPos = nsaveStartPos + ninputKeyWordPos ;
                    }
                }

                /*������Ӧλ��*/
                /*�ҵ� �����ŵ� ";" */
                struct SEARCH_STRING_STRUCTURE inextFind ;
                inextFind.m_etype = SearchSemicolonPos ;
                inextFind.m_icontent.m_nreserved = 0 ;
                int  nsemicolonPos = 0 ;
                if(!skipCommentaryFind(rangestring , nlastPortKeyWordPos , inextFind , nsemicolonPos))
                {
                    // Ȼ�󱣴� ������һ���ж�
                    modulepos.m_nlastPortKeyWordPos = nsemicolonPos + 1;
                }
                else
                {
                    return 1 ;
                }
            }
        }


        QMap<QString,SEARCH_INSTANCE_POS_STRUCTURE*>::iterator i = instanceposmap.begin();
        while (i != instanceposmap.end())
        {
            struct SEARCH_INSTANCE_POS_STRUCTURE *pinstance = i.value() ;
            imoduleName = QString::fromAscii(pinstance->m_amoduleName);
            iinstanceName = QString::fromAscii(pinstance->m_ainstanceName) ;
            ifindInstanceExp.setPattern(tr("\\b")+ imoduleName + tr("\\s+") + iinstanceName +tr("\\b"));
            ifindInstanceExpOther.setPattern(tr("\\b")+imoduleName+tr("\\b"));

            /*���� ����ģ�� ���ֵ�λ��*/
            /*�ж����������� �����ؽ����ŵ����һ�γ��� "(" ���Ӧ�� ")" ��λ�� */
            nfindPosOther = ipartString.indexOf(ifindInstanceExpOther) ;
            nfindPos = ipartString.indexOf(ifindInstanceExp) ;
            noffset = nsaveStartPos + nfindPos + ifindInstanceExp.capturedTexts().at(0).count() ;

            if(NO_STRING_FINDED == nfindPos)
            {
                if(NO_STRING_FINDED == nfindPosOther)
                {
                    /*����ע������²��ҵ�*/
                    if(NO_COMMENTARY == isNoCommentary)
                    {
                        return 1 ;
                    }
                    /*������һ�ֲ��� 1 �� ���� */
                    i++ ;
                    continue ;
                }
                else
                {
                    noffset = nsaveStartPos + nfindPosOther + ifindInstanceExpOther.capturedTexts().count() ;
                    /*���ҽ����ŵ������� */
                    /*�����������Ŷ�Ӧ�� ������ */
                    /*�����ҵ���λ�� ��������� ����*/
                    struct SEARCH_STRING_STRUCTURE inextFindSemicolon ;
                    inextFindSemicolon.m_etype = SearchSemicolonPos ;
                    inextFindSemicolon.m_icontent.m_nreserved = 0 ;

                    int nrightSemicolonPos = 0 ;
                    /*���� ��һ����Ч�� ";" �ַ� */
                    if(!skipCommentaryFind(rangestring,noffset,inextFindSemicolon,nrightSemicolonPos))
                    {
                        /*��ȡ�� module ���ֳ��ֵ�λ��  �� ;���е��ַ�*/
                        QString itruncateString  = rangestring.mid(nfindPosOther,nrightSemicolonPos-nfindPosOther +1);
                        INSTANCE_FORMAT iinstance_format = NonStardardFormat ;
                        int nresult = 0 ;
                        int nrelativeRightBracketPos = 0 ;
                        nresult = isModuleInstance(itruncateString,imoduleName,iinstanceName,iinstance_format,nrelativeRightBracketPos) ;

                        if(rangestring.lastIndexOf(QRegExp(tr("\\S")),(nsaveStartPos + nfindPosOther -1)) != -1)
                        {
                            nannounceInsertPos = rangestring.lastIndexOf(QRegExp(tr("\\S")),(nsaveStartPos + nfindPosOther -1)) ;
                        }

                        /*�ж�����ַ��Ƿ� ƥ�� module��+������  ���ж��Ƿ�Ϊ�淶�Ķ˿�����*/
                        if(!nresult)
                        {
                            /*�ҵ� module ������Ϣ*/
                            pinstance->m_nnextRightBracketPos = nrelativeRightBracketPos +  nfindPosOther ;
                            pinstance->m_nstartPos  = nannounceInsertPos + 1 ;
                            pinstance->m_einstanceFormat = iinstance_format ;
                            i++ ;
                            continue ;
                        }
                        else if(1 == nresult)
                        {
//                          isNeededFindFlag = true ;
                            /*������һ�ֲ���*/
                            i++ ;
                            continue ;
                        }
                        else
                        {
                            return 1 ;
                        }
                    }
                    else
                    {
                        return 1 ;
                    }
                }
            }

            struct SEARCH_STRING_STRUCTURE inextFind ;
            inextFind.m_etype = SearchLeftBracketPos ;
            inextFind.m_icontent.m_eInstanceFormat = NonStardardFormat ;

            if(rangestring.lastIndexOf(QRegExp(tr("\\S")),(nsaveStartPos + nfindPos -1)) != -1)
            {
                nannounceInsertPos = rangestring.lastIndexOf(QRegExp(tr("\\S")),(nsaveStartPos + nfindPos -1)) ;
            }

            int nleftBracketPos = 0 ;
//          QString itest1 = rangestring.mid(noffset);
//          qDebug() << itest1 ;
            if(!skipCommentaryFind(rangestring,noffset,inextFind,nleftBracketPos))
            {
                /*���� ��֮��Ӧ�� ��һ�� ")"*/
                struct SEARCH_STRING_STRUCTURE inextFindRightBracket ;
                inextFindRightBracket.m_etype = SearchInstancePos ;
                inextFindRightBracket.m_icontent.m_eInstanceFormat = NonStardardFormat ;

                int nrightBracketPos = 0 ;
                if(!findOppositeBracket(rangestring,nleftBracketPos+1,inextFindRightBracket,nrightBracketPos))
                {
                      QString itest2 = rangestring.mid(noffset,nrightBracketPos - nleftBracketPos - 1);
                      QString itest3 = rangestring.mid(nannounceInsertPos , nrightBracketPos - nannounceInsertPos - 1);
                      pinstance->m_nnextRightBracketPos = nrightBracketPos ;
                      pinstance->m_nstartPos  = nannounceInsertPos + 1 ;
                      pinstance->m_einstanceFormat = inextFindRightBracket.m_icontent.m_eInstanceFormat ;

                      /*���ŷ��� module �˿ڵ���д���� �Ƿ�Ϊ��׼ */
//                    stringtype.m_isearchContent.m_iinstanceStructure.m_einstanceFormat =  \
//                            inextFindRightBracket.m_isearchContent.m_iinstanceStructure.m_einstanceFormat ;
                      ++i ;
                      continue ;
                }
                else
                {
                    return 1 ;
                }
            }
            else
            {
                return 1 ;
            }

            ++i;
        }

        if(ismoduleEnd)
        {
            return 0 ;
        }

        // ����ʱɨ�赽 endmodule �ؼ��� ���˳� ����Ӧ������ע���� ûɨ�赽  endmodule �˳�
        if(isNoCommentary)
        {
            return 1 ;
        }
    }
}



int EziDebugVlgFile::findOppositeBracket(const QString &rangestring,int startpos ,SEARCH_STRING_STRUCTURE &stringtype,int &targetpos)
{
    int ncommentaryBegin = 0 ;
    int ncommentaryEnd   = startpos ;
    QString ipartString ;
    QString inoCommentaryStr ;
    QString icheckString ;
    QString itestString ;

    int commentaryBegin_row  = 0 ; // ��ע�Ϳ�ʼ
    int commentaryBegin_sec  = 0 ; // ��ע�Ϳ�ʼ
    int scanPos = startpos ;
    int nsavePos = 0 ;
    int nstartBracketPos = 0 ;

    //bool isNeededFindFlag = 0 ;
    int  nappearanceCount = 1 ;
    int  nleftBracketPos = 0 ;
    int  nrightBracketPos = 0 ;
    //inoCommentaryStr = replaceCommentaryByBlank(rangestring) ;
//    QFile itest("d:/save.txt") ;
//    itest.open(QIODevice::WriteOnly|QIODevice::Truncate);

//    QTextStream iout(&itest) ;
//    iout << inoCommentaryStr ;
//    int Pos1 = rangestring.indexOf("ifft_airif_rdctrl");
//    int Pos2 = inoCommentaryStr.indexOf("ifft_airif_rdctrl");

    while(1)
    {
        // ������һ�� ע�͵Ŀ�ʼλ��
        commentaryBegin_row = rangestring.indexOf(tr("//"),scanPos) ;
        commentaryBegin_sec = rangestring.indexOf(tr("/*"),scanPos) ;

        if((commentaryBegin_row == -1)&&(commentaryBegin_sec == -1))
        {
            /*û��ע���� ��scanPos ��ȡʣ�����е��ַ��� */
            ipartString = rangestring.mid(ncommentaryEnd) ;
            nsavePos = ncommentaryEnd ;
            ncommentaryEnd = NO_COMMENTARY ;
        }
        else if(commentaryBegin_row == -1)
        {
            /*��һ��ע�Ϳ�ʼλ��Ϊ */
            ncommentaryBegin = commentaryBegin_sec ;
            ipartString = rangestring.mid(ncommentaryEnd,ncommentaryBegin-ncommentaryEnd) ;
            nsavePos = ncommentaryEnd ;
            // ��һ��ע�ͽ���λ�� */
            ncommentaryEnd = rangestring.indexOf(tr("*/"),ncommentaryBegin) ;

            if(ncommentaryEnd == -1)
            {
                return 1;
            }
            scanPos = ncommentaryEnd + 2 ;
        }
        else if(commentaryBegin_sec == -1)
        {
            /*��һ��ע�Ϳ�ʼλ��Ϊ */
            ncommentaryBegin = commentaryBegin_row ;
            ipartString = rangestring.mid(ncommentaryEnd,ncommentaryBegin-ncommentaryEnd) ;

            nsavePos = ncommentaryEnd ;
            ncommentaryEnd = rangestring.indexOf(tr("\n"),ncommentaryBegin) ;
            if(ncommentaryEnd == -1)
            {
                return 1;
            }
            scanPos = ncommentaryEnd + 1;
        }
        else
        {
            if(commentaryBegin_row < commentaryBegin_sec)
            {
                ncommentaryBegin = commentaryBegin_row ;
                ipartString = rangestring.mid(ncommentaryEnd,ncommentaryBegin-ncommentaryEnd) ;
                nsavePos = ncommentaryEnd ;
                ncommentaryEnd = rangestring.indexOf(tr("\n"),ncommentaryBegin) ;
                if(ncommentaryEnd == -1)
                {
                    return 1 ;
                }
                scanPos = ncommentaryEnd + 1;
            }
            else
            {
                ncommentaryBegin = commentaryBegin_sec ;
                ipartString = rangestring.mid(ncommentaryEnd,ncommentaryBegin-ncommentaryEnd) ;
                nsavePos = ncommentaryEnd ;
                ncommentaryEnd = rangestring.indexOf(tr("*/"),ncommentaryBegin) ;
                if(ncommentaryEnd == -1)
                {
                    return 1;
                }
                scanPos = ncommentaryEnd + 2 ;
            }
        }

        //int  nstartPos = startpos ;

        /*�������ע�͵Ĵ����� �Ҷ�Ӧ�����ŵ�λ��*/

        /*��������ע�� ���� ���� "output" ���� "input "�ؼ��֣���Ϊ*/

        if(-1 == nsavePos)
        {
            return 1 ;
        }


        qDebug() << ipartString ;
        nstartBracketPos = 0 ;
        while(1)
        {
            /*�Ƿ���� ������ �� ������*/
            nleftBracketPos = ipartString.indexOf("(",nstartBracketPos);
            nrightBracketPos = ipartString.indexOf(")",nstartBracketPos);

            if((NO_STRING_FINDED == nleftBracketPos)&&(NO_STRING_FINDED == nrightBracketPos))
            {
                break ;
            }
            else if(NO_STRING_FINDED == nleftBracketPos)
            {
                /*ֻ�� ")"*/
                nappearanceCount-- ;
                if(0 == nappearanceCount)
                {
                    goto CheckType ;
                }
                nstartBracketPos = nrightBracketPos + 1 ;
            }
            else if(NO_STRING_FINDED == nrightBracketPos)
            {
                /*ֻ�� "("*/
                nstartBracketPos = nleftBracketPos + 1 ;
                nappearanceCount++ ;
            }
            else
            {
                if(nleftBracketPos < nrightBracketPos)
                {
                    /*���� "("*/
                    nappearanceCount++ ;
                    nstartBracketPos = nleftBracketPos + 1 ;
                }
                else
                {
                    /*���� ")"*/
                    nappearanceCount-- ;

                    if(0 == nappearanceCount)
                    {

                        goto CheckType ;
                    }
                    nstartBracketPos = nrightBracketPos + 1 ;
                }
            }
        }
    }

CheckType:
        qDebug() << rangestring.mid(nsavePos,nrightBracketPos);
        targetpos = nsavePos + nrightBracketPos ;
        int n = targetpos - startpos + 1 ;
        icheckString = rangestring.mid(startpos , n );
        qDebug() << "origin string! "<<icheckString ;
        icheckString = replaceCommentaryByBlank(icheckString);
        qDebug() << "port string! "<<icheckString ;

        //itestString = rangestring.mid(scanPos ,targetpos - scanPos + 1 );
        if(SearchModuleKeyWordPos == stringtype.m_etype)
        {
            if(icheckString.contains("input",Qt::CaseInsensitive)||icheckString.contains("output",Qt::CaseInsensitive))
            {
                stringtype.m_icontent.m_imodulest.m_eportAnnounceFormat = AnsicFormat ;
            }
        }
        else if(SearchInstancePos == stringtype.m_etype)
        {
            // .mc_ul_start       ( mc_ul_start       ),
            // ��׼�� ���� ��һ�� ")"
            if(icheckString.contains(QRegExp(tr(".")+ tr("\\s*\\w+") + tr("\\s*\\(")+ tr(".*") + tr("\\)"))))
            {
                stringtype.m_icontent.m_eInstanceFormat = StardardForamt ;
                targetpos = ipartString.lastIndexOf(')',nrightBracketPos-1);
                if(-1 == targetpos)
                {
                   return 1 ;
                }
                targetpos = nsavePos + targetpos + 1 ;
            }
            else
            {
                // �Ǳ�׼�� ���� ��һ�� �ǿհ��ַ�
                targetpos = ipartString.lastIndexOf(QRegExp(tr("\\S")),nrightBracketPos-1);
                if(-1 == targetpos)
                {
                   return 1 ;
                }
                targetpos = nsavePos + targetpos + 1 ;
            }
        }
        else
        {
            return 1 ;
        }

    return 0 ;
}


QString EziDebugVlgFile::getNoCommentaryString(const QString &rangestring,int &lastcommentaryend ,int &nocommontaryflag)
{
    int commentaryBegin_row = 0 ;
    int commentaryBegin_sec = 0 ;
    int scanPos = lastcommentaryend ;
    int ncommentaryEnd = 0 ;
    int ncommentaryBegin = 0 ;
    QString ipartString ;
    // ������һ�� ע�͵Ŀ�ʼλ��

    if(lastcommentaryend == -1)
    {
        /*ȫ��ע��  ���ؿ��ַ���*/
        ipartString.clear();
        return ipartString ;
    }

    commentaryBegin_row = rangestring.indexOf(tr("//"),scanPos) ;
    commentaryBegin_sec = rangestring.indexOf(tr("/*"),scanPos) ;

    if((commentaryBegin_row == -1)&&(commentaryBegin_sec == -1))
    {
        ipartString = rangestring.mid(scanPos) ;
        /*û��ע���� ��scanPos ��ȡʣ�����е��ַ��� */
        ncommentaryEnd = -1 ;
        // ���һ��ɨ�� ����ʼλ��  ����ע��
        lastcommentaryend =  scanPos ;
        nocommontaryflag = true ;
    }
    else if(commentaryBegin_row == -1)
    {
        /*��һ��ע�Ϳ�ʼλ��Ϊ */
        ncommentaryBegin = commentaryBegin_sec ;
        ipartString = rangestring.mid(scanPos,ncommentaryBegin-scanPos) ;

        // ��һ��ע�ͽ���λ�� */
        ncommentaryEnd = rangestring.indexOf(tr("*/"),ncommentaryBegin) ;
        if(ncommentaryEnd == -1)
        {
            // ����������ע��
            lastcommentaryend = -1 ;
            return ipartString ;
        }
        lastcommentaryend = ncommentaryEnd  + 2;
    }
    else if(commentaryBegin_sec == -1)
    {
        /*��һ��ע�Ϳ�ʼλ��Ϊ // */
        ncommentaryBegin = commentaryBegin_row ;
        ipartString = rangestring.mid(scanPos,ncommentaryBegin-scanPos) ;


        ncommentaryEnd = rangestring.indexOf(tr("\n"),ncommentaryBegin + 2);
        if(ncommentaryEnd == -1)
        {
            // ����������ע��
            lastcommentaryend = -1 ;
            return ipartString ;
        }
        lastcommentaryend =  ncommentaryEnd + 1;
    }
    else
    {
        if(commentaryBegin_row < commentaryBegin_sec)
        {
            ncommentaryBegin = commentaryBegin_row ;
            ipartString = rangestring.mid(scanPos,ncommentaryBegin-scanPos) ;

            ncommentaryEnd = rangestring.indexOf(tr("\n"),ncommentaryBegin) ;
            if(ncommentaryEnd == -1)
            {
                // ����������ע��
                lastcommentaryend = -1 ;
                return ipartString ;
            }
            lastcommentaryend =  ncommentaryEnd + 1 ;
        }
        else
        {
            ncommentaryBegin = commentaryBegin_sec ;
            ipartString = rangestring.mid(scanPos,ncommentaryBegin-scanPos) ;

            ncommentaryEnd = rangestring.indexOf(tr("*/"),ncommentaryBegin) ;
            if(ncommentaryEnd == -1)
            {
                // ����������ע��
                lastcommentaryend = -1 ;
                return ipartString ;

            }
            lastcommentaryend =  ncommentaryEnd + 2 ;
        }
    }
    return  ipartString ;
}



QString EziDebugVlgFile::replaceCommentaryByBlank(const QString &rangestring)
{
    QString data = rangestring ;
    int commentaryBegin_row = 0 ; // ��ע�Ϳ�ʼ
    int commentaryBegin_sec = 0 ; // ��ע�Ϳ�ʼ
    int commentaryEnd_row = 0 ;  // ��ע�ͽ���
    int commentaryEnd_sec = 0 ;  // ��ע�ͽ���
    int nstartPos = 0 ;

    commentaryBegin_row =  data.indexOf(tr("//"),nstartPos);
    commentaryBegin_sec =  data.indexOf(tr("/*"),nstartPos);
    while((commentaryBegin_row != -1)||(commentaryBegin_sec != -1))
    {
        if(commentaryBegin_row == -1)
        {
            commentaryEnd_sec = data.indexOf(tr("*/"), commentaryBegin_sec);
            data.replace(commentaryBegin_sec,commentaryEnd_sec - commentaryBegin_sec + 2,tr(" ").repeated(commentaryEnd_sec - commentaryBegin_sec + 2));
            commentaryBegin_sec =  data.indexOf(tr("/*"), commentaryBegin_sec);
        }
        else if(commentaryBegin_sec == -1)
        {
            commentaryEnd_row = data.indexOf(tr("\n"), commentaryBegin_row);
            if(commentaryEnd_row == -1)
            {
                int ncharPos = data.lastIndexOf(QRegExp(".*")) ;
                data.replace(commentaryBegin_row , ncharPos - commentaryBegin_row + 1,tr(" ").repeated(ncharPos - commentaryBegin_row + 1));
            }
            else
            {
                data.replace(commentaryBegin_row,commentaryEnd_row - commentaryBegin_row,tr(" ").repeated(commentaryEnd_row - commentaryBegin_row));
            }
            commentaryBegin_row =  data.indexOf(tr("//"), commentaryBegin_row);

        }
        else
        {
            /*��ע�Ϳ�ʼ �� ��ע�Ϳ�ʼ ֮ǰ*/
            if( commentaryBegin_row < commentaryBegin_sec )
            {
                /*ɾ����ע�ͱ�ʾ����ʼ���һ���ַ�*/
                commentaryEnd_row = data.indexOf(tr("\n"), commentaryBegin_row);
                if(commentaryEnd_row == -1)
                {
                    int ncharPos = data.lastIndexOf(QRegExp(".*")) ;
                    data.replace(commentaryBegin_row , ncharPos - commentaryBegin_row + 1,tr(" ").repeated(ncharPos - commentaryBegin_row + 1));
                }
                else
                {
                    data.replace(commentaryBegin_row,commentaryEnd_row - commentaryBegin_row,tr(" ").repeated(commentaryEnd_row - commentaryBegin_row));
                }
                commentaryBegin_sec =  data.indexOf(tr("/*"), commentaryBegin_row);
                commentaryBegin_row =  data.indexOf(tr("//"), commentaryBegin_row);
            }
            /*��ע�Ϳ�ʼ �� ��ע�Ϳ�ʼ ֮ǰ*/
            else
            {
                commentaryEnd_sec = data.indexOf(tr("*/"), commentaryBegin_sec);
                data.replace(commentaryBegin_sec,commentaryEnd_sec - commentaryBegin_sec + 2,tr(" ").repeated(commentaryEnd_sec - commentaryBegin_sec + 2));
                commentaryBegin_row =  data.indexOf(tr("//"), commentaryBegin_sec);
                commentaryBegin_sec =  data.indexOf(tr("/*"), commentaryBegin_sec);
            }
        }

    }
    return data ;
}

int EziDebugVlgFile::isModuleInstance(const QString &rangestring,const QString &modulename , const QString& instancename,INSTANCE_FORMAT &type ,int &targetpos)
{
    QString data = replaceCommentaryByBlank(rangestring);

    QRegExp ifindExp(tr("\\b")+ modulename + tr("\\s+") + instancename +tr("\\b")) ;

    if(NO_STRING_FINDED == data.indexOf(ifindExp))
    {
        return 1 ;
    }

    /*�ҵ����ŵ� "("*/
    struct SEARCH_STRING_STRUCTURE ifindLeftBracket ;
    ifindLeftBracket.m_etype = SearchLeftBracketPos ;
    ifindLeftBracket.m_icontent.m_nreserved = 0  ;

    int nleftBracketPos = 0 ;
    if(!skipCommentaryFind(rangestring,0,ifindLeftBracket,nleftBracketPos))
    {
        struct SEARCH_STRING_STRUCTURE ifindRightBracket ;
        ifindRightBracket.m_etype = SearchInstancePos ;
        ifindRightBracket.m_icontent.m_eInstanceFormat = NonStardardFormat ;

        int nrightBracketPos = 0 ;
        if(!findOppositeBracket(rangestring,nleftBracketPos+1,ifindRightBracket,nrightBracketPos))
        {
            targetpos = nrightBracketPos ;
            type = ifindRightBracket.m_icontent.m_eInstanceFormat ;
            return 0 ;
        }
        else
        {
            return 2 ;
        }
    }
    else
    {
        return 2 ;
    }
}


int EziDebugVlgFile::isModuleDefinition(const QString &rangestring,const QString &modulename ,PORT_ANNOUNCE_FORMAT &type,int &targetpos)
{
    QString data = replaceCommentaryByBlank(rangestring);

    QRegExp ifindExp(tr("\\b")+ modulename + tr("\\b")) ;

    if(NO_STRING_FINDED == data.indexOf(ifindExp))
    {
        return 1 ;
    }

    /*�ҵ����ŵ� "("*/
    struct SEARCH_STRING_STRUCTURE ifindLeftBracket ;
    ifindLeftBracket.m_etype = SearchLeftBracketPos ;
    ifindLeftBracket.m_icontent.m_nreserved = 0 ;

    int nleftBracketPos = 0 ;
    if(!skipCommentaryFind(rangestring,0,ifindLeftBracket,nleftBracketPos))
    {
        struct  SEARCH_MODULE_STRUCTURE imoduleSt ;
        struct SEARCH_STRING_STRUCTURE ifindRightBracket ;
        ifindRightBracket.m_etype = SearchRightBracketPos ;
        imoduleSt.m_eportAnnounceFormat = NonAnsicFormat ;
        strcpy(imoduleSt.m_amoduleName,modulename.toAscii().data());
        ifindRightBracket.m_icontent.m_imodulest = imoduleSt ;


        int nrightBracketPos = 0 ;
        if(!findOppositeBracket(rangestring,nleftBracketPos+1,ifindRightBracket,nrightBracketPos))
        {
            targetpos = nrightBracketPos ;
            type = ifindRightBracket.m_icontent.m_imodulest.m_eportAnnounceFormat ;
            return 0 ;
        }
        else
        {
            return 2 ;
        }
    }
    else
    {
        return 2 ;
    }

//    // ���� output ���� input �ؼ���
//    ifindExp.setPattern(tr("\\b") + tr("output") + tr("\\b"));
//    QRegExp ifindExpOther(tr("\\b") + tr("input") + tr("\\b"));

//    if(data.contains(ifindExp)||data.contains(ifindExpOther))
//    {
//        type = AnsicFormat ;
//    }
//    else
//    {
//        type = NonAnsicFormat ;
//    }

}

int EziDebugVlgFile::isStringReiteration(const QString &poolstring ,const QString& string)
{
    if(poolstring.contains(QRegExp(tr("\b")+string + tr("\b"))))
    {
        return 0 ;
    }
    return 1 ;
}

QString  EziDebugVlgFile::constructChainRegString(EziDebugModule::RegStructure* reg, int regnum , int startbit ,int endbit ,EziDebugInstanceTreeItem *item)
{
    QString iregName ;
    QString istartBit ;
    QString iendBit ;
    QString imoduleName = QString::fromAscii(reg->m_pMouduleName);
    QStringList ifullNameList ;
    QString ibitWitdth = QString::fromAscii(reg->m_pExpString);

    QString istartRegNum = QString::number(reg->m_unStartNum);
    QString iendRegNum = QString::number(reg->m_unEndNum);

    QString ihiberarchyname = item->getItemHierarchyName() ;
    QString iinstanceName = item->getInstanceName() ;

    QString iregNum = QString::fromAscii(reg->m_pregNum);
    QString iclockName = QString::fromAscii(reg->m_pclockName);

    QString iresult ;
    if(!reg)
    {
        return QString();
    }
    else
    {
        // �Ĵ����� ��������ʱ ʹ��  aaa[m]
        if(reg->m_unRegNum != 1)
        {
            iregName.append(tr("%1[%2]").arg(QString::fromAscii(reg->m_pRegName)).arg(regnum));
        }
        else
        {
            iregName.append(tr("%1").arg(QString::fromAscii(reg->m_pRegName)));
        }

        // ��ʼλ
        istartBit.append(QString::number(startbit));
        // ����λ
        iendBit.append(QString::number(endbit));

    }

    ifullNameList << imoduleName << iinstanceName << iclockName << ihiberarchyname << iregName << istartBit << iendBit << ibitWitdth << istartRegNum  << iendRegNum << iregNum   ;
    iresult = ifullNameList.join(tr("#")) ;
    return (iresult) ;
}


int EziDebugVlgFile::scanFile(EziDebugPrj* prj,EziDebugPrj::SCAN_TYPE type,QList<EziDebugPrj::LOG_FILE_INFO*> &addedinfolist,QList<EziDebugPrj::LOG_FILE_INFO*> &deletedinfolist)
{
    qDebug() << "verilog file scanfile!" << fileName();
    bool echainExistFlag = false ;
    int i = 0 ;
    QString ifileName = fileName() ;
    EziDebugModule *poldModule  = NULL ;
    EziDebugVlgFile *poldFile = NULL ;
    QDir icurrentDir = prj->getCurrentDir() ;
    QString irelativeFileName = icurrentDir.relativeFilePath(this->fileName()) ;
    QStringList ieziPort ;
    QFileInfo ifileInfo(this->fileName()) ;
    QStringList ichangedchainList ;
    QStringList icheckChainList ;
    int nresult = 0 ;

    QDateTime ilastModifedTime = ifileInfo.lastModified() ;


    QMap<QString,QString> iclockMap ;
    QMap<QString,QString> iresetMap ;


    poldFile = prj->getPrjVlgFileMap().value(irelativeFileName ,NULL);

    if(poldFile)
    {
        for( ; i < poldFile->getModuleList().count();i++)
        {
            EziDebugModule *pmodule = prj->getPrjModuleMap().value(poldFile->getModuleList().at(i),NULL) ;
            if(pmodule)
            {
                QString imoduleName = pmodule->getModuleName() ;
                struct EziDebugPrj::LOG_FILE_INFO* pdelmoduleInfo = new EziDebugPrj::LOG_FILE_INFO ;
                pdelmoduleInfo->etype = EziDebugPrj::infoTypeModuleStructure ;
                pdelmoduleInfo->pinfo = NULL ;
                qstrcpy(pdelmoduleInfo->ainfoName,imoduleName.toAscii().data());
                deletedinfolist.append(pdelmoduleInfo);
            }
        }
    }

#if 1
    if(fileName().endsWith("SspTxFIFO.v"))
    {
        qDebug() << "SspTxFIFO.v";
    }
#endif

    // ���ԭ�ļ���  modulelist
    clearModuleList();

    unModuCnt = 0  ;
    unMacroCnt = 0 ;

    //  memset((void*)module_tab,0,MAX_T_LEN*sizeof(struct Module_Mem)) ;
    //  memset((void*)macro_table,0,MAX_T_LEN*sizeof(struct macro_Mem)) ;

    inst_map.clear();
    iinstNameList.clear();

    reg_scan reg_search ;

    memset((void*)buffer,0,sizeof(buffer)) ;

    //qDebug()  <<  ifileName.toAscii().data() ;
    if(reg_search.LoadVeriFile(buffer,ifileName.toAscii().data()))
    {
        reg_search.prog = buffer ;
        try
        {
            reg_search.ScanPre();
            reg_search.Interp();
        }
        catch (InterpExc &except)
        {
            qDebug() << "EziDebug file parse Error!" ;
            return 1 ;
        }
    }
    else
    {
        qDebug() << "EziDebug Error: read file error!" ;
        return 1 ;
    }


    //qDebug() << "Find Module Number:"  << mod_count;

    for(i = 0 ; i < unModuCnt ;i++)
    {    	
        QString imoduleName = QString::fromAscii(ModuleTab[i].cModuleName) ;
        EziDebugModule *pmodule = new EziDebugModule(imoduleName) ;
        if(!pmodule)
        {
            qDebug() << "There is not memory left!"  ;
            return 1 ;
        }

        //qDebug() << "GET A Module!"  << module_tab[i].inst_map.count() ;

        QMap<QString,QMap<QString,QString> > iinstMap = inst_map ;
        int ninstanceCount = 0 ;
        for( ; ninstanceCount < iinstNameList.count() ;ninstanceCount++)
        {
            QString iinst = iinstNameList.at(ninstanceCount) ;
            QString iinstanceName = iinst.split('#').at(1) ;

            qDebug() << __LINE__  << "EziDebug instance:" << iinst;

            if(QRegExp(QObject::tr("_EziDebug_\\w+")).exactMatch(iinstanceName))
            {
                //ieziInstList.append(iinst.split('#').at(1));

                //echainExistFlag = true ;
                continue ;
            }

            pmodule->m_iinstanceNameList << iinst.replace("#",":");
            pmodule->m_iinstancePortMap.insert(iinstanceName,iinstMap.value(iinstNameList.at(ninstanceCount)));
        }


        for(int m = 0 ; m< ModuleTab[i].unParaCnt ;m++)
        {
            pmodule->addToParameterMap(QString::fromAscii(ModuleTab[i].ParaTab[m].cParaName),\
                                       ModuleTab[i].ParaTab[m].iParaVal);

        }

        // ���ļ������ defparameter ��Ϣ
        QMap<QString,QString>::const_iterator idefParamIter = def_map.constBegin() ;
        while(idefParamIter != def_map.constEnd())
        {
             // <inst_name.para_name,para_value>
            QString icombName = idefParamIter.key() ;
            if(!icombName.contains("."))
            {
                qDebug() << "EziDebug Error: scan file Error , parameter pattern error!"  ;
                return 1 ;
            }
            QString iinstanceName = icombName.split(".").at(0) ;

            if(QRegExp(QObject::tr("_EziDebug_\\w+")).exactMatch(iinstanceName))
            {
                ++idefParamIter ;
                continue ;
            }

            QString iparamterStr = icombName.split(".").at(1) ;
            QString iparamterVal = idefParamIter.value() ;

            addToDefParameterMap(iinstanceName,iparamterStr,iparamterVal);
            ++idefParamIter ;
        }

        int j = 0 ;
        // ���ļ������ define  ��Ϣ
        for(; j < unMacroCnt ; j++)
        {
           addToMacroMap(QString::fromAscii(MacroTab[j].cMacroName),MacroTab[j].iMacroVal);
        }

        for(j = 0 ;j < ModuleTab[i].unRegCnt ; j++)
        {
            QString iedge ;

            if(QRegExp(QObject::tr("_EziDebug_\\w+")).exactMatch(QString::fromAscii(ModuleTab[i].RegTab[j].cRegName)))
            {
                // ��ʾ �ļ��а�����EziDebug��ӵĴ���  �Ƿ����ɾ��

                // ɾ��ָ��
                //echainExistFlag = true ;
                continue ;
            }

            if(QString::fromAscii(ModuleTab[i].RegTab[j].ClkAttri.cClkName).isEmpty())
            {
                continue ;
            }

            if(ModuleTab[i].RegTab[j].IsFlag == 0)
            {
                continue ;
            }

            struct EziDebugModule::RegStructure * preg = new EziDebugModule::RegStructure ;


            memset((char*)preg,0,sizeof(struct EziDebugModule::RegStructure));


            qstrcpy(preg->m_pRegName,ModuleTab[i].RegTab[j].cRegName) ;

            if(ModuleTab[i].RegTab[j].iRegWidth.size() >= 64)
            {
                qDebug() << "EziDebug Error: the reg number string is too long!";
                continue ;
            }

            if(ModuleTab[i].RegTab[j].iRegCnt.isEmpty())
            {
                qstrcpy(preg->m_pregNum,"1") ;
            }
            else
            {
                qstrcpy(preg->m_pregNum,ModuleTab[i].RegTab[j].iRegCnt.toAscii().constData());
            }


            if(ModuleTab[i].RegTab[j].iRegWidth.count() >= 64 )
            {
                qDebug() << "EziDebug Error: the reg width string is too long!";
                continue ;
            }

            qstrcpy(preg->m_pExpString , ModuleTab[i].RegTab[j].iRegWidth.toAscii().constData());


            //  ��ʼ���Ĵ�������
            preg->m_unStartNum = 0 ;
            preg->m_unEndNum = 0 ;
            preg->m_unRegNum = 0 ;

            preg->m_unStartBit = 0 ;
            preg->m_unEndBit = 0 ;
            preg->m_unRegBitWidth = 0 ;


            preg->m_unMaxRegNum = 0 ;
            preg->m_eRegNumEndian = EziDebugModule::endianOther ;
            preg->m_eRegNumType = EziDebugModule::attributeOther ;
            preg->m_unMaxBitWidth = 0 ;
            preg->m_eRegBitWidthEndian = EziDebugModule::endianOther ;
            preg->m_eRegBitWidthType = EziDebugModule::attributeOther ;


            qstrcpy(preg->m_pclockName,ModuleTab[i].RegTab[j].ClkAttri.cClkName);


            if(ModuleTab[i].RegTab[j].ClkAttri.eClkEdge == POSE)
            {
                iedge = QObject::tr("posedge");
                preg->m_eedge =  EziDebugModule::signalPosEdge ;
            }
            else if(ModuleTab[i].RegTab[j].ClkAttri.eClkEdge == NEGE)
            {
                iedge = QObject::tr("negedge");
                preg->m_eedge =  EziDebugModule::signalNegEdge ;
            }
            else if(ModuleTab[i].RegTab[j].ClkAttri.eClkEdge == LOW)
            {
                iedge = QObject::tr("low");
                preg->m_eedge =  EziDebugModule::signalLow ;
            }
            else if(ModuleTab[i].RegTab[j].ClkAttri.eClkEdge == HIGH)
            {
                iedge = QObject::tr("high");
                preg->m_eedge =  EziDebugModule::signalHigh ;
            }
            else
            {
                iedge = QObject::tr("posedge");
                preg->m_eedge =  EziDebugModule::signalPosEdge ;
            }

            if(QString::fromAscii(ModuleTab[i].RegTab[j].ClkAttri.cClkName).isEmpty())
            {
                qDebug() << "no clock " << preg->m_pRegName;
            }

            iclockMap.insert(QString::fromAscii(ModuleTab[i].RegTab[j].ClkAttri.cClkName),\
                             iedge);

            if(ModuleTab[i].RegTab[j].RstAttri.eRstEdge == POSE)
            {
                iedge = QObject::tr("posedge");
            }
            else if(ModuleTab[i].RegTab[j].RstAttri.eRstEdge == NEGE)
            {
                iedge = QObject::tr("negedge");
            }
            else
            {
                iedge = QObject::tr("posedge");
            }


            qstrcpy(preg->m_pMouduleName,ModuleTab[i].cModuleName) ;

            preg->m_unStartNum = 0 ;

            if(!QString::fromAscii(ModuleTab[i].RegTab[j].RstAttri.cRstName).isEmpty())
            {
                iresetMap.insert(QString::fromAscii(ModuleTab[i].RegTab[j].RstAttri.cRstName),\
                                 iedge);
            }

            QString iclockName = QString::fromAscii(preg->m_pclockName);

            // ȫ������ ���̶�λ�� , ��������״�ڵ�ʱ,���ֳ��� �ǹ̶�λ��
            pmodule->AddToRegMap(iclockName ,preg);

        }

        pmodule->m_iclockMap = iclockMap ;

        pmodule->m_iresetMap = iresetMap ;

        QVector<EziDebugModule::PortStructure*> iportVec ;

        //qDebug() << "scan port !"  ;
        int k = 0 ;
        for(; k < ModuleTab[i].unIOCnt ; k++)
        {
            // ��ʱ�޸�
            if(QRegExp(QObject::tr("_EziDebug_\\w+")).exactMatch(QString::fromAscii(ModuleTab[i].IOTab[k].cIOName)))
            {
                QString ieziPortName = QString::fromAscii(ModuleTab[i].IOTab[k].cIOName) ;
                ieziPort <<  ieziPortName ;

                echainExistFlag = true ;
                // ��ʾ �ļ��а�����EziDebug��ӵĴ���  �Ƿ����ɾ��
                continue ;

            }

            struct EziDebugModule::PortStructure * pport = new EziDebugModule::PortStructure ;

            memset((char*)pport,0,sizeof(struct EziDebugModule::PortStructure)) ;

            qstrcpy(pport->m_pPortName,ModuleTab[i].IOTab[k].cIOName);

            pport->m_unStartBit = 0 ;
            pport->m_unBitwidth = 0 ;
            pport->m_unEndBit = 0 ;
            pport->m_eEndian  = EziDebugModule::endianOther ;

            if(ModuleTab[i].IOTab[k].iIOWidth.size() >= 64)
            {
                qDebug() << "EziDebug Error: the reg width string is too long!";
                continue ;
            }

            qstrcpy(pport->m_pBitWidth,ModuleTab[i].IOTab[k].iIOWidth.toAscii().constData()) ;

            if(ModuleTab[i].IOTab[k].eIOAttri == IO_INPUT)
            {
                pport->eDirectionType = EziDebugModule::directionTypeInput ;
            }
            else if(ModuleTab[i].IOTab[k].eIOAttri == IO_OUTPUT)
            {
                pport->eDirectionType = EziDebugModule::directionTypeOutput ;
            }
            else if(ModuleTab[i].IOTab[k].eIOAttri == IO_INOUT)
            {
                pport->eDirectionType = EziDebugModule::directionTypeInoutput ;
            }
            else
            {
                pport->eDirectionType = EziDebugModule::directionTypeInoutput ;
            }

            qstrcpy(pport->m_pModuleName,ModuleTab[i].cModuleName);

            iportVec.append(pport);

        }

        pmodule->m_iportVec = iportVec ;

        // QDir(����·��). relativeFilePath(�������ļ�·��) �õ��ļ����·��
        pmodule->m_ilocatedFile = prj->getCurrentDir().relativeFilePath(fileName());

        // bool m_isLibaryCore ;
        pmodule->m_isLibaryCore |= ModuleTab[i].nIPCore ;

        if(this->isLibaryFile())
        {
            pmodule->m_isLibaryCore = true ;
        }

        qDebug() << "add to moudle list" << imoduleName << __FILE__ << __LINE__;

        // ���� module map
        qDebug() << "Add to moudle map" << imoduleName << pmodule << __FILE__ << __LINE__;


        poldModule = prj->getPrjModuleMap().value(imoduleName,NULL) ;

        // ɨ����ɺ��� ��ʾ ��Ϣ ɨ�������ƻ�  �Ƿ����������
#if 0
        if(pmodule->getModuleName() == "ifft")
        {
            qDebug() << "ifft" ;
        }
#endif
        addToModuleList(imoduleName);
        prj->addToModuleMap(imoduleName,pmodule);

        if(echainExistFlag)
        {
            /*
                1����log�ļ����򿪹��̽��и���ɨ��
                  �޶Աȶ��󣬲����жԱ�
                2����log�ļ����򿪹��̽��и���ɨ��
                  ��ɨ�������жԱȣ����ж��Ƿ������ƻ�
                3�����Ѵ򿪹��̽��и���
                  ֱ�Ӹ����Ѿ���¼�ġ�module ���жԱ�
                  ע:2013.3.26  ����parameter���� ���� ��ԭmodule���  �����,�Ժ�����ø���
                  ɨ��������Ϣ���бȽ�
            */
            // ����log�ļ������жԱ�
            if(true == prj->getLogFileExistFlag())
            {
                for(int i = 0 ; i < ieziPort.count() ; i++)
                {
                    QString iportName = ieziPort.at(i) ;
                    if(iportName.split('_',QString::SkipEmptyParts).count() >= 4)
                    {
                        QString ichainName = iportName.split('_',QString::SkipEmptyParts).at(1) ;
                        // prj->addToCheckedChainList(ichainName);
                        if(!icheckChainList.contains(ichainName))
                        {
                            icheckChainList.append(ichainName);
                            EziDebugScanChain* pchain = prj->getScanChainInfo().value(ichainName ,NULL) ;
                            if(pchain)
                            {
                                if(!pmodule->isChainCompleted(pchain))
                                {
                                    ichangedchainList.append(ichainName);                                    
                                    prj->addToDestroyedChainList(ichainName);
                                }
                            }
                            else
                            {
                                // log �ļ����ƻ�(����log�ļ������ʽ����,���Ǵ����е��� log�ļ��в�����)
                                prj->setLogfileDestroyedFlag(true);
                            }
                        }
                    }
                }

            }

        }
    }

    // ����Щmodule��
    // ����ɨ�������� ���ɨ�����������module Ȼ����  line_code block_code �Ƿ����
    // All code in this file
    // QStringList icodeList ;
    QStringList ichainNameList ;
    for(int nmoduleNum = 0 ; nmoduleNum < this->getModuleList().count() ;nmoduleNum++)
    {
        QString imoduleName = this->getModuleList().at(nmoduleNum) ;
        QMap<QString,EziDebugScanChain*> iscanChainMap = prj->getScanChainInfo() ;
        QMap<QString,EziDebugScanChain*>::const_iterator iscanchainIter = iscanChainMap.constBegin() ;
        while(iscanchainIter != iscanChainMap.constEnd())
        {
            EziDebugScanChain* pchain = iscanchainIter.value() ;
            QString ichainName = iscanchainIter.key() ;
            if(pchain->getLineCode().contains(imoduleName))
            {
                if(!ichainNameList.contains(ichainName))
                {
                    ichainNameList.append(ichainName);
                }
            }

            if(pchain->getBlockCode().contains(imoduleName))
            {
                if(!ichainNameList.contains(ichainName))
                {
                    ichainNameList.append(ichainName);
                }
            }
            ++iscanchainIter ;
        }

        if((nresult = checkedEziDebugCodeExist(prj,imoduleName,ichainNameList)) != 0)
        {
            return nresult ;
        }
    }


//    // �����ɨ�豻�޸ĵ��ļ��� ����� ��ص� module ����
//    if(!(this->getLastStoredTime().isNull()))
//    {
//        // ���ĵ��ļ�
//        if(isModifedRecently())
//        {
//            // �ļ����Ķ� ���� ��ص� module ����

//            // ���ļ� �� module ���� ɾ����������
////            struct EziDebugPrj::LOG_FILE_INFO* pdelFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
////            pdelFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;
////            pdelFileInfo->pinfo = NULL ;
////            memcpy(pdelFileInfo->ainfoName,this->fileName().toAscii(),this->fileName().size()+1);
////            deletedinfolist.append(pdelFileInfo);

////            for(int i = 0 ; i < getModuleList().count();i++)
////            {
////                EziDebugModule *pmodule = prj->getPrjModuleMap().value(getModuleList().at(i),NULL) ;
////                if(!pmodule)
////                {
////                    struct EziDebugPrj::LOG_FILE_INFO* pdelmoduleInfo = new EziDebugPrj::LOG_FILE_INFO ;

////                    pdelmoduleInfo->etype = EziDebugPrj::infoTypeModuleStructure ;
////                    pdelmoduleInfo->pinfo = NULL ;
////                    memcpy(pdelmoduleInfo->ainfoName,pmodule->getModuleName().toAscii(),pmodule->getModuleName().size()+1);
////                    deletedinfolist.append(pdelmoduleInfo);
////                }
////            }

//            // �����ĺ���ļ� �� �µ�module ���� ��ӵ������� ��Ҫ�������
//            struct EziDebugPrj::LOG_FILE_INFO* paddFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
//            paddFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;
//            paddFileInfo->pinfo = this ;
//            memcpy(paddFileInfo->ainfoName,this->fileName().toAscii(),this->fileName().size()+1);
//            addedinfolist.append(paddFileInfo);

//            for(int i = 0 ; i < getModuleList().count();i++)
//            {
//                EziDebugModule *pmodule = prj->getPrjModuleMap().value(getModuleList().at(i),NULL) ;
//                if(pmodule)
//                {
//                    struct EziDebugPrj::LOG_FILE_INFO* paddModuleInfo = new EziDebugPrj::LOG_FILE_INFO ;
//                    paddModuleInfo->etype = EziDebugPrj::infoTypeModuleStructure ;
//                    paddModuleInfo->pinfo = pmodule ;
//                    memcpy(paddModuleInfo->ainfoName,pmodule->getModuleName().toAscii(),pmodule->getModuleName().size()+1);
//                    addedinfolist.append(paddModuleInfo);
//                }
//                else
//                {
//                    return 1 ;
//                }
//            }
//        }
//    }
//    else
//    {
        // ������ɨ����� ��Ϣ ����


        qDebug() << " !!module count !!"<< getModuleList().count();
        for(i = 0 ; i < getModuleList().count();i++)
        {
            EziDebugModule *pmodule = prj->getPrjModuleMap().value(getModuleList().at(i),NULL) ;
            if(pmodule)
            {   
                qDebug() << " add module info " ;
                struct EziDebugPrj::LOG_FILE_INFO* paddModuleInfo = new EziDebugPrj::LOG_FILE_INFO ;
                paddModuleInfo->etype = EziDebugPrj::infoTypeModuleStructure ;
                paddModuleInfo->pinfo = pmodule ;
                memcpy(paddModuleInfo->ainfoName,pmodule->getModuleName().toAscii().data(),pmodule->getModuleName().size()+1);
                addedinfolist.append(paddModuleInfo);
            }
            else
            {
                return 1 ;
            }
        }
//    }
    //qDebug() << addedinfolist.count() <<  deletedinfolist.count();
    // �����ļ�ɨ��ʱ��
    modifyStoredTime(ilastModifedTime);

    return 0 ;
}

int EziDebugVlgFile::checkedEziDebugCodeExist(EziDebugPrj* prj ,QString imoduleName ,QStringList &chainnamelist)
{
    QMap<QString,EziDebugScanChain*> ichainMap = prj->getScanChainInfo() ;
    QMap<QString,int> ilinesearchposMap ;
    QMap<QString,int> iblocksearchposMap ;
    QMap<QString,QStringList> ichainLineCodeMap ;
    QMap<QString,QStringList> ichainBlockCodeMap ;
    QStringList idestroyChainList ;
    QStringList icheckChainList ;
    QList<int>  iposList ;

    if(!open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // ���û����  �ļ��򲻿�
        qDebug() << errorString() << fileName() ;
        return 1 ;
    }

    QTextStream iin(this);
    QString ifileContent = iin.readAll();
    QString inoCommentaryStr = replaceCommentaryByBlank(ifileContent) ;
    QRegExp imoduleStartExp(tr("module\\s+%1").arg(imoduleName)) ;
    int nstartPos = inoCommentaryStr.indexOf(imoduleStartExp);
    if(-1 == nstartPos)
    {
        qDebug() << "EziDebug Error: parse file error ,please check the file!";
        return -2 ;
    }
    int nendPos = inoCommentaryStr.indexOf("endmodule",nstartPos);
    if(-1 == nendPos)
    {
        qDebug() << "EziDebug Error: parse file error ,please check the file!";
        return -2 ;
    }
    QString icheckStr = inoCommentaryStr.mid(nstartPos,nendPos-1);
    // �ر�
    close();

    for(int nchainNum = 0 ; nchainNum < chainnamelist.count() ; nchainNum++)
    {
        QString ichainName = chainnamelist.at(nchainNum) ;
        EziDebugScanChain * pchain = ichainMap.value(ichainName,NULL);
        if(pchain)
        {
            QMap<QString,QStringList> icodeListMap = pchain->getLineCode() ;
            QStringList icodeList ;
            icodeList = icodeListMap.value(imoduleName,icodeList) ;
            if(icodeList.count())
            {
                ichainLineCodeMap.insert(ichainName,icodeList);
            }

            icodeList.clear();
            icodeListMap = pchain->getBlockCode();
            icodeList = icodeListMap.value(imoduleName,icodeList);
            if(icodeList.count())
            {
                ichainBlockCodeMap.insert(ichainName,icodeList) ;
            }
        }
        else
        {
            qDebug() << "NULL Pointer!" << ichainName << "is not exist!";
            return -1 ;
        }
    }

    // linecode
    QMap<QString ,QStringList>::const_iterator icodeIter = ichainLineCodeMap.constBegin() ;
    while(icodeIter != ichainLineCodeMap.constEnd())
    {
        int ncodeNum = 0 ;
        QString chainname = icodeIter.key() ;
        QStringList icode = icodeIter.value() ;
        iposList.clear();
        EziDebugScanChain * pchain = ichainMap.value(chainname);
        int nsearchPos = 0 ;
        for(;ncodeNum < icode.count();ncodeNum++)
        {
            QString isearchLineStr = icode.at(ncodeNum) ;
            if(-1 != (nsearchPos = icheckStr.indexOf(QRegExp(isearchLineStr))))
            {
                ilinesearchposMap.insert(isearchLineStr ,nsearchPos);
                iposList.append(nsearchPos);
            }
            else
            {
                qDebug() <<"EziDebug Warning:"<< isearchLineStr << "is not finded!";
                if(!idestroyChainList.contains(chainname))
                {
                    idestroyChainList.append(chainname);
                }
                break ;
            }
        }

        // check code sequence
        if(ncodeNum == icode.count())
        {
            QSet<int> isimplifiedSet = iposList.toSet();
            if(isimplifiedSet.count() == iposList.count())
            {
                 QStringList inewCodeList ;
                 qSort(iposList.begin(), iposList.end(), qLess<int>());
                 for(int ncodeNum = 0 ; ncodeNum < iposList.count() ; ncodeNum++)
                 {
                    int npos = iposList.at(ncodeNum) ;
                    QString icodeStr = ilinesearchposMap.key(npos) ;
                    inewCodeList.append(icodeStr);
                 }

                 if(inewCodeList != icode)
                 {
                    pchain->replaceLineCodeMap(imoduleName,inewCodeList);
                    prj->addToCheckedChainList(chainname);
                 }
            }
            else
            {
                qDebug() << "Please remove the repeated code !";
                for(int i = 0 ; i < icode.count() ; i++)
                {
                    qDebug() << icode.at(i) << endl ;
                }
                return -2 ;
            }

        }
        ilinesearchposMap.clear();
        ++icodeIter ;
    }


    // blockcode
    icodeIter = ichainBlockCodeMap.constBegin() ;
    while(icodeIter != ichainBlockCodeMap.constEnd())
    {
        int ncodeNum = 0 ;
        iposList.clear();
        QString chainname = icodeIter.key() ;
        QStringList icode = icodeIter.value() ;
        EziDebugScanChain * pchain = ichainMap.value(chainname);
        // the code in icode is from little to big
        int nsearchPos = 0 ;
        for(;ncodeNum < icode.count();ncodeNum++)
        {
            QString isearchLineStr = icode.at(ncodeNum) ;
            if(-1 != (nsearchPos = icheckStr.indexOf(QRegExp(isearchLineStr))))
            {
                iblocksearchposMap.insert(isearchLineStr ,nsearchPos);
                iposList.append(nsearchPos);
            }
            else
            {
                qDebug() <<"EziDebug Warning:"<< isearchLineStr << "is not finded!";
                if(!idestroyChainList.contains(chainname))
                {
                    idestroyChainList.append(chainname);
                }
                break ;
            }
        }

        // check code sequence
        if(ncodeNum == icode.count())
        {
            QSet<int> isimplifiedSet = iposList.toSet();
            if(isimplifiedSet.count() == iposList.count())
            {
                 QStringList inewCodeList ;
                 qSort(iposList.begin(), iposList.end(), qLess<int>());
                 for(int ncodeNum = 0 ; ncodeNum < iposList.count() ; ncodeNum++)
                 {
                    int npos = iposList.at(ncodeNum) ;
                    QString icodeStr = iblocksearchposMap.key(npos) ;
                    inewCodeList.append(icodeStr);
                 }

                 if(inewCodeList != icode)
                 {
                    pchain->replaceBlockCodeMap(imoduleName,inewCodeList);

                    prj->addToCheckedChainList(chainname);
                 }
            }
            else
            {
                qDebug() << "Please remove the repeated code !\n";
                for(int i = 0 ; i < icode.count() ; i++)
                {
                    qDebug() << icode.at(i) << endl ;
                }
                return -2 ;
            }
        }
        iblocksearchposMap.clear();
        ++icodeIter ;
    }

    // check the destroyed chain!
    for(int nchaincount = 0 ; nchaincount < idestroyChainList.count() ;nchaincount++)
    {
        QString ichainName = idestroyChainList.at(nchaincount) ;
        qDebug()  << "EziDebug Error:" << ichainName << "is destroyed!" << "checkedEziDebugCodeExist !";
        prj->addToDestroyedChainList(ichainName);
    }

    return 0 ;
}

int EziDebugVlgFile::createUserCoreFile(EziDebugPrj* prj)
{
    if(!prj)
        return 1 ;
    QString iregModuleName(tr("_EziDebug_ScnReg")) ;
    QString itimerName(tr("_EziDebug_TOUT_m"));


    QFileInfo iPrjFileInfo(prj->getPrjName());
    /*�����µ��ļ���  �� �����Լ���core�ļ�*/
    QDir idirPrj(iPrjFileInfo.absolutePath());
    QString inewDirName = iPrjFileInfo.absolutePath() + tr("/") + tr("EziDebug_1.0") ;
    QDir idir(inewDirName);
    QTime icurrentTime = QTime::currentTime() ;

    if(idir.exists())
    {
        qDebug() <<  "There is already exist folder!" << inewDirName ;
        // ��� �ļ��Ƿ����
        if(idir.exists("_EziDebug_ScanChainReg.v"))
        {
            if(idir.exists("_EziDebug_TOUT_m.v"))
            {
               EziDebugScanChain::saveEziDebugAddedInfo(iregModuleName,itimerName,QObject::tr("/EziDebug_1.0"));
               // ���´��� tout �ļ�
               QString ifileToutName(tr("_EziDebug_TOUT_m.v"));
               QFile itoutfile(inewDirName + tr("/") + ifileToutName);
               if(!itoutfile.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
               {
                    /*���ı��� ��ʾ���������� �޷������ļ� �ļ��޷���*/
                    return 1 ;
               }


               /*д�� tout �ļ�*/
               QTextStream itoutStream(&itoutfile);

               itoutStream <<"module ";

               /*д��module����*/
               itoutStream << itimerName;
               itoutStream << g_ptoutfileContentFirst ;
               itoutStream << prj->getMaxRegNumPerChain() << ";" ;
               itoutStream << g_ptoutfileContentSecond ;

               itoutfile.close();
               return 0 ;
            }
        }
        // ���´��� �ļ�
        idir.remove("_EziDebug_ScanChainReg.v") ;
        idir.remove("_EziDebug_TOUT_m.v");
    }
    else
    {
        idirPrj.mkdir(tr("EziDebug_1.0"));
    }

    QString ifileRegName(tr("_EziDebug_ScanChainReg.v")) ;
    QString ifileToutName(tr("_EziDebug_TOUT_m.v"));
    QStringList ifileList = prj->getFileNameList();

    for (int i = 0; i < ifileList.size(); ++i)
    {
       QFileInfo ifileInfo(prj->getCurrentDir(),ifileList.at(i));
       if(ifileInfo.fileName() == ifileRegName)
       {
           ifileRegName = tr("_EziDebug_ScanChainReg")+tr("_")+ icurrentTime.toString("hh_mm_ss") + tr(".v") ;
           if(ifileInfo.fileName() == ifileRegName)
           {
                /*���ı��� ��ʾ����������  �޷������ļ�  �ļ����ظ�*/
               qDebug() << "EziDebug encounter error,Please ensure your fileName is right" << ifileRegName ;
               return 1 ;
           }
       }


       if(ifileInfo.fileName() == ifileToutName)
       {
           ifileToutName = tr("_EziDebug_TOUT_m")+tr("_")+ icurrentTime.toString("hh_mm_ss") + tr(".v") ;
           if(ifileInfo.fileName() == ifileToutName)
           {
                /*���ı��� ��ʾ����������  �޷������ļ�  �ļ����ظ�*/
               qDebug() << "EziDebug encounter error,Please ensure your fileName is right" << ifileToutName ;
               return 1 ;
           }
       }
    }

#if 0
    /*���module�����Ƿ�����ظ�*/
    QMap<QString,EziDebugModule*>::const_iterator i = prj->getPrjModuleMap().constBegin();
    while (i != prj->getPrjModuleMap().constEnd())
    {

    }
 #endif

    QFile itoutfile(inewDirName + tr("/") + ifileToutName);
    if(!itoutfile.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
    {
         /*���ı��� ��ʾ���������� �޷������ļ� �ļ��޷���*/
         return 1 ;
    }


    /*д�� tout �ļ�*/
    QTextStream itoutStream(&itoutfile);

    itoutStream <<"module ";

    /*д��module����*/
    itoutStream << itimerName;
    itoutStream << g_ptoutfileContentFirst ;
    itoutStream << prj->getMaxRegNumPerChain() << ";" ;
    itoutStream << g_ptoutfileContentSecond ;

    itoutfile.close();


    QFile iscanRegfile(inewDirName + tr("/") + ifileRegName);
    if(!iscanRegfile.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
         /*���ı��� ��ʾ���������� �޷������ļ� �ļ��޷���*/
         return 1 ;
    }
    /*д�� ScanReg �ļ�*/
    QTextStream iscanRegout(&iscanRegfile);

    iscanRegout <<"module ";

    /*д��module����*/
    iscanRegout << iregModuleName ;
    iscanRegout << g_pScanRegfileContentFirst ;
    iscanRegout << g_pScanRegfileContentSecond ;

    iscanRegfile.close();

    EziDebugScanChain::saveEziDebugAddedInfo(iregModuleName,itimerName,QObject::tr("/EziDebug_1.0"));

    return 0 ;
}

int EziDebugVlgFile::caculateExpression(QString string)
{
    return 0 ;
}

void EziDebugVlgFile::addToMacroMap(const QString &macrostring , const QString &macrovalue)
{
    m_imacro.insert(macrostring,macrovalue) ;
}

void EziDebugVlgFile::addToDefParameterMap(const QString &instancename ,const QString &parameter ,const QString &value)
{
    QMap<QString,QString> iparameterMap ;
    iparameterMap = m_idefparameter.value(instancename ,iparameterMap) ;
    iparameterMap.insert(parameter,value);
    m_idefparameter.insert(instancename,iparameterMap) ;
}

const QMap<QString,QString> & EziDebugVlgFile::getMacroMap(void) const
{
    return m_imacro ;
}

const QMap<QString,QMap<QString,QString> > & EziDebugVlgFile::getDefParamMap(void) const
{
    return m_idefparameter ;
}


#if 0
bool EziDebugVlgFile::isLibaryFile()
{
    return 0 ;
}
#endif


int  EziDebugVlgFile::deleteEziDebugCode(void)
{
    // ֻ����ʽ ���ļ�
    QString ifileContent ;
    QString inewContent ;
    //QString ikeyWords ;
    int nkeyWordsPos = 0 ;
    QList<int> iposList ;
    QMap<int,int> ideleteCodePosMap ;

    //QRegExp ieziDebugFlagExp(QObject::tr("\\b_EziDebug_\\w+\\b"));
    QRegExp ieziDebugFlagWithEnterExp(QObject::tr("\\s*\\b_EziDebug_\\w+\\b"));
    QRegExp ikeyWordsExp(QObject::tr("\\s*\\b[a-z]+\\b"));
    //QRegExp ikeyWordsWithEnterExp(QObject::tr("\\s*\\b\\w+\\b"));



    /*
        _EziDebug_ScnReg _EziDebug_ScnReg_chn_inst0(
        .clock	(HCLK) ,
        .resetn	(_EziDebug_chn_rstn) ,
        .TDI_reg	(_EziDebug_chn_HCLK_TDI_reg) ,
        .TDO_reg	(_EziDebug_chn_HCLK_tdo1) ,
        .TOUT_reg	(_EziDebug_chn_TOUT_reg) ,
        .shift_reg	(_EziDebug_chn_HCLK_sr0)
        )
    */
    QRegExp ieziDebugScnInstExp(QObject::tr("\\s*_EziDebug_ScnReg\\s+_EziDebug_ScnReg_chn\\d*_inst\\d*\\s*\\(.+\\)\\s*"));


    QRegExp ieziDebugToutInstExp(QObject::tr("\\s*_EziDebug_TOUT_m\\s+_EziDebug_TOUT_m_chn\\d*_inst\\d*\\s*\\(.+\\)\\s*"));
    //_EziDebug_TOUT_m _EziDebug_TOUT_m_chn_inst

    int npostion = 0 ;
    if(!open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // ���û����  �ļ��򲻿�
        qDebug() << errorString() << fileName() ;
        return 1 ;
    }
    // ȫ������
    QTextStream iin(this);
    ifileContent = iin.readAll();
    // �ر�
    close();

    // �滻����ע��Ϊ  �ո�
    inewContent = replaceCommentaryByBlank(ifileContent);
    qDebug() << "EziDebug info: start file---" << fileName();
    if(fileName().endsWith("fft_ram_256x17.v"))
    {
        qDebug() << "EziDebug Info : halt Point!";
    }

     qDebug() << fileName() ;

    // ���� �� "_EziDebug" ��ͷ���ַ���
    while((npostion = ieziDebugFlagWithEnterExp.indexIn(inewContent,npostion)) != -1)
    {
        // EziDebug �Զ��� core ������
        int nnextSemicolonPos = inewContent.indexOf(';',npostion + ieziDebugFlagWithEnterExp.matchedLength()) ;
        QString itruncateStr = inewContent.mid(npostion , (nnextSemicolonPos - npostion));
        qDebug() << "test string!" << itruncateStr  << ieziDebugScnInstExp.exactMatch(itruncateStr) << ieziDebugToutInstExp.exactMatch(itruncateStr) ;

        int nin = ieziDebugScnInstExp.indexIn(itruncateStr) ;
        qDebug() << nin <<  ieziDebugScnInstExp.matchedLength() << ieziDebugScnInstExp.capturedTexts().at(0);

        if(ieziDebugScnInstExp.exactMatch(itruncateStr)||ieziDebugToutInstExp.exactMatch(itruncateStr))
        {
            iposList.append(npostion);
            ideleteCodePosMap.insert(npostion,(nnextSemicolonPos - npostion + 1));
            qDebug() << " instance match!" <<  ifileContent.mid(npostion ,(nnextSemicolonPos - npostion + 1)) ;

            npostion = nnextSemicolonPos + 1 ;
        }
        else
        {
            if(QRegExp(QObject::tr("\\s*\\b_EziDebug\\w+\\s*<=.*")).exactMatch(itruncateStr))
            {
                // Ѱ�� always
                int nalwaysPos = inewContent.lastIndexOf(QRegExp(QObject::tr("\\balways\\b")),npostion);
                int nlastNoneblankCharPos = inewContent.lastIndexOf(QRegExp(QObject::tr("\\S")),nalwaysPos);
                if(nlastNoneblankCharPos != -1)
                {
                    // nalwaysPos   =  nlastNoneblankCharPos + 1 ;
                    nalwaysPos   =  nlastNoneblankCharPos ;
                }
                int nfirstBeginPos = inewContent.indexOf(QRegExp(QObject::tr("\\bbegin\\b")),nalwaysPos);
                /*����ƥ���  end*/
                QRegExp iwordsExp(QObject::tr("\\b\\w+\\b"));
                int nmatch = 1 ;
                int nendPos = 0 ;
                int nbeginPos = nfirstBeginPos + 5 ;

                while((nbeginPos = iwordsExp.indexIn(ifileContent,nbeginPos)) != -1)
                {
                    if(iwordsExp.capturedTexts().at(0) == "begin")
                    {
                        nmatch++ ;
                    }
                    else if(iwordsExp.capturedTexts().at(0) == "end")
                    {
                        nmatch-- ;
                        if(0 == nmatch)
                        {
                            nendPos = nbeginPos ;
                            break ;
                        }
                    }
                    else
                    {
                        // do nothing
                    }
                    nbeginPos = nbeginPos + iwordsExp.matchedLength();
                }

                if(nmatch != 0)
                {
                    return 1 ;
                }

                iposList.append(nalwaysPos);
                ideleteCodePosMap.insert(nalwaysPos,(nendPos - nalwaysPos + 3));

                qDebug()<< "1" << ifileContent.mid(nalwaysPos ,(nendPos - nalwaysPos + 3)) ;

                npostion = nendPos + 3 ;
            }
            else
            {
                // ��������ĵ���
                nkeyWordsPos = ikeyWordsExp.lastIndexIn(inewContent,npostion) ;
                QString ikeyWord = ikeyWordsExp.capturedTexts().at(0) ;
                // ����� ','
                int nlastCommaPos = inewContent.lastIndexOf(',',npostion);

                if(nlastCommaPos > nkeyWordsPos)
                {
                    // �Ƿ���� '.'
                    /*ֻ���� ���� �м��� �Ķ˿����� */
                    // ��׼�� ,.(),.(),.()
                    // ��ȡ ��һ�� ���� ֮��� �ַ� ���Ƿ���� ".( �ַ�"
                    QString ipartStr = inewContent.mid((nlastCommaPos+1) , (npostion - nlastCommaPos -1));
                    if(QRegExp(QObject::tr("\\s*\\.")).exactMatch(ipartStr))
                    {
                        // ɾ����һ������ �� ��һ�� ) ֮���ַ���
                        int nnextRightBracketPos = inewContent.indexOf(')',npostion);
                        iposList.append(nlastCommaPos);
                        ideleteCodePosMap.insert(nlastCommaPos,(nnextRightBracketPos - nlastCommaPos + 1));
                        qDebug()<< "2" << ifileContent.mid(nlastCommaPos ,(nnextRightBracketPos - nlastCommaPos + 1)) ;

                        npostion = nnextRightBracketPos + 1 ;
                    }
                    else
                    {
                        // ɾ����һ������ �� ���ַ�������
                        iposList.append(nlastCommaPos);
                        ideleteCodePosMap.insert(nlastCommaPos,(npostion + ieziDebugFlagWithEnterExp.matchedLength()- nlastCommaPos));
                        qDebug()<< "3" << ifileContent.mid(nlastCommaPos ,(npostion + ieziDebugFlagWithEnterExp.matchedLength()- nlastCommaPos)) ;

                        npostion =  npostion + ieziDebugFlagWithEnterExp.matchedLength() ;
                    }
                }
                else
                {
                    if(ikeyWordsExp.capturedTexts().at(0).contains(QRegExp(QObject::tr("\\binput\\b"))))
                    {
                        int nlastCommaPos = inewContent.lastIndexOf(',',npostion);
                        int nlastSemicolon = inewContent.lastIndexOf(';',npostion);
                        int nnextSemicolon = inewContent.indexOf(';',npostion);

                        if(nlastCommaPos > nlastSemicolon)
                        {
                            // �˿�����
                            iposList.append(nlastCommaPos);
                            ideleteCodePosMap.insert(nlastCommaPos,(npostion + ieziDebugFlagWithEnterExp.matchedLength() - nlastCommaPos));
                            qDebug() << "4" << ifileContent.mid(nlastCommaPos ,(npostion + ieziDebugFlagWithEnterExp.matchedLength() - nlastCommaPos)) ;

                            npostion = npostion + ieziDebugFlagWithEnterExp.matchedLength() ;
                        }
                        else
                        {
                            nkeyWordsPos = inewContent.lastIndexOf(QRegExp(QObject::tr("\\S")),nkeyWordsPos-1) + 1 ;
                            iposList.append(nkeyWordsPos);
                            ideleteCodePosMap.insert(nkeyWordsPos,(nnextSemicolon - nkeyWordsPos + 1));
                            qDebug() << "5" << ifileContent.mid(nkeyWordsPos ,(nnextSemicolon - nkeyWordsPos + 1)) ;

                            npostion = nnextSemicolon + 1 ;
                        }
                    }
                    else if(ikeyWordsExp.capturedTexts().at(0).contains(QRegExp(QObject::tr("\\boutput\\b"))))
                    {
                        int nlastCommaPos = inewContent.lastIndexOf(',',npostion);
                        int nlastSemicolon = inewContent.lastIndexOf(';',npostion);
                        int nnextSemicolon = inewContent.indexOf(';',npostion);

                        if(nlastCommaPos > nlastSemicolon)
                        {
                            iposList.append(nlastCommaPos);
                            ideleteCodePosMap.insert(nlastCommaPos,(npostion + ieziDebugFlagWithEnterExp.matchedLength() - nlastCommaPos));
                            qDebug() << "6" << ifileContent.mid(nlastCommaPos ,(npostion + ieziDebugFlagWithEnterExp.matchedLength() - nlastCommaPos)) ;

                            npostion = npostion + ieziDebugFlagWithEnterExp.matchedLength() ;
                        }
                        else
                        {
                            nkeyWordsPos = inewContent.lastIndexOf(QRegExp(QObject::tr("\\S")),nkeyWordsPos-1) + 1 ;
                            iposList.append(nkeyWordsPos);
                            ideleteCodePosMap.insert(nkeyWordsPos,(nnextSemicolon - nkeyWordsPos + 1));
                            qDebug() << "7" << ifileContent.mid(nkeyWordsPos ,(nnextSemicolon - nkeyWordsPos + 1)) ;
                            npostion = nnextSemicolon + 1 ;
                        }
                    }
                    else if(ikeyWordsExp.capturedTexts().at(0).contains(QRegExp(QObject::tr("\\breg\\b"))))
                    {
                        int nnextSemicolon = inewContent.indexOf(';',npostion);
                        nkeyWordsPos = inewContent.lastIndexOf(QRegExp(QObject::tr("\\S")),nkeyWordsPos-1) + 1 ;
                        iposList.append(nkeyWordsPos);
                        ideleteCodePosMap.insert(nkeyWordsPos,(nnextSemicolon - nkeyWordsPos + 1));
                        qDebug() << "8" << ifileContent.mid(nkeyWordsPos ,(nnextSemicolon - nkeyWordsPos + 1)) ;
                        npostion = nnextSemicolon + 1;
                    }
                    else if(ikeyWordsExp.capturedTexts().at(0).contains(QRegExp(QObject::tr("\\bwire\\b"))))
                    {
                        int nnextSemicolon = inewContent.indexOf(';',npostion);
                        nkeyWordsPos = inewContent.lastIndexOf(QRegExp(QObject::tr("\\S")),nkeyWordsPos-1) + 1 ;
                        iposList.append(nkeyWordsPos);
                        ideleteCodePosMap.insert(nkeyWordsPos,(nnextSemicolon - nkeyWordsPos + 1));
                        qDebug() << "9" << ifileContent.mid(nkeyWordsPos ,(nnextSemicolon - nkeyWordsPos + 1)) ;
                        npostion = nnextSemicolon + 1;
                    }
                    else if(ikeyWordsExp.capturedTexts().at(0).contains(QRegExp(QObject::tr("\\bdefparam\\b"))))
                    {
                        int nnextSemicolon = inewContent.indexOf(';',npostion);
                        nkeyWordsPos = inewContent.lastIndexOf(QRegExp(QObject::tr("\\S")),nkeyWordsPos-1) + 1 ;
                        iposList.append(nkeyWordsPos);
                        ideleteCodePosMap.insert(nkeyWordsPos,(nnextSemicolon - nkeyWordsPos + 1));
                        qDebug() << "10" << ifileContent.mid(nkeyWordsPos ,(nnextSemicolon - nkeyWordsPos + 1)) ;
                        npostion = nnextSemicolon + 1;
                    }
                    else if(ikeyWordsExp.capturedTexts().at(0).contains(QRegExp(QObject::tr("\\bassign\\b"))))
                    {
                        int nnextSemicolon = inewContent.indexOf(';',npostion);
                        nkeyWordsPos = inewContent.lastIndexOf(QRegExp(QObject::tr("\\S")),nkeyWordsPos-1) + 1 ;
                        iposList.append(nkeyWordsPos);
                        ideleteCodePosMap.insert(nkeyWordsPos,(nnextSemicolon - nkeyWordsPos + 1));
                        qDebug() << "11" << ifileContent.mid(nkeyWordsPos ,(nnextSemicolon - nkeyWordsPos + 1)) ;
                        npostion = nnextSemicolon + 1;
                    }
                    else
                    {
                        // do nothing
                        npostion = npostion + ikeyWordsExp.matchedLength();
                    }
                }
            }
        }
    }

    // from big to small
    qSort(iposList.begin(), iposList.end(), qGreater<int>());

    // j < iposList.count()
    for(int j = 0 ; j < iposList.count() ; j++)
    {
        int nstartPos = iposList.at(j) ;
        int nlength = ideleteCodePosMap.value(nstartPos , 0) ;
        ifileContent.replace(nstartPos , nlength ,"");
    }


    if(!open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        // ���û����  �ļ��򲻿�
        qDebug() << "EziDebug Error:" << errorString() << ":" << fileName() ;
        return 1 ;
    }

    qDebug() << "EziDebug info: finish file---" << fileName();
    // ȫ��д��
    QTextStream iout(this);
    iout << ifileContent ;
    close() ;

    QFileInfo ifileInfo(fileName()) ;
    this->modifyStoredTime(ifileInfo.lastModified()) ;

    return 0 ;
}

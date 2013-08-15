#include <QtGui>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QDir>
#include <QList>


#include "button.h"
#include "toolwindow.h"
#include "ezidebugprj.h"

#include "ezidebugmodule.h"
#include "ezidebugvlgfile.h"
#include "ezidebugvhdlfile.h"
#include "ezidebuginstancetreeitem.h"
#include "ezidebugscanchain.h"
#include "importdatadialog.h"
#include "textquery.h"
//#include "updatedetectthread.h"

#define ZERO_REG_NUM  0
#define PARAMETER_OK            0x0
#define NO_PARAMETER_TOOL       0x01
#define NO_PARAMETER_REG_NUM    0x02
#define NO_PARAMETER_DIR        0x04
#define NO_PARAMETER_ALL        (0x01|0x02|0x04)
static unsigned long long tic = 0 ;


ToolWindow::ToolWindow(QWidget *parent) :
    QWidget(parent)
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("gb18030"));
    isNeededUpdate = false ;
    currentPrj = NULL  ;
    readSetting() ;  // ��ȡ��������������Ϣ
    setWindowTitle(tr("EziDebug"));
    //��ͨģʽ�µ��б��� Qt::FramelessWindowHint
    listWindow = new ListWindow(this,Qt::FramelessWindowHint);
    m_proSetWiz = 0 ;
    //m_pcurrentPrj = 0 ;
    createActions();   //�����Ҽ��˵���ѡ��
    createButtons();   //������ť
    creatTrayIcon();   //��������ͼ��

    //���ñ���
    QPixmap backgroundPix;
    QPixmap maskPix;
    QPalette palette;
    maskPix.load(":/images/toolWindowMask.bmp");
    setMask(maskPix);
    backgroundPix.load(":/images/mainBackground.bmp",0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
    palette.setBrush(QPalette::Background, QBrush(backgroundPix));
    setPalette(palette);
    //setMask(backgroundPix.mask());   //ͨ��QPixmap�ķ������ͼƬ�Ĺ��˵�͸���Ĳ��ֵõ���ͼƬ����ΪWidget�Ĳ�����߿�
    //setWindowOpacity(1.0);  //����ͼƬ͸����
    //���öԻ����λ�úʹ�С
    //setGeometry(QRect(250,100,355,25));
    setFixedSize(backgroundPix.size());//���ô��ڵĳߴ�ΪͼƬ�ĳߴ�
    move((qApp->desktop()->width() - this->width()) / 2,
         (qApp->desktop()->height() - this->height()) /2 - 100);//������������Ļ�м俿�ϵ�λ��
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);//����Ϊ�ޱ߿� ������������ť�Ҽ��˵� ������С���뻹ԭ
    //
    //������ͨģʽ�°�ť��λ�úʹ�С
    //��������ť
    minimizeButton->setGeometry(251, 0, 27, 19);//��С��
    miniModeButton->setGeometry(QRect(277, 0, 27, 19));//����ģʽ
    showListWindowButton->setGeometry(QRect(303, 0, 27, 19));//չ��listWindow�İ�ť
    exitButton->setGeometry(QRect(329, 0, 33, 19));//�ر�
    //���ܰ�ť
    proSettingButton->setGeometry(QRect(23, 37, 42, 41));//��������
    proUpdateButton->setGeometry(QRect(65, 37, 42, 41));//����
    proPartlyUpdateButton->setGeometry(QRect(107, 37, 42, 41));//���ָ���
    deleteChainButton->setGeometry(QRect(149, 37, 42, 41));//ɾ��
    testbenchGenerationButton->setGeometry(QRect(191, 37, 42, 41));//testbench����
    proUndoButton->setGeometry(QRect(233, 37, 42, 41));//������undo��

    //������
    progressBar = new QProgressBar(this);
    progressBar->setGeometry(QRect(28, 88, 248, 10));
    progressBar->setRange(0, 100);
    progressBar->setValue(0);

    //progressBar->setStyleSheet("QProgressBar { border: 2px solid grey; border-radius: 5px; }");
//    progressBar->setStyleSheet("QProgressBar::chunk { background-color: #6cccff;width: 6px;}");
//    progressBar->setStyleSheet("QProgressBar { border: 0px solid grey;border-radius: 2px; text-align: right;}");

    progressBar->setTextVisible(false);
    progressBar->setStyleSheet(
    "QProgressBar {"
    "border: 0px solid black;"
    "width: 10px;"
    "background: QLinearGradient( x1: 0, y1: 0.1, x2: 0, y2: 0.9,"
    "stop: 0 #fff,"
    "stop: 0.4999 #eee,"
    "stop: 0.5 #ddd,"
    "stop: 1 #eee );}");

    progressBar->setStyleSheet(
    "QProgressBar::chunk {"
    "background: QLinearGradient( x1: 0, y1: 0.1, x2: 0, y2: 0.9,"
    "stop: 0 #ace5ff,"
    "stop: 0.4999 #42c8ff,"
    "stop: 0.5 #22b8ff,"
    "stop: 1 #ace5ff );"
    "border: 0px solid black;}");


//    progressBar->setStyleSheet("  \
//    QProgressBar {  \
//    border: 0px solid black;  \
//    text-align: right;    \
//    padding: 1px;   \
//    border-top-left-radius: 5px;    \
//    border-bottom-left-radius: 5px; \
//    width: 8px;   \
//    background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1,    \
//    stop: 0 #fff,   \
//    stop: 0.4999 #eee,  \
//    stop: 0.5 #ddd, \
//    stop: 1 #eee );}");

//    progressBar->setStyleSheet("    \
//    QProgressBar::chunk {   \
//    background: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1,   \
//    stop: 0 #78d,   \
//    stop: 0.4999 #46a,   \
//    stop: 0.5 #45a,   \
//    stop: 1 #238 );   \
//    border-top-left-radius: 5px;    \
//    border-bottom-left-radius: 5px; \
//    border: 0px solid black;}");


    //m_iplogolabel = new QLabel(this) ;
    //EziDebugIcon1
    //m_iplogolabel->setPixmap(QPixmap(":/images/indication.bmp"));
    //m_iplogolabel->setGeometry(QRect(30, 20, 80, 44));

    updatehintButton = new QPushButton(this) ;
    //updatehintButton->setIcon(QIcon(":/images/update2.png"));
    updatehintButton->setIcon(QIcon(":/images/update2.png"));
    updatehintButton->setFlat(true);
    updatehintButton->setIconSize(QSize(30,29));
    updatehintButton->setGeometry(QRect(320, 75, 30, 29));
    updatehintButton->setFocusPolicy(Qt::NoFocus);
    updatehintButton->setDisabled(true);

    iChangeUpdateButtonTimer = new QTimer(this) ;
    connect(iChangeUpdateButtonTimer, SIGNAL(timeout()), this, SLOT(changeUpdatePic()));


    // setListWindowAdsorbedFlag(true);

//��������ʾ
//   QTimer *timer = new QTimer(this);
//       connect(timer, SIGNAL(timeout()), this, SLOT(progressBarDemo()));
//       timer->start(1000);



//�����Ĵ�����ʾ������
//    QProgressDialog progressDialog(this);
//    progressDialog.setCancelButtonText(tr("ȡ��"));
//    progressDialog.setRange(0, 100);
//    progressDialog.setWindowTitle(tr("������"));

//    for (int i = 0; i < 100; ++i) {
//        progressDialog.setValue(i);
//        progressDialog.setLabelText(tr("����Ϊ %1 / %2...")
//                                    .arg(i).arg(100));
//        qApp->processEvents();
//        if (progressDialog.wasCanceled()){
//            //���ȡ��ʱ�Ĺ���
//            break;
//        }
//        for (int j = 0; j < 100000000; ++j);
//    }



    listWindow->setWindowFlags(Qt::SplashScreen);

    // Ϊ listwindow ��װ�¼�������
    // listWindow->installEventFilter(this);
    // listWindow->setWindowFlags(Qt::Window);

//    aa = new QWidget(this,Qt::FramelessWindowHint) ;
//    aa->setWindowTitle("zenmehuishi");
//    aa->show();
//    aa->raise();
//    aa->activateWindow();
//    qDebug()<< this->frameGeometry().left() << this->frameGeometry().bottom();


    listWindow->move(this->frameGeometry().bottomLeft());//�б��ڳ�ʼλ��ΪtoolWindow�����·�
    listWindow->setWindowStick(true);
//  listWindow->move(this->frameGeometry().left()-3,this->frameGeometry().bottom()-29);
//  listWindow->move(0,0);

//    qDebug()<< listWindow->pos().x()<<listWindow->pos().y();
//    qDebug()<< listWindow->pos().rx()<<listWindow->pos().ry();

//    qDebug()<< this->frameGeometry().bottom();
//    qDebug()<< listWindow->frameGeometry().top();




    //�������б���
    listWindow->setListWindowHidden(false);
    //�б���Ĭ�������ڹ����������·�
    isListWindowAdsorbed = true;

    //���޸�
#if 0
    connect(listWindow, SIGNAL(mouseReleased(const QRect)),
            this, SLOT(listWindowMouseReleased(const QRect)));
#endif

    connect(this,SIGNAL(updateTreeView(EziDebugInstanceTreeItem*)),listWindow,SLOT(generateTreeView(EziDebugInstanceTreeItem*)));


    //����ģʽ�µ������ں�״̬��
    //miniWindow = 0;
    miniWindow = new MiniWindow;
    miniWindow->hide();
    isNormalMode = true ;
    //����ģʽת������ͨģʽ
    connect(miniWindow, SIGNAL(toNormalMode()), this, SLOT(toNormalMode()));
    //miniWindow��С��ʱ���޸���ز˵���Ϣ
    connect(miniWindow->minimizeButton, SIGNAL(clicked()), this, SLOT(miniWindowMinimized()));

    //��������
    connect(miniWindow->proSettingButton, SIGNAL(clicked()), this, SLOT(proSetting()));
    //����
    connect(miniWindow->proUpdateButton, SIGNAL(clicked()), this, SLOT(proUpdate()));
    //���ָ���
    connect(miniWindow->proPartlyUpdateButton, SIGNAL(clicked()), this, SLOT(fastUpdate()));
    //ɾ��
    connect(miniWindow->deleteChainButton, SIGNAL(clicked()), this, SLOT(deleteAllChain()));
    //testbench����
    connect(miniWindow->testbenchGenerationButton, SIGNAL(clicked()), this, SLOT(testbenchGeneration()));
    //������undo��
    connect(miniWindow->proUndoButton, SIGNAL(clicked()), this, SLOT(undoOperation()));
}

ToolWindow::~ToolWindow()
{
    qDebug() << "Attention:Begin to destruct toolwindow!";
    if(currentPrj)
        delete currentPrj ;
}

//bool ToolWindow::eventFilter(QObject *obj, QEvent *event)
//{
//    if(obj == listWindow)
//    {
//        qDebug()<< "nothing to do !";
//        return 0 ;
//    }
//    else
//    {
//        return QWidget::eventFilter(obj,event);
//    }
//}
void ToolWindow::proSetting()
{
    int nexecResult = 0 ;
    if(!m_proSetWiz)
    {
        m_proSetWiz = new ProjectSetWizard(this);
    }
    else
    {
        delete m_proSetWiz ;
        m_proSetWiz = new ProjectSetWizard(this);
    }
    //connect
    if((nexecResult = m_proSetWiz->exec()))
    {   
#if 0
        QMessageBox::information(this, QObject::tr("������"),QObject::tr("�����������"));
#else
        QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Finishing setting the project configurations!"));
#endif

        /*�������õĲ����� �ж��Ƿ�������´������̶���*/
        if(!currentPrj)
        {
            currentPrj = new EziDebugPrj(m_proSetWiz->m_uncurrentRegNum,m_proSetWiz->m_icurrentDir,m_proSetWiz->m_ecurrentTool,this);
                            currentPrj->setXilinxErrCheckedFlag(m_proSetWiz->m_isXilinxErrChecked);
            UpdateDetectThread * pthread =  currentPrj->getThread() ;
            connect(pthread,SIGNAL(codeFileChanged()),this,SLOT(updateIndicate()));
            connect(currentPrj,SIGNAL(updateProgressBar(int)),this,SLOT(changeProgressBar(int)));

            listWindow->addMessage("info","EziDebug info: open new project!");
            QStandardItem * pitem = listWindow->addMessage("info",tr("The current project parameter:"));
            listWindow->addMessage("process",tr("      The  maximum register number of scan chain: %1").arg(m_proSetWiz->m_uncurrentRegNum),pitem);
            listWindow->addMessage("process",tr("      The current project path: %1").arg(m_proSetWiz->m_icurrentDir),pitem);
            listWindow->addMessage("process",tr("      The compile software: \"%1\"").arg((m_proSetWiz->m_ecurrentTool == EziDebugPrj::ToolQuartus) ? ("quartus") :("ise")),pitem);
        }
        else
        {
            if((m_proSetWiz->m_uncurrentRegNum == currentPrj->getMaxRegNumPerChain())\
                &&(m_proSetWiz->m_ecurrentTool == currentPrj->getToolType())\
                &&(QDir::toNativeSeparators(m_proSetWiz->m_icurrentDir) == QDir::toNativeSeparators(currentPrj->getCurrentDir().absolutePath()))\
                    &&(m_proSetWiz->m_isXilinxErrChecked == currentPrj->getSoftwareXilinxErrCheckedFlag()))
            {
                /*do nothing*/
                listWindow->addMessage("info","EziDebug info: The project parameters are the same as before!");
                QStandardItem * pitem = listWindow->addMessage("process",tr("The new project parameters:"));
                listWindow->addMessage("info",tr("      The  maximum register number of scan chain: %1").arg(m_proSetWiz->m_uncurrentRegNum),pitem);
                listWindow->addMessage("info",tr("      The current project path: %1").arg(m_proSetWiz->m_icurrentDir),pitem);
                listWindow->addMessage("info",tr("      The compile software: \"%1\"").arg((m_proSetWiz->m_ecurrentTool == EziDebugPrj::ToolQuartus) ? ("quartus") :("ise")),pitem);
            }
            else if(QDir::toNativeSeparators(m_proSetWiz->m_icurrentDir) != QDir::toNativeSeparators(currentPrj->getCurrentDir().absolutePath()))
            {
                qDebug() << m_proSetWiz->m_icurrentDir << endl << currentPrj->getCurrentDir().absolutePath() << QDir::toNativeSeparators(currentPrj->getCurrentDir().absolutePath()) ;
                delete currentPrj ;
                currentPrj = NULL ;
                EziDebugInstanceTreeItem::setProject(NULL);
                listWindow->clearTreeView();

                currentPrj = new EziDebugPrj(m_proSetWiz->m_uncurrentRegNum,m_proSetWiz->m_icurrentDir,m_proSetWiz->m_ecurrentTool,this);
                currentPrj->setXilinxErrCheckedFlag(m_proSetWiz->m_isXilinxErrChecked);
                //pparent->setCurrentProject(pcurrentPrj);
                UpdateDetectThread * pthread =  currentPrj->getThread() ;
                connect(pthread,SIGNAL(codeFileChanged()),this,SLOT(updateIndicate()));
                connect(currentPrj,SIGNAL(updateProgressBar(int)),this,SLOT(changeProgressBar(int)));

				#if 0
                QMessageBox::information(this, QObject::tr("��������"),QObject::tr("׼�������µĹ��̣�"));
				#else
				QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Ready to parse new project!"));
				#endif
				
                listWindow->addMessage("info","EziDebug info: The project is changed !");
                QStandardItem * pitem = listWindow->addMessage("warning",tr("The new project parameters:"));
                listWindow->addMessage("warning",tr("      The  maximum register number of scan chain: %1").arg(m_proSetWiz->m_uncurrentRegNum),pitem);
                listWindow->addMessage("warning",tr("      The current project path: %1").arg(m_proSetWiz->m_icurrentDir),pitem);
                listWindow->addMessage("warning",tr("      The compile software: \"%1\"").arg((m_proSetWiz->m_ecurrentTool == EziDebugPrj::ToolQuartus) ? ("quartus") :("ise")),pitem);


                /*�ȴ��û� update all ���½�������*/
                /*toolwindow ��ʼ��ʱ ���ָ��� ȫ������ ɾ��ȫ���� ����testbench ��������*/
            }
            else if(m_proSetWiz->m_ecurrentTool != currentPrj->getToolType())
            {

                /*�������ù��̲���*/
                currentPrj->setToolType(m_proSetWiz->m_ecurrentTool);
                listWindow->addMessage("warning","EziDebug warning: The project's parameters have been changed!");
                listWindow->addMessage("warning","EziDebug warning: Please delete all scan chains and insert the chain again!");


                bool eneededCreateTestBenchFlag = false ;
                QMap<QString,EziDebugScanChain*>::const_iterator i = currentPrj->getScanChainInfo().constBegin();
                while (i != currentPrj->getScanChainInfo().constEnd())
                {
                    EziDebugScanChain * pchain = i.value() ;
                    if(!pchain->getCfgFileName().isEmpty())
                    {
                        eneededCreateTestBenchFlag = true ;
                        break ;
                    }
                }

                if(eneededCreateTestBenchFlag)
                {
                    /*��ʾ�Ƿ���������testbench*/
					#if 0
                    QMessageBox::information(this, QObject::tr("��������"),QObject::tr("ԭ�����д���ɨ����,��ɾ��������֮��������Ӳ�������Ӧ��testBench�ļ�!"));
					#else
					QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("The existed scan chain isn't available, Plesase delete it !"));
					#endif
                }

                delete currentPrj ;
                currentPrj = NULL ;
                EziDebugInstanceTreeItem::setProject(NULL);
                listWindow->clearTreeView();

                currentPrj = new EziDebugPrj(m_proSetWiz->m_uncurrentRegNum,m_proSetWiz->m_icurrentDir,m_proSetWiz->m_ecurrentTool,this);
                currentPrj->setXilinxErrCheckedFlag(m_proSetWiz->m_isXilinxErrChecked);
                //pparent->setCurrentProject(pcurrentPrj);
                UpdateDetectThread * pthread =  currentPrj->getThread() ;
                connect(pthread,SIGNAL(codeFileChanged()),this,SLOT(updateIndicate()));
                connect(currentPrj,SIGNAL(updateProgressBar(int)),this,SLOT(changeProgressBar(int)));

				#if 0
                QMessageBox::information(this, QObject::tr("��������"),QObject::tr("׼�������µĹ��̣�"));
				#else
				QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Ready to parse project!"));
				#endif
				
                listWindow->addMessage("info","EziDebug info: The project is changed !");
                QStandardItem * pitem = listWindow->addMessage("warning",tr("The new project parameter:"));
                listWindow->addMessage("warning",tr("      The  maximum register number of scan chain: %1").arg(m_proSetWiz->m_uncurrentRegNum),pitem);
                listWindow->addMessage("warning",tr("      The current project path: %1").arg(m_proSetWiz->m_icurrentDir),pitem);
                listWindow->addMessage("warning",tr("      The compile software: \"%1\"").arg((m_proSetWiz->m_ecurrentTool == EziDebugPrj::ToolQuartus) ? ("quartus") :("ise")),pitem);

            }
            else
            {
                if(m_proSetWiz->m_uncurrentRegNum != currentPrj->getMaxRegNumPerChain())
                {
                    /*�������ù��̲���*/
                    currentPrj->setMaxRegNumPerChain(m_proSetWiz->m_uncurrentRegNum);
                    listWindow->addMessage("info","EziDebug info: The project parameter is changed !");
                    if(currentPrj->getScanChainInfo().count())
                    {
                        /*��ʾ��������� ����������testbench*/
						#if 0
                        QMessageBox::information(this, QObject::tr("��������"),QObject::tr("������Ĵ��������Ѹ���,֮ǰ����ɨ����������,��ɾ������ɨ������"));
						#else
						QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("The  maximum register number of scan chain has been changed, \n Please delete all scan chain codes inserted before!"));
						#endif
                    }
                }


                if((m_proSetWiz->m_isXilinxErrChecked != currentPrj->getSoftwareXilinxErrCheckedFlag())&&(m_proSetWiz->m_ecurrentTool == EziDebugPrj::ToolIse))
                {
                    /*��ʾ�Ƿ���������testbench*/
					#if 0
                    QMessageBox::warning(this, QObject::tr("��������"),QObject::tr("ע���� Xilinx ������ ���ܻᵼ�� ɨ������ȡ�źŲ���ȷ!"));
					#else
					QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("Note: With Xilinx,information of scan chains may be mistaken!"));
					#endif
                    /**/
                }

                QStandardItem * pitem = listWindow->addMessage("warning",tr("The new project parameters:"));
                listWindow->addMessage("process",tr("      The  maximum register number of scan chain: %1").arg(m_proSetWiz->m_uncurrentRegNum),pitem);
                listWindow->addMessage("process",tr("      The current project path: %1").arg(m_proSetWiz->m_icurrentDir),pitem);
                listWindow->addMessage("process",tr("      The compile software: \"%1\"").arg((m_proSetWiz->m_ecurrentTool == EziDebugPrj::ToolQuartus) ? ("quartus") :("ise")),pitem);

            }

        }
        return ;
    }
}

void ToolWindow::proUpdate()
{
    EziDebugInstanceTreeItem *pitem = NULL ;
    UpdateDetectThread* pthread = NULL ;

    QMap<QString,EziDebugVlgFile*> ivlgFileMap ;
    QMap<QString,EziDebugVhdlFile*> ivhdlFileMap ;
    QList<EziDebugPrj::LOG_FILE_INFO*> iaddedinfoList ;
    QList<EziDebugPrj::LOG_FILE_INFO*> ideletedinfoList ;
    EziDebugPrj::SCAN_TYPE etype = EziDebugPrj::partScanType ;
    QStringList ideletedChainList ;

    if(!currentPrj)
    {   
#if 0
        QMessageBox::information(this, QObject::tr("ȫ������"),QObject::tr("����ָ���Ĺ��̲����ڻ���δ���ù��̲���!"));
#else
        QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("The project is not existed or the project parameter is NULL!"));
#endif
		
        return ;
    }

    // progress

    pthread = currentPrj->getThread() ;
    /*
      1�����¹��� ���� ����ɨ��
      2�������и��µ��ļ�������ɨ��
      3�����Ĺ��̲��� ����ɨ��  = �ؽ����̽���ɨ�� ; ��ʱ����Ҫ ���� ·�� �ԱȽ�� ���жϣ� Ȼ������ǰ���̣��ٺ󴴽��µĹ���
    */

    // �����и��µ��ļ�,����ȫ������
    if((currentPrj->getPrjVlgFileMap().count()!= 0)||(currentPrj->getPrjVhdlFileMap().count()!= 0))
    {
        if(isNeededUpdate)
        {
            listWindow->addMessage("info","EziDebug info: You can continue to update project!");
        }
        else
        {   
#if 0
            QMessageBox::information(this, QObject::tr("ȫ������"),QObject::tr("���ļ��ɸ���!"));
#else
            QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("No file changed in project!"));
#endif
            progressBar->setValue(0);

            return ;
        }

        if(pthread->isRunning())
        {
            pthread->quit();
            pthread->wait();
        }

        // 5%
        progressBar->setValue(5);

        // ���µõ����д����ļ�
        if(currentPrj->parsePrjFile(ivlgFileMap,ivhdlFileMap))
        {
            listWindow->addMessage("error","EziDebug error: parse project file Error!");

            QMap<QString,EziDebugVlgFile*>::iterator i =  ivlgFileMap.begin() ;
            while(i != ivlgFileMap.end())
            {
               EziDebugVlgFile* pfile = ivlgFileMap.value(i.key());
               if(pfile)
               {
                   delete pfile ;
               }
               ++i ;
            }
            ivlgFileMap.clear() ;

            // vhdl file pointer destruct
            QMap<QString,EziDebugVhdlFile*>::iterator j =  ivhdlFileMap.begin() ;
            while(j != ivhdlFileMap.end())
            {
               EziDebugVhdlFile* pfile = ivhdlFileMap.value(j.key());
               if(pfile)
               {
                   delete pfile ;
               }
               ++j ;
            }
            ivhdlFileMap.clear() ;

            delete currentPrj ;
            setCurrentProject(NULL);

            // 0%
#if 0
            QMessageBox::critical(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ڲ�����"));
#else
            QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Update project failed -- The software interior error!"));
#endif

            progressBar->setValue(0);
            return ;
        }
        etype = EziDebugPrj::partScanType ;

        // 10%
        progressBar->setValue(10);

        // check deleted files
        currentPrj->checkDelFile(ivlgFileMap,ivhdlFileMap,ideletedinfoList);

        // clear up the related chainlist last time
        currentPrj->clearupCheckedChainList();
        currentPrj->clearupDestroyedChainList();

        // 15%
        progressBar->setValue(15);

        // scan file all over (can't find the deleted file)
        if(currentPrj->traverseAllCodeFile(etype ,ivlgFileMap , ivhdlFileMap ,iaddedinfoList,ideletedinfoList))
        {
            listWindow->addMessage("error","EziDebug error: traverse code file error !");

            QMap<QString,EziDebugVlgFile*>::iterator i =  ivlgFileMap.begin() ;
            while(i != ivlgFileMap.end())
            {
               EziDebugVlgFile* pfile = ivlgFileMap.value(i.key());
               if(pfile)
               {
                   delete pfile ;
               }
               ++i ;
            }
            ivlgFileMap.clear() ;

            // vhdl file pointer destruct
            QMap<QString,EziDebugVhdlFile*>::iterator j =  ivhdlFileMap.begin() ;
            while(j != ivhdlFileMap.end())
            {
               EziDebugVhdlFile* pfile = ivhdlFileMap.value(j.key());
               if(pfile)
               {
                   delete pfile ;
               }
               ++j ;
            }
            ivhdlFileMap.clear() ;

            delete currentPrj ;
            setCurrentProject(NULL);

            qDeleteAll(iaddedinfoList);
            qDeleteAll(ideletedinfoList);

#if 0
            QMessageBox::critical(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ڲ�����"));
#else
            QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Update project failed -- The software interior error!"));
#endif
            // 0%
            progressBar->setValue(0);

            return ;
        }

        // 60%
        progressBar->setValue(60);

        // update file  map
        currentPrj->updateFileMap(ivlgFileMap,ivhdlFileMap);

        currentPrj->addToMacroMap();
        // �ϵ���״ ͷ�ڵ� �ÿ�
        currentPrj->setInstanceTreeHeadItem(NULL);


        QString itopModule = currentPrj->getTopModule() ;

        if(!currentPrj->getPrjModuleMap().contains(itopModule))
        {
            listWindow->addMessage("error","EziDebug error: There is no top module definition!");
            delete currentPrj ;
            setCurrentProject(NULL);

#if 0
            QMessageBox::critical(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ڲ�����"));
#else
            QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Update project failed -- The software interior error!"));
#endif

            progressBar->setValue(0);

            return ;
        }

        QString itopModuleComboName = itopModule + QObject::tr(":")+ itopModule ;

        // �����µ���״ ͷ�ڵ�
        EziDebugInstanceTreeItem* pnewHeadItem = new EziDebugInstanceTreeItem(itopModule,itopModule);
        if(!pnewHeadItem)
        {
            listWindow->addMessage("error","EziDebug error: There is no memory left!");
            delete currentPrj ;
            setCurrentProject(NULL);

#if 0
            QMessageBox::critical(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ڲ�����"));
#else
            QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Update project failed -- The software interior error!"));
#endif

            progressBar->setValue(0);

            return ;
        }

        // 65%
        progressBar->setValue(65);

        //  ������������ �ڵ�
        if(currentPrj->traverseModuleTree(itopModuleComboName,pnewHeadItem))
        {
            listWindow->addMessage("error","EziDebug error: fast update failed!");

#if 0
            QMessageBox::critical(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ڲ�����"));
#else
            QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Update project failed -- The software interior error!"));
#endif
			
            progressBar->setValue(0);
            return ;
        }

        // 75%
        progressBar->setValue(75);

        currentPrj->setInstanceTreeHeadItem(pnewHeadItem);

        if(currentPrj->getDestroyedChainList().count())
        {
            // �������ƻ���������ӡ����
            QString ichain ;
            QStringList idestroyedChainList = currentPrj->getDestroyedChainList() ;

            listWindow->addMessage("warning","EziDebug warning: Some chains are destroyed!");
            listWindow->addMessage("warning","the destroyed chains are:");
            for(int i = 0 ; i < idestroyedChainList.count() ;i++)
            {
                QString ichainName = idestroyedChainList.at(i) ;

                EziDebugInstanceTreeItem *pitem = currentPrj->getChainTreeItemMap().value(ichainName,NULL);
                if(pitem)
                {
                    ichain.append(tr("EziDebug chain:%1  topInstance:%2:%3").arg(ichainName)\
                                  .arg(pitem->getModuleName()).arg(pitem->getInstanceName())) ;
                }
                listWindow->addMessage("warning",ichain);
            }

            // ɨ�������ƻ� ,��ʾɾ��
            #if 0
            QMessageBox::StandardButton rb = QMessageBox::question(this, tr("����ɨ�������ƻ�"), tr("�Ƿ�ɾ�����ɨ��������"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			#else
			QMessageBox::StandardButton rb = QMessageBox::question(this, tr("EziDebug"), tr("Some scan chains have been destroyed ,\n Do you want to delete all scan chain code ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			#endif
			
            if(rb == QMessageBox::Yes)
            {
                QStringList iunDelChainList = currentPrj->deleteDestroyedChain(iaddedinfoList,ideletedinfoList) ;
                if(iunDelChainList.count())
                {
                    listWindow->addMessage("error","EziDebug error: Some chains can not be deleted for some reasons!");
                    for(int i = 0 ; i < iunDelChainList.count() ;i++)
                    {
                        listWindow->addMessage("error",tr("EziDebug chain:%1").arg(iunDelChainList.at(i)));
                    }
                    listWindow->addMessage("error","EziDebug error: Please check the code file is compiled successfully or not!");
                }

                for(int i = 0 ; i < idestroyedChainList.count() ; i++)
                {
                    QString idestroyedChain = idestroyedChainList.at(i) ;
                    ideletedChainList.append(idestroyedChain);
                    if(!iunDelChainList.contains(idestroyedChain))
                    {
                        struct EziDebugPrj::LOG_FILE_INFO* pdelChainInfo = new EziDebugPrj::LOG_FILE_INFO ;
                        memcpy(pdelChainInfo->ainfoName,idestroyedChain.toAscii().data(),idestroyedChain.size()+1);
                        pdelChainInfo->pinfo = NULL ;
                        pdelChainInfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
                        ideletedinfoList << pdelChainInfo ;
                    }
                }
            }
        }

        QStringList icheckChainList = currentPrj->checkChainExist();

        for(int i = 0 ; i < icheckChainList.count() ;i++)
        {
            QString iupdatedChain = icheckChainList.at(i) ;
            if(!ideletedChainList.contains(iupdatedChain))
            {
                struct EziDebugPrj::LOG_FILE_INFO* pdelChainInfo = new EziDebugPrj::LOG_FILE_INFO ;
                memcpy(pdelChainInfo->ainfoName,iupdatedChain.toAscii().data(),iupdatedChain.size()+1);
                pdelChainInfo->pinfo = NULL ;
                pdelChainInfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
                ideletedinfoList << pdelChainInfo ;

                struct EziDebugPrj::LOG_FILE_INFO* paddChainInfo = new EziDebugPrj::LOG_FILE_INFO ;
                memcpy(paddChainInfo->ainfoName,iupdatedChain.toAscii().data(),iupdatedChain.size()+1);
                paddChainInfo->pinfo = paddChainInfo ;
                paddChainInfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
                iaddedinfoList << paddChainInfo ;
            }
        }



        // 80%
        progressBar->setValue(80);

        if(currentPrj->changedLogFile(iaddedinfoList,ideletedinfoList))
        {
           listWindow->addMessage("info","EziDebug info: save log file failed!");
        }

        qDeleteAll(iaddedinfoList);
        qDeleteAll(ideletedinfoList);

        // 90%
        progressBar->setValue(90);

        currentPrj->cleanupChainTreeItemMap();
        currentPrj->cleanupBakChainTreeItemMap();

        if(currentPrj->getLastOperation() == EziDebugPrj::OperateTypeDelAllScanChain)
        {
            currentPrj->cleanupChainQueryTreeItemMap();
        }
        else
        {
            currentPrj->cleanupBakChainQueryTreeItemMap();
        }

        // ԭ������ ���Ľڵ� ��Ϣ ����
        currentPrj->updateTreeItem(pnewHeadItem);

        if(currentPrj->getLastOperation() == EziDebugPrj::OperateTypeDelAllScanChain)
        {
            // ChainTreeItemMap ����µĽڵ�map
            // �ָ� bakChainTreeItemMap ɾ�� ChainTreeItemMap

            // ChainQueryTreeItemMap ����µĽڵ�map
            // �ָ� bakChainQueryTreeItemMap ɾ�� ChainQueryTreeItemMap
            // update �õ� BakChainQueryTreeItemMap ��ԭʼ�ġ� ChainQueryTreeItemMap �ŵ��µ�
            currentPrj->cleanupBakChainQueryTreeItemMap();
            currentPrj->backupChainQueryTreeItemMap();
            currentPrj->cleanupChainQueryTreeItemMap();
        }
        else
        {
            // update �õ� ChainQueryTreeItemMap ��ԭʼ�ġ� bakChainQueryTreeItemMap ���µ�
            currentPrj->cleanupChainQueryTreeItemMap();
            currentPrj->resumeChainQueryTreeItemMap();
            currentPrj->cleanupBakChainQueryTreeItemMap();
        }


        isNeededUpdate = false ;
        iChangeUpdateButtonTimer->stop();
        updatehintButton->setIcon(QIcon(":/images/update2.png"));
        updatehintButton->setDisabled(true);


        pthread->start();

        emit updateTreeView(pnewHeadItem);

        // 100%
        progressBar->setValue(100);

		#if 0
        QMessageBox::information(this, QObject::tr("ȫ������"),QObject::tr("���´�����ϣ�"));
		#else
		QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Update project finished!"));

		#endif


    }
    else
    {
        /*������蹤���ļ��Ƿ���� 0:���� 1:������*/
        if(!currentPrj->isPrjFileExist())
        {
            listWindow->addMessage("error","EziDebug error: The project file is not exist!");
            delete currentPrj ;
            setCurrentProject(NULL);

#if 0
            QMessageBox::warning(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ָ���Ĺ����ļ������ڣ�"));
#else
            QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("Update Poject failed -- The project file is not existed!"));
#endif

            progressBar->setValue(0);

            return ;
        }
        // 2%
        progressBar->setValue(2);

        if(currentPrj->getCurrentDir().exists("config.ezi"))
        {
            currentPrj->setLogFileExistFlag(true);

            currentPrj->setLogFileName(currentPrj->getCurrentDir().absoluteFilePath("config.ezi")) ;

            if(currentPrj->parsePrjFile(ivlgFileMap,ivhdlFileMap))
            {
                listWindow->addMessage("error","EziDebug error: parse project file failed!");
                delete currentPrj ;
                setCurrentProject(NULL);
				
#if 0
                QMessageBox::critical(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ڲ�����"));
#else
                QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Update Project failed -- The software interior error !"));
#endif
				
                progressBar->setValue(0);

                return ;
            }

            // 5%
            progressBar->setValue(5);

            if(currentPrj->detectLogFile(READ_CHAIN_INFO))
            {   
#if 0
                QMessageBox::StandardButton rb = QMessageBox::question(this, tr("log�ļ��ڲ����ƻ�"), tr("�Ƿ�ɾ���ڲ����ܴ��ڵ�ɨ����������ٽ���ɨ��"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
#else
                QMessageBox::StandardButton rb = QMessageBox::question(this, tr("EziDebug"), tr("EziDebug configuration file contains one or more errors ,\n Do you want to delete all scan chain codes before updating project ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

#endif

                if(rb == QMessageBox::Yes)
                {
                    // ���������ļ���ɾ�����ܴ��ڵġ�EziDebug ����
                    currentPrj->deleteAllEziDebugCode(ivlgFileMap,ivhdlFileMap);

                    QMap<QString,EziDebugScanChain*> ichainMap = currentPrj->getScanChainInfo();
                    QMap<QString,EziDebugScanChain*>::const_iterator k = ichainMap.constBegin() ;
                    while(k != ichainMap.constEnd())
                    {
                        EziDebugScanChain* pchain = k.value() ;
                        if(pchain)
                        {
                            delete pchain ;
                        }
                        ++k ;
                    }
                    currentPrj->cleanupChainMap();

                    // ԭlog�ļ� ��ȫɾ�� ���κ���ǰ����Ϣ
                    currentPrj->setLogFileExistFlag(false);

                }
            }

            // ɾ����ԭlog���ļ�
            QFile ilogFile(currentPrj->getCurrentDir().absoluteFilePath("config.ezi")) ;
            ilogFile.remove();

            // �����Դ��EziDebug_v1.0�ļ��д��ڡ� �ݲ�ɾ��

            // ���¡�������log�ļ�
            if(currentPrj->createLogFile())
            {
                listWindow->addMessage("error","EziDebug error: EziDebug configuration file is failed to create!");
                delete currentPrj ;
                setCurrentProject(NULL);

				#if 0
                QMessageBox::critical(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ڲ�����"));
				#else
				QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Update Project failed -- The software interior error !"));
				#endif
				
                progressBar->setValue(0);

                return  ;
            }

            progressBar->setValue(10);

            etype = EziDebugPrj::fullScanType ;
        }
        else
        {
            // �������򴴽��ļ�
            if(currentPrj->createLogFile())
            {
                listWindow->addMessage("error","EziDebug error: EziDebug configuration file is failed to create!");
                delete currentPrj ;
                setCurrentProject(NULL);

				#if 0
                QMessageBox::critical(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ڲ�����"));
				#else
				QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Update Project failed -- The software interior error !"));
				#endif
				
                progressBar->setValue(0);

                return  ;
            }
            // 5%
            progressBar->setValue(5);

            currentPrj->setLogFileName(currentPrj->getCurrentDir().absoluteFilePath("config.ezi")) ;

            if(currentPrj->parsePrjFile(ivlgFileMap,ivhdlFileMap))
            {
                listWindow->addMessage("error","EziDebug error: parse project file error !");
                QMap<QString,EziDebugVlgFile*>::iterator i =  ivlgFileMap.begin() ;
                while(i != ivlgFileMap.end())
                {
                   EziDebugVlgFile* pfile = ivlgFileMap.value(i.key());
                   if(pfile)
                   {
                       delete pfile ;
                   }
                   ++i ;
                }

                ivlgFileMap.clear() ;

                // vhdl file ָ������
                QMap<QString,EziDebugVhdlFile*>::iterator j =  ivhdlFileMap.begin() ;
                while(j != ivhdlFileMap.end())
                {
                   EziDebugVhdlFile* pfile = ivhdlFileMap.value(j.key());
                   if(pfile)
                   {
                       delete pfile ;
                   }
                   ++j ;
                }
                ivhdlFileMap.clear() ;

                delete currentPrj ;
                setCurrentProject(NULL);
				
#if 0
                QMessageBox::critical(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ڲ�����"));
#else
                QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Update Project failed -- The software interior error !"));
#endif
				
                progressBar->setValue(0);

                return ;
            }
            // 10%
            progressBar->setValue(10);

            etype = EziDebugPrj::partScanType ;
        }

#if 0
        QMessageBox::information(this, QObject::tr("ɨ�����д����ļ�"),QObject::tr("׼����ȡmodule��Ϣ"));
#else
        QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("ready to get module information!"));
#endif

        if(currentPrj->traverseAllCodeFile(etype , ivlgFileMap , ivhdlFileMap ,iaddedinfoList,ideletedinfoList))
        {
            qDeleteAll(iaddedinfoList) ;
            qDeleteAll(ideletedinfoList) ;
            iaddedinfoList.clear();
            ideletedinfoList.clear();

            listWindow->addMessage("error","EziDebug error: traverse code file failed !");
            QMap<QString,EziDebugVlgFile*>::iterator i =  ivlgFileMap.begin() ;
            while(i != ivlgFileMap.end())
            {
                EziDebugVlgFile* pfile = ivlgFileMap.value(i.key());
                if(pfile)
                {
                    delete pfile ;
                }
                ++i ;
            }
            ivlgFileMap.clear() ;

            // vhdl file ָ������
            QMap<QString,EziDebugVhdlFile*>::iterator j =  ivhdlFileMap.begin() ;
            while(j != ivhdlFileMap.end())
            {
                EziDebugVhdlFile* pfile = ivhdlFileMap.value(j.key());
                if(pfile)
                {
                    delete pfile ;
                }
                ++j ;
            }
            ivhdlFileMap.clear() ;


            delete currentPrj ;
            setCurrentProject(NULL);


            qDeleteAll(iaddedinfoList);
            qDeleteAll(ideletedinfoList);

#if 0
            QMessageBox::critical(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ڲ�����"));
#else
            QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Project Update failed -- The software interior error !"));
#endif
            progressBar->setValue(0);

            return ;
        }


        currentPrj->updateFileMap(ivlgFileMap,ivhdlFileMap);

        currentPrj->addToMacroMap() ;
        listWindow->addMessage("info","EziDebug info: ready to traverse instances tree !");

        progressBar->setValue(65);

        if(currentPrj->getScanChainInfo().count())
        {
            currentPrj->backupChainMap();
            currentPrj->cleanupBakChainTreeItemMap();
            currentPrj->cleanupChainTreeItemMap();
        }

        if(currentPrj->generateTreeView())
        {
            listWindow->addMessage("error","EziDebug error: traverse instances tree error !");
            delete currentPrj ;
            setCurrentProject(NULL);
			
            #if 0
            QMessageBox::critical(this, QObject::tr("ȫ������ʧ��"),QObject::tr("����ڲ�����"));
			#else
			QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr(" Project Update failed -- The software interior error !"));
			#endif
			
            progressBar->setValue(0);

            return ;
        }

        progressBar->setValue(70);

        if(currentPrj->getBackupChainMap().count())
        {
            currentPrj->resumeChainMap();
            currentPrj->resumeChainTreeItemMap();
            // ������� ���ݵ� map
            currentPrj->updateOperation(EziDebugPrj::OperateTypeNone,NULL,NULL);
        }

        listWindow->addMessage("info","EziDebug info: finishing traverse instances tree !");

        pitem = currentPrj->getInstanceTreeHeadItem() ;

        EziDebugInstanceTreeItem::setProject(currentPrj);

        progressBar->setValue(75);

        // (�����е�ɨ���� �� log�ļ��в�����  ˵��log�ļ��������ƻ���!)
        if(currentPrj->getLogFileExistFlag()&&currentPrj->getLogfileDestroyedFlag())
        {
            // ��ʾɾ��������
            #if 0
            QMessageBox::StandardButton rb = QMessageBox::question(this, tr("����ɨ������Ϣ��ʧ"), tr("�Ƿ�ɾ������ɨ��������"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			#else
			QMessageBox::StandardButton rb = QMessageBox::question(this, tr("EziDebug"), tr("Some scan chain information have been lost, Do you want to delete all scan chain code ?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			#endif
			
            if(rb == QMessageBox::Yes)
            {
                // ɾ������������
                currentPrj->deleteAllEziDebugCode(ivlgFileMap,ivhdlFileMap);
                // ɾ����������ص���Ϣ
                QMap<QString,EziDebugScanChain*> ichainMap = currentPrj->getScanChainInfo();
                QMap<QString,EziDebugScanChain*>::const_iterator k = ichainMap.constBegin() ;
                while(k != ichainMap.constEnd())
                {
                    EziDebugScanChain* pchain = k.value() ;
                    if(pchain)
                    {
                        delete pchain ;
                        pchain = NULL ;
                    }
                    ++k ;
                }
                currentPrj->cleanupChainMap();
                currentPrj->cleanupChainTreeItemMap();
                currentPrj->cleanupChainQueryTreeItemMap();

                // ɾ��log��������Ϣ (�µ�log�ļ����Բ��������Ϣ,�˲�������)

                // ��� ���ƻ���ɨ����list
                currentPrj->clearupDestroyedChainList();

            }
            else
            {
                // ������
            }

        }

        progressBar->setValue(80);


        if(currentPrj->getDestroyedChainList().count())
        {
            // �������ƻ���������ӡ����
            QString ichain ;
            QStringList idestroyedChain = currentPrj->getDestroyedChainList() ;

            listWindow->addMessage("warning","EziDebug warning: Some chains are destroyed!");
            listWindow->addMessage("warning","the chain :");
            for(int i = 0 ; i < idestroyedChain.count() ;i++)
            {
                QString ichainName = idestroyedChain.at(i) ;

                EziDebugInstanceTreeItem *pitem = currentPrj->getChainTreeItemMap().value(ichainName);
                if(pitem)
                {
                    ichain.append(tr("EziDebug chain:%1  topInstance:%2:%3").arg(ichainName)\
                                  .arg(pitem->getModuleName()).arg(pitem->getInstanceName())) ;
                }
                listWindow->addMessage("warning",ichain);
            }

            // ɨ�������ƻ� ,��ʾɾ��
            #if 0
            QMessageBox::StandardButton rb = QMessageBox::question(this, tr("����ɨ�������ƻ�"), tr("�Ƿ�ɾ�����ɨ��������"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            #else
			QMessageBox::StandardButton rb = QMessageBox::question(this, tr("EziDebug"), tr("Some scan chains have been destroyed ,Do you want to delete all the scan chain code?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			#endif
			
			if(rb == QMessageBox::Yes)
            {
                QStringList iunDelChainList = currentPrj->deleteDestroyedChain(iaddedinfoList,ideletedinfoList) ;
                if(iunDelChainList.count())
                {
                    listWindow->addMessage("error","EziDebug error: Some chains can not be deleted for some reasons!");
                    for(int i = 0 ; i < iunDelChainList.count() ;i++)
                    {
                        listWindow->addMessage("error",tr("EziDebug chain:%1").arg(iunDelChainList.at(i)));
                    }
                    listWindow->addMessage("error","EziDebug error: Please check the code file is compiled successfully or not!");
                }
            }
        }
        progressBar->setValue(85);


        QMap<QString,EziDebugScanChain*>::const_iterator iaddedChainIter = currentPrj->getScanChainInfo().constBegin() ;
        while(iaddedChainIter !=  currentPrj->getScanChainInfo().constEnd())
        {
           QString ichainName = iaddedChainIter.key();
           struct EziDebugPrj::LOG_FILE_INFO* pinfo = new EziDebugPrj::LOG_FILE_INFO ;
           memcpy(pinfo->ainfoName ,ichainName.toAscii().data(),ichainName.size()+1);
           pinfo->pinfo = iaddedChainIter.value() ;
           pinfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
           iaddedinfoList << pinfo ;
           ++iaddedChainIter ;
        }

        // modify the project file at last
        currentPrj->preModifyPrjFile();

        progressBar->setValue(90);

        if(currentPrj->changedLogFile(iaddedinfoList,ideletedinfoList))
        {
            qDebug() << QObject::tr("����log�ļ�����!");
        }

        qDeleteAll(iaddedinfoList);
        qDeleteAll(ideletedinfoList);

        pthread->start();

        emit updateTreeView(pitem);

        progressBar->setValue(100);

		#if 0
        QMessageBox::information(this, QObject::tr("ȫ������"),QObject::tr("���´�����ϣ�"));
		#else
		QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Project update  finished!"));
		#endif

    }
    progressBar->setValue(0);

}


int ToolWindow::deleteScanChain()
{
    EziDebugInstanceTreeItem * ptreeItem = NULL ;
    EziDebugScanChain *pchain = NULL ;
    UpdateDetectThread* pthread = NULL ;
    QList<EziDebugPrj::LOG_FILE_INFO*> iaddedinfoList ;
    QList<EziDebugPrj::LOG_FILE_INFO*> ideletedinfoList ;

    // �Ƿ���Ҫ����
    if(isNeededUpdate)
    {
        // ��ʾ��Ҫ ����ٸ��º��ٽ��� ����
        #if 0
        QMessageBox::information(this, QObject::tr("ɾ��ɨ����"),QObject::tr("��⵽���ļ�������,����º��ٽ���ɾ��"));
		#else
		QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Please update project before deleting scan chain !"));
		#endif

        return 0 ;
    }
    else
    {
        //
        UpdateDetectThread *pthread = currentPrj->getThread();
        pthread->update() ;
        if(isNeededUpdate)
        {   
            #if 0
            QMessageBox::information(this, QObject::tr("ɾ��ɨ����"),QObject::tr("��⵽���ļ�������,����º��ٽ���ɾ��"));
			#else
			QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Please update project before deleting scan chain !"));
			#endif
			
            return 0 ;
        }
    }


    ptreeItem = listWindow->getCurrentTreeItem();
    if(!ptreeItem)
    {   
        #if 0
        QMessageBox::critical(this, QObject::tr("ɾ��ɨ����"),QObject::tr("����ڲ�����"));
		#else
		QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Delete scan chain failed -- The software interior error!"));
		#endif
		
        return 1;
    }

    pchain = ptreeItem->getScanChainInfo();
    if(!pchain)
    {   
        #if 0
        QMessageBox::critical(this, QObject::tr("ɾ��ɨ����"),QObject::tr("����ڲ�����"));
		#else 
		QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Delete scan chain failed -- The software interior error!"));
		#endif
		
        return 1;
    }


    pchain->backupFileList();

    pchain->clearupFileList();


    // ֹͣ������   �˳��߳�
    pthread = currentPrj->getThread() ;
    if(pthread->isRunning())
    {
        pthread->quit();
        pthread->wait();
    }

    if(!ptreeItem->deleteScanChain(EziDebugPrj::OperateTypeDelSingleScanChain))
    {
        /*����һ�����������ƺ�*/
        if(currentPrj->eliminateLastOperation())
        {
            listWindow->addMessage("error","EziDebug error: delete last chain error!");
			
			#if 0
            QMessageBox::critical(this, QObject::tr("ɾ��ɨ����"),QObject::tr("����ڲ�����"));
			#else
			QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Delete scan chain failed -- The software interior error!"));

			#endif

            return  1 ;
        }

        /*������һ������*/
        currentPrj->updateOperation(EziDebugPrj::OperateTypeDelSingleScanChain,pchain,ptreeItem);

        /*��ɨ���� �� �� map ȡ����*/
        currentPrj->eliminateChainFromMap(pchain->getChainName());

        /*��ɨ���� �� ��״�ڵ� map ȡ����*/
        currentPrj->eliminateTreeItemFromMap(pchain->getChainName());

        /*���ýڵ��� ��ָ��*/
        ptreeItem->setScanChainInfo(NULL);

        /*ɾ�����ڲ�ѯ�� map*/
        currentPrj->eliminateTreeItemFromQueryMap(ptreeItem->getNameData());

        /*�Ķ� log �ļ� */
        struct EziDebugPrj::LOG_FILE_INFO* pdelChainInfo = new EziDebugPrj::LOG_FILE_INFO ;
        memcpy(pdelChainInfo->ainfoName,pchain->getChainName().toAscii().data(),pchain->getChainName().size()+1);
        pdelChainInfo->pinfo = NULL ;
        pdelChainInfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
        ideletedinfoList << pdelChainInfo ;

        QStringList iscanFileList = pchain->getScanedFileList() ;

        for(int i = 0 ; i < iscanFileList.count() ; i++)
        {
            QString ifileName = iscanFileList.at(i) ;
            QString irelativeFileName = currentPrj->getCurrentDir().relativeFilePath(ifileName);

            // �ļ����޸��� ��Ҫ���±����ļ�����
            struct EziDebugPrj::LOG_FILE_INFO* pdelFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
            pdelFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;
            pdelFileInfo->pinfo = NULL ;
            memcpy(pdelFileInfo->ainfoName , irelativeFileName.toAscii().data() , irelativeFileName.size()+1);
            ideletedinfoList.append(pdelFileInfo);

            struct EziDebugPrj::LOG_FILE_INFO* paddFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
            paddFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;


            if(irelativeFileName.endsWith(".v"))
            {
                EziDebugVlgFile *pvlgFile = currentPrj->getPrjVlgFileMap().value(irelativeFileName,NULL);
                paddFileInfo->pinfo = pvlgFile ;
            }
            else if(irelativeFileName.endsWith(".vhd"))
            {
                EziDebugVhdlFile *pvhdlFile = currentPrj->getPrjVhdlFileMap().value(irelativeFileName,NULL);
                paddFileInfo->pinfo = pvhdlFile ;
            }
            else
            {
                delete paddFileInfo ;
                continue ;
            }

            memcpy(paddFileInfo->ainfoName , irelativeFileName.toAscii().data(), irelativeFileName.size()+1);
            iaddedinfoList.append(paddFileInfo);
        }


        if(currentPrj->changedLogFile(iaddedinfoList,ideletedinfoList))
        {
            // ��ʾ ���� log �ļ�����
            qDebug() << tr("����log�ļ�����") ;
        }

//        // �������ĸ��� ɾ���������Զ���core�ļ�
//        if(!currentPrj->getScanChainInfo().count())
//        {
//            // ɾ�� �������ļ���
//            QDir idir(currentPrj->getCurrentDir().absolutePath() + EziDebugScanChain::getUserDir());
//            idir.remove(EziDebugScanChain::getChainRegCore()+tr(".v"));
//            idir.remove(EziDebugScanChain::getChainToutCore()+tr(".v"));
//            currentPrj->getCurrentDir().rmdir(idir.absolutePath());
//        }


        qDeleteAll(ideletedinfoList);
        qDeleteAll(iaddedinfoList);

        pthread->start();

		#if 0
        QMessageBox::information(this, QObject::tr("ɾ��ɨ����"),QObject::tr("ɾ��ɨ�����ɹ���"));
		#else
		QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Delete scan chain successfully!"));

		#endif

    }
    else
    {
        /*��ȡɾ���� �Ѿ�ɨ������ļ�,���Ѿ����ݵ��ļ����лָ�*/
        for(int i = 0 ; i < pchain->getScanedFileList().count();i++)
        {
            // ��ȡ���ݵ��ļ���ȫ��
            QString ifileName = pchain->getScanedFileList().at(i) ;
            QFileInfo ifileInfo(pchain->getScanedFileList().at(i));
            QString ieziDebugFileSuffix ;
            ieziDebugFileSuffix.append(QObject::tr(".delete.%1").arg(currentPrj->getLastOperateChain()->getChainName()));

            QString ibackupFileName = currentPrj->getCurrentDir().absolutePath() \
                    + EziDebugScanChain::getUserDir() + ifileInfo.fileName() \
                    + ieziDebugFileSuffix;
            QFile ibackupFile(ibackupFileName) ;

            QFileInfo ibakfileInfo(ibackupFileName);
            QDateTime idateTime = ibakfileInfo.lastModified();
            // �Ѿ��Ǿ���·����

            // ����ʱ��
            QString irelativeName = currentPrj->getCurrentDir().relativeFilePath(ifileName) ;

            if(ibakfileInfo.exists())
            {
                if(ifileName.endsWith(".v"))
                {
                    currentPrj->getPrjVlgFileMap().value(irelativeName)->remove();
                    ibackupFile.copy(ifileName);
                    currentPrj->getPrjVlgFileMap().value(irelativeName)->modifyStoredTime(idateTime);
                }
                else if(ifileName.endsWith(".vhd"))
                {
                    currentPrj->getPrjVlgFileMap().value(irelativeName)->remove();
                    ibackupFile.copy(ifileName);
                    currentPrj->getPrjVlgFileMap().value(irelativeName)->modifyStoredTime(idateTime);
                }
                else
                {
                    // do nothing
                }
                // ɾ����ǰ���ݵ��ļ�
                ibackupFile.remove();
            }
        }

        pchain->resumeFileList();

        qDeleteAll(ideletedinfoList);
        qDeleteAll(iaddedinfoList);

        pthread->start();

		#if 0
        QMessageBox::warning(this, QObject::tr("ɾ��ɨ����"),QObject::tr("ɾ��ɨ����ʧ�ܣ�"));
		#else
		QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("Delete scan chain failed!"));
		#endif

        return 1;
    }

    return 0 ;
}

void ToolWindow::addScanChain()
{
    EziDebugInstanceTreeItem * ptreeItem = NULL ;
    EziDebugModule* pmodule = NULL ;
    UpdateDetectThread* pthread = NULL ;
    QList<EziDebugPrj::LOG_FILE_INFO*> iaddedinfoList ;
    QList<EziDebugPrj::LOG_FILE_INFO*> ideletedinfoList ;
    QMap<QString,int> iregNumMap ;
    bool isrepeatFlag = false ;
    QString ichainName = tr("chn") ;
    int nresult = 0 ;
    int i = 0 ;


    // �Ƿ���Ҫ����
    if(isNeededUpdate)
    {
        // ��ʾ��Ҫ ����ٸ��º��ٽ��� ����
        #if 0
        QMessageBox::information(0, QObject::tr("���ɨ����"),QObject::tr("��⵽���ļ�������,����º��ٽ��������!"));
        #else
		QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Please update project before inserting scan chain !"));
		#endif
		
        return ;
    }
    else
    {
        //
        UpdateDetectThread *pthread = currentPrj->getThread();
        pthread->update() ;
        if(isNeededUpdate)
        {   
            #if 0
            QMessageBox::information(0, QObject::tr("���ɨ����"),QObject::tr("��⵽���ļ�������,����º��ٽ��������!"));
			#else
		    QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Please update project before inserting scan chain !"));
			#endif
			
            return ;
        }
    }

    ptreeItem = listWindow->getCurrentTreeItem();
    if(!ptreeItem)
    {
        /*���ı�����ʾ ��������� �ýڵ㲻����*/
		
        listWindow->addMessage("error","EziDebug error: The tree item is not exist!");

		#if 0
        QMessageBox::critical(this, QObject::tr("���ɨ����"),QObject::tr("����ڲ�����!"));
		#else
		QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Insert scan chain failed -- The software interior error!"));
		#endif

        return ;
    }

    if(currentPrj->getPrjModuleMap().contains(ptreeItem->getModuleName()))
    {
        pmodule =  currentPrj->getPrjModuleMap().value(ptreeItem->getModuleName());
        if(!pmodule)
        {
            /*���ı�����ʾ ��������� �ýڵ��Ӧ��module�޶���*/
            listWindow->addMessage("error", tr("EziDebug error: the module:%1 object is null !").arg(ptreeItem->getModuleName()));
			
			#if 0
            QMessageBox::critical(this, QObject::tr("���ɨ����"),QObject::tr("����ڲ�����!"));
			#else
			QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Insert scan chain failed -- The software interior error!"));
			#endif

            return ;
        }
    }
    else
    {
        /*���ı�����ʾ ��������� �ýڵ��Ӧ��module�޶���*/
        listWindow->addMessage("error", tr("EziDebug error: the module:").arg(ptreeItem->getModuleName()) + tr("has no definition!"));

		#if 0
        QMessageBox::critical(this, QObject::tr("���ɨ����"),QObject::tr("����ڲ�����!"));
		#else
		QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Insert scan chain failed -- The software interior error!"));
		#endif

        return ;
    }
    //QString iinstanceName = ptreeItem->getInstanceName() ;

    /*�ж�ɨ���������Ƿ��ظ�*/
    isrepeatFlag = currentPrj->getScanChainInfo().contains(ichainName) ;
    while(isrepeatFlag)
    {
        ichainName = tr("chn") + tr("%1").arg(i) ;
        isrepeatFlag = currentPrj->getScanChainInfo().contains(ichainName) ;
        i++ ;
    }

    if((pmodule->getLocatedFileName()).endsWith(".v"))
    {
        QMap<QString,EziDebugInstanceTreeItem::SCAN_CHAIN_STRUCTURE *> ichainListStructureMap ;
        int nmaxRegNum = currentPrj->getMaxRegNumPerChain() ;
        EziDebugVlgFile* pvlgFile =  currentPrj->getPrjVlgFileMap().value(pmodule->getLocatedFileName());
//      EziDebugInstanceTreeItem *pparent = ptreeItem->parent();
        EziDebugModule* pmodule = currentPrj->getPrjModuleMap().value(ptreeItem->getModuleName()) ;
        EziDebugScanChain * pscanChain = new EziDebugScanChain(ichainName);
        pscanChain->traverseAllInstanceNode(ptreeItem) ;
        pscanChain->traverseChainAllReg(ptreeItem) ;
        QMap<QString,QString>::const_iterator i =  pmodule->getClockSignal().constBegin() ;

        //1��������״�ṹ �Ȳ����м���clock �ֱ����ÿ�� clock ���� �Ĵ��� ����,
        // ���Ҽ� ѡ��� ����ڵ㿪ʼ ����һ������
        while(i != pmodule->getClockSignal().constEnd())
        {
            //QString iclock = pparent->getModuleClockMap().value(i.key());
            int nbitWidth  = 0  ;
            /*��õ�ǰ�ڵ��clock����*/
            int nregBitCount = 0 ;
            ptreeItem->getAllRegNum(i.key() ,ichainName, nregBitCount , nbitWidth , pscanChain->getInstanceItemList());
            if((!nregBitCount) && (!nbitWidth))
            {
                listWindow->addMessage("warning", tr("EziDebug warning: There is no register with clock:%1 in the chain:%2").arg(i.key()).arg(ichainName));
                ++i;
                continue ;
            }
            iregNumMap.insert(i.key(),nbitWidth*currentPrj->getMaxRegNumPerChain()+nregBitCount);
            nbitWidth++ ;
            
            // �������õ� ��������� �õ�Ҫ ÿ�� clock Ҫ ����� TDI��TDO λ��  ,���뵽��Ӧ�� map ��

            EziDebugInstanceTreeItem::SCAN_CHAIN_STRUCTURE *pchainStructure = (EziDebugInstanceTreeItem::SCAN_CHAIN_STRUCTURE *)operator new(sizeof(EziDebugInstanceTreeItem::SCAN_CHAIN_STRUCTURE));
            pchainStructure->m_uncurrentChainNumber = 0 ;
            pchainStructure->m_untotalChainNumber = nbitWidth ;
            pchainStructure->m_unleftRegNumber = nmaxRegNum ;


            ichainListStructureMap.insert(i.key(),pchainStructure);
            ++i ;

        }

        if(!ichainListStructureMap.count())
        {
            listWindow->addMessage("warning", tr("EziDebug warning: There is no register in the chain!"));
            delete pscanChain ;
			
			#if 0
            QMessageBox::warning(this, QObject::tr("���ɨ����"),QObject::tr("���ɨ����ʧ��!"));
			#else
			QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("Insert scan chain failed!"));
			#endif

            return  ;
        }


        QMap<QString,EziDebugInstanceTreeItem::SCAN_CHAIN_STRUCTURE *>::const_iterator iscanchain = ichainListStructureMap.constBegin();
        while(iscanchain != ichainListStructureMap.constEnd())
        {
            // parent clock(key) -> child clock(value)
            
            QString iclock = ptreeItem->parent()->getModuleClockMap(ptreeItem->getInstanceName()).key(iscanchain.key(),QString());
            if(iclock.isEmpty())
            {
                listWindow->addMessage("warning", tr("EziDebug warning: can't find the module %1 's clock ,insert scan chain failed !").arg(ptreeItem->getModuleName()));
                delete pscanChain ;
				
				#if 0
                QMessageBox::critical(this, QObject::tr("���ɨ����"),QObject::tr("���ɨ����ʧ��!"));
				#else
				QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Insert scan chain failed!"));
				#endif

                return ;
            }
            EziDebugInstanceTreeItem::SCAN_CHAIN_STRUCTURE * pchainSt = iscanchain.value();
            pscanChain->setChildChainNum(iclock,pchainSt->m_untotalChainNumber);
            ++iscanchain ;
        }

        //2��������Ӧ���û�module core
        // ����ǵ�һ�μ���ɨ���� ����Ҫ����  ���� ��ɾ������ ����������Զ��� module
        if(!currentPrj->getScanChainInfo().count())
        {
            if(pvlgFile->createUserCoreFile(currentPrj))
            {
                delete pscanChain ;
                listWindow->addMessage("warning", tr("EziDebug error: create EziDebug core error!"));
				
				#if 0
                QMessageBox::critical(this, QObject::tr("���ɨ����"),QObject::tr("����ڲ�����!"));
				#else
				QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Insert scan chain failed -- The software interior error!"));
				#endif

                return ;
            }
        }
        else
        {
            /*��ɾ�� 1��ʹ�ù����� 2��û��ʹ��ʱ��ɾ������  ��ɨ��log�ļ�ʱ��û����Ӧ��*/
        }

        /*������״�������� ÿ��module ��ɨ�����������Ĵ���*/
        //ptreeItem->traverseChainTreeItem();

        /*�ڲ���ɨ����֮ǰ ���ɨ������ļ��б�*/
        pscanChain->clearupFileList();

        // ����ɨ����֮ǰ  ֹͣ������   �˳��߳�
        pthread = currentPrj->getThread() ;
        if(pthread->isRunning())
        {
            pthread->quit();
            pthread->wait();
        }



        // m_pheadItem
        pscanChain->setHeadTreeItem(ptreeItem);

        if(!(nresult = ptreeItem->insertScanChain(ichainListStructureMap,pscanChain,ptreeItem->getInstanceName())))
        {
            if(currentPrj->eliminateLastOperation())
            {
                listWindow->addMessage("error", tr("EziDebug error: delete last chain error!"));

				#if 0
                QMessageBox::critical(this, QObject::tr("���ɨ����"),QObject::tr("����ڲ�����!"));
				#else
	            QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Insert scan chain failed -- The software interior error!"));
				#endif

                return ;
            }

            /*������һ������*/
            currentPrj->updateOperation(EziDebugPrj::OperateTypeAddScanChain,pscanChain,ptreeItem);

            /*������ӵ� ɨ�������� �� prj �� �� map ��*/
            currentPrj->addToChainMap(pscanChain);

            /*���뵽 ��״�ڵ� map ��*/
            currentPrj->addToTreeItemMap(pscanChain->getChainName(),ptreeItem);

            /*���뵽 ���ڲ�ѯ �� �ڵ� map ��*/
            currentPrj->addToQueryItemMap(ptreeItem->getNameData(),ptreeItem);

            /* ���� �ڵ��� ��ָ�� */
            ptreeItem->setScanChainInfo(pscanChain);

            /*�������ʱ�ӼĴ��� ���� ��Ϣ*/
            QMap<QString,int>::const_iterator iclkRegNumIter = iregNumMap.constBegin();
            while(iclkRegNumIter != iregNumMap.constEnd())
            {
                QString ichainClock = pscanChain->getChainClock(ptreeItem->getInstanceName(),iclkRegNumIter.key());
                pscanChain->setChainRegCount(ichainClock,iclkRegNumIter.value());
                ++iclkRegNumIter ;
            }

            struct EziDebugPrj::LOG_FILE_INFO* pinfo = new EziDebugPrj::LOG_FILE_INFO ;
            memcpy(pinfo->ainfoName ,pscanChain->getChainName().toAscii().data(),pscanChain->getChainName().size()+1);
            pinfo->pinfo = pscanChain ;
            pinfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
            iaddedinfoList << pinfo ;

            pscanChain->removeScanedFileListDuplicate();
            for(int i = 0 ; i < pscanChain->getScanedFileList().count() ; i++)
            {
                // �ļ����޸��� ��Ҫ���±����ļ�����

                QString ifileName = pscanChain->getScanedFileList().at(i) ;

                struct EziDebugPrj::LOG_FILE_INFO* pdelFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
                pdelFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;
                pdelFileInfo->pinfo = NULL ;

                qstrcpy(pdelFileInfo->ainfoName,currentPrj->getCurrentDir().relativeFilePath(ifileName).toAscii().data()) ;
                ideletedinfoList.append(pdelFileInfo);

                struct EziDebugPrj::LOG_FILE_INFO* paddFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
                paddFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;

                if(ifileName.endsWith(".v"))
                {
                    paddFileInfo->pinfo = currentPrj->getPrjVlgFileMap().value(currentPrj->getCurrentDir().relativeFilePath(ifileName)) ;
                }
                else if(ifileName.endsWith(".vhd"))
                {
                    paddFileInfo->pinfo = currentPrj->getPrjVhdlFileMap().value(currentPrj->getCurrentDir().relativeFilePath(ifileName)) ;
                }
                else
                {
                    qDeleteAll(iaddedinfoList);
                    qDeleteAll(ideletedinfoList);
                    pthread->start();

                    listWindow->addMessage("warning", tr("EziDebug warning: detect the unknown file!"));
                    continue ;
                }
                qstrcpy(pdelFileInfo->ainfoName,currentPrj->getCurrentDir().relativeFilePath(ifileName).toAscii().data()) ;
                iaddedinfoList.append(paddFileInfo);
            }


            if(currentPrj->changedLogFile(iaddedinfoList,ideletedinfoList))
            {
                qDebug() << tr("Warnning:addScanChain Save Log File Error!");
            }

            qDeleteAll(iaddedinfoList);
            qDeleteAll(ideletedinfoList);

            listWindow->addMessage("info","EziDebug info: Create a new scan chain!");
            QStandardItem * pitem = listWindow->addMessage("info",tr("The New ScanChain Parameter:"));
            listWindow->addMessage("process" , tr("      The chain name: %1").arg(pscanChain->getChainName()),pitem);
            listWindow->addMessage("process" , tr("      The chain topNode: %1").arg(ptreeItem->getNameData()),pitem);
            QString iclockNumStr ;
            QString itraversedInstStr ;
            QMap<QString,QVector<QStringList> > iregChain = pscanChain->getRegChain();
            QMap<QString,QVector<QStringList> >::const_iterator iregChainIter = iregChain.constBegin() ;
            while( iregChainIter != iregChain.constEnd())
            {
                iclockNumStr.append(tr("%1 (%2)     ").arg(pscanChain->getChainRegCount(iregChainIter.key())).arg(iregChainIter.key()));
                ++iregChainIter ;
            }

            listWindow->addMessage("process" , tr("      The total register number of chain: %1").arg(iclockNumStr),pitem);

            listWindow->addMessage("process" , tr("      The traversed NodeList:"),pitem);

            for(int j = 0 ; j < pscanChain->getInstanceItemList().count() ;j++)
            {
                if(j == 0)
                {
                    itraversedInstStr.append("  ->  ");
                }
                if(j == (pscanChain->getInstanceItemList().count()-1))
                {
                    itraversedInstStr.append(pscanChain->getInstanceItemList().at(j)) ;
                }
                else
                {
                    itraversedInstStr.append(pscanChain->getInstanceItemList().at(j) + tr("  ->  ")) ;
                }
                if((j+1)%3 == 0)
                {
                    listWindow->addMessage("process" , tr("      ") + itraversedInstStr,pitem);
                    itraversedInstStr.clear();
                }
            }

            if(!itraversedInstStr.isEmpty())
            {
                listWindow->addMessage("process" , tr("      ") + itraversedInstStr,pitem);
            }

            /*����ɨ�����ɹ��� ��ȥʹ�� ��״�ڵ�� �Ҽ��˵��� ���������*/
//          listWindow->m_paddChainAct->setEnabled(false);
//          listWindow->m_pdeleteChainAct->setEnabled(true);

            // �򹤳��ļ��м��� ����ӵ��ļ�
            // "EziDebug_1.0/_EziDebug_ScanChainReg.v"
            // "EziDebug_1.0/_EziDebug_TOUT_m.v"
            // �����޸� restore �ļ���Ч  �����ֹ���� �����ļ�
            nresult = currentPrj->chkEziDebugFileInvolved() ;
            if(nresult == 0)
            {   
                #if 0
                QMessageBox::information(this , QObject::tr("EziDebug") , QObject::tr("�뽫��ǰ����·����EziDebug_v1.0Ŀ¼�µ��ļ����뵽������!"));
				#else
				QMessageBox::information(this , QObject::tr("EziDebug") , QObject::tr("Please add the source files under \"EziDebug_v1.0\" directory to your project"
				                                                                      "Before you synthetize the project!"));
				#endif
            }
            else if(nresult == -1)
            {
                listWindow->addMessage("error" , tr("EziDebug Error:Please check project file!"));
            }
            else
            {
                //
            }


            // ���� signaltap �ļ� ���� cdc �ļ�
            currentPrj->createCfgFile(ptreeItem);

            listWindow->addMessage("info",tr("EziDebug info: Before you synthetize the project ,Please "
                                             "add the source files under \"EziDebug_v1.0\" directory to your project!"));

            //qDebug() << "add Scan Chain Success !";

            pthread->start();

			#if 0
            QMessageBox::information(this, QObject::tr("���ɨ����"),QObject::tr("���ɨ�����ɹ�!"));
			#else
			QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Insert scan chain successfully!"));
			#endif

        }
        else
        {

            /*��ȡɾ���� �Ѿ�ɨ������ļ�,���Ѿ����ݵ��ļ��лָ�*/
            for(int i = 0 ; i < pscanChain->getScanedFileList().count();i++)
            {
                // ��ȡ���ݵ��ļ���ȫ��
                QString ifileName = pscanChain->getScanedFileList().at(i) ;
                QFileInfo ifileInfo(ifileName);
                QString ieziDebugFileSuffix ;

                ieziDebugFileSuffix.append(QObject::tr(".add.%1").arg(pscanChain->getChainName()));

                QString ibackupFileName = currentPrj->getCurrentDir().absolutePath() \
                        + EziDebugScanChain::getUserDir()+ tr("/")+ ifileInfo.fileName() \
                        + ieziDebugFileSuffix;
                QFile ibackupFile(ibackupFileName) ;
                QFileInfo ibakfileInfo(ibackupFileName);
                QDateTime idateTime = ibakfileInfo.lastModified();
                // �Ѿ��Ǿ���·����

                // ����ʱ��
                QString irelativeName = currentPrj->getCurrentDir().relativeFilePath(ifileName) ;

                if(ibakfileInfo.exists())
                {
                    if(ifileName.endsWith(".v"))
                    {
                        currentPrj->getPrjVlgFileMap().value(irelativeName)->remove();
                        ibackupFile.copy(ifileName);
                        currentPrj->getPrjVlgFileMap().value(irelativeName)->modifyStoredTime(idateTime);
                    }
                    else if(ifileName.endsWith(".vhd"))
                    {
                        currentPrj->getPrjVlgFileMap().value(irelativeName)->remove();
                        ibackupFile.copy(ifileName);
                        currentPrj->getPrjVlgFileMap().value(irelativeName)->modifyStoredTime(idateTime);
                    }
                    else
                    {
                        // do nothing
                    }
                    // ɾ����ǰ���ݵ��ļ�
                    ibackupFile.remove();
                }
            }

            pscanChain->setHeadTreeItem(NULL);

            /*ɾ���´�����ɨ����*/
            delete pscanChain ;

            qDeleteAll(iaddedinfoList);
            qDeleteAll(ideletedinfoList);

            pthread->start();

            if(nresult == 2)
            {
                listWindow->addMessage("error" , tr("The top clock is not found for the clock name is not corresponding!"));
				
				#if 0
                QMessageBox::warning(this, QObject::tr("���ɨ����"),QObject::tr("���ɨ����ʧ��!"));
				#else
				QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Insert scan chain failed!"));
				#endif
				
                return ;
            }
            goto ErrorHandle ;
        }
    }
    else if((pmodule->getLocatedFileName()).endsWith(".vhd"))
    {
        //currentPrj->m_ivhdlFileMap ;
    }
    else
    {

    }

    return ;

ErrorHandle:
	
	#if 0
    QMessageBox::critical(this, QObject::tr("���ɨ����"),QObject::tr("���ɨ����ʧ��!"));
	#else
	QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Insert scan chain failed!"));
	#endif

    return ;

}

int ToolWindow::deleteAllChain()
{
    //EziDebugScanChain *plastChain = NULL ;
    QMap<QString,EziDebugInstanceTreeItem*> ichainTreeItemMap ;
    UpdateDetectThread* pthread = NULL ;
    QList<EziDebugPrj::LOG_FILE_INFO*> iaddedinfoList ;
    QList<EziDebugPrj::LOG_FILE_INFO*> ideletedinfoList ;
    QStringList ifileList  ;

    //qDebug() << "deleteAllChain";


    if(!currentPrj)
    {   
#if 0
        QMessageBox::information(this, QObject::tr("ɾ������ɨ����"),QObject::tr("���̲�����!"));
#else
        QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("The project is not existed , please set the project parameter first !"));
#endif
        return 0 ;
    }

    // �Ƿ���Ҫ����
    if(isNeededUpdate)
    {
        // ��ʾ��Ҫ ����ٸ��º��ٽ��� ����
        #if 0
        QMessageBox::information(this, QObject::tr("ɾ������ɨ����"),QObject::tr("��⵽���ļ�������,��⵽���ļ�������,����º��ٽ���ɾ������������!"));
		#else
	    QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Please update project before you delete all scan chains !"));
		#endif
        return 0 ;
    }
    else
    {
        //
        UpdateDetectThread *pthread = currentPrj->getThread();
        pthread->update() ;
        if(isNeededUpdate)
        {   
            #if 0
            QMessageBox::information(this, QObject::tr("ɾ������ɨ����"),QObject::tr("��⵽���ļ�������,����º��ٽ���ɾ������������!"));
			#else
			QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Please update project before you delete all scan chains !"));
			#endif
            return 0 ;
        }
    }


    // ��ɨ���� ��ȡ���е� ����Ϣ
    ichainTreeItemMap = currentPrj->getChainTreeItemMap() ;

    if(!ichainTreeItemMap.size())
    {   
        #if 0
        QMessageBox::information(this, QObject::tr("ɾ������ɨ����"),QObject::tr("�������κ�ɨ����!"));
		#else
		QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Delete all scan chain failed -- There is no scan chain!"));
		#endif
        return 0 ;
    }
    //QMap<QString,EziDebugScanChain*> ichainMap = currentPrj->getScanChainInfo();

    /*�ɹ�ɾ��������֮�� ɾ����һ������ ���ݵ��ļ�*/
//    plastChain = currentPrj->getLastOperateChain() ;
//    QStringList iscanFileList = plastChain->getScanedFileList() ;

    /*��������״�ڵ� ���� ��� undo  deleteAllChain ����*/
    currentPrj->backupChainTreeItemMap();

    /*�����������б���*/
    currentPrj->backupChainMap();

    //
    currentPrj->backupChainQueryTreeItemMap();

    QMap<QString,EziDebugScanChain*>::const_iterator ibakiterator = currentPrj->getBackupChainMap().constBegin();

    // ɾ��ɨ����֮ǰ  ֹͣ������   �˳��߳�
    pthread = currentPrj->getThread() ;
    if(pthread->isRunning())
    {
        pthread->quit();
        pthread->wait();
    }

    QMap<QString,EziDebugInstanceTreeItem*>::const_iterator i = ichainTreeItemMap.constBegin();
    QMap<QString,EziDebugInstanceTreeItem*>::const_iterator backup = i ;
    while(i != ichainTreeItemMap.constEnd())
    {
        EziDebugInstanceTreeItem * ptreeItem = i.value();
        if(!ptreeItem)
        {
            pthread->start();
			#if 0
            QMessageBox::critical(this, QObject::tr("ɾ������ɨ����"),QObject::tr("����ڲ�����!"));
			#else 
			QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Delete all scan chain failed -- The software interior error!"));
			#endif
            return 1;
        }

        EziDebugScanChain *pchain = ptreeItem->getScanChainInfo();
        if(!pchain)
        {
            pthread->start();
			#if 0
            QMessageBox::critical(this, QObject::tr("ɾ������ɨ����"),QObject::tr("����ڲ�����!"));
			#else
			QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Delete all scan chain failed -- The software interior error!"));
			#endif
            return 1;
        }

        pchain->backupFileList();

        pchain->clearupFileList();


        if(!ptreeItem->deleteScanChain(EziDebugPrj::OperateTypeDelAllScanChain))
        {
           /*���ýڵ��� ��ָ��*/
           ptreeItem->setScanChainInfo(NULL);
        }
        else
        {
            backup = i ;
            /*�����ɾ��ɨ���������� ���ִ���*/
            goto ErrorHandle;
        }
        ++i ;
    }


    if(currentPrj->eliminateLastOperation())
    {
        qDebug() << tr("ɾ����һ��������");
        pthread->start();
        //QMessageBox::critical(this, QObject::tr("ɾ������ɨ����"),QObject::tr("����ڲ�����!"));

        goto ErrorHandle;
    }


    /*������һ������*/
    currentPrj->updateOperation(EziDebugPrj::OperateTypeDelAllScanChain,NULL,NULL);

    /*��� treeItemMap*/
    currentPrj->cleanupChainTreeItemMap();

    /*��� chainMap*/
    currentPrj->cleanupChainMap();

    // ȫ��ɾ�� ���� ȥʹ��
    while(ibakiterator != currentPrj->getBackupChainMap().constEnd())
    {
        QString ichainName = ibakiterator.key() ;
        EziDebugScanChain *pchain = ibakiterator.value();
        struct EziDebugPrj::LOG_FILE_INFO* pinfo = new EziDebugPrj::LOG_FILE_INFO ;
        qstrcpy(pinfo->ainfoName,ichainName.toAscii().data());
        pinfo->pinfo = NULL ;
        pinfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
        ideletedinfoList << pinfo ;

        QStringList iscanFileList = pchain->getScanedFileList() ;

        for(int i = 0 ; i < iscanFileList.count() ; i++)
        {
            QString ifileName = iscanFileList.at(i) ;
            if(!ifileList.contains(ifileName))
            {
                ifileList << ifileName ;
                QString irelativeFileName = currentPrj->getCurrentDir().relativeFilePath(ifileName);

                // �ļ����޸��� ��Ҫ���±����ļ�����
                struct EziDebugPrj::LOG_FILE_INFO* pdelFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
                pdelFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;
                pdelFileInfo->pinfo = NULL ;
                memcpy(pdelFileInfo->ainfoName , irelativeFileName.toAscii().data() , irelativeFileName.size()+1);
                ideletedinfoList.append(pdelFileInfo);

                struct EziDebugPrj::LOG_FILE_INFO* paddFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
                paddFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;


                if(irelativeFileName.endsWith(".v"))
                {
                    EziDebugVlgFile *pvlgFile = currentPrj->getPrjVlgFileMap().value(irelativeFileName,NULL);
                    paddFileInfo->pinfo = pvlgFile ;
                }
                else if(irelativeFileName.endsWith(".vhd"))
                {
                    EziDebugVhdlFile *pvhdlFile = currentPrj->getPrjVhdlFileMap().value(irelativeFileName,NULL);
                    paddFileInfo->pinfo = pvhdlFile ;
                }
                else
                {
                    delete paddFileInfo ;
                    continue ;
                }

                memcpy(paddFileInfo->ainfoName , irelativeFileName.toAscii().data(), irelativeFileName.size()+1);
                iaddedinfoList.append(paddFileInfo);
            }
        }
        ++ibakiterator;
    }

    if(currentPrj->changedLogFile(iaddedinfoList,ideletedinfoList))
    {
        // ��ʾ ���� log �ļ�����
        qDebug() << "Error:Save log info Error In delete all chain!";
    }

    qDeleteAll(iaddedinfoList);
    qDeleteAll(ideletedinfoList);

    // �����������߳�
    pthread->start();

	#if 0
    QMessageBox::information(this, QObject::tr("ɾ������ɨ����"),QObject::tr("ɾ������ɨ�����ɹ�!"));
	#else
	QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Delete scan chain successfully!"));
	#endif

    return 0 ;

ErrorHandle:

    /*�ָ� treeItemMap*/
    currentPrj->resumeChainTreeItemMap();

    /*�ָ� chainMap*/
    currentPrj->resumeChainMap();

    i = ichainTreeItemMap.constBegin();
    {
        /*��chainָ����뵽 ��״�ڵ�����*/
        EziDebugInstanceTreeItem* pitem = i.value() ;
        EziDebugScanChain * plastChain = currentPrj->getScanChainInfo().value(i.key());
        pitem->setScanChainInfo(plastChain);
        plastChain->resumeFileList();

        /*��ȡɾ���� �Ѿ�ɨ������ļ�,���Ѿ����ݵ��ļ��лָ�*/
        for(int p = 0 ; p < plastChain->getScanedFileList().count();p++)
        {
            // ��ȡ���ݵ��ļ���ȫ��
            QFileInfo ifileInfo(plastChain->getScanedFileList().at(p));
            QString ieziDebugFileSuffix ;

            ieziDebugFileSuffix.append(QObject::tr(".deleteall"));

            QString ibackupFileName = currentPrj->getCurrentDir().absolutePath() \
                    + EziDebugScanChain::getUserDir() + ifileInfo.fileName() \
                    + ieziDebugFileSuffix;
            QFile ibackupFile(ibackupFileName) ;

            // �Ѿ��Ǿ���·����
            ibackupFile.copy(plastChain->getScanedFileList().at(p));

            // ɾ����ǰ���ݵ��ļ�
            ibackupFile.remove();
        }


        i++ ;
    }while(i != backup)

    qDeleteAll(iaddedinfoList);
    qDeleteAll(ideletedinfoList);

    // �����������߳�
    pthread->start();
	#if 0
    QMessageBox::warning(this, QObject::tr("ɾ������ɨ����"),QObject::tr("ɾ������ɨ����ʧ��!"));
	#else
	QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("Delete all scan chain failed!"));
	#endif

    return 1 ;
}

void ToolWindow::testbenchGeneration()
{
    qDebug() << "testbenchGeneration!" ;
    QString ichainName ;
    QStringList idataFileNameList ;
    QString ioutputDirectory ;
    UpdateDetectThread* pthread ;

    QList<TextQuery::module_top*> inoutList;
    QList<TextQuery::sample*> isampleList ;
    QList<TextQuery::system_port*> isysinoutList ;
    QVector<QList<TextQuery::regchain*> > ichainVec ;
    QMap<int,QString> ifileMap ;
    QMap<int,QString>::const_iterator ifileMapIter ;

    TextQuery::FPGA_Type ifgpaType ;

    QString iclockPortName ;
    QString iresetPortName ;

    ImportDataDialog::EDGE_TYPE eresetType ;
    TextQuery::EDGE_TYPE eresetTypeLast ;
    QString iresetSigVal ;


    if(!currentPrj)
    {   
#if 0
        QMessageBox::warning(this, QObject::tr("����testBench"),QObject::tr("���̲����ڣ���������ù��̲���������ɨ��!"));
#else
        QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("The project is not existed , please set the project parameter !"));
#endif
        return ;
    }

    if(!currentPrj->getScanChainInfo().count())
    {   
#if 0
        QMessageBox::warning(this, QObject::tr("����testBench"),QObject::tr("�������κ�ɨ����!"));
#else
        QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("There is no scan chain in project!"));
#endif
        return ;
    }

    //QMessageBox::information(this, QObject::tr("undo ����"),QObject::tr("Ŀǰֻ֧�ֶ����ڵ���һ��������ȡ��!"));
    //undoOperation();
    // Qt::WA_DeleteOnClose
    ImportDataDialog *idataDialg = new ImportDataDialog(currentPrj->getChainTreeItemMap(),this);
    if(idataDialg->exec())
    {
        qDebug() << "generate test bench" << "chainName:" << idataDialg->getChainName() \
                 << "fileName:" << idataDialg->getDataFileName();
        ichainName = idataDialg->getChainName() ;
        ifileMap = idataDialg->getFileIndexMap() ;

        for(int i = 0 ; i < ifileMap.count() ;i++)
        {
            if(!ifileMap.value(i,QString()).isEmpty())
            {
                QString ifileName = ifileMap.value(i);
                if(currentPrj->getToolType() == EziDebugPrj::ToolIse)
                {
                    if(!ifileName.endsWith(".prn"))
                    {   
                        #if 0
                        QMessageBox::warning(this, QObject::tr("����testBench"),QObject::tr("�����ļ���ʽ����ȷ!"));
						#else
						QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("The data file type is not correct!"));
						#endif
                        return ;
                    }
                }
                else if(currentPrj->getToolType() == EziDebugPrj::ToolQuartus)
                {
                    if(!ifileName.endsWith(".txt"))
                    {   
                        #if 0
                        QMessageBox::warning(this, QObject::tr("����testBench"),QObject::tr("�����ļ���ʽ����ȷ!"));
						#else
						QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("The data file type is not correct!"));
						#endif

                        return ;
                    }
                }
                else
                {   
                    #if 0
                    QMessageBox::warning(this, QObject::tr("����testBench"),QObject::tr("����������Ͳ���ȷ!"));
					#else
					QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("The tool type is not correct!"));
					#endif
                    return ;
                }
                idataFileNameList.append(ifileName);
            }
        }

        ioutputDirectory = idataDialg->getOutputDirectory() ;

        idataDialg->getResetSig(iresetPortName,eresetType,iresetSigVal);

        EziDebugScanChain *pchain = currentPrj->getScanChainInfo().value(idataDialg->getChainName(),NULL);
        if(!pchain)
        {   
            #if 0
            QMessageBox::critical(this, QObject::tr("����testBench"),QObject::tr("%1 ������!").arg(idataDialg->getChainName()));
			#else
			QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("The scan chain \"%1\" is not existed!").arg(idataDialg->getChainName()));
			#endif
			
            return ;
        }

        EziDebugInstanceTreeItem * pitem = currentPrj->getChainTreeItemMap().value(ichainName ,NULL);
        if(!pitem)
        {   
            #if 0
            QMessageBox::critical(this, QObject::tr("����testBench"),QObject::tr("%1 ��Ӧ�Ľڵ㲻����!").arg(idataDialg->getChainName()));
			#else
			QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("The node \"%1\" is not existed!").arg(idataDialg->getChainName()));
			#endif
            return ;
        }

        listWindow->addMessage("info","EziDebug info: Generating TestBench !");
        QStandardItem * pmessageitem = listWindow->addMessage("info","EziDebug info: The input and output parameters are as follows:");
        listWindow->addMessage("process",tr("%1ChainName: %2").arg(tr(" ").repeated(6)).arg(ichainName),pmessageitem);

        for(int j = 0 ; j < idataFileNameList.count() ; j++)
        {
            QString ifileName = idataFileNameList.at(j) ;
            listWindow->addMessage("process",tr("%1FileName: %2").arg(tr(" ").repeated(6)).arg(QDir::toNativeSeparators(ifileName)),pmessageitem);
        }

        listWindow->addMessage("process",tr("%1Output directory: %2").arg(tr(" ").repeated(6)).arg(QDir::toNativeSeparators(ioutputDirectory)),pmessageitem);




        // tdo tout
        EziDebugModule *pmodule = currentPrj->getPrjModuleMap().value(pitem->getModuleName(),NULL);

        QMap<QString,QString>::const_iterator iclockiter = pmodule->getClockSignal().constBegin() ;
        while(iclockiter != pmodule->getClockSignal().constEnd())
        {
            QString icombinedName ;
            QString iclock = iclockiter.key() ;
			
            QString ichainClock = pitem->parent()->getModuleClockMap(pitem->getInstanceName()).key(iclock,QString());
            int nchildChainNum =  pitem->getScanChainInfo()->getChildChainNum(ichainClock);

            QString itdoPortName(QObject::tr("_EziDebug_%1_%2_tdo_r").arg(pitem->getScanChainInfo()->getChainName()).arg(iclock));
            if(nchildChainNum > 1)
            {
                icombinedName = itdoPortName + QObject::tr("[%1:0]").arg(nchildChainNum -1);
            }
            else
            {
                icombinedName = itdoPortName ;
            }

            struct TextQuery::sample* psample = (struct TextQuery::sample*)malloc(sizeof(struct TextQuery::sample)) ;
            memset((void*)psample,0,sizeof(struct TextQuery::sample));

            psample->sample_name = (char*) malloc(icombinedName.size()+1);
            memset((void*)psample->sample_name,0,icombinedName.size()+1);
            qstrcpy(psample->sample_name , icombinedName.toAscii().constData());

            *(psample->sample_name + icombinedName.size()) = '\0' ;

            if(nchildChainNum > 1)
            {
                psample->width_first = nchildChainNum -1 ;
                psample->width_second = 0 ;
            }
            else
            {
                psample->width_first = 0 ;
                psample->width_second = 0 ;
            }


            isampleList.append(psample);

            ++iclockiter ;
        }


        struct TextQuery::sample* psample = (struct TextQuery::sample*)malloc(sizeof(struct TextQuery::sample)) ;
        memset((void*)psample,0,sizeof(struct TextQuery::sample));
        //_EziDebug_%1_TOUT_reg
        QString itoutPortName = QObject::tr("_EziDebug_%1_tout_r").arg(pitem->getScanChainInfo()->getChainName());
        psample->sample_name = (char*)malloc(itoutPortName.size()+1);
        memset((void*)psample->sample_name,0,itoutPortName.size()+1);

        qstrcpy(psample->sample_name , itoutPortName.toAscii().constData());
        *(psample->sample_name + itoutPortName.size()) = '\0' ;

        psample->width_first = 0 ;
        psample->width_second = 0 ;

        isampleList.append(psample);

        // port
        QVector<EziDebugModule::PortStructure*> iportVec = pmodule->getPort(currentPrj , pitem->getInstanceName()) ;
        for(int i = 0 ; i < iportVec.count() ;i++)
        {
            EziDebugModule::PortStructure*  pmodulePort = iportVec.at(i) ;
            QString iportName = QString::fromAscii(pmodulePort->m_pPortName);

            struct TextQuery::module_top* pport = (struct TextQuery::module_top*)malloc(sizeof(struct TextQuery::module_top)) ;
            memset((void*)pport,0,(sizeof(struct TextQuery::module_top)));

            pport->port_name = (char*)malloc(strlen(pmodulePort->m_pPortName)+1);
            memset((void*)pport->port_name,0,(strlen(pmodulePort->m_pPortName)+1));

            strncpy(pport->port_name,pmodulePort->m_pPortName,strlen(pmodulePort->m_pPortName)+1);

            if(pmodulePort->eDirectionType == EziDebugModule::directionTypeInput)
            {
                pport->inout = 1 ;
            }
            else if(pmodulePort->eDirectionType == EziDebugModule::directionTypeOutput)
            {
                pport->inout = 0 ;
            }
            else
            {

            }

            pport->width_first = pmodulePort->m_unStartBit ;
            pport->width_second = pmodulePort->m_unEndBit ;

            inoutList.append(pport);

            if(!(pmodule->getClockSignal().value(iportName,QString())).isEmpty())
            {
                iclockPortName = iportName ;
                continue ;
            }

#if 0
            if(!(pmodule->getResetSignal().value(iportName,QString())).isEmpty())
            {
                iresetName = iportName ;
                continue ;
            }
#endif


            if(iresetPortName == iportName)
            {
                continue ;
            }

            struct TextQuery::sample* psample = (struct TextQuery::sample*)malloc((sizeof(struct TextQuery::sample)/4+1)*4) ;
            memset((void*)psample,0,(sizeof(struct TextQuery::sample)/4+1)*4);

            // strlen(pmodulePort->m_pPortName + 1)
            psample->sample_name = (char*)malloc(strlen(pmodulePort->m_pPortName) +1);
            memset((void*)psample->sample_name,0,(strlen(pmodulePort->m_pPortName) + 1));
            strcpy(psample->sample_name,pmodulePort->m_pPortName);


            psample->width_first = pmodulePort->m_unStartBit ;
            psample->width_second = pmodulePort->m_unEndBit ;

            isampleList.append(psample);

        }

        // memory fifo
        QStringList isysPortList = pchain->getSyscoreOutputPortList() ;
        for(int i = 0 ; i < isysPortList.count() ; i++)
        {
            QString ihierarchicalName = isysPortList.at(i).split("#").at(0);
            QString iportName = isysPortList.at(i).split("#").at(1);
            QString iregName = isysPortList.at(i).split("#").at(2);
            int nbitWidth = isysPortList.at(i).split("#").at(3).toInt();


            QRegExp ireplaceRegExp(QObject::tr("\\b\\w*:"));

            struct TextQuery::system_port* pport = (struct TextQuery::system_port*)malloc(sizeof(struct TextQuery::system_port)) ;
            memset((void*)pport,0,sizeof(struct TextQuery::system_port));

            iportName.replace("|",".");
            iportName.replace(ireplaceRegExp,"");


            pport->port_name = (char*)malloc(iportName.size()+1);
            pport->reg_name = (char*)malloc(iregName.size()+1);
            memset((void*)pport->port_name,0,iportName.size()+1);
            memset((void*)pport->reg_name,0,iregName.size()+1);

            strcpy(pport->port_name,iportName.toAscii().constData());
            *(pport->port_name +iportName.size()) = '\0' ;
            strcpy(pport->reg_name,iregName.toAscii().constData());
            *(pport->reg_name +iregName.size()) = '\0' ;

            pport->width_first = nbitWidth -1 ;
            pport->width_second = 0 ;

            isysinoutList.append(pport);

            struct TextQuery::sample* psample = (struct TextQuery::sample*)malloc(sizeof(struct TextQuery::sample)) ;
            memset((void*)psample,0,sizeof(struct TextQuery::sample));

            psample->sample_name = (char*) malloc(iregName.size() + 1);
            memset((void*)psample->sample_name,0,iregName.size() + 1);
            strcpy(psample->sample_name , iregName.toAscii().constData());
            *(psample->sample_name + iregName.size()) = '\0' ;

            psample->width_first = nbitWidth -1 ;
            psample->width_second = 0 ;

            isampleList.append(psample);
        }


        // tdo  _EziDebug_%1_%2_TDO_reg

        //EziDebugModule *pmodule = currentPrj->getPrjModuleMap().value(pitem->getModuleName(),NULL);


        // ���� �Ĵ���
        // "reg" << sample_table[i].sample_name  << "_temp[1:`DATA_WIDTH]
        // reglist
        QMap<QString,QVector<QStringList> > iregChainMap = pchain->getRegChain();
        QMap<QString,QVector<QStringList> >::const_iterator iregChainIter = iregChainMap.constBegin() ;
        while(iregChainIter != iregChainMap.constEnd())
        {
            QVector<QStringList> iregList = iregChainIter.value();
            for(int i = 0 ; i < iregList.count() ; i++)
            {
                QStringList iregChainStr = iregList.at(i) ;
                QList<TextQuery::regchain*> iregChainList ;
                for(int j = 0 ; j < iregChainStr.count() ; j++)
                {
                    QString iregStr = iregChainStr.at(j);
                    QString ireghiberarchyName = iregStr.split("#").at(3) ;
                    QString iregName = iregStr.split("#").at(4);

                    int nstartBit = iregStr.split("#").at(5).toInt();
                    int nendBit = iregStr.split("#").at(6).toInt();

                    struct TextQuery::regchain* pregChain = (struct TextQuery::regchain*)malloc(sizeof(struct TextQuery::regchain)) ;
                    memset((char*)pregChain ,0 ,sizeof(struct TextQuery::regchain));

                    int nstartPos = ireghiberarchyName.indexOf(tr("%1:%2").arg(pitem->getModuleName()).arg(pitem->getInstanceName()));
                    ireghiberarchyName = ireghiberarchyName.mid(nstartPos);
                    ireghiberarchyName.replace(QRegExp(tr("\\w+:")),"");
                    ireghiberarchyName.replace("|",".");
                    nstartPos = ireghiberarchyName.indexOf(".");
                    ireghiberarchyName = ireghiberarchyName.mid(nstartPos+1);
                    QString icombinedName = tr("%1%2").arg(ireghiberarchyName).arg(iregName);

                    pregChain->reg_name = (char*)malloc(icombinedName.size()+1);
                    memset((char*)pregChain->reg_name ,0 ,icombinedName.size()+1);
                    strcpy(pregChain->reg_name,icombinedName.toAscii().data());
                    *(pregChain->reg_name+icombinedName.size()) = '\0' ;

                    pregChain->width_first = nstartBit ;
                    pregChain->width_second = nendBit ;

                    iregChainList.append(pregChain);
                }
                ichainVec.append(iregChainList);
            }
            ++iregChainIter ;
        }

        if(currentPrj->getToolType() == EziDebugPrj::ToolQuartus)
        {
            ifgpaType = TextQuery::Altera ;
        }
        else
        {
            ifgpaType = TextQuery::Xilinx ;
        }

        //pitem->getModuleName(),idataFileName,ioutputDirectory ,
        TextQuery itest(pitem->getModuleName(),idataFileNameList , ioutputDirectory , inoutList ,isampleList ,ichainVec , isysinoutList ,ifgpaType) ;


        eresetTypeLast = static_cast<TextQuery::EDGE_TYPE>(eresetType);

        itest.setNoNeedSig(iclockPortName , iresetPortName , eresetTypeLast ,iresetSigVal);

        // ����ɨ����֮ǰ  ֹͣ������   �˳��߳�
        pthread = currentPrj->getThread() ;

        if(pthread->isRunning())
        {
            pthread->quit();
            pthread->wait();
        }


        itest.doit();

        pthread->start();

        qDebug() << "Generate testBench finish !" << __LINE__ ;

        listWindow->addMessage("info","EziDebug info: Finishing generating testBench file!");



        // �ͷ��ڴ�
        // malloc �� free �ͷ�

        int nfreecount = 0 ;

        for(;nfreecount < inoutList.count(); nfreecount++)
        {
            TextQuery::module_top* pmodule = inoutList.at(nfreecount) ;
            char * pportName = pmodule->port_name ;

            free(pportName);
            pportName = NULL ;


            free((char*)pmodule);
            pmodule = NULL ;
        }

        for(nfreecount = 0 ; nfreecount < isampleList.count(); nfreecount++)
        {
            TextQuery::sample *psample = isampleList.at(nfreecount) ;
            char* psampleName = psample->sample_name ;

            free(psampleName) ;
            psampleName = NULL ;
            free((char*)psample);
            psample = NULL ;
        }

        for(nfreecount = 0 ; nfreecount < isysinoutList.count(); nfreecount++)
        {
            TextQuery::system_port* pport = isysinoutList.at(nfreecount) ;
            char *pportName = pport->port_name ;
            char *pregName = pport->reg_name ;
            free(pportName);
            pportName = NULL ;
            free(pregName);
            pregName = NULL ;
            free((char*)pport);
            pport = NULL ;
        }

        for(nfreecount = 0 ; nfreecount < ichainVec.count() ; nfreecount++)
        {
            int nregcount = 0 ;
            QList<TextQuery::regchain*> iregChainList = ichainVec.at(nfreecount) ;
            for(;nregcount < iregChainList.count() ; nregcount++)
            {
                TextQuery::regchain* pregChain = iregChainList.at(nregcount);
                char* pregName = pregChain->reg_name ;
                free(pregName);
                pregName = NULL ;
                free((char*)pregChain);
                pregChain = NULL ;
            }
        }

		#if 0
        QMessageBox::information(this, QObject::tr("����testBench"),QObject::tr("testBench�������!"));
		#else
		QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("The testbench is generated successfully!"));
		#endif

    }
    else
    {
        qDebug() << "do nothing!" ;
    }
}

void ToolWindow::miniWindowMinimized()
{
    miniSizeAction->setDisabled(true);
    restoreWinAction->setDisabled(false);
}

//��������ʾ
void ToolWindow::progressBarDemo()
{
    //    QProgressDialog progressDialog(this);
//    progressDialog.setCancelButtonText(tr("ȡ��"));
//    progressDialog.setRange(0, 100);
//    progressDialog.setWindowTitle(tr("������"));


    for (int i = 0; i < 100; ++i) {
        progressBar->setValue(i);
        //progressBarsetLabelText(tr("����Ϊ %1 / %2...")
                                   // .arg(i).arg(100));
        qApp->processEvents();


//        if (progressDialog.wasCanceled()){
//            //���ȡ��ʱ�Ĺ���
//            break;
//        }

        for (int j = 0; j < 100000000; ++j);
    }
}

const EziDebugPrj* ToolWindow::getCurrentProject(void) const
{
    return  currentPrj ;
}

void ToolWindow::setCurrentProject(EziDebugPrj* prj)
{
    currentPrj = prj ;
    return ;
}

void  ToolWindow::listwindowInfoInit(void)
{
    listWindow->welcomeinfoinit(currentPrj);
}

void ToolWindow::setListWindowAdsorbedFlag(bool flag)
{
    isListWindowAdsorbed = flag ;
}

void ToolWindow::createButtons()
{
    //��������ť
    // projectSetting AprojectSetting
//    proSettingButton = new Button(tr(":/images/projectSetting4.bmp"), this);
//    proSettingButton->setIconSize(QSize(30, 29));
//    connect(proSettingButton, SIGNAL(clicked()), this, SLOT(proSetting()));
//    proSettingButton->setToolTip(tr("��������"));

//    proUpdateButton = new Button(tr(":/images/projectUpdate4.bmp"), this);
//    proUpdateButton->setIconSize(QSize(30, 29));
//    connect(proUpdateButton, SIGNAL(clicked()), this, SLOT(proUpdate()));
//    proUpdateButton->setToolTip(tr("����"));

//    proPartlyUpdateButton = new Button(tr(":/images/"), this);
//    proPartlyUpdateButton->setIconSize(QSize(30, 29));
//    connect(proPartlyUpdateButton, SIGNAL(clicked()), this, SLOT(fastUpdate()));
//    proPartlyUpdateButton->setToolTip(tr("���ָ���"));

//    deleteChainButton = new Button(tr(":/images/deleteChain4.bmp"), this);
//    deleteChainButton->setIconSize(QSize(30, 29));
//    connect(deleteChainButton, SIGNAL(clicked()), this, SLOT(deleteAllChain()));
//    deleteChainButton->setToolTip(tr("ɾ��"));

//    proUndoButton = new Button(tr(":/images/undo4.bmp"), this);
//    proUndoButton->setIconSize(QSize(30, 29));
//    connect(proUndoButton, SIGNAL(clicked()), this, SLOT(undoOperation()));
//    proUndoButton->setToolTip(tr("����"));

//    testbenchGenerationButton = new Button(tr(":/images/testbenchGeneration4.bmp"), this);
//    testbenchGenerationButton->setIconSize(QSize(30, 29));
//    connect(testbenchGenerationButton, SIGNAL(clicked()), this, SLOT(testbenchGeneration()));
//    testbenchGenerationButton->setToolTip(tr("testbench����"));



    //��������ť
    //tr("��������")
    proSettingButton = createToolButton(tr("Project parameter settings  "),
                                        tr(":/images/projectSetting.bmp"),
                                        QSize(42, 41),
                                        this,
                                        SLOT(proSetting()));
    // tr("����")
    proUpdateButton = createToolButton(tr("Update"),
                                       tr(":/images/projectUpdate.bmp"),
                                       QSize(42, 41),
                                       this,
                                       SLOT(proUpdate()));

	// tr("���ָ���")
    proPartlyUpdateButton = createToolButton(tr("Update fast"),
                                       tr(":/images/projectPartlyUpdate.bmp"),
                                       QSize(42, 41),
                                       this,
                                       SLOT(fastUpdate()));

	// tr("ɾ��")
    deleteChainButton = createToolButton(tr("Delete all scan chain"),
                                         tr(":/images/deleteChain.bmp"),
                                         QSize(42, 41),
                                         this,
                                         SLOT(deleteAllChain()));

	// tr("Testbench����")
    testbenchGenerationButton = createToolButton(tr("Testbench Generation "),
                                                 tr(":/images/testbenchGeneration.bmp"),
                                                 QSize(42, 41),
                                                 this,
                                                 SLOT(testbenchGeneration()));
    // tr("����")
    proUndoButton = createToolButton(tr("Undo"),
                                                 tr(":/images/undo.bmp"),
                                                 QSize(42, 41),
                                                 this,
                                                 SLOT(undoOperation()));


    //���ϽǱ�������ť
    // tr("��С��")
    minimizeButton = createToolButton(tr("Minimize"),
                                      tr(":/images/ToolWindowminimize.bmp"),
                                      QSize(27, 19),
                                      this,
                                      SLOT(minimize()));

	// tr("����ģʽ")
    miniModeButton = createToolButton(tr("Mini mode"),
                                      tr(":/images/ToolWindowNormal.bmp"),
                                      QSize(27, 19),
                                      this,
                                      SLOT(toMiniMode()));
     // tr("����ģʽ")
    showListWindowButton = createToolButton(tr("Normal mode"),
                                            tr(":/images/showListWindow.bmp"),
                                            QSize(27, 19),
                                            this,
                                            SLOT(showListWindow()));

	// tr("�˳�")
    exitButton = createToolButton(tr("Quit"),
                                  tr(":/images/ToolWindowExit.bmp"),
                                  QSize(33, 19),
                                  this,
                                  SLOT(close()));


}

void ToolWindow::createActions()
{   
    #if 0
    exitAct = new QAction(tr("��  ��"), this);
	#else
	exitAct = new QAction(tr("Quit"), this);
	#endif
	
    exitAct->setShortcuts(QKeySequence::Quit);
    //exitAct->setStatusTip(tr("�˳�"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	#if 0
    minimizeAct = new QAction(tr("��С��"), this);
	#else
	minimizeAct = new QAction(tr("Minimize"), this);
	#endif
    //minimizeAct->setShortcuts(QKeySequence::);
    //minimizeAct->setStatusTip(tr("Exit the application"));
    connect(minimizeAct, SIGNAL(triggered()), this, SLOT(minimize()));

	#if 0
    toMiniModeAct = new QAction(tr("����ģʽ"), this);
	#else
	toMiniModeAct = new QAction(tr("Mini mode"), this);
	#endif
    //normalAct->setShortcuts(QKeySequence::Quit);
    //normalAct->setStatusTip(tr("Exit the application"));
    connect(toMiniModeAct, SIGNAL(triggered()), this, SLOT(toMiniMode()));

	#if 0
    aboutAct = new QAction(tr("�� ��..."),this);
	#else
	aboutAct = new QAction(tr("About..."),this);
	#endif
    connect(aboutAct,SIGNAL(triggered()),this,SLOT(about()));

	#if 0
    helpAct = new QAction(tr("��  ��"),this);
	#else
	helpAct = new QAction(tr("Help"),this);
	#endif
    connect(helpAct,SIGNAL(triggered()),this,SLOT(help()));

}

//����ϵͳ���̵��Ҽ��˵�
void ToolWindow::CreatTrayMenu()
{   
    #if 0
    miniSizeAction = new QAction(tr("��С��"),this);
    maxSizeAction = new QAction(tr("���"),this);
    restoreWinAction = new QAction(tr("��  ԭ"),this);
    quitAction = new QAction(tr("��  ��"),this);
    aboutAction = new QAction(tr("�� ��..."),this);
    helpAction = new QAction(tr("��  ��"),this);
	#else
	miniSizeAction = new QAction(tr("Minimize"),this);
    maxSizeAction = new QAction(tr("Maximize"),this);
    restoreWinAction = new QAction(tr("Resume to normal mode"),this);
    quitAction = new QAction(tr("Quit"),this);
    aboutAction = new QAction(tr("About..."),this);
    helpAction = new QAction(tr("Help"),this);
	#endif

    this->connect(miniSizeAction,SIGNAL(triggered()),this,SLOT(minimize()));
    this->connect(maxSizeAction,SIGNAL(triggered()),this,SLOT(showMaximized()));
    this->connect(restoreWinAction,SIGNAL(triggered()),this,SLOT(showNormal()));
    this->connect(quitAction,SIGNAL(triggered()),qApp,SLOT(quit()));
    this->connect(aboutAction,SIGNAL(triggered()),this,SLOT(about()));
    this->connect(helpAction,SIGNAL(triggered()),this,SLOT(help()));

    myMenu = new QMenu((QWidget*)QApplication::desktop());

    myMenu->addAction(miniSizeAction);
    miniSizeAction->setDisabled(false);
    myMenu->addAction(maxSizeAction);
    maxSizeAction->setDisabled(true);
    myMenu->addAction(restoreWinAction);
    restoreWinAction->setDisabled(false);
    myMenu->addSeparator();     //����һ�������
    myMenu->addAction(aboutAction);
    aboutAction->setDisabled(false);
    myMenu->addAction(helpAction);
    helpAction->setDisabled(false);
    myMenu->addSeparator();     //����һ�������
    myMenu->addAction(quitAction);
}

//����ϵͳ����ͼ��
void ToolWindow::creatTrayIcon()
{
    CreatTrayMenu();

    if (!QSystemTrayIcon::isSystemTrayAvailable())      //�ж�ϵͳ�Ƿ�֧��ϵͳ����ͼ��
    {
        return ;
    }

    myTrayIcon = new QSystemTrayIcon(this);

    QPixmap objPixmap(tr(":/images/EziDebugIcon.bmp"));
    QPixmap iconPix;

//    iconPix = objPixmap.copy(0, 0, 127, 120);//.scaled(21, 20);
//    iconPix.setMask(QPixmap(tr(":/images/EziDebugIconMask.bmp")));

    objPixmap.setMask(QPixmap(tr(":/images/EziDebugIconMask.bmp")));
    iconPix = objPixmap.copy(0, 0, 127, 104).scaled(21, 20);
    myTrayIcon->setIcon(iconPix);   //����ͼ��ͼƬ
    setWindowIcon(iconPix);  //��ͼƬ���õ�������

    myTrayIcon->setToolTip("EziDebug");    //����ʱ��������ȥ����ʾ��Ϣ

    myTrayIcon->showMessage("EziDebug", "Hi,This is my EziDebug.",QSystemTrayIcon::Information,10000);



    myTrayIcon->setContextMenu(myMenu);     //�������������Ĳ˵�

    myTrayIcon->show();
    this->connect(myTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}


Button *ToolWindow::createToolButton(const QString &toolTip, const QString &iconstr,
                                    const QSize &size, const QObject * receiver, const char *member)
{
    Button *button = new Button(iconstr, this);
    button->setToolTip(toolTip);
    //button->setIcon(icon);
    button->setIconSize(size);//(QSize(10, 10));
    // button->setSizeIncrement(size);
    //button->setSizePolicy(size.width(), size.height());
    button->setFlat(true);
    connect(button, SIGNAL(clicked()), receiver, member);

//    Button *button = new Button(this);

//    button->setToolTip(toolTip);
//    //button->setIcon(icon);
//    button->setIconSize(size);//(QSize(10, 10));
//    // button->setSizeIncrement(size);
//    //button->setSizePolicy(size.width(), size.height());
//    button->setFlat(true);
//    connect(button, SIGNAL(clicked()), receiver, member);

    return button;
}

//���ظ����¼�
void ToolWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(minimizeAct);
    menu.addAction(toMiniModeAct);
    menu.addAction(aboutAct);
    menu.addAction(helpAct);
    menu.addSeparator();
    menu.addAction(exitAct);
    menu.exec(event->globalPos());
}

void ToolWindow::mousePressEvent(QMouseEvent * event)
{

//    static int i = 0 ;
//    qDebug()<<"mouse Press Event"<<i++ ;
    if (event->button() == Qt::LeftButton) //���������
    {
        //globalPos()��ȡ�����ڵ����·����frameGeometry().topLeft()��ȡ���������Ͻǵ�λ��
//        dragPosition = event->globalPos() - frameGeometry().topLeft();
        dragPosition = event->globalPos() - geometry().topLeft();
        oriGlobalPos = event->globalPos() - listWindow->geometry().topLeft();

        event->accept();   //����¼���ϵͳ����
    }
//        if (event->button() == Qt::RightButton)
//        {
//             close();
//        }
}

void ToolWindow::mouseMoveEvent(QMouseEvent * event)
{

//    qDebug()<<"mouse Move Event"<< event->globalX()<< event->globalY() ;
//    qDebug()<<"mouse Move Event"<<event->pos().x()<<event->pos().y() ;
//    qDebug()<< this->pos().x()<< this->pos().y()<< this->geometry().left() \
//            << this->geometry().bottom() <<this->width() << this->height();
//    qDebug()<< listWindow->isListWindowHidden()<< isListWindowAdsorbed ;

    if (event->buttons() == Qt::LeftButton) //�����������������ʱ��
    {
        move(event->globalPos() - dragPosition);//�ƶ�����
        if((!listWindow->isListWindowHidden()) &&
                (listWindow->windowIsStick()))
        {
            listWindow->move(event->globalPos() - oriGlobalPos);//�ƶ�
        }
        event->accept();
    }
}

void ToolWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);//����һ��QPainter����
    painter.drawPixmap(0,0,QPixmap(":/images/Watermelon.png"));//����ͼƬ������
    /*
      QPixmap(":/images/Watermelon.png")�����ΪQPixmap()����ֻ�ܿ������Ƴ��Ŀ�ܣ�������ͼƬ��ɫ��Ҳ���ǿ�����ͼƬ��
    */
}

void ToolWindow::showEvent(QShowEvent* event)
{
//    qDebug() << "toolwindow show event!";
//    readSetting() ;
//    return ;
}

void ToolWindow::readSetting()
{
    int  nparameterFlag  = PARAMETER_OK ;
    UpdateDetectThread * pthread = NULL ;
    EziDebugPrj::TOOL etool = EziDebugPrj::ToolOther ;
    QSettings isettings("EDA Center.", "EziDebug");
    // ��ȡ���̲���������оʹ������̶���û�еĻ���������
    isettings.beginGroup("project");
    unsigned int unMaxRegNum = isettings.value("MaxRegNum").toInt();
    QString idir = isettings.value("dir").toString();
    QString itool = isettings.value("tool").toString();
    bool isXilinxERRChecked = isettings.value("isXilinxERRChecked").toBool();
    isettings.endGroup();

    if(idir.isEmpty())
    {
        nparameterFlag |=  NO_PARAMETER_DIR ;
        qDebug() << "EziDebug Info: No the Dir parameter!" ;
    }


    if(ZERO_REG_NUM == unMaxRegNum)
    {
        nparameterFlag |=  NO_PARAMETER_REG_NUM ;
        qDebug() << "EziDebug Info: No Reg Num parameter!" ;
    }


    if(itool.isEmpty())
    {
        nparameterFlag |=  NO_PARAMETER_TOOL ;
        qDebug() << "EziDebug Info: No the Tool parameter!" ;
    }
    else
    {
        if(!itool.compare("quartus"))
        {
            etool =  EziDebugPrj::ToolQuartus ;
        }
        else if(!itool.compare("ise"))
        {
            etool =  EziDebugPrj::ToolIse ;
        }
        else
        {
            nparameterFlag |=  NO_PARAMETER_TOOL ;
        }
    }

    if(nparameterFlag)
    {
        if(nparameterFlag == NO_PARAMETER_ALL)
        {
            //û��ʹ�ù����
            qDebug() << "EziDebug Info:There is no software infomation finded!";
            goto Parameter_incomplete ;
        }
        else
        {
            //��һ�α���Ĳ��� ���������߲���ȷ
            #if 0
            QMessageBox::information(this, QObject::tr("��ȡ���ʹ����Ϣ"),QObject::tr("��������!"));
			#else
			QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("There were parameter errors when reading software using information !"));
			#endif
            goto Parameter_incomplete ;
        }
    }

    currentPrj = new EziDebugPrj(unMaxRegNum,idir,etool,this) ;
    currentPrj->setXilinxErrCheckedFlag(isXilinxERRChecked);
    pthread =  currentPrj->getThread() ;
    connect(pthread,SIGNAL(codeFileChanged()),this,SLOT(updateIndicate()));
    connect(currentPrj,SIGNAL(updateProgressBar(int)),this,SLOT(changeProgressBar(int)));

Parameter_incomplete:
    return ;

}

void ToolWindow::writeSetting()
{
    QString itool ;
    unsigned int unMaxRegNum = 0 ;
    QString idir ;
    bool isXilinxERRChecked = false ;

#if 1
    if(currentPrj)
    {
        unMaxRegNum = currentPrj->getMaxRegNumPerChain() ;

        idir = currentPrj->getCurrentDir().absolutePath();

        if(currentPrj->getToolType() == EziDebugPrj::ToolQuartus)
        {
            itool = "quartus" ;
        }
        else if(currentPrj->getToolType() == EziDebugPrj::ToolIse)
        {
            itool = "ise" ;
        }
        else
        {
            itool = "" ;
        }

        isXilinxERRChecked = currentPrj->getSoftwareXilinxErrCheckedFlag() ;
    }
#endif

    qDebug() << "Attention: Begin to writtingSetting!" ;
    QSettings isettings("EDA Center.", "EziDebug");
    // ��ȡ���̲���������оʹ������̶���û�еĻ���������
    isettings.beginGroup("project");
    isettings.setValue("MaxRegNum",unMaxRegNum);
    isettings.setValue("dir",idir);
    isettings.setValue("tool",itool);
    isettings.setValue("isXilinxERRChecked",isXilinxERRChecked);
    isettings.endGroup();
    qDebug() << "Attention: End to writtingSetting!" << unMaxRegNum  \
                << idir << itool ;

}

void ToolWindow::closeEvent(QCloseEvent *event)
{
    myTrayIcon->hide(); //test
    if (myTrayIcon->isVisible())
    {
        myTrayIcon->showMessage("EziDebug", "EziDebug.",QSystemTrayIcon::Information,5000);

        hide();     //��С��
        event->ignore();
    }
    else
    {
        writeSetting() ;
        event->accept();
    }
}


//-----------------------����slot------------------------
//��С��//�����С����ťʱ�����������ں��б��ڶ�����
void ToolWindow::minimize()
{
    //showMinimized();
    this->hide();
    //if(!isNormalListWindowHidden)
    listWindow->hide();

    miniWindow->hide();

    miniSizeAction->setDisabled(true);
    restoreWinAction->setDisabled(false);
}

//ת��������ģʽ
void ToolWindow::toMiniMode()
{
//    mainWindow->show();
//    if(isNormalListWindowHidden == false)
//        listWindow->show();
//    statusWidget->hide();


//    if(isNormalListWindowHidden == false)
//        emit hideListWindow();
//    this->hide();
    isNormalMode = false;
    this->hide();
    listWindow->hide();
    miniWindow->show();


}

//����ģʽ�͹��������ڹر�ʱ�������ò�
void ToolWindow::close()
{
//    statusWidget->close();
    listWindow->close();
    miniWindow->close();
    QWidget::close();
}


void ToolWindow::updateIndicate() // ������ʾ
{
    tic = 0 ;
    isNeededUpdate = true ;
    // ���ٸ���  ȫ������ ��ť����
    updatehintButton->setEnabled(true);
    iChangeUpdateButtonTimer->start(300);

    return ;
}

void ToolWindow::fastUpdate()
{
    QStringList iaddFileList ;
    QStringList idelFileList ;
    QStringList ichgFileList ;
    UpdateDetectThread *pthread = NULL ;
    QStringList ideletedChainList ;
    QList<EziDebugPrj::LOG_FILE_INFO*> iaddedinfoList ;
    QList<EziDebugPrj::LOG_FILE_INFO*> ideletedinfoList ;

    if(!currentPrj)
    {   
#if 0
        QMessageBox::warning(this, QObject::tr("���ٸ���"),QObject::tr("����ָ���Ĺ��̲�����!"));
#else
        QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("The project is not existed!"));
#endif
        return ;
    }


    if(!isNeededUpdate)
    {   
        #if 0
        QMessageBox::information(this, QObject::tr("���ٸ���"),QObject::tr("���ļ��ɸ���!"));
		#else
		QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("There is no updated file in project!"));
		#endif
        return ;
    }

    progressBar->setValue(2);

    /*�˳������߳�*/
    pthread = currentPrj->getThread() ;
    if(pthread->isRunning())
    {
        pthread->quit();
        pthread->wait();
    }

    idelFileList = currentPrj->getUpdateFileList(EziDebugPrj::deletedUpdateFileType) ;
    iaddFileList = currentPrj->getUpdateFileList(EziDebugPrj::addedUpdateFileType) ;
    ichgFileList = currentPrj->getUpdateFileList(EziDebugPrj::changedUpdateFileType) ;

    // clear up the related chainlist last time
    currentPrj->clearupCheckedChainList();
    currentPrj->clearupDestroyedChainList();

    progressBar->setValue(10);

    if(currentPrj->updatePrjAllFile(iaddFileList,idelFileList,ichgFileList,iaddedinfoList,ideletedinfoList,true))
    {   
        #if 0
        QMessageBox::warning(this, QObject::tr("���ٸ���ʧ��"),QObject::tr("����ڲ�����!"));
		#else
	    QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("Fast update failed -- The software interior error!"));
		#endif

        /*���������߳�*/
        pthread->start();

        progressBar->setValue(0);
        return ;

    }

    currentPrj->setInstanceTreeHeadItem(NULL);
    QString itopModule = currentPrj->getTopModule() ;


    QString itopModuleComboName = itopModule + QObject::tr(":")+ itopModule ;
    EziDebugInstanceTreeItem* pnewHeadItem = new EziDebugInstanceTreeItem(itopModule,itopModule);
    if(!pnewHeadItem)
    {   
        #if 0
        QMessageBox::critical(this, QObject::tr("���ٸ���ʧ��"),QObject::tr("����ڲ�����!"));
		#else
		QMessageBox::critical(this, QObject::tr("EziDebug"),QObject::tr("Fast update failed -- The software interior error!"));
		#endif
        return ;
    }

    progressBar->setValue(50);

    if(currentPrj->traverseModuleTree(itopModuleComboName,pnewHeadItem))
    {
        qDebug() << tr("���ٸ���ʧ��") << __FILE__ << __LINE__ ;
        delete pnewHeadItem ;
        qDeleteAll(iaddedinfoList);
        qDeleteAll(ideletedinfoList);
        pthread->start();
		#if 0
        QMessageBox::warning(this, QObject::tr("���ٸ���ʧ��"),QObject::tr("����ڲ�����!"));
		#else
		QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("Fast update failed -- The software interior error!"));
		#endif
        return ;
    }

    progressBar->setValue(70);

    currentPrj->setInstanceTreeHeadItem(pnewHeadItem);

    /////////////////////////////
    if(currentPrj->getDestroyedChainList().count())
    {
        // �������ƻ���������ӡ����
        QString ichain ;
        QStringList idestroyedChainList = currentPrj->getDestroyedChainList() ;

        listWindow->addMessage("warning","EziDebug warning: Some chains are destroyed!");
        listWindow->addMessage("warning","the destroyed chain are:");
        for(int i = 0 ; i < idestroyedChainList.count() ;i++)
        {
            QString ichainName = idestroyedChainList.at(i) ;

            EziDebugInstanceTreeItem *pitem = currentPrj->getChainTreeItemMap().value(ichainName,NULL);
            if(pitem)
            {
                ichain.append(tr("EziDebug chain:%1  topInstance:%2:%3").arg(ichainName)\
                              .arg(pitem->getModuleName()).arg(pitem->getInstanceName())) ;
            }
            listWindow->addMessage("warning",ichain);
        }

        // ɨ�������ƻ� ,��ʾɾ��
        #if 0
        QMessageBox::StandardButton rb = QMessageBox::question(this, tr("����ɨ�������ƻ�"), tr("�Ƿ�ɾ�����ɨ��������"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) ;
		#else
		QMessageBox::StandardButton rb = QMessageBox::question(this, tr("EziDebug"), tr("some scan chains has been destroyed, \n Do you want to delete all scan chain code?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) ;
		#endif
		
        if(rb == QMessageBox::Yes)
        {
            QStringList iunDelChainList = currentPrj->deleteDestroyedChain(iaddedinfoList,ideletedinfoList) ;
            if(iunDelChainList.count())
            {
                listWindow->addMessage("error","EziDebug error: Some chains can not be deleted for some reasons!");
                for(int i = 0 ; i < iunDelChainList.count() ;i++)
                {
                    listWindow->addMessage("error",tr("EziDebug chain:%1").arg(iunDelChainList.at(i)));
                }
                listWindow->addMessage("error","EziDebug error: Please check the code file is compiled successed!");
            }

            for(int i = 0 ; i < idestroyedChainList.count() ; i++)
            {
                QString idestroyedChain = idestroyedChainList.at(i) ;
                ideletedChainList.append(idestroyedChain);
                if(!iunDelChainList.contains(idestroyedChain))
                {
                    struct EziDebugPrj::LOG_FILE_INFO* pdelChainInfo = new EziDebugPrj::LOG_FILE_INFO ;
                    memcpy(pdelChainInfo->ainfoName,idestroyedChain.toAscii().data(),idestroyedChain.size()+1);
                    pdelChainInfo->pinfo = NULL ;
                    pdelChainInfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
                    ideletedinfoList << pdelChainInfo ;
                }
            }
        }
    }

    QStringList icheckChainList = currentPrj->checkChainExist();

    for(int i = 0 ; i < icheckChainList.count() ;i++)
    {
        QString iupdatedChain = icheckChainList.at(i) ;
        if(!ideletedChainList.contains(iupdatedChain))
        {
            struct EziDebugPrj::LOG_FILE_INFO* pdelChainInfo = new EziDebugPrj::LOG_FILE_INFO ;
            memcpy(pdelChainInfo->ainfoName,iupdatedChain.toAscii().data(),iupdatedChain.size()+1);
            pdelChainInfo->pinfo = NULL ;
            pdelChainInfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
            ideletedinfoList << pdelChainInfo ;

            struct EziDebugPrj::LOG_FILE_INFO* paddChainInfo = new EziDebugPrj::LOG_FILE_INFO ;
            memcpy(paddChainInfo->ainfoName,iupdatedChain.toAscii().data(),iupdatedChain.size()+1);
            paddChainInfo->pinfo = paddChainInfo ;
            paddChainInfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
            iaddedinfoList << paddChainInfo ;
        }
    }

    /////////////////////////////
    progressBar->setValue(80);

    if(currentPrj->changedLogFile(iaddedinfoList,ideletedinfoList))
    {
        //��ʾ ���� log �ļ�����
        qDebug() << "Error: changedLogFile Error!";
    }

    // ɾ�� �·����  log_file_info ָ��
    qDeleteAll(iaddedinfoList);
    qDeleteAll(ideletedinfoList);

    progressBar->setValue(90);

    currentPrj->cleanupChainTreeItemMap();
    currentPrj->cleanupBakChainTreeItemMap();

    if(currentPrj->getLastOperation() == EziDebugPrj::OperateTypeDelAllScanChain)
    {
        currentPrj->cleanupChainQueryTreeItemMap();
    }
    else
    {
        currentPrj->cleanupBakChainQueryTreeItemMap();
    }

    currentPrj->updateTreeItem(pnewHeadItem);


    if(currentPrj->getLastOperation() == EziDebugPrj::OperateTypeDelAllScanChain)
    {
        // ChainTreeItemMap ����µĽڵ�map
        // �ָ� bakChainTreeItemMap ɾ�� ChainTreeItemMap

        // ChainQueryTreeItemMap ����µĽڵ�map
        // �ָ� bakChainQueryTreeItemMap ɾ�� ChainQueryTreeItemMap
        // update �õ� BakChainQueryTreeItemMap ��ԭʼ�ġ� ChainQueryTreeItemMap �ŵ��µ�
        currentPrj->cleanupBakChainQueryTreeItemMap();
        currentPrj->backupChainQueryTreeItemMap();
        currentPrj->cleanupChainQueryTreeItemMap();
    }
    else
    {
        // update �õ� ChainQueryTreeItemMap ��ԭʼ�ġ� bakChainQueryTreeItemMap ���µ�
        currentPrj->cleanupChainQueryTreeItemMap();
        currentPrj->resumeChainQueryTreeItemMap();
        currentPrj->cleanupBakChainQueryTreeItemMap();
        // treeitemmap ��ChainQueryTreeItemMap��m_ichainInfoMap �ǿ�
    }

    emit updateTreeView(pnewHeadItem);

    // ���� ������ʾ
    isNeededUpdate = false ;
    updatehintButton->setIcon(QIcon(":/images/update2.png"));
    updatehintButton->setDisabled(true);
    iChangeUpdateButtonTimer->stop();

    /*���������߳�*/
    pthread->start();

    progressBar->setValue(100);

	#if 0
    QMessageBox::warning(this, QObject::tr("���ٸ���"),QObject::tr("���ٸ������!"));
	#else
	QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Fast update completed!"));
	#endif

    progressBar->setValue(0);

}

void ToolWindow::undoOperation()
{
    QList<EziDebugPrj::LOG_FILE_INFO*> iaddedinfoList ;
    QList<EziDebugPrj::LOG_FILE_INFO*> ideletedinfoList ;
    QString irelativeFileName ;
    UpdateDetectThread* pthread = NULL ;
    QDateTime ilastModifedTime ;
    QString ifileName ;
    EziDebugVlgFile *pvlgFile = NULL ;
    EziDebugVhdlFile *pvhdlFile = NULL ;
    QStringList ifileList ;
    bool isstopFlag = false ;
    int i = 0 ;

    if(!currentPrj)
    {   
#if 0
        QMessageBox::warning(this, QObject::tr("������һ������"),QObject::tr("����ָ���Ĺ��̲�����!"));
#else
        QMessageBox::warning(this, QObject::tr("EziDebug"),QObject::tr("The project is not existed!"));
#endif
        return ;
    }

    // ֹͣ������   �˳��߳�
    pthread = currentPrj->getThread() ;
    if(pthread->isRunning())
    {
        pthread->quit();
        pthread->wait();
        isstopFlag = true ;
    }

    // 5%
    if(EziDebugPrj::OperateTypeAddScanChain == currentPrj->getLastOperation())
    {
        // ɾ������ص�ָ�����
        EziDebugScanChain *plastOperatedChain = currentPrj->getLastOperateChain() ;
        if(!plastOperatedChain)
        {
            /*��ʾ ��ɨ����*/
			#if 0
            QMessageBox::information(this, QObject::tr("undo ʧ��"),QObject::tr("��һ��������ɨ����������!"));
			#else
			QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("There is no scan chain in last operation!"));
			#endif
            pthread->start();
            return ;
        }


        ifileList = plastOperatedChain->getScanedFileList() ;
        // 10%
        // check the backup file exist
        i = 0 ;
        for( ; i < ifileList.count() ; i++)
        {
            // ��ȡ���ݵ��ļ���ȫ��
            // 10% +
            ifileName = ifileList.at(i) ;
            QFileInfo ifileInfo(ifileName);
            QString ieziDebugFileSuffix ;
            irelativeFileName = currentPrj->getCurrentDir().relativeFilePath(ifileName);

            ieziDebugFileSuffix.append(QObject::tr(".add.%1").arg(currentPrj->getLastOperateChain()->getChainName()));

            QString ibackupFileName = currentPrj->getCurrentDir().absolutePath() \
                    + EziDebugScanChain::getUserDir() + QObject::tr("/") + ifileInfo.fileName() \
                    + ieziDebugFileSuffix;
            QFile ibackupFile(ibackupFileName) ;

            if(!ibackupFile.exists())
            {   
                #if 0
                QMessageBox::information(this, QObject::tr("undo"),QObject::tr("�����ļ�%1������!").arg(ibackupFileName));
				#else
				QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Undo failed ,there is no backup file \"%1\"").arg(ibackupFileName));
				#endif
                pthread->start();
                return ;
            }
        }

        /*�ӱ����ļ��лָ��ļ�*/
        i = 0 ;
        for( ; i < ifileList.count() ; i++)
        {
            // ��ȡ���ݵ��ļ���ȫ��
            ifileName = ifileList.at(i) ;
            QFileInfo ifileInfo(ifileName);
            QString ieziDebugFileSuffix ;
            irelativeFileName = currentPrj->getCurrentDir().relativeFilePath(ifileName);

            ieziDebugFileSuffix.append(QObject::tr(".add.%1").arg(currentPrj->getLastOperateChain()->getChainName()));

            QString ibackupFileName = currentPrj->getCurrentDir().absolutePath() \
                    + EziDebugScanChain::getUserDir() + QObject::tr("/") + ifileInfo.fileName() \
                    + ieziDebugFileSuffix;
            QFile ibackupFile(ibackupFileName) ;

            if(!ibackupFile.exists())
            {   
                #if 0
                QMessageBox::information(this, QObject::tr("undoʧ��"),QObject::tr("�����ļ�%1������!").arg(ibackupFileName));
				#else
				QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Undo failed ,there is no backup file \"%1\"!").arg(ibackupFileName));
				#endif
                pthread->start();
                return ;
            }

            // �ļ����޸��� ��Ҫ���±����ļ�����
            struct EziDebugPrj::LOG_FILE_INFO* pdelFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
            pdelFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;
            pdelFileInfo->pinfo = NULL ;
            memcpy(pdelFileInfo->ainfoName , irelativeFileName.toAscii().data() , irelativeFileName.size()+1);
            ideletedinfoList.append(pdelFileInfo);


            struct EziDebugPrj::LOG_FILE_INFO* paddFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
            paddFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;

            if(ifileName.endsWith(".v"))
            {
                pvlgFile = currentPrj->getPrjVlgFileMap().value(currentPrj->getCurrentDir().relativeFilePath(ifileName)) ;
                pvlgFile->remove();
                // �Ѿ��Ǿ���·����
                ibackupFile.copy(ifileName);
                pvlgFile->modifyStoredTime(ifileInfo.lastModified());
                paddFileInfo->pinfo = pvlgFile ;
            }
            else if(ifileName.endsWith(".vhd"))
            {
                // �Ѿ��Ǿ���·����
                // ibackupFile.copy(plastOperatedChain->getScanedFileList().at(i));
            }
            else
            {
                continue ;
            }

            memcpy(paddFileInfo->ainfoName , irelativeFileName.toAscii().data(), irelativeFileName.size()+1);
            iaddedinfoList.append(paddFileInfo);

            // ɾ�����ݵ��ļ�
            ibackupFile.remove();
        }

        // undo success delete scanchain info in log file!
        struct EziDebugPrj::LOG_FILE_INFO* pdelChainInfo = new EziDebugPrj::LOG_FILE_INFO ;
        memcpy(pdelChainInfo->ainfoName,plastOperatedChain->getChainName().toAscii().data(),plastOperatedChain->getChainName().size()+1);
        pdelChainInfo->pinfo = NULL ;
        pdelChainInfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
        ideletedinfoList.append(pdelChainInfo) ;

        // ��chain map ��ɾ��
        currentPrj->eliminateChainFromMap(plastOperatedChain->getChainName());

        // �� item �� chain ��Ϊ��
        EziDebugInstanceTreeItem * item = currentPrj->getChainTreeItemMap().value(plastOperatedChain->getChainName());
        if(item)
        {
            item->setScanChainInfo(NULL);
        }
        else
        {
            pthread->start();
            return ;
        }
        // ��item map ��ɾ��
        currentPrj->eliminateTreeItemFromMap(plastOperatedChain->getChainName());

        currentPrj->eliminateTreeItemFromQueryMap(item->getNameData());

        // ɾ��ָ�����
        delete  plastOperatedChain ;
        plastOperatedChain = NULL ;

        // ��һ��������ض��� �ÿ�
        currentPrj->updateOperation(EziDebugPrj::OperateTypeNone,NULL,NULL);
    }
    else if(EziDebugPrj::OperateTypeDelSingleScanChain == currentPrj->getLastOperation())
    {
        // �ָ� �ϴ�ɾ������
        EziDebugScanChain *plastOperatedChain = currentPrj->getLastOperateChain() ;
        if(!plastOperatedChain)
        {
            /*��ʾ ��ɨ����*/
            pthread->start();
            return ;
        }
        EziDebugInstanceTreeItem * plastOperatedItem = currentPrj->getLastOperateTreeItem();
        if(!plastOperatedItem)
        {
            /*��ʾ �޴˽ڵ�*/
            pthread->start();
            return ;
        }

        // check the back up file
        for(i = 0 ; i < plastOperatedChain->getScanedFileList().count();i++)
        {
            // ��ȡ���ݵ��ļ���ȫ��
            ifileName = plastOperatedChain->getScanedFileList().at(i) ;
            irelativeFileName = currentPrj->getCurrentDir().relativeFilePath(ifileName);

            QFileInfo ifileInfo(ifileName);
            QString ieziDebugFileSuffix ;
            ieziDebugFileSuffix.append(QObject::tr(".delete.%1").arg(plastOperatedChain->getChainName()));

            QString ibackupFileName = currentPrj->getCurrentDir().absolutePath() \
                    + EziDebugScanChain::getUserDir() + QObject::tr("/") + ifileInfo.fileName() \
                    + ieziDebugFileSuffix;
            QFile ibackupFile(ibackupFileName) ;
            if(!ibackupFile.exists())
            {   
                #if 0
                QMessageBox::information(this, QObject::tr("undoʧ��"),QObject::tr("�����ļ�%1������!").arg(ibackupFileName));
				#else
				QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Undo failed , there is no backup file \"%1\"!").arg(ibackupFileName));
				#endif
                pthread->start();
                return ;
            }
        }

         // �ָ�Դ�ļ�
        for(i = 0 ; i < plastOperatedChain->getScanedFileList().count();i++)
        {
            // ��ȡ���ݵ��ļ���ȫ��
            ifileName = plastOperatedChain->getScanedFileList().at(i) ;
            irelativeFileName = currentPrj->getCurrentDir().relativeFilePath(ifileName);

            QFileInfo ifileInfo(ifileName);
            QString ieziDebugFileSuffix ;
            ieziDebugFileSuffix.append(QObject::tr(".delete.%1").arg(plastOperatedChain->getChainName()));

            QString ibackupFileName = currentPrj->getCurrentDir().absolutePath() \
                    + EziDebugScanChain::getUserDir() + QObject::tr("/") + ifileInfo.fileName() \
                    + ieziDebugFileSuffix;
            QFile ibackupFile(ibackupFileName) ;


            struct EziDebugPrj::LOG_FILE_INFO* pdelFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
            pdelFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;
            pdelFileInfo->pinfo = NULL ;
            memcpy(pdelFileInfo->ainfoName , irelativeFileName.toAscii().data() , irelativeFileName.size()+1);
            ideletedinfoList.append(pdelFileInfo);

            struct EziDebugPrj::LOG_FILE_INFO* paddFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
            paddFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;

            if(ifileName.endsWith(".v"))
            {
                pvlgFile = currentPrj->getPrjVlgFileMap().value(currentPrj->getCurrentDir().relativeFilePath(ifileName)) ;
                pvlgFile->remove();
                // �Ѿ��Ǿ���·����
                ibackupFile.copy(ifileName);
                pvlgFile->modifyStoredTime(ifileInfo.lastModified());
                paddFileInfo->pinfo = pvlgFile ;
            }
            else if(ifileName.endsWith(".vhd"))
            {
                // �Ѿ��Ǿ���·����
                // ibackupFile.copy(plastOperatedChain->getScanedFileList().at(i));
            }
            else
            {
                continue ;
            }
            memcpy(paddFileInfo->ainfoName , irelativeFileName.toAscii().data(), irelativeFileName.size()+1);
            iaddedinfoList.append(paddFileInfo);

            // ɾ�����ݵ��ļ�
            ibackupFile.remove();
        }


        // add chain info to log file
        struct EziDebugPrj::LOG_FILE_INFO* paddChainInfo = new EziDebugPrj::LOG_FILE_INFO ;
        memcpy(paddChainInfo->ainfoName ,plastOperatedChain->getChainName().toAscii().data(),plastOperatedChain->getChainName().size()+1);
        paddChainInfo->pinfo = plastOperatedChain ;
        paddChainInfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
        iaddedinfoList << paddChainInfo ;

        //
        plastOperatedItem->setScanChainInfo(plastOperatedChain);

        //  ���� item map
        currentPrj->addToTreeItemMap(plastOperatedChain->getChainName(),plastOperatedItem);


        //  ���� item map
        currentPrj->addToChainMap(plastOperatedChain);

        //
        currentPrj->addToQueryItemMap(plastOperatedItem->getNameData(),plastOperatedItem);

        // ��һ��������ض��� �ÿ�
        currentPrj->updateOperation(EziDebugPrj::OperateTypeNone,NULL,NULL);

    }
    else if(EziDebugPrj::OperateTypeDelAllScanChain == currentPrj->getLastOperation())
    {
        QStringList irepeatedFileList ;

        QMap<QString,EziDebugInstanceTreeItem*> ichainTreeItemMap = currentPrj->getBackupChainTreeItemMap();
        QMap<QString,EziDebugInstanceTreeItem*>::const_iterator i = ichainTreeItemMap.constBegin();
        // check backup file exist
        while( i != ichainTreeItemMap.constEnd())
        {
            EziDebugScanChain * plastChain = currentPrj->getBackupChainMap().value(i.key());
            /*��ȡɾ���� �Ѿ�ɨ������ļ�,���Ѿ����ݵ��ļ��лָ�*/
            for(int p = 0 ; p < plastChain->getScanedFileList().count();p++)
            {
                // ��ȡ���ݵ��ļ���ȫ��
                ifileName = plastChain->getScanedFileList().at(p) ;
                irelativeFileName = currentPrj->getCurrentDir().relativeFilePath(ifileName);

                QFileInfo ifileInfo(ifileName);
                QString ieziDebugFileSuffix ;

                ieziDebugFileSuffix.append(QObject::tr(".deleteall"));

                QString ibackupFileName = currentPrj->getCurrentDir().absolutePath() \
                        + EziDebugScanChain::getUserDir()+ QObject::tr("/") + ifileInfo.fileName() \
                        + ieziDebugFileSuffix;
                QFile ibackupFile(ibackupFileName) ;
                if(!ibackupFile.exists())
                {   
                    #if 0
                    QMessageBox::information(this, QObject::tr("undoʧ��"),QObject::tr("�����ļ�%1������!").arg(ibackupFileName));
					#else
					QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("Undo failed , there is no backup file \"%1\" !").arg(ibackupFileName));
					#endif
                    pthread->start();
                    return ;
                }
            }
            ++i ;
        }

        // �ָ�Դ�ļ�
        i = ichainTreeItemMap.constBegin();
        while(i != ichainTreeItemMap.constEnd())
        {
            /*��chainָ����뵽 ��״�ڵ�����*/
            EziDebugInstanceTreeItem* pitem = i.value() ;

            EziDebugScanChain * plastChain = currentPrj->getBackupChainMap().value(i.key());
            pitem->setScanChainInfo(plastChain);


            struct EziDebugPrj::LOG_FILE_INFO* paddChainInfo = new EziDebugPrj::LOG_FILE_INFO ;
            memcpy(paddChainInfo->ainfoName ,plastChain->getChainName().toAscii().data(),plastChain->getChainName().size()+1);
            paddChainInfo->pinfo = plastChain ;
            paddChainInfo->etype = EziDebugPrj::infoTypeScanChainStructure ;
            iaddedinfoList << paddChainInfo ;


            /*��ȡɾ���� �Ѿ�ɨ������ļ�,���Ѿ����ݵ��ļ��лָ�*/
            for(int p = 0 ; p < plastChain->getScanedFileList().count();p++)
            {
                // ��ȡ���ݵ��ļ���ȫ��
                ifileName = plastChain->getScanedFileList().at(p) ;
                irelativeFileName = currentPrj->getCurrentDir().relativeFilePath(ifileName);

                QFileInfo ifileInfo(ifileName);
                QString ieziDebugFileSuffix ;

                ieziDebugFileSuffix.append(QObject::tr(".deleteall"));

                QString ibackupFileName = currentPrj->getCurrentDir().absolutePath() \
                        + EziDebugScanChain::getUserDir()+ QObject::tr("/") + ifileInfo.fileName() \
                        + ieziDebugFileSuffix;
                QFile ibackupFile(ibackupFileName) ;

                struct EziDebugPrj::LOG_FILE_INFO* pdelFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
                pdelFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;
                pdelFileInfo->pinfo = NULL ;
                memcpy(pdelFileInfo->ainfoName , irelativeFileName.toAscii().data() , irelativeFileName.size()+1);
                ideletedinfoList.append(pdelFileInfo);

                struct EziDebugPrj::LOG_FILE_INFO* paddFileInfo = new EziDebugPrj::LOG_FILE_INFO ;
                paddFileInfo->etype = EziDebugPrj::infoTypeFileInfo ;

                if(ifileName.endsWith(".v"))
                {
                    pvlgFile = currentPrj->getPrjVlgFileMap().value(currentPrj->getCurrentDir().relativeFilePath(ifileName)) ;
                    pvlgFile->remove();

                    ibackupFile.copy(ifileName);
                    pvlgFile->modifyStoredTime(ifileInfo.lastModified());
                    paddFileInfo->pinfo = pvlgFile ;
                }
                else if(ifileName.endsWith(".vhd"))
                {
                    // �Ѿ��Ǿ���·����
                    // ibackupFile.copy(plastOperatedChain->getScanedFileList().at(i));
                }
                else
                {
                    continue ;
                }
                memcpy(paddFileInfo->ainfoName , irelativeFileName.toAscii().data(), irelativeFileName.size()+1);
                iaddedinfoList.append(paddFileInfo);
                // ɾ����ǰ���ݵ��ļ�
                ibackupFile.remove();
            }

            i++ ;
        }

        //
        currentPrj->resumeChainMap();

        //
        currentPrj->resumeChainTreeItemMap();

        //
        currentPrj->resumeChainQueryTreeItemMap();

        // ��һ��������ض��� �ÿ�
        currentPrj->updateOperation(EziDebugPrj::OperateTypeNone,NULL,NULL);
    }
    else
    {   
        #if 0
        QMessageBox::information(this , QObject::tr("ע��") , QObject::tr("��һ���޲���!"));
		#else
		QMessageBox::information(this , QObject::tr("EziDebug") , QObject::tr("Note: there is no last operation!"));
		#endif

        if(isstopFlag == true)
        {
            pthread->start();
        }
        return ;
    }

    // 90%
    if(currentPrj->changedLogFile(iaddedinfoList,ideletedinfoList))
    {
        // ��ʾ ���� log �ļ�����
        qDebug() << tr("Error:Save log file error in undo operation!") ;
    }

    qDeleteAll(ideletedinfoList);
    qDeleteAll(iaddedinfoList);

    if(isstopFlag == true)
    {
        pthread->start();
    }

    // 100%
    #if 0
    QMessageBox::information(this, QObject::tr("undo����"),QObject::tr("������һ���������!"));
	#else
	QMessageBox::information(this, QObject::tr("EziDebug"),QObject::tr("The last operation is undo successfully!"));
	#endif

    return ;
}

void ToolWindow::changeUpdatePic()
{
    tic++ ;
    if(tic%2)
    {
        updatehintButton->setIcon(QIcon(":/images/update3.png"));
    }
    else
    {
        updatehintButton->setIcon(QIcon(":/images/update2.png"));
    }
    iChangeUpdateButtonTimer->start();
}

//����С����ԭ
void ToolWindow::showNormal()
{
    miniSizeAction->setDisabled(false);
    restoreWinAction->setDisabled(true);
    if(isNormalMode)
    {

    if(!listWindow->isListWindowHidden())
        listWindow->show();
        this->show();
    }
    else
        miniWindow->show();

}

void ToolWindow::changeProgressBar(int value)
{
    progressBar->setValue(value);
}


//�Ҳ��в���ť����toolwindow�·����б���
void ToolWindow::showListWindow()
{

    if(listWindow->isListWindowHidden()){
        listWindow->setListWindowHidden(false);

    }
    else{
        listWindow->setListWindowHidden(true);
    }
}

//����
void ToolWindow::about()
{   
#if 0
    QMessageBox::about(this, tr("����"), tr("    ��Ȩ���п�ԺEDA��������    \n\n"));
#else
    QMessageBox::about(this,tr("About EziDebug"),tr("    EziDebug CopyRight(c) 2013-2018 by EziGroup.    \n\n"));
#endif

}

//����
void ToolWindow::help()
{

}


//ϵͳ������غ���
void ToolWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        //��ͨģʽ�£�������С�����߲��ڶ���
        if(isNormalMode)
        {
            if(this->isHidden() || !this->isTopLevel())
            {
                if(!listWindow->isListWindowHidden()){
                    listWindow->show();
                    listWindow->raise();
                    listWindow->activateWindow();
                }
                this->show();
                this->raise();
                this->activateWindow();

                miniSizeAction->setDisabled(false);
                restoreWinAction->setDisabled(true);
            }
            else
            {

                this->hide();
                listWindow->hide();

                miniSizeAction->setDisabled(true);
                restoreWinAction->setDisabled(false);
            }
        }
        //����ģʽ�£�������С��
        else
        {
            if(miniWindow->isHidden())
            {
                miniWindow->showNormal();

                miniSizeAction->setDisabled(false);
                restoreWinAction->setDisabled(true);
            }
            else
            {
                miniWindow->hide();
                miniSizeAction->setDisabled(true);
                restoreWinAction->setDisabled(false);
            }
        }
        break;
    case QSystemTrayIcon::MiddleClick:
        myTrayIcon->showMessage("EziDebug", "EziDebug.",QSystemTrayIcon::Information,10000);
        break;

    default:
        break;
    }
}

//����ģʽ�£���ԭΪ��ͨģʽ
void ToolWindow::toNormalMode()
{
    isNormalMode = true;
    this->show();
    if(!listWindow->isListWindowHidden())
        listWindow->show();
    miniWindow->hide();
    //miniWindow->setStatusWidgetHidden(true);
}


//listWindow�Ĵ����ƶ������ź��ж�listWindow�Ƿ�����
void ToolWindow::listWindowMouseReleased(const QRect listWinGeo)
{
    const int disMax = 15;
    isListWindowAdsorbed = false;
    int dis = this->geometry().bottom() - listWinGeo.top();
    if((dis < disMax) && (dis > -disMax)){
        listWindow->move(listWinGeo.left(), this->geometry().bottom());//�ƶ�����
        isListWindowAdsorbed = true;
    }
    dis = this->geometry().top() - listWinGeo.bottom();
    if((dis < disMax) && (dis > -disMax)){
        listWindow->move(listWinGeo.left(), listWinGeo.top() + dis);//�ƶ�����
        isListWindowAdsorbed = true;
    }
    dis = this->geometry().right() - listWinGeo.left();
    if((dis < disMax) && (dis > -disMax)){
        listWindow->move(this->geometry().right(), listWindow->geometry().top());//�ƶ�����
        isListWindowAdsorbed = true;
    }

    dis = this->geometry().left() - listWinGeo.right();
    if((dis < disMax) && (dis > -disMax)){
        listWindow->move(listWindow->geometry().left() + dis, listWindow->geometry().top());//�ƶ�����
        isListWindowAdsorbed = true;
    }
}

//void ToolWindow::proSetting()
//{
////    ProjectSetWizard proSetWiz(this);
////    if (proSetWiz.exec()) {
////        QString str = tr("aa");

////    }

//}












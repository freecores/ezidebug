#include "projectsetwizard.h"
#include "toolwindow.h"
#include "ezidebugscanchain.h"
#include <QDebug>
#include <QtGui>

ProjectSetWizard::ProjectSetWizard(QWidget *parent):QWizard(parent)
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("gb18030"));

    createPage1();
    createPage2();
    createConclusionPage();

    addPage(m_iprjSetPage);
    addPage(m_iregSetPage);
    addPage(m_itoolSetPage);

    setWindowTitle(tr("����������"));

    connect(this,SIGNAL(currentIdChanged(int)),this,SLOT(saveModifiedPage(int)));
}

ProjectSetWizard::~ProjectSetWizard()
{

}

void ProjectSetWizard::done (int result)
{
   // ToolWindow* pparent = 0 ;

    /*����ʹ�ù���*/
    if(!(alteraCheckBox->isChecked()||xilinxCheckBox->isChecked()))
    {
        qDebug() << "No Tool Selected!";
        m_ecurrentTool = EziDebugPrj::ToolQuartus ;
    }
    else if(alteraCheckBox->isChecked()&&xilinxCheckBox->isChecked())
    {
        qDebug() << "Wrong Tool selected!";
        m_ecurrentTool = EziDebugPrj::ToolQuartus ;
    }
    else if(alteraCheckBox->isChecked())
    {
        m_ecurrentTool = EziDebugPrj::ToolQuartus ;
    }
    else if(xilinxCheckBox->isChecked())
    {
        m_ecurrentTool = EziDebugPrj::ToolIse ;
    }

    if(m_ixilinxComplieOption->checkState() == Qt::Checked)
    {
        m_isXilinxErrChecked = true ;
    }
    else
    {
        m_isXilinxErrChecked = false ;
    }
    QWizard::done(result);

}


void ProjectSetWizard::saveModifiedPage(int m)
{
    m = m ;
    if(m_iprjSetPage == m_currentPage)
    {
        /*����·����Ϣ*/
        m_icurrentDir = QDir::fromNativeSeparators(proPathComboBox->itemText(proPathComboBox->currentIndex()));
        qDebug() << m_icurrentDir << __LINE__;

    }
    else if(m_iregSetPage == m_currentPage)
    {
        /*����Ĵ�����������Ϣ*/
        m_uncurrentRegNum = m_nregNumComBox->currentText().toInt() ;
    }
    else if(m_itoolSetPage == m_currentPage)
    {
        /*����ʹ�ù���*/
        /*��������������*/
    }
    else
    {
        /*��ָ��*/
    }

    m_currentPage = this->currentPage() ;
}

void ProjectSetWizard::changeXilinxCompileOption(bool checked)
{
    if(checked)
    {
        m_ixilinxComplieOption->setCheckable(true);
    }
    else
    {
        m_ixilinxComplieOption->setCheckable(false);
    }
}

void ProjectSetWizard::browse()
{
//    QString directory = QFileDialog::getExistingDirectory(this,
//                               tr("Chose a File"), QDir::currentPath());

//    if (!directory.isEmpty()) {
//        if (proPathComboBox->findText(directory) == -1)
//            proPathComboBox->addItem(directory);
//        proPathComboBox->setCurrentIndex(proPathComboBox->findText(directory));
//    }


//    QFileDialog::Options options;
//    //options |= QFileDialog::DontUseNativeDialog;
//    QString selectedFilter;
//    QString fileName = QFileDialog::getOpenFileName(this,
//                                tr("QFileDialog::getOpenFileName()"),
//                                proPathComboBox->currentText(),
//                                tr("�����ļ� (*.*);;�ı��ļ� (*.txt)"),
//                                &selectedFilter,
//                                options);
//    if (!fileName.isEmpty()){
//        if (proPathComboBox->findText(fileName) == -1)
//                proPathComboBox->addItem(fileName);
//        proPathComboBox->setCurrentIndex(proPathComboBox->findText(fileName));
//    }


    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    //options |= QFileDialog::DontUseNativeDialog;
    QString directory = QFileDialog::getExistingDirectory(this,
                                tr("���ù���Ŀ¼"),
                                proPathComboBox->currentText(),
                                options);
    if (!directory.isEmpty()){
        if (proPathComboBox->findText(directory) == -1)
                proPathComboBox->addItem(directory);
        proPathComboBox->setCurrentIndex(proPathComboBox->findText(directory));
    }
}

void ProjectSetWizard::createPage1()
{
    m_iprjSetPage = new QWizardPage(this);
    m_iprjSetPage->setTitle(tr("���ù���Ŀ¼"));

    QLabel *label = new QLabel(tr("����Ŀ¼:"));
    proPathComboBox = createComboBox(QDir::currentPath());
    label->setBuddy(proPathComboBox);
    proPathComboBox->clear();
    proPathComboBox->clearEditText();
    ToolWindow *pparent = dynamic_cast<ToolWindow*>(this->parentWidget());
    if(pparent->getCurrentProject())
    {
        proPathComboBox->addItem(pparent->getCurrentProject()->getCurrentDir().absolutePath());
        m_icurrentDir = QDir::toNativeSeparators(pparent->getCurrentProject()->getCurrentDir().absolutePath()) ;
        qDebug() << m_icurrentDir << __LINE__;
    }


    browseButton = createButton(tr("���"), SLOT(browse()));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(label, 0, 0);
    layout->addWidget(proPathComboBox, 3, 0, 1, 4);
    layout->addWidget(browseButton, 3, 4, 1, 1);
    m_iprjSetPage->setLayout(layout);

    return ;
}

void ProjectSetWizard::createPage2()
{
    m_iregSetPage = new QWizardPage(this);
    m_iregSetPage->setTitle(QObject::tr("�������ɨ�����ļĴ�������"));


    ///////////test/////////////////
    // chainNum = 1;
    QStringList chainNameList;
    chainNameList << "Chain 1"
                  << "Chain 2"
                  << "Chain 3"
                  << "Chain 4"
                  << "Chain 5";
    ///////////end of test/////////////////


    //QVector<int> regNumVector;
    QStringList regNumList;

    QGridLayout *layout = new QGridLayout;

#if 0
    for(int i = 0; i < chainNum; i++){
        QLabel *label = new QLabel(chainNameList[i] + tr(":"));
        spinBox = new QSpinBox;
        spinBox->setMaximum(2048);
        //regNumVector.append(0);
        //connect(spinBox, SIGNAL(valueChanged(int)), regNumVector, SLOT(replace(i, int)));
        regNumList << tr("0");
        //connect(spinBox, SIGNAL(valueChanged(int)), regNumList[i], SLOT(setNum()));

        label->setBuddy(spinBox);
        layout->addWidget(label, i, 0, 1, 1);
        layout->addWidget(spinBox, i, 1, 1, 2);
    }
#endif
    m_nregNumComBox = new QComboBox(this) ;
    m_imaxregNumLabel = new QLabel("The Max Regnum Per Chain :",this);

    m_imaxregNumLabel->setBuddy(m_imaxregNumLabel);
    layout->addWidget(m_imaxregNumLabel, 0, 0, 1, 1,Qt::AlignLeft);
    layout->addWidget(m_nregNumComBox, 0 , 1, 1, 1);
    //m_nregNumComBox->addItem("256");
    m_nregNumComBox->addItem("512");
    m_nregNumComBox->addItem("1024");
    m_nregNumComBox->addItem("2048");
    layout->setMargin(50);
    m_iregSetPage->setLayout(layout);

    ToolWindow *pparent = dynamic_cast<ToolWindow*>(this->parentWidget());
    if(pparent->getCurrentProject())
    {
        int nNum = pparent->getCurrentProject()->getMaxRegNumPerChain() ;
        if((nNum != 512)&&(nNum != 1024)&&(nNum != 2048))
        {
            qDebug() << "EziDebug Error: The project 's max reg num is not right!";
            nNum = 512 ;
        }

        m_nregNumComBox->setCurrentIndex(m_nregNumComBox->findText(QString::number(nNum)));
        m_uncurrentRegNum = nNum ;
    }
}

void ProjectSetWizard::createConclusionPage()
{
    m_itoolSetPage = new QWizardPage(this);
    m_itoolSetPage->setTitle(QObject::tr("��������FPGA����"));

    #if 0
    QLabel *languageLabel = new QLabel(tr("��ʹ�ñ������:"));
    QCheckBox *verilogCheckBox = new QCheckBox(tr("Verilog"));
    QCheckBox *vhdlCheckBox = new QCheckBox(tr("VHDL"));

    QLabel *slotLabel = new QLabel(tr("��ʹ�ü�����ʱ��:"));
    slotComboBox = createComboBox(tr(""));
    #endif

    //��Ӹ���ʱ�ӡ�������
    QLabel *fpgaLabel = new QLabel(tr("��ʹ��FPGA:"));
    alteraCheckBox = new QRadioButton(tr("Altera"),this);
    xilinxCheckBox = new QRadioButton(tr("Xilinx"),this);

    alteraCheckBox->setCheckable(true);
    xilinxCheckBox->setCheckable(true);

    m_ixilinxComplieOption = new QCheckBox(tr("Disable Equivalent Register Removal"),this);
    m_ixilinxComplieOption->setCheckable(false);
    m_ixilinxComplieOption->setVisible(false);
    connect(xilinxCheckBox,SIGNAL(toggled(bool)),this,SLOT(changeXilinxCompileOption(bool)));
    QGridLayout *layout = new QGridLayout;
    //layout->addWidget(languageLabel, 0, 0);
    //layout->addWidget(verilogCheckBox, 1, 0, 1, 2);
    //layout->addWidget(vhdlCheckBox, 1, 2, 1, 2);
    //layout->addWidget(slotLabel, 3, 0);
    //layout->addWidget(slotComboBox, 3, 1, 1, 4);
    ToolWindow *pparent = dynamic_cast<ToolWindow*>(this->parentWidget());
    if(pparent->getCurrentProject())
    {
        if(pparent->getCurrentProject()->getToolType() == EziDebugPrj::ToolQuartus)
        {
            alteraCheckBox->setChecked(true);
            m_ecurrentTool = EziDebugPrj::ToolQuartus ;
        }
        else if(pparent->getCurrentProject()->getToolType() == EziDebugPrj::ToolIse)
        {
            xilinxCheckBox->setChecked(true);
            m_ecurrentTool = EziDebugPrj::ToolIse ;
        }
        else
        {
            qDebug() << "EziDebug Error: The project 's fpag type is not right!" ;
        }
    }

    layout->addWidget(fpgaLabel, 4, 0);
    layout->addWidget(alteraCheckBox, 5, 0, 1, 2);
    layout->addWidget(xilinxCheckBox, 5, 2, 1, 2);
    layout->addWidget(m_ixilinxComplieOption,6,2,1,2);

    m_itoolSetPage->setLayout(layout);
}

QPushButton *ProjectSetWizard::createButton(const QString &text, const char *member)
{
    QPushButton *button = new QPushButton(text);
    connect(button, SIGNAL(clicked()), this, member);
    return button;
}

QComboBox *ProjectSetWizard::createComboBox(const QString &text, const QStringList &items)
{
    QComboBox *comboBox = new QComboBox;
    comboBox->setEditable(true);
    if(text != tr(""))
        comboBox->addItem(text);
    if(!items.isEmpty())
        comboBox->insertItems(comboBox->count(), items);
    comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    return comboBox;
}

QString ProjectSetWizard::getProPath()
{
    return proPathComboBox->currentText();
}

void ProjectSetWizard::cc()
{
    qDebug("flag is cc");

    proPathComboBox->addItem(tr("aa"));
}



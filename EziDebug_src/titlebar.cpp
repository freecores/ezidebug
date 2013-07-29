#include "titlebar.h"
#include "listwindow.h"

#include <QtGui>
TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent)
{
    //test
//    setAutoFillBackground(true);
//    QPixmap backgroundPix;
//    QPalette palette;
//    backgroundPix.load(":/images/mainBackground.bmp",0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
//    palette.setBrush(QPalette::Background, QBrush(backgroundPix.copy(0,0,290,17)));
//    this->setPalette(palette);


    QTextCodec::setCodecForTr(QTextCodec::codecForName("gb18030"));
    setMouseTracking (true);
    //setCursor(Qt::ArrowCursor);
    isLeftButtonPress = false;

    CreateWidget();//�����Ӳ���
    //SetWidgetStyle();//�����Ӳ�����ʽ(qss)
    //CreateLayout();//�������ò���
}






//�����Ӳ���
void TitleBar::CreateWidget()
{
//    background.load(":/images/listBackground.bmp",
//                        0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
//    //background = background.copy(0, 0, background.width(), 17);
//    background = background.copy(0, 4, background.width(), 13);




//    QPixmap objPixmap(background);

//    iconLabel = new QLabel(this);//������Сͼ��
//    iconLabel->setPixmap(objPixmap.copy(5, 0, 20, 13));

//    lineLabel = new QLabel(this);//�м�ĳ�ֱ��
//    lineLabel->setPixmap(objPixmap.copy(25, 0, 235, 13));

//    closeButtonLabel = new QLabel(this);//���Ҳ�رհ�����ͼ��
//    closeButtonLabel->setPixmap(objPixmap.copy(260, 0, background.width() - 259-5, 13));
//    closeButtonLabel->setCursor(Qt::PointingHandCursor);


    //setGeometry(QRect(0, 0, 290, 17));
    setMinimumSize(423, 26);
    setFixedHeight(26);
    closeButton = new Button(tr(":/images/ListWindowExit.bmp"), this);
    closeButton->setIconSize(QSize(33, 19));
    closeButton->setGeometry(QRect(385, 0, 33, 19));
    setContentsMargins(0,0,0,0);


//    //ͼ���ǩ--logo
//    m_pLabelIcon = new QLabel(this);
//    QPixmap objPixmap(":/image/360AboutLogo.png");
//    m_pLabelIcon->setPixmap(objPixmap.scaled(TITLE_H,TITLE_H));
//    //�ı���ǩ--����
//    m_pLabelTitle = new QLabel(this);
//    m_pLabelTitle->setText(QString("360 Safe Guard V8.5"));
//    //�ı���ǩ--��ʽ�汾
//    m_pLabelVersion = new QLabel(this);
//    m_pLabelVersion->setText(QString("Use Class Style"));
//    //���������״
//    m_pLabelVersion->setCursor(Qt::PointingHandCursor);
//    //��ť--����Ƥ��
//    m_pBtnSkin = new QToolButton(this);
//    //���ó�ʼͼƬ
//    SetBtnIcon(m_pBtnSkin,eBtnStateDefault,true);
//    //��ť--�˵�
//    m_pBtnMenu = new QToolButton(this);
//    SetBtnIcon(m_pBtnMenu,eBtnStateDefault,true);
//    //��ť--��С��
//    m_pBtnMin = new QToolButton(this);
//    SetBtnIcon(m_pBtnMin,eBtnStateDefault,true);
//    //��ť--���/��ԭ
//    m_pBtnMax = new QToolButton(this);
//    SetBtnIcon(m_pBtnMax,eBtnStateDefault,true);
//    //��ť--�ر�
//    m_pBtnClose = new QToolButton(this);
//    SetBtnIcon(m_pBtnClose,eBtnStateDefault,true);
    //����Ӳ���
    const QObjectList &objList = children();
    for(int nIndex=0; nIndex < objList.count(); ++nIndex)
    {
        //�����Ӳ�����MouseTracking����
        ((QWidget*)(objList.at(nIndex)))->setMouseTracking(true);
//        //�����QToolButton����
//        if(0==qstrcmp(objList.at(nIndex)->metaObject()->className(),"QToolButton"))
//        {
//            //����pressed�ź�Ϊslot_btnpress
//            connect(((QToolButton*)(objList.at(nIndex))),SIGNAL(pressed()),this,SLOT(slot_btnpress()));
//            //����clicked�ź�Ϊslot_btnclick
//            connect(((QToolButton*)(objList.at(nIndex))),SIGNAL(clicked()),this,SLOT(slot_btnclick()));
//            //���ö������
//            ((QToolButton*)(objList.at(nIndex)))->setContentsMargins(0,VALUE_DIS,0,0);
//        }
    }
}

//�����Ӳ�����ʽ(qss)
void TitleBar::SetWidgetStyle()
{
    //���ñ�ǩ���ı���ɫ����С���Լ���ť�ı߿�
    setStyleSheet("QLabel{color:#CCCCCC;font-size:12px;font-weight:bold;}Button{border:0px;}");
    //������߾�
   // m_pLabelTitle->setStyleSheet("margin-left:6px;");
    //�����ұ߾��Լ��������ȥʱ���ı���ɫ
    //m_pLabelVersion->setStyleSheet("QLabel{margin-right:10px;}QLabel:hover{color:#00AA00;}");
}

//�������ò���
void TitleBar::CreateLayout()
{
    //ˮƽ����
//    layout = new QHBoxLayout(this);
//    //��Ӳ���
////    layout->addWidget(iconLabel);
////    layout->addWidget(lineLabel);
//    //���������
//    layout->addStretch(1);
//    //��Ӳ���
//    layout->addWidget(closeButton);
////    layout->addWidget(m_pBtnSkin);
////    layout->addWidget(m_pBtnMenu);
////    layout->addWidget(m_pBtnMin);
////    layout->addWidget(m_pBtnMax);
////    layout->addWidget(m_pBtnClose);
//    //����Margin
//    layout->setContentsMargins(0,0,0,0);
////    layout->setContentsMargins(0,0,VALUE_DIS,0);

//    //���ò���֮���space
//    layout->setSpacing(0);
//    setLayout(layout);
}



//���ð�ť��ͬ״̬�µ�ͼ��
//void TitleBar::SetBtnIcon(QToolButton *pBtn,eBtnMoustState state,bool bInit/*=false*/)
//{
//    //���ͼƬ·��
//    QString strImagePath = GetBtnImagePath(pBtn,bInit);
//    //����QPixmap����
//    QPixmap objPixmap(strImagePath);
//    //�õ�ͼ���͸�
//    int nPixWidth = objPixmap.width();
//    int nPixHeight = objPixmap.height();
//    //���״̬������Чֵ
//    if(state!=eBtnStateNone)
//    {
//        /*���ð�ťͼƬ
//��ť��ͼƬ��������һ��ģ���ǰ1/4���ֱ�ʾĬ��״̬�µ�ͼƬ����,�Ӻ��1/4���ֱ�ʾ����Ƶ���ť״̬�µ�ͼƬ����
//*/
//        pBtn->setIcon(objPixmap.copy((nPixWidth/4)*(state-1),0,nPixWidth/4,nPixHeight));
//        //���ð�ťͼƬ��С
//        pBtn->setIconSize(QSize(nPixWidth/4,nPixHeight));
//    }
//}

////���ͼƬ·��(�̶�ֵ)
//const QString TitleBar::GetBtnImagePath(QToolButton *pBtn,bool bInit/*=false*/)
//{
//    QString strImagePath;
//    //Ƥ����ť
//    if(m_pBtnSkin==pBtn)
//    {
//        strImagePath = ":/image/SkinButtom.png";
//    }
//    //�˵���ť
//    if(m_pBtnMenu==pBtn)
//    {
//        strImagePath = ":/image/title_bar_menu.png";
//    }
//    //��С��
//    if(m_pBtnMin==pBtn)
//    {
//        strImagePath = ":/image/sys_button_min.png";
//    }
//    //���/��ԭ��ť�����԰�����󻯺ͻ�ԭ����ͼƬ
//    if(m_pBtnMax==pBtn)
//    {
//        //����ǳ�ʼ���û������������󻯱�־��Ϊ��(����MainWindow::Instance()ʹ�õ������ģʽ)
//        if(bInit==true || MainWindow::Instance()->GetMaxWin()==false)
//        {
//            //��󻯰�ťͼƬ·��
//            strImagePath = ":/image/sys_button_max.png";
//        }
//        else
//        {
//            //��ԭ��ťͼƬ·��
//            strImagePath = ":/image/sys_button_restore.png";
//        }
//    }
//    //�رհ�ť
//    if(m_pBtnClose==pBtn)
//    {
//        strImagePath = ":/image/sys_button_close.png";
//    }
//    return strImagePath;
//}

////�����¼�������
//void TitleBar::CreateEventFiter()
//{
//    m_pBtnSkin->installEventFilter(this);
//    m_pBtnMenu->installEventFilter(this);
//    m_pBtnMin->installEventFilter(this);
//    m_pBtnMax->installEventFilter(this);
//    m_pBtnClose->installEventFilter(this);
//}
////�¼�����
//bool TitleBar::eventFilter(QObject *obj, QEvent *event)
//{
//    //��ť״̬
//    eBtnMoustState eState = eBtnStateNone;
//    //�ж��¼�����--QEvent::Enter
//    if (event->type() == QEvent::Enter)
//    {
//        eState = eBtnStateHover;
//    }
//    //�ж��¼�����--QEvent::Leave
//    if (event->type() == QEvent::Leave)
//    {
//        eState = eBtnStateDefault;
//    }
//    //�ж��¼�����--QEvent::MouseButtonPress
//    if (event->type() == QEvent::MouseButtonPress && ((QMouseEvent*)(event))->button()== Qt::LeftButton)
//    {
//        eState = eBtnStatePress;
//    }
//    //�ж�Ŀ��
//    if(m_pBtnSkin==obj || m_pBtnMenu==obj || m_pBtnMin==obj || m_pBtnMax==obj || m_pBtnClose==obj)
//    {
//        //���״̬��Ч
//        if(eState != eBtnStateNone)
//        {
//            //����״̬���ð�ťͼ��
//            SetBtnIcon((QToolButton *)obj,eState);
//            return false;
//        }
//    }
//    return QWidget::eventFilter(obj,event);
//}

////�ۺ���--slot_btnclick
//void TitleBar::slot_btnclick()
//{
//    QToolButton *pBtn = (QToolButton*)(sender());
//    if(pBtn==m_pBtnMin)
//    {
//        emit signal_min();
//    }
//    if(pBtn==m_pBtnMax)
//    {
//        emit signal_maxrestore();
//    }
//    if(pBtn==m_pBtnClose)
//    {
//        emit signal_close();
//    }
//}













//��갴���¼�
void TitleBar::mousePressEvent(QMouseEvent *event)
{
//    qDebug()<< "TitleBar mousePress Event" << "the relative coordination"<< event->x()<< event->y();
//    if (event->button() == Qt::LeftButton)
//    {
//        if(event->y()<VALUE_DIS||event->x()<VALUE_DIS||rect().width()-event->x()<5)
//        {
//            event->ignore();
//            return;
//        }
//        else
//        {
//            pointPress = event->globalPos();
//            isLeftButtonPress = true;
//            //test
//            event->accept();
//            return;
//        }
//    }

    event->ignore();
}



//����ƶ��¼�
void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug()<< "TitleBar mouseMove Event" << "the relative coordination"<< event->x()<< event->y();
//    if(isLeftButtonPress)//�Ѿ����������ק����
//    {
//        pointMove = event->globalPos();
//        //�ƶ�������
//        ListWindow *pMainWindow = (qobject_cast<ListWindow *>(parent()));
//        pMainWindow->move(pMainWindow->pos() + pointMove - pointPress);
//        //��������pointPress;
//        pointPress = pointMove;

//        //test
//        event->accept();
////        event->ignore();
//        return;//
//    }
    event->ignore();
}
//����ͷ��¼�
void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
//    qDebug() << "TitleBar::mouseReleaseEvent";
    if (event->button() == Qt::LeftButton)
    {
        isLeftButtonPress = false;

//        //��ȡ������ָ��
//        ListWindow *pMainWindow = (qobject_cast<ListWindow *>(parent()));
//                //���ListWindow���ϱ��غ�ToolWindow���±��ؿ������������������ϵ�λ��
//                int disY = pMainWindow->toolWindow->geometry().bottom() - pMainWindow->geometry().top();
//                if((disY < 15) && (disY > -15)){
//                    move(pMainWindow->geometry().left(), pMainWindow->geometry().top() + disY);//�ƶ�����
//                    pMainWindow->toolWindow->setListWindowAdsorbedFlag(true);
//                }
//                else{
//                    pMainWindow->toolWindow->setListWindowAdsorbedFlag(false);
//                }

    }
    event->ignore();
}

//����ʱ���øú���
void TitleBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    closeButton->move(size().width() - 38, 0);

}

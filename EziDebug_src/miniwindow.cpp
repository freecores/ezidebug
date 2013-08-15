#include "miniwindow.h"
//#include "button.h"//����"miniwindow.h"��

#include <QtGui>

MiniWindow::MiniWindow(QWidget *parent) :
    QWidget(parent)
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("gb18030"));
    setMouseTracking (true);
    setWindowTitle(tr("EziDebug"));

    statusLabel = new QLabel(tr("         Status"));


    createActions();   //�����Ҽ��˵���ѡ��0
    createButtons();   //������ť

    //����ϵͳͼ��,״̬��ͼ��

    QPixmap objPixmap(tr(":/images/EziDebugIcon.bmp"));
    QPixmap iconPix;
    objPixmap.setMask(QPixmap(tr(":/images/EziDebugIconMask.bmp")));
    iconPix = objPixmap.copy(0, 0, 127, 104).scaled(32, 31);
    setWindowIcon(iconPix);


    //���ñ���
    //QPixmap maskPix;
    QPalette palette;
    backgroundPix.load(":/images/miniBackground.bmp",0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither);
    //maskPix.load(":/images/miniMask.bmp");
    palette.setBrush(QPalette::Background, QBrush(backgroundPix));
    //setMask(maskPix);
    setPalette(palette);
    //setMask(backgroundPix.mask());   //ͨ��QPixmap�ķ������ͼƬ�Ĺ��˵�͸���Ĳ��ֵõ���ͼƬ����ΪWidget�Ĳ�����߿�
    //setWindowOpacity(1.0);  //����ͼƬ͸����

    //���öԻ����λ�úʹ�С
    setGeometry(QRect(250,100,298,34));
    //setMinimumSize(480,42);
    setFixedSize(backgroundPix.size());//���ô��ڵĳߴ�ΪͼƬ�ĳߴ�
    setContentsMargins(0,0,0,0);
    Qt::WindowFlags flags = Qt::Widget;
    flags |= Qt::WindowStaysOnTopHint;//��ǰ����ʾ
    flags |= Qt::FramelessWindowHint;//����Ϊ�ޱ߿�
    setWindowFlags(flags);

    //�����ڰ�
    QPixmap maskPix;
    maskPix.load(":/images/miniMask.bmp");
    setMask(maskPix);
//    //����һ��λͼ
//    QBitmap objBitmap(size());
//    //QPainter������λͼ�ϻ滭
//    QPainter painter(&objBitmap);
//    //���λͼ���ο�(�ð�ɫ���)
//    painter.fillRect(rect(),Qt::white);
//    painter.setBrush(QColor(0,0,0));
//    //��λͼ�ϻ�Բ�Ǿ���(�ú�ɫ���)
//    painter.drawRoundedRect(this->rect(),8,8);
//    //ʹ��setmask���˼���
//    setMask(objBitmap);


    //��������ģʽ�µİ���
    //����λ��
//    proSettingButton->setGeometry(QRect(60, 1, 42, 41));//��������
//    proUpdateButton->setGeometry(QRect(102, 1, 42, 41));//����
//    proPartlyUpdateButton->setGeometry(QRect(144, 1, 42, 41));//���ָ���
//    deleteChainButton->setGeometry(QRect(186, 1, 42, 41));//ɾ��
//    testbenchGenerationButton->setGeometry(QRect(228, 1, 42, 41));//testbench����
//    proUndoButton->setGeometry(QRect(270, 1, 42, 41));//������undo��

//    minimizeButton->setGeometry(QRect(390, 0, 27, 19));
//    normalModeButton->setGeometry(QRect(416, 0, 27, 19));
//    exitButton->setGeometry(QRect(442, 0, 33, 19));

    //��������ģʽ�µİ���
     //�а��λ��
     proSettingButton->setGeometry(QRect(40, 4, 26, 25));
     proUpdateButton->setGeometry(QRect(65, 4, 26, 25));
     proPartlyUpdateButton->setGeometry(QRect(90, 4, 26, 25));
     deleteChainButton->setGeometry(QRect(115, 4, 26, 25));
     testbenchGenerationButton->setGeometry(QRect(140, 4, 26, 25));
     proUndoButton->setGeometry(QRect(165, 4, 26, 25));

     minimizeButton->setGeometry(QRect(240, 0, 16, 11));
     normalModeButton->setGeometry(QRect(257, 0, 16, 11));
     exitButton->setGeometry(QRect(274, 0, 20, 11));

     //    //С���λ��
//    proSettingButton->setGeometry(QRect(30, 2, 21, 20));//��������
//    proUpdateButton->setGeometry(QRect(50, 2, 21, 20));//����
//    proPartlyUpdateButton->setGeometry(QRect(70, 2, 21, 20));//���ָ���
//    deleteChainButton->setGeometry(QRect(90, 2, 21, 20));//ɾ��
//    testbenchGenerationButton->setGeometry(QRect(110, 2, 21, 20));//testbench����
//    proUndoButton->setGeometry(QRect(130, 2, 21, 20));//������undo��

//    minimizeButton->setGeometry(QRect(255, 0, 14, 10));
//    normalModeButton->setGeometry(QRect(268, 0, 14, 10));
//    exitButton->setGeometry(QRect(281, 0, 17, 10));

//    //miniIconLabel->setGeometry(QRect(100, 1, 105, 20));
//    minimizeButton->setGeometry(QRect(317, 4, 11, 10));
//    //showStatusButton->hide();
//    //minimizeButton->setGeometry(QRect(305, 4, 11, 10));
//    //showStatusButton->setGeometry(QRect(317, 4, 11, 10));
//    normalModeButton->setGeometry(QRect(329, 4, 11, 10));
//    exitButton->setGeometry(QRect(341, 4, 11, 10));


    //״̬��ʾ
//    statusRoll = new RollCaption(this);
//    statusRoll->setGeometry(QRect(118, 2, 162, 21));
//    statusRoll->setText(tr("status of EziDebug, this is a demo, just for test. "));
////    statusRoll->setSpeed(50);
////    statusRoll->setcolor(QColor(0, 0, 0));

////    QLabel *statusLabel1 = new QLabel(tr("Status"));
////    //statusLabel1->setTextFormat();
////    statusLabel1->setGeometry(QRect(118, 2, 170, 21));

//    //����ģʽ�µ�״̬��
    statusWidget = new QWidget;
//    QHBoxLayout *StatusLayout = new QHBoxLayout;
//    StatusLayout->addWidget(statusLabel);
//    StatusLayout->setMargin(0);
//    statusWidget->setLayout(StatusLayout);
//    flags =  Qt::Widget;
//    flags |= Qt::WindowStaysOnTopHint;//��ǰ����ʾ
//    flags |= Qt::FramelessWindowHint;//����Ϊ�ޱ߿�
//    statusWidget->setWindowFlags(flags);

//    palette.setColor(QPalette::Background, QColor(27,61,125));
//    statusWidget->setPalette(palette);
//    //statusWidget->setFont();

//    statusWidget->resize(size());
//    QPoint p = frameGeometry().topLeft();
//    p.setX(p.rx() + width());
//    statusWidget->move(p);

    statusWidget->setHidden(true);
    isMiniStatusLabelHidden = true;

}

void MiniWindow::createButtons()
{
    //��������ť  tr("��������")
    proSettingButton = createToolButton(tr("Set Project Parameter"),
                                        tr(":/images/projectSetting.bmp"),
                                        QSize(26, 25));//(42, 41)

	// tr("����")
    proUpdateButton = createToolButton(tr("Update"),
                                       tr(":/images/projectUpdate.bmp"),
                                       QSize(26, 25));
    // tr("���ָ���")
    proPartlyUpdateButton = createToolButton(tr("Update fast"),
                                       tr(":/images/projectPartlyUpdate.bmp"),
                                       QSize(26, 25));

	// tr("ɾ��")
    deleteChainButton = createToolButton(tr("Delete all scan chain"),
                                         tr(":/images/deleteChain.bmp"),
                                         QSize(26, 25));

	// tr("testbench����")
    testbenchGenerationButton = createToolButton(tr("Gnerate testbench"),
                                                 tr(":/images/testbenchGeneration.bmp"),
                                                 QSize(26, 25));
    // tr("����")
    proUndoButton = createToolButton(tr("Undo"),
                                                     tr(":/images/undo.bmp"),
                                                     QSize(26, 25));



    //���ϽǱ�������ť
    // tr("��С��")
    minimizeButton = createToolButton(tr("Minimize"),
                                          tr(":/images/ToolWindowminimize.bmp"),
                                          QSize(20, 14));//QSize(27, 19)

	// tr("��ͨģʽ")
    normalModeButton = createToolButton(tr("Normal mode"),
                                        tr(":/images/ToolWindowNormal.bmp"),
                                        QSize(20, 14));
    // tr("�˳�")
    exitButton = createToolButton(tr("Quit"),
                                      tr(":/images/ToolWindowExit.bmp"),
                                      QSize(24, 14));//QSize(33, 19)



//ԭ����,��connect���󹩲ο�
//    minimizeButton = createToolButton(tr("��С��"),
//                                          QIcon(":/images/miniMinimize.bmp"),
//                                          QSize(11, 10),
//                                           SLOT(minimize()));

//    showStatusButton = createToolButton(tr("����״̬"),
//                                            QIcon(":/images/miniShowStatus.bmp"),
//                                            QSize(11, 10),
//                                            SLOT(showStatusWedgit()));

//    normalModeButton = createToolButton(tr("��ͨģʽ"),
//                                        QIcon(":/images/miniNormal.bmp"),
//                                        QSize(11, 10),
//                                        SIGNAL(toNormalMode()));

//    exitButton = createToolButton(tr("�˳�"),
//                                      QIcon(":/images/miniExit.bmp"),
//                                      QSize(11, 10),
//                                      SLOT(close()));
    //connect(minimizeButton, SIGNAL(clicked()), this, SLOT(minimize()));//��toolwindow��connect
    connect(normalModeButton, SIGNAL(clicked()), this, SIGNAL(toNormalMode()));
    connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));

}

void MiniWindow::createActions()
{   
    // tr("�˳�")
    exitAct = new QAction(tr("Quit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    //exitAct->setStatusTip(tr("�˳�"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	// tr("��С��")
    minimizeAct = new QAction(tr("Minimize"), this);
    //minimizeAct->setShortcuts(QKeySequence::);
    //minimizeAct->setStatusTip(tr("Exit the application"));
    connect(minimizeAct, SIGNAL(triggered()), this, SLOT(minimize()));

	// tr("��ͨģʽ")
    toNormalModeAct = new QAction(tr("Normal mode"), this);
    //normalAct->setShortcuts(QKeySequence::Quit);
    //normalAct->setStatusTip(tr("Exit the application"));
    connect(toNormalModeAct, SIGNAL(triggered()), this, SIGNAL(toNormalMode()));

}

Button *MiniWindow::createToolButton(const QString &toolTip,
                                          const QString &iconstr,const QSize &size)
{
    Button *button = new Button(iconstr, this);
    button->setToolTip(toolTip);
    //button->setIcon(icon);
    button->setIconSize(size);//(QSize(10, 10));
    // button->setSizeIncrement(size);
    //button->setSizePolicy(size.width(), size.height());
    button->setFlat(true);
    //connect(button, SIGNAL(clicked()), this, member);

    return button;
}

void MiniWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(minimizeAct);
    menu.addAction(toNormalModeAct);
    menu.addAction(exitAct);
    menu.exec(event->globalPos());
}

void MiniWindow::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) //���������
    {
        //globalPos()��ȡ�����ڵ����·����frameGeometry().topLeft()��ȡ���������Ͻǵ�λ��
            dragPosition = event->globalPos() - frameGeometry().topLeft();

        event->accept();   //����¼���ϵͳ����
    }
    //    if (event->button() == Qt::RightButton)
    //    {
    //         close();
    //    }
}

void MiniWindow::mouseMoveEvent(QMouseEvent * event)
{
    if (event->buttons() == Qt::LeftButton) //�����������������ʱ��
    {

            QPoint p = event->globalPos() - dragPosition;
            move(p);//�ƶ�����
            p.setX(p.rx() + width());
            statusWidget->move(p);

        event->accept();
    }
}


//���´���(ֱ��"exitButton->move(size().width() - 17, 4);"һ��)
//��Ϊmini���ڿɷŴ���С��ǰ�������õ�,��ע�͵�,�Ժ����������������
////������ƶ����������ڲ���Χ5����ʱ���ı������״�������������϶�ʱ�������϶���������������λ�úʹ�С���ü��ɡ�
////��갴���¼�
//void MiniWindow::mousePressEvent(QMouseEvent *event)
//{
//    if (event->button() == Qt::LeftButton)
//    {
//        isLeftButtonPress = true;

//        //�ж��Ǵ����ƶ���������
//        eDirection = (enum_Direction)PointValid(event->x());
//        if(eDirection == eNone)//�Ǵ����ƶ�
//        {
//            isWindowMovement = true;
//            //globalPos()��ȡ�����ڵ����·����frameGeometry().topLeft()��ȡ���������Ͻǵ�λ��
//            dragPosition = event->globalPos() - frameGeometry().topLeft();
//        }
//        else//�Ǵ�������
//        {
//            isWindowMovement = false;
//            pointPressGlobal = event->globalPos();
//        }

//        event->accept();   //����¼���ϵͳ����
//    }
//}
////����ƶ��¼�
//void MiniWindow::mouseMoveEvent(QMouseEvent *event)
//{
//    if(!isLeftButtonPress)
//    {
//        eDirection = (enum_Direction)PointValid(event->x());
//        SetCursorStyle(eDirection);
//    }
//    else
//    {
//        if(eDirection == eNone)//�Ǵ����ƶ�
//        {
//            QPoint p = event->globalPos() - dragPosition;
//            move(p);//�ƶ�����
//            //                        p.setX(p.rx() + width());
//            //                        statusWidget->move(p);

//        }
//        else//�Ǵ�������
//        {
//            int nXGlobal = event->globalX();
//            int nYGlobal = event->globalY();
//            SetDrayMove(nXGlobal, eDirection);
//            pointPressGlobal =QPoint(nXGlobal,nYGlobal);
//        }
//    }

//    event->accept();

//}

////����ͷ��¼�
//void MiniWindow::mouseReleaseEvent(QMouseEvent *event)
//{
//    if (event->button() == Qt::LeftButton)
//    {
//        isLeftButtonPress = false;
//        eDirection = eNone;
//    }
//}

////�ж��������λ���ڵ�ǰ���ڵ�ʲôλ�ã����� �߽��ϣ����߶�����
//int MiniWindow::PointValid(int x)
//{
//    enum_Direction direction = eNone;
//    if ((x >= 0) && (x < 6))
//    {

//            direction = eLeft;
//    }
//    else if((x > this->width() - 6) && (x <= this->width()))
//    {

//            direction = eRight;
//    }

//   return (int)direction;
//}

////���������ʽ
//void MiniWindow::SetCursorStyle(enum_Direction direction)
//{
//    //�������ҵ������״
//    switch(direction)
//    {
//    case eRight:
//    case eLeft:
//        setCursor(Qt::SizeHorCursor);
//        break;
//    default:
//        setCursor(Qt::ArrowCursor);
//        break;
//    }
//}

////��������϶��Ĵ���λ����Ϣ
//void MiniWindow::SetDrayMove(int nXGlobal, enum_Direction direction)
//{
//    //���������λ����Ϣ
//    QRect rectWindow = geometry();
//    //�б���
//    switch(direction)
//    {

//    case eRight:
//        rectWindow.setRight(nXGlobal);
//        break;
//    case eLeft:
//        rectWindow.setLeft(nXGlobal);
//        break;
//    default:
//        break;
//    }

//    if(rectWindow.width()< minimumWidth())
//    {
//        return;
//    }
//    //�������ô���λ��Ϊ��λ����Ϣ
//    setGeometry(rectWindow);
//}

//// ���Ŵ���仯�����ñ���
//void MiniWindow::resizeEvent(QResizeEvent *event)
//{
//    QWidget::resizeEvent(event);

//    //����ͼƬ����
//    //����һ��sizeΪ�仯��size���»���
//    QImage img(event->size(), QImage::Format_ARGB32);
//    QPainter *paint = new QPainter(&img);
//    //��������ͼ����-fixed����-scaled����-fixed
//    paint->drawPixmap(0, 0, backgroundPix.copy(0, 0, 120, 25));
//    paint->drawPixmap(120, 0, backgroundPix.copy(120, 0, 156, 25).scaled(QSize(event->size().width() - 202, 25),
//                                                                         Qt::IgnoreAspectRatio,
//                                                                         Qt::SmoothTransformation));
//    paint->drawPixmap(event->size().width() - 82, 0, backgroundPix.copy(276, 0, 82, 25));
//    paint->end();
//    //setPixmap(QPixmap::fromImage(displayImg));
//    QPalette pal(palette());
//    pal.setBrush(QPalette::Window,QBrush(img));
//    //    pal.setBrush(QPalette::Window,
//    //                 QBrush(backgroundPix.scaled(event->size(),
//    //                                              Qt::IgnoreAspectRatio,
//    //                                              Qt::SmoothTransformation)));
//    setPalette(pal);



//    //�����ڰ�
//    //����һ��λͼ
//    QBitmap objBitmap(size());
//    //QPainter������λͼ�ϻ滭
//    QPainter painter(&objBitmap);
//    //���λͼ���ο�(�ð�ɫ���)
//    painter.fillRect(rect(),Qt::white);
//    painter.setBrush(QColor(0,0,0));
//    //��λͼ�ϻ�Բ�Ǿ���(�ú�ɫ���)
//    painter.drawRoundedRect(this->rect(),8,8);
//    //ʹ��setmask���˼���
//    setMask(objBitmap);

//    //�ƶ��Ҳఴť��λ��
//    minimizeButton->move(size().width() - 41, 4);
//    normalModeButton->move(size().width() - 29, 4);
//    exitButton->move(size().width() - 17, 4);


//}






//void MiniWindow::paintEvent(QPaintEvent *)
//{
//    QPainter painter(this);//����һ��QPainter����
//    painter.drawPixmap(0,0,QPixmap(":/images/Watermelon.png"));//����ͼƬ������
//    /*
//      QPixmap(":/images/Watermelon.png")�����ΪQPixmap()����ֻ�ܿ������Ƴ��Ŀ�ܣ�������ͼƬ��ɫ��Ҳ���ǿ�����ͼƬ��
//    */
//}

void MiniWindow::minimize()
{
    this->hide();
    if(isMiniStatusLabelHidden == false)
        statusWidget->hide();
}

void MiniWindow::showStatusWedgit()
{

    if(isMiniStatusLabelHidden == true){
        isMiniStatusLabelHidden = false;
        statusWidget->setHidden(false);

    }
    else{
        isMiniStatusLabelHidden = true;
        statusWidget->setHidden(true);
    }

}

//void MiniWindow::toNormalMode()
//{
////    mainWindow->show();
////    if(isNormalListWindowHidden == false)
////        listWindow->show();
//    statusWidget->hide();
//    this->hide();
//}

void MiniWindow::close()
{
//    listWindow->close();
//    mainWindow->close();
    statusWidget->close();
    QWidget::close();
}

bool MiniWindow::statusWidgetHidden()
{
    return isMiniStatusLabelHidden;
}

void MiniWindow::setStatusWidgetHidden(bool flag)
{
    if(flag == true)
        statusWidget->hide();
    else
        statusWidget->show();
}



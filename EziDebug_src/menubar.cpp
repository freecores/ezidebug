#include "menubar.h"

#include <QtGui>

MenuBar::MenuBar(QWidget *parent):
    QMenuBar(parent)
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("gb18030"));
    setMouseTracking (false);
//    setMinimumSize(290, 18);
//    setFixedHeight(18);
    //setStyleSheet("QAction{color:#CCCCCC;font-size:16px;border:0px;}");


}

//��갴���¼�
void MenuBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if(event->x()<5 || rect().width()-event->x()<5)
        {
            event->ignore();
            return;
        }
        else
        {
            QMenuBar::mousePressEvent(event);
            event->accept();
            return;
        }
    }

    event->ignore();
}

//����ƶ��¼�
void MenuBar::mouseMoveEvent(QMouseEvent *event)
{
    QMenuBar::mouseMoveEvent(event);
    event->ignore();
}

void MenuBar::mouseReleaseEvent(QMouseEvent *event)
{
    QMenuBar::mouseReleaseEvent(event);
    event->ignore();
}


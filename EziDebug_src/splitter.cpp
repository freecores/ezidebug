#include "splitter.h"

#include <QtGui>
#include <QDebug>

Splitter::Splitter(QWidget *parent) :
    QSplitter(parent)
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("gb18030"));
    setMouseTracking (true);

}

Splitter::Splitter(Qt::Orientation orientation, QWidget *parent):
    QSplitter(orientation, parent)
{
    setMouseTracking (true);

}

//��갴���¼�
void Splitter::mousePressEvent(QMouseEvent *event)
{
//    if (event->button() == Qt::LeftButton)
//    {
//        if(event->y()<5 || event->x()<5 || rect().width()-event->x()<5 || rect().height()-event->y()<5 )
//        {
////            int a  = rect().height();
////            int b = event->y();
////            int c = rect().width();
////            QRect r = geometry();
//            event->ignore();
//            return;
//        }
//        else
//        {
//            event->accept();
//        }
//    }
    //qDebug()<< "Splitter::mousePressEvent";
    QSplitter::mousePressEvent(event);
    event->ignore();
//    event->accept();
}
//����ƶ��¼�
void Splitter::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug()<< "Splitter::mouseMoveEvent";
    QSplitter::mouseMoveEvent(event);
    event->ignore();
//  event->accept();
}

void Splitter::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug()<< "Splitter::mouseReleaseEvent";
    QSplitter::mouseReleaseEvent(event);
    event->ignore();
//    event->accept();
}

////����ʱ���øú���
//void Splitter::resizeEvent(QResizeEvent *event)
//{
//    QSize s = event->size();
//    QRect r = rect();
//    QSize s2 = event->oldSize();
//    QWidget::resizeEvent(event);
//    this->resize(event->size());

//}
void Splitter::contextMenuEvent(QContextMenuEvent *event)
{
//    QMenu* popMenu = new QMenu(this);
//    popMenu->addAction(new QAction(tr("���"), this));
//    popMenu->addAction(new QAction(tr("ɾ��"), this));
////    if(this->itemAt(mapFromGlobal(QCursor::pos())) != NULL) //�����item�����"�޸�"�˵� [1]*
////    {
////        popMenu->addAction(new QAction("�޸�", this));
////    }

//    popMenu->exec(QCursor::pos()); // �˵����ֵ�λ��Ϊ��ǰ����λ��
}


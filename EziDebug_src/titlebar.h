#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>

#include "button.h"

//#typedef  5 VALUE_DIS
const int VALUE_DIS = 5;

QT_BEGIN_NAMESPACE
//class QAction;
//class QActionGroup;
class QLabel;
class QPoint;
class QHBoxLayout;
//class QPushButton;
class QToolButton;
//class QTreeWidget;
//class QImage;
//class FindDialog;
QT_END_NAMESPACE

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = 0);

    //ö�٣���ť״̬
    enum eBtnMoustState{
        eBtnStateNone,//��Ч
        eBtnStateDefault,//Ĭ��ֵ(�簴ť��ʼ��ʾ)
        eBtnStateHover,//����Ƶ���ť��״̬
        eBtnStatePress//��갴�°�ťʱ״̬
    };

    Button *closeButton;//�رհ�ť


//    QLabel *iconLabel;//������Сͼ��
//    QLabel *lineLabel;//�м�ĳ�ֱ��
//    QLabel *closeButtonLabel;//���Ҳ�رհ�����ͼ��

signals:

public slots:

private:
    void mousePressEvent(QMouseEvent *);        //�Զ���һ��������¼�����
    void mouseMoveEvent(QMouseEvent *);         //�Զ���һ������϶��¼�����
    void mouseReleaseEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);

    QPoint pointPress;//��¼����������ʱ��ȫ��λ��
    QPoint pointMove;//��¼����ƶ����ȫ��λ��
    bool isLeftButtonPress;//�Ƿ���������

    QHBoxLayout *layout;
//  QPixmap background;//����ͼƬ



    void CreateWidget();//�����Ӳ���
    void SetWidgetStyle();//�����Ӳ�����ʽ(qss)
    void CreateLayout();//�������ò���

    //void SetBtnIcon(QToolButton *pBtn,eBtnMoustState state,bool bInit=false);//���ð�ť��ͬ״̬�µ�ͼ��

};

#endif // TITLEBAR_H

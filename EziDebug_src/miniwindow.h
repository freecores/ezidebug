#ifndef MINIWINDOW_H
#define MINIWINDOW_H

#include <QWidget>
#include "button.h"

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QPushButton;
class QPoint;
QT_END_NAMESPACE

class MiniWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MiniWindow(QWidget *parent = 0);
    bool statusWidgetHidden();
    void setStatusWidgetHidden(bool);
    enum enum_Direction{
        eNone = 0,
        eRight,
        eLeft};

    //��������ť  
    Button *proSettingButton;  //��������
    Button *proUpdateButton;   //����
    Button *proPartlyUpdateButton;   //���ָ���
    Button *deleteChainButton; //ɾ��
    Button *testbenchGenerationButton;//testbench����
    Button *proUndoButton;   //������undo��

    //���ϽǱ�������ť
    QPushButton *minimizeButton;    //��С��
    QPushButton *showStatusButton;  //��ʾ����״̬//��ɾ��
    QPushButton *normalModeButton;  //��ͨģʽ
    QPushButton *exitButton;        //�˳�
signals:
    void toNormalMode();

public slots:
    //���ϽǱ�������ť
    void minimize();
    void showStatusWedgit();
    void close();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
//    //�ĸ����̲�����Ӧ�İ�ť//����toolwindow��ʵ��
//    void proSetting();
//    void proUpdate();
//    void deleteChain();
//    void testbenchGeneration();

private:
    void createActions();   //�����Ҽ��˵���ѡ��
    void createButtons();   //������ť
    //������ʾ��,connect����toolwindow�����
    //QPushButton *createToolButton(const QString &toolTip, const QIcon &icon, const QSize &size, const char *member);
    Button *createToolButton(const QString &toolTip, const QString &iconstr,const QSize &size);

    void mousePressEvent(QMouseEvent *);        //�Զ���һ��������¼�����
    void mouseMoveEvent(QMouseEvent *);         //�Զ���һ������϶��¼�����
//    void mouseReleaseEvent(QMouseEvent *event);
//    //void paintEvent(QPaintEvent *);             //�Զ���һ��ˢ���¼�����
//    void resizeEvent(QResizeEvent *event);      //�Զ���һ���ı䴰�ڴ�С�¼����������Ŵ���仯�����ñ���

    //void mouseDoubleClickEvent(QMouseEvent *event);//�Զ���һ�����˫���¼�����
//    void SetCursorStyle(enum_Direction direction);//���������ʽ
//    int PointValid(int x);//�ж��������λ���ڵ�ǰ���ڵ��ĸ��߽磨���ң���
//    void SetDrayMove(int nXGlobal,enum_Direction direction);//��������϶��Ĵ���λ����Ϣ

    //�Ҽ��˵���ѡ��
    QAction *exitAct;//�˳�
    QAction *minimizeAct;   //��С��
    QAction *toNormalModeAct; //ת������ͨģʽģʽ


    //����һ��QPoint�ĳ�Ա��������¼�����ƶ���λ��
    QPoint dragPosition;

    //����ģʽ�µ�״̬��
    QWidget *statusWidget;
    QLabel *statusLabel;//����״̬
    bool isMiniStatusLabelHidden;//miniģʽ��״̬���Ƿ�����

    //����������Ҫ�ı���
    QPixmap backgroundPix;//���ڱ���ͼƬ
    QPoint pointPressGlobal;//��¼����������ʱ��ȫ��λ��
    QPoint pointMove;//��¼����ƶ����ȫ��λ��
    QRect rectRestoreWindow;//��¼��ǰ���ڵĴ�С��λ��
    bool isLeftButtonPress;//�Ƿ���������
    bool isWindowMovement ;//�Ƿ��Ǵ����ƶ�
    enum_Direction eDirection;//��¼������Ǹ�����


    //����Action��Button���õĶԻ���


};

#endif // MINIWINDOW_H

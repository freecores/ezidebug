#ifndef LISTWINDOW_H
#define LISTWINDOW_H

#include <QWidget>

//#include "toolwindow.h"
//#include "miniwindow.h"
#include "listwindow.h"
#include "projectsetwizard.h"
#include "finddialog.h"

#include "button.h"

#include "titlebar.h"
#include "menubar.h"
#include "splitter.h"
#include "treemodel.h"


QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QMenuBar;
class QPushButton;
class QSplitter;
class QTreeWidget;
class QImage;
class TreeView;
class EziDebugTreeModel ;
class QStandardItemModel ;
class QStandardItem ;
class EziDebugPrj ;
//class FindDialog;
QT_END_NAMESPACE

class ListWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ListWindow(QWidget *parent = 0, Qt::WindowFlags f = 0 );
    ~ListWindow();
    QStandardItem *addMessage(const QString &type ,const QString &message,QStandardItem *parentitem = NULL) ;
    //�б����Ƿ�����
    bool isListWindowHidden();
    void setListWindowHidden(bool flag);
    void enterEvent(QEvent * event);
    void leaveEvent(QEvent * event);
    void setChainActEnable(bool addenable,bool delenable);
    EziDebugInstanceTreeItem * getCurrentTreeItem(void);
    void setCurrentTreeItem(EziDebugInstanceTreeItem * item);
    void clearTreeView(void) ;
    void welcomeinfoinit(EziDebugPrj *prj) ;

    //�б����Ƿ������ڹ���������
    //bool isListWindowAdsorbed();
    //��ȡToolWindow��ָ�룬�Ի�ȡ��λ�õ���Ϣ
    void getToolWindowPointer(QWidget *toolWin);
    bool windowIsStick(void) const
    {
        return isStick;
    }
    void setWindowStick(bool stick)
    {
        isStick = stick ;
    }


    enum enum_Direction{
        eNone = 0,
        eTop,
        eBottom,
        eRight,
        eLeft,
        eTopRight,
        eBottomLeft,
        eRightBottom,
        eLeftTop
    };


signals:
    void mouseReleased(const QRect rect);//�����ƶ��󣬽�listWindow�Ĵ���λ�÷��͸�toolWindow

public slots:
    //���ϽǱ�������ť
    void close();//�������б���

    //�㷨���õ��Ĳ�
    //void proSetting();
    //void proUpdate();
    //void deleteChain();
    void testbenchGeneration();

private slots:

//    //�������ںͱ������йصı�������ť
//    void closeAll();//����ģʽ�͹��������ڹر�ʱ�������òۣ��ر����д���
//    void toMiniMode();  //��ͨģʽת��������ģʽ
//    void toNormalMode();//����ģʽת������ͨģʽ
    //void moveListWindow(const QPoint &p, QPoint bottomLeft, QPoint bottomRight);//�ƶ�ListWindow����p




    void find();
    void about();
    void show_contextmenu(const QPoint& pos) ;
    void generateTreeView(EziDebugInstanceTreeItem* item);



private:
    void createActions();   //����ѡ��
    void createMenus();     //�����˵�
    void createButtons();   //������ť
    //listwindow������button,�ú������Ѳ���,�رհ�ť����titlebar��ʵ��
    QPushButton *createToolButton(const QString &toolTip, const QIcon &icon, const QSize &size, const char *member);
    void mousePressEvent(QMouseEvent *);        //�Զ���һ��������¼�����
    void mouseMoveEvent(QMouseEvent *);         //�Զ���һ������϶��¼�����
    void mouseReleaseEvent(QMouseEvent *);
    void moveEvent(QMoveEvent *);  // �����ƶ��¼�
//    void paintEvent(QPaintEvent *);             //�Զ���һ��ˢ���¼�����
    void resizeEvent(QResizeEvent *event);      //�Զ���һ���ı䴰�ڴ�С�¼����������Ŵ���仯�����ñ���

    void mouseDoubleClickEvent(QMouseEvent *event);//�Զ���һ�����˫���¼�����
    void SetCursorStyle(enum_Direction direction);//���������ʽ
    int PointValid(int x, int y);//�ж��������λ���ڵ�ǰ���ڵ��ĸ��߽磨�������ң���
    void SetDrayMove(int nXGlobal,int nYGlobal,enum_Direction direction);//��������϶��Ĵ���λ����Ϣ
    void magneticMove(const QRect &bechmarkRect, const QRect &targetRect) ;

    //�˵�
    MenuBar *menuBar;
    QMenu *addMenu;
    QMenu *checkMenu;
    QMenu *sortMenu;
    QMenu *findMenu;
    QMenu *helpMenu;
    QMenu * m_pcontextMenu ;


    QAction *proSettingWizardAct;   //����������
    QAction *setProPathAct;     //���ù���Ŀ¼
    QAction *setRegNumACt;      //����ÿ�����ļĴ�������
    QAction *useVerilogAct;    //ʹ��verilog����
    QAction *useVHDLAct;        //ʹ��VHDL����
    QAction *useMixeLanguagedAct;    //ʹ�û������
    QAction *setSlotAct;        //���ü�ʱ����ʱ��
    QAction *useAlteraAct;      //ʹ��Altera
    QAction *useXilinxAct;      //ʹ��Xilinx
    QAction *exitAct;           //�˳�

    QAction *rankAfterFileNameAct;  //���ļ���
    QAction *rankAfterPathNameAct;  //��·����

    QAction *fastOrientAct; //���ٶ�λ
    QAction *findAct;       //����
    QAction *findNextAct;   //������һ��

    QAction *aboutEziDebugAct;  //����
    QAction *helpFileAct;   //�����ļ�

    QAction *m_paddChainAct ;  // �����
    QAction *m_pdeleteChainAct ; // ɾ����

    //���ϽǱ�������ť
    //QPushButton *exitButton;  //�˳�
    Button * closeButton ;
    QPushButton * button1 ;

    Splitter *mainSplitter;
    QTreeWidget *modulesTreeWidget;
    QTreeWidget *chainsTreeWidget;
    TreeItem*  m_ptestTreeItem ; // ���Դ���Ҽ������õ���״�ڵ�
    EziDebugInstanceTreeItem * m_ptreeItem ; //
    TreeModel *moduleTree;//��Ÿ���module�����໥��ϵ�������б�
    EziDebugTreeModel * m_peziDebugTreeModel ;
    TreeView *moduleTreeView;  //��ʾmodule����view
    TreeView *m_pmessageTreeView ;
    QStandardItemModel *m_iShowMessageModel ;
	
	
    //����������Ҫ�ı���
    TitleBar *titleBar;//������
    QPoint pointPressGlobal;//��¼����������ʱ��ȫ��λ��
    QPoint pointMove;//��¼����ƶ����ȫ��λ��
    QRect rectRestoreWindow;//��¼��ǰ���ڵĴ�С��λ��
    bool isLeftButtonPress;//�Ƿ���������
    bool isMaxWin;//�ô����Ƿ����
    bool isStick;
    bool isDrag;


    QPoint oldPointGlobal;
    QPoint NewPointGlobal;
    QPoint oldWindowPoint ;
    QPoint mousePressPoint ;  //��¼��갴��ʱ���������
    QPoint mouseMovePoint ;  // ��¼����ƶ��е��������
    QRect  parentRect ; // ��¼�����ڵ�����
    QPoint diffPos;    // ��������

    enum_Direction eDirection;//��¼������Ǹ�����

    QWidget *toolWindow;//ָ��ToolWindow��ָ��
	
	
    bool isListWindowHiddenFlag;//��ͨģʽ�£�ToolWindow������ʱ���б����Ƿ�����
    //bool isListWindowAdsorbedFlag;//��ͨģʽ��,�б�����Ƿ������ڹ��������ڵ��·�



    //����һ��QPoint�ĳ�Ա��������¼�����ƶ���λ��
    QPoint dragPosition;

    //���ڱ���ͼƬ
    QPixmap listBackground;

    //����Action��Button���õĶԻ���

    FindDialog *findDialog;//����

    //�㷨���õ��Ĳ���
    QString proPathStr;         //����Ŀ¼
    QVector<int> regNumVector;  //ÿ�����ļĴ�������
    bool isUseVerilog;          //ʹ��verilog����
    bool isUseVHDL;             //ʹ��VHDL����
    int slot;                   //��ʱ����ʱ��
    bool isUseAlteraAct;        //ʹ��Altera
    bool isUseXilinxAct;        //ʹ��Xilinx

    //QList<QStandardItem*> m_iitemList ;
};

#endif // LISTWINDOW_H

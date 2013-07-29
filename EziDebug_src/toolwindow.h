#ifndef TOOLWINDOW_H
#define TOOLWINDOW_H

#include <QWidget>
#include <QSystemTrayIcon>

#include "miniwindow.h"
#include "listwindow.h"
//#include "ezidebugprj.h"
#include "projectsetwizard.h"


QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class Button;
class QProgressBar;
class QMenu;
class QSystemTrayIcon;
class EziDebugPrj ;
class QTimer ;
QT_END_NAMESPACE

class ToolWindow : public QWidget
{
    Q_OBJECT
public:

    explicit ToolWindow(QWidget *parent = 0);
//    bool eventFilter(QObject *, QEvent *);
    ~ToolWindow();

//    //�б����Ƿ�����
//    void setListWindowHiddenFlag(bool flag);
//    bool getListWindowHiddenFlag();
//    //�б����Ƿ������ڹ���������
      void  setListWindowAdsorbedFlag(bool flag);
      const EziDebugPrj* getCurrentProject(void) const; // ��ȡ��ǰ����ָ��
      void  setCurrentProject(EziDebugPrj*);  // ���õ�ǰ����ָ��
      void  listwindowInfoInit(void) ;


//    bool getListWindowAdsorbedFlag();
    //void getListWindowPointer(QWidget *listWindow);

signals:
     void updateTreeView(EziDebugInstanceTreeItem* item);
    //void closeToolWindow();//�رձ�����
    //void hideListWindow();
    //void showListWindow();
    //void moveListWindow(const QPoint &p, QPoint bottomLeft, QPoint bottomRight);//�ƶ�ListWindow����p
    //void toMiniMode();  //��ͨģʽת��������ģʽ

public slots:
    //���ϽǱ�������ť
    void minimize();    //��С��
    void toMiniMode();  //ת��������ģʽ
    void close();       //�ر�
    void updateIndicate(); // ������ʾ
    void fastUpdate();    // ���ٸ���
    void changeUpdatePic() ;
    void changeProgressBar(int value) ;

    void about();//����
    void help();//����

    //����С����ԭ
    void showNormal();
    //�Ҳ��в���ť����toolwindow�·����б���
    void showListWindow();
    //ϵͳ������غ���
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    //����ģʽ
    void toNormalMode();//ת������ͨģʽ
    void miniWindowMinimized();//��С��ʱ�޸���Ӧ�˵�ѡ��
    void proSetting(); // ��������
    void proUpdate() ; // ���̸���
    //void proPartlyUpdate();// ���̲��ָ���
    int  deleteScanChain(); // ɾ��һ��ɨ����
    void addScanChain();    // ���һ��ɨ����
    int  deleteAllChain();  // ɾ������ɨ����
    void undoOperation(); // ȡ����һ������
    void testbenchGeneration(); // ����testbench

    //    ��Ϊ��listWindow�е�slotִ��//�ĸ����̲�����Ӧ�İ�ť
    //    void proSetting();
    //    void proUpdate();
    //    void deleteChain();
    //    void testbenchGeneration();



    void progressBarDemo();//��������ʾ
    void listWindowMouseReleased(const QRect rect);//listWindow�Ĵ����ƶ������ź��ж�listWindow�Ƿ�����

protected:


private:
    void createActions();   //�����Ҽ��˵���ѡ��
    void createButtons();   //������ť
    Button *createToolButton(const QString &toolTip, const QString &iconstr,
                             const QSize &size, const QObject * receiver, const char *member);
    void contextMenuEvent(QContextMenuEvent *event);//�Զ����Ҽ��˵�
    void mousePressEvent(QMouseEvent *);        //�Զ���һ��������¼�����
    void mouseMoveEvent(QMouseEvent *);         //�Զ���һ������϶��¼�����
    void paintEvent(QPaintEvent *);             //�Զ���һ��ˢ���¼�����
    void showEvent(QShowEvent*);                //�Զ���һ��������ʾ�¼�����
    void readSetting();                         //��ȡ�ϴ��������Ĺ�����Ϣ
    void writeSetting();                        //��������򿪹��̵���Ϣ


    //ϵͳ������غ���
    void CreatTrayMenu();
    void creatTrayIcon();
    void closeEvent(QCloseEvent *event);



    //������������
    MiniWindow *miniWindow;//����ģʽ�µ������ں�״̬��
    ListWindow *listWindow;//��ͨģʽ�µĹ���������
    ProjectSetWizard * m_proSetWiz;//����������
//  EziDebugPrj* m_pcurrentPrj ;

    //ϵͳ���̼���˵�
    QSystemTrayIcon *myTrayIcon;
    QAction *miniSizeAction;
    QAction *maxSizeAction;
    QAction *restoreWinAction;
    QAction *quitAction;
    QAction *aboutAction;
    QAction *helpAction;
    QMenu *myMenu;



    //�Ҽ��˵���ѡ��
    QAction *exitAct;//�˳�
    QAction *minimizeAct;   //��С��
    QAction *toMiniModeAct; //ת��������ģʽģʽ
    QAction *aboutAct;
    QAction *helpAct;

    //��������ť
    Button *proSettingButton;  //��������
    Button *proUpdateButton;   //����
    Button *proPartlyUpdateButton;   //���ָ���
    Button *deleteChainButton; //ɾ��
    Button *testbenchGenerationButton;//testbench����
    Button *proUndoButton;   //������undo��
    QPushButton *updatehintButton ; // ��ʾ���°�ť
    QLabel * m_iplogolabel ;  // ��logo

    EziDebugPrj *currentPrj ;


    //���ϽǱ�������ť
    Button *minimizeButton;   //��С��
    Button *miniModeButton;//����ģʽ
    Button *exitButton;  //�˳�

    //������
    QProgressBar *progressBar ;

    //��ListWindow
    Button *showListWindowButton;

    //����һ��QPoint�ĳ�Ա��������¼�����ƶ���λ��
    QPoint dragPosition;
    QPoint oriGlobalPos;//���������µ�λ��

//    bool isNormalListWindowHidden;//��ͨģʽ�£�ToolWindow������ʱ���б����Ƿ�����
    bool isListWindowAdsorbed;//��ͨģʽ��,�б�����Ƿ������ڹ��������ڵ��·�
//    QWidget *listWindowPointer;//ָ��ListWindow��ָ��
    bool isNormalMode;//true--Normal Mode,false--Mini Mode
    bool isNeededUpdate ;

    QTimer *iChangeUpdateButtonTimer ;



};

#endif // TOOLWINDOW_H

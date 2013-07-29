#ifndef MENUBAR_H
#define MENUBAR_H

#include <QMenuBar>

class MenuBar : public QMenuBar
{
public:
    MenuBar(QWidget *parent = 0 );

private:
    void mousePressEvent(QMouseEvent *);        //�Զ���һ��������¼�����
    void mouseMoveEvent(QMouseEvent *);         //�Զ���һ������϶��¼�����
    void mouseReleaseEvent(QMouseEvent *);

};

#endif // MENUBAR_H

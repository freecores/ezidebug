#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>
class TreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit TreeView(QWidget *parent = 0);

private:
    void mouseMoveEvent(QMouseEvent *event);         //�Զ���һ������϶��¼�����

};

#endif // TREEVIEW_H

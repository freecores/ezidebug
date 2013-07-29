#include "button.h"

Button::Button(QWidget *parent) : QPushButton(parent)
{
        //����ͼƬ��Ա��ʼ��
        buttonPicture = new QPixmap();
        pressPicture = new QPixmap();
        releasePicture = new QPixmap();

        enterPicture = new QPixmap();
        leavePicture = new QPixmap();

        //�رհ�ť��Ĭ����ʾ
        this -> setFlat(true);
        this->setFocusPolicy(Qt::NoFocus);

        //��ʼ��flag
        flag=false;


}

Button::Button(QString str, QWidget *parent) : QPushButton(parent)
{
        //����ͼƬ��Ա��ʼ��
        buttonPicture = new QPixmap();
        pressPicture = new QPixmap();
        releasePicture = new QPixmap();

        enterPicture = new QPixmap();
        leavePicture = new QPixmap();

        //�رհ�ť��Ĭ����ʾ
        this -> setFlat(true);
        this->setFocusPolicy(Qt::NoFocus);

        //��ȡͼ��
        QPixmap objPixmap(str);
        //�õ�ͼ���͸�
        int nPixWidth = objPixmap.width() / 4;
        int nPixHeight = objPixmap.height();
        this->setButtonPicture(objPixmap.copy(nPixWidth*0,0,nPixWidth,nPixHeight));
        this->setPressPicture(objPixmap.copy(nPixWidth*2,0,nPixWidth,nPixHeight));
        this->setReleasePicture(objPixmap.copy(nPixWidth*3,0,nPixWidth,nPixHeight));
        this->setEnterPicture(objPixmap.copy(nPixWidth*1,0,nPixWidth,nPixHeight));
        this->setLeavePicture(objPixmap.copy(nPixWidth*3,0,nPixWidth,nPixHeight));

        //��ʼ��flag
        flag=false;


}

void Button::setButtonPicture(QPixmap pic)
{
        *buttonPicture = pic;

        this -> setIcon(QIcon(*buttonPicture));
}

void Button::setPressPicture(QPixmap pic)
{
        *pressPicture = pic;
}

void Button::setReleasePicture(QPixmap pic)
{
        *releasePicture = pic;
}

void Button::setEnterPicture(QPixmap pic)
{
    *enterPicture = pic;
}

void Button::setLeavePicture(QPixmap pic)
{
    *leavePicture = pic;
}

void Button::set_X_Y_width_height(int x, int y, int width, int height)
{
        this -> setIconSize(QSize(width, height));
        this -> setGeometry(x, y, width, height);
}

void Button::mouseDoubleClickEvent(QMouseEvent *event)
{
        //null
}

void Button::mousePressEvent (QMouseEvent *event)
{
        this -> setIcon (QIcon(*pressPicture));
}

void Button::mouseMoveEvent(QMouseEvent *event)
{
        //null
}


void Button::mouseReleaseEvent (QMouseEvent *event)
{
        this -> setIcon(QIcon(*releasePicture));
        emit clicked();
}

void Button::enterEvent(QEvent *)
{
    this->setIcon(QIcon(*enterPicture));
    flag=true;
    setCursor(Qt::PointingHandCursor);
  //  this->resizeit();

}

void Button::leaveEvent(QEvent *)
{
    this->setIcon(QIcon(*leavePicture));
    flag=false;
}

void Button::resizeit(int w , int h)
{
    this->raise();
    this->resize(w,h);
    this ->setIconSize(QSize(w, h));

}

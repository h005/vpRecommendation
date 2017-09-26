#include "imglabel.h"
#include <QDebug>
#include <QPainter>

ImgLabel::ImgLabel(QWidget *parent) :
    QLabel(parent)
{
    img = NULL;
    setStyleSheet("background-color:rgb(75,75,75);");
    flagPosNeg = 0;
}

ImgLabel::~ImgLabel()
{
    clean();
}

void ImgLabel::loadPixmap(QString file)
{
    img = new QPixmap();
    img->load(file);
}

void ImgLabel::mousePressEvent(QMouseEvent *event)
{

}

void ImgLabel::mouseMoveEvent(QMouseEvent *event)
{

}

void ImgLabel::mouseReleaseEvent(QMouseEvent *event)
{

}

void ImgLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    if(flagPosNeg == 1)
    {
        QPainter *painter = new QPainter(this);
        painter->setPen(QPen(Qt::red,5));
        painter->drawRect(0, 0, this->width(), this->height());
        delete painter;
    }
    else if(flagPosNeg == -1)
    {
        QPainter *painter = new QPainter(this);
        painter->setPen(QPen(Qt::green,5));
        painter->drawRect(0, 0, this->width(), this->height());
        delete painter;
    }

}

void ImgLabel::setFlagPosNeg(int flagPosNeg)
{
    this->flagPosNeg = flagPosNeg;
    update();
}

void ImgLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
/* this code was failed to exchange the background
    QPalette pal = this->palette();
    pal.setBrush(QPalette::Background,
                 QBrush(img.scaled(event->size(),
                                   Qt::IgnoreAspectRatio,
                                   Qt::SmoothTransformation)));
    setPalette(pal);
    setAutoFillBackground(true);
*/
    setPixmap(img->scaled(event->size(),
                         Qt::KeepAspectRatio,
                         Qt::SmoothTransformation));
}

QSize ImgLabel::sizeHint()
{
    return QSize(360,240);
//    return QSize(240, 360);
}

void ImgLabel::clean()
{
    delete img;
}

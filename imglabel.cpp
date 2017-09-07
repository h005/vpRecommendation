#include "imglabel.h"
#include <QDebug>

ImgLabel::ImgLabel(QWidget *parent) :
    QLabel(parent)
{
    setStyleSheet("background-color:rgb(176,196,222);");
}

ImgLabel::~ImgLabel()
{

}

void ImgLabel::loadPixmap(QString file)
{
    img.load(file);
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
    setPixmap(img.scaled(event->size(),
                         Qt::KeepAspectRatio,
                         Qt::SmoothTransformation));
}

QSize ImgLabel::sizeHint()
{
    return QSize(480,320);
}

void ImgLabel::setBackground()
{
//    QPixmap backGround(this->width(),this->height());
////    qDebug() << this->width() << " " << this->height() << endl;
//    backGround.fill(Qt::black);
//    this->setPixmap(backGround);
//    update();

}

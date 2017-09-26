#ifndef IMGLABEL_H
#define IMGLABEL_H

#include <QLabel>
#include <QResizeEvent>

class ImgLabel : public QLabel
{
    Q_OBJECT
public:
    ImgLabel(QWidget *parent = 0);
    ~ImgLabel();

    void loadPixmap(QString file);
    // events
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

    void setFlagPosNeg(int flagPosNeg);

    void resizeEvent(QResizeEvent *event);
    QSize sizeHint();

private:
    void clean();

private:

    QPixmap *img;

    int flagPosNeg;

};

#endif // IMGLABEL_H

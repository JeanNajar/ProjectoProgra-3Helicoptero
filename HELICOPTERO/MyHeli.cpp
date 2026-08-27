#include "MyHeli.h"
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QAudioOutput>
#include <QGraphicsItem>



void MyHeli::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left){
        if(pos().x()>0){
        setPos(x()-10,y());
        }
    }else if(event->key() == Qt::Key_Right){
        if (pos().x() + rect().width() < scene()->width())
        setPos(x()+10,y());

    }else if(event->key() == Qt::Key_Up){
        if(pos().y()>0){
        setPos(x(),y()-10);
        }
    }else if(event->key() == Qt::Key_Down){
        if (pos().y() + rect().height() < scene()->height())

        setPos(x(),y()+10);

    }

}

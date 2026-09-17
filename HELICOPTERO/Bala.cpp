#include "Bala.h"
#include "ObstacleH.h"

#include <QGraphicsScene>
#include <QTimer>
#include <QPixmap>
#include <typeinfo>

Bala::Bala(qreal xPos, qreal yPos, QGraphicsScene *escena)
    : QObject(), QGraphicsPixmapItem()
{
    this->escena = escena;

    setPixmap(QPixmap(":/Sprites/recursosh/bala.png"));
    setPos(xPos, yPos);

    // mover la bala cada 16ms (8px por tick = 500px/s)
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(move()));
    timer->start(16);
}

void Bala::move(){

    if(scene() == nullptr){
        return;
    }

    // colisión con obstáculos: la bala le pega y se destruye
    QList<QGraphicsItem*> colliding = collidingItems();
    for(int i = 0; i < colliding.size(); i++){
        if(typeid(*(colliding[i])) == typeid(ObstacleH)){
            ObstacleH *obstaculo = dynamic_cast<ObstacleH*>(colliding[i]);
            if(obstaculo != nullptr){
                obstaculo->recibirImpacto();
            }
            escena->removeItem(this);
            delete this;
            return;
        }
    }

    // avanzar hacia la derecha
    setPos(x() + 8, y());

    // salió de la pantalla: se destruye sola
    if(x() > escena->width()){
        escena->removeItem(this);
        delete this;
    }
}
#include "BidonCombustible.h"
#include "MyHeli.h"
#include "GAME.h"

#include <QGraphicsScene>
#include <QTimer>
#include <QPixmap>
#include <typeinfo>

extern Game *game;

BidonCombustible::BidonCombustible(qreal xPos, qreal yPos, QGraphicsScene *escena)
    : QObject(), QGraphicsPixmapItem()
{
    this->escena = escena;

    setPixmap(QPixmap(":/Sprites/recursosh/bidon_combustible_32x32.png"));
    setPos(xPos, yPos);

    // Mover hacia la izquierda cada 16ms (3px por tick = 187.5px/s, igual que obstáculos)
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(move()));
    timer->start(16);
}

void BidonCombustible::move(){

    if(scene() == nullptr){
        return;
    }

    // Si el juego terminó, no hacer nada
    if(game == nullptr || game->juegoTerminado){
        return;
    }

    // Colisión con el helicóptero: recargar combustible y destruir el bidón
    MyHeli *heli = game->heli;
    if(heli != nullptr && heli->scene() != nullptr){
        QList<QGraphicsItem*> colliding = collidingItems();
        for(int i = 0; i < colliding.size(); i++){
            if(typeid(*(colliding[i])) == typeid(MyHeli)){
                heli->recargarCombustible(35.0);
                escena->removeItem(this);
                delete this;
                return;
            }
        }
    }

    // Avanzar hacia la izquierda
    setPos(x() - 3, y());

    // Salió de la pantalla por la izquierda: destruir
    if(x() + boundingRect().width() < 0){
        escena->removeItem(this);
        delete this;
    }
}

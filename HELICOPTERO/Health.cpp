#include "Health.h"
#include <QFont>

Health::Health(QGraphicsItem *parent): QGraphicsTextItem(parent){
    //empieze en 0
    health = 3;
    //dibujar el texto
    setPlainText(QString("Health: ")+QString::number(health)); //health = 3
    setDefaultTextColor(Qt::red);
    setFont(QFont("times",16));

}

void Health::decrease(){
    health--;
    setPlainText(QString("Health: ")+QString::number(health));//health = 2
}

int Health::getHealth(){
    return health;
}
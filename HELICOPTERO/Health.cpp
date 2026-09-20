#include "Health.h"
#include <QFont>
#include <QBrush>
#include <QPen>

Health::Health(QGraphicsItem *parent): QGraphicsTextItem(parent){
    //empieze en 3
    health = 3;
    // Usar fuente mucho más grande para mayor visibilidad
    setFont(QFont("Arial", 28, QFont::Bold));
    setDefaultTextColor(Qt::red);
    updateDisplay();

    // Fondo de la barra de vida (más visible)
    fondoBarra = new QGraphicsRectItem(0, 0, 200, 30, this);
    fondoBarra->setBrush(QBrush(QColor(60, 0, 0)));
    fondoBarra->setPen(QPen(QColor(255, 50, 50), 2));
    fondoBarra->setPos(-50, -35);
    fondoBarra->setZValue(100);

    // Barra de vida rellena
    barraVida = new QGraphicsRectItem(0, 0, 200, 30, this);
    barraVida->setBrush(QBrush(QColor(0, 200, 50)));
    barraVida->setPen(QPen(QColor(0, 150, 30), 1));
    barraVida->setPos(-50, -35);
    barraVida->setZValue(101);
}

void Health::decrease(){
    health--;
    updateDisplay();
}

int Health::getHealth(){
    return health;
}

void Health::updateDisplay(){
    // Mostrar corazones rojos según la cantidad de vidas
    QString corazones;
    for(int i = 0; i < 3; i++){
        if(i < health){
            corazones += QString::fromUtf8("❤️ ");
        } else {
            corazones += QString::fromUtf8("🖤 ");
        }
    }
    setPlainText(corazones);

    // Actualizar la barra de vida visual
    int anchoBarra = (health * 200) / 3;
    if(anchoBarra < 0) anchoBarra = 0;
    barraVida->setRect(0, 0, anchoBarra, 30);
}
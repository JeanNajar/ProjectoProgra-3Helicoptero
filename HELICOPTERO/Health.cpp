#include "Health.h"
#include <QFont>

Health::Health(QGraphicsItem *parent): QGraphicsTextItem(parent){
    //empieze en 3
    health = 3;
    // Usar fuente mucho más grande para mayor visibilidad
    setFont(QFont("Arial", 28, QFont::Bold));
    setDefaultTextColor(Qt::red);
    updateDisplay();
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
    // (los corazones ya indican la vida: no hace falta barra adicional)
    QString corazones;
    for(int i = 0; i < 3; i++){
        if(i < health){
            corazones += QString::fromUtf8("❤️ ");
        } else {
            corazones += QString::fromUtf8("🖤 ");
        }
    }
    setPlainText(corazones);
}
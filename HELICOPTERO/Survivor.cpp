#include "Survivor.h"
#include <QTimer>
#include <QBrush>
#include <QPen>

Survivor::Survivor(qreal xPos, qreal yPos, QGraphicsScene *scene)
    : QObject(), QGraphicsRectItem()
{
    this->escena = scene;
    progreso = 0;
    rescatado = false;
    heliEncima = false;

    //CUADRADITO VERDE temporal

    setRect(0, 0, 40, 40);
    setBrush(QBrush(Qt::green));
    setPen(QPen(Qt::darkGreen));
    setPos(xPos, yPos);

    //  FONDO DE LA BARRA DE PROGRESO
    fondoBarra = new QGraphicsRectItem(0, 0, 40, 6, this);
    fondoBarra->setBrush(QBrush(QColor(60, 60, 60)));
    fondoBarra->setPen(QPen(Qt::black));
    fondoBarra->setPos(0, -10);  // Encima del superviviente

    //  BARRA DE PROGRESO

    barra = new QGraphicsRectItem(0, 0, 0, 6, this);
    barra->setBrush(QBrush(Qt::yellow));
    barra->setPen(QPen(Qt::NoPen));
    barra->setPos(0, -10);  // Encima del superviviente, sobre el fondo

    // TIMER DE PROGRESO
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateProgress()));
    timer->start(50);

    // TIMER DE MOVIMIENTO (funciona para que se mueva junto con la pantalla)
    moveTimer = new QTimer(this);
    connect(moveTimer, SIGNAL(timeout()), this, SLOT(move()));
    moveTimer->start(50);
}

Survivor::~Survivor(){

    if(timer != nullptr){
        timer->stop();
    }
    if(moveTimer != nullptr){
        moveTimer->stop();
    }
    // La barra y el fondo son hijos (this), Qt los borra automáticamente.
    // No hace falta delete manual aquí
}

bool Survivor::isRescued() const{
    return rescatado;
}

bool Survivor::isHeliOver(QRectF heliRect) const{
    // Verificar si el rectángulo del heli se superpone con el del superviviente
    return heliRect.intersects(rect().translated(pos()));
}

void Survivor::setHeliEncima(bool encima){
    heliEncima = encima;
}

void Survivor::updateProgress(){
    // Si ya fue rescatado, no hacer nada
    if(rescatado){
        return;
    }

    // Si el heli NO está encima, reiniciar el progreso (el rescate se cancela)
    if(!heliEncima){
        progreso = 0;
        barra->setRect(0, 0, 0, 6);  // Barra vacía
        return;
    }

    // El heli está encima → avanzar el progreso.
    // 2 segundos = 40 ticks de 50ms. Para llegar a 100 en 40 ticks,
    // avanzamos 2.5 por tick. Como usamos enteros, acumulamos con float.
    progreso += 2.5;
    if(progreso > 100){
        progreso = 100;
    }

    // Actualizar la barra según el progreso (0 a 100 → 0 a 40 px de ancho)
    int anchoBarra = static_cast<int>((progreso / 100.0) * 40.0);
    barra->setRect(0, 0, anchoBarra, 6);

    // Si llegó al 100%, rescatar
    if(progreso >= 100){
        rescatado = true;
        timer->stop();
        moveTimer->stop();  // Detener también el movimiento (ya no se mueve)
        escena->removeItem(this);
        // No hacemos delete this: SurvivorManager decide cuándo borrar.
    }
}

void Survivor::move(){

    if(scene() == nullptr){
        return;
    }

    // Mover hacia la izquierda a la MISMA velocidad que los obstáculos (3px/tick)
    setPos(x() - 3, y());

    // Si salió por la izquierda, quitarlo de la escena
    if(pos().x() + rect().width() < 0){
        escena->removeItem(this);
        // No hacemos delete this: SurvivorManager decide cuándo borrar.
        // Marcamos rescatado para que el manager lo limpie (aunque no fue
        // rescatado, simplemente salió del mapa).
        rescatado = true;
        timer->stop();
        moveTimer->stop();
    }
}

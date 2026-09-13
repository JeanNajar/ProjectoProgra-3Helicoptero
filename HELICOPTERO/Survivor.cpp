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

    //cuadrito verde (Es temporal)

    setRect(0, 0, 40, 40);
    setBrush(QBrush(Qt::green));
    setPen(QPen(Qt::darkGreen));
    setPos(xPos, yPos);

    //  fondo de la barra de progresion
    fondoBarra = new QGraphicsRectItem(0, 0, 40, 6, this);
    fondoBarra->setBrush(QBrush(QColor(60, 60, 60)));
    fondoBarra->setPen(QPen(Qt::black));
    fondoBarra->setPos(0, -10);  // Encima del superviviente

    // barra de progresion

    barra = new QGraphicsRectItem(0, 0, 0, 6, this);
    barra->setBrush(QBrush(Qt::yellow));
    barra->setPen(QPen(Qt::NoPen));
    barra->setPos(0, -10);  // Encima del superviviente, sobre el fondo verdesito

    // timer de progresion
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateProgress()));
    timer->start(50);

    // timer de movimiento(el movimiento con patalla)
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
    // La barra y el fondo (para que se dentengan y desaparezcan)
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

    // El heli está encima -> avanzar el progreso.
    progreso += 2.5;
    if(progreso > 100){
        progreso = 100;
    }

    // Actualizar la barra según el progreso
    int anchoBarra = static_cast<int>((progreso / 100.0) * 40.0);
    barra->setRect(0, 0, anchoBarra, 6);

    // Si llego a rescatar
    if(progreso >= 100){
        rescatado = true;
        timer->stop();
        moveTimer->stop();  // Detener también el movimiento
        escena->removeItem(this);
        // No hacemos delete this porque eso se controla en survivor manager
    }
}

void Survivor::move(){

    if(scene() == nullptr){
        return;
    }

    // Mover hacia la izquierda
    setPos(x() - 3, y());

    // Si salio por la izquierda, quitarlo de la escena
    if(pos().x() + rect().width() < 0){
        escena->removeItem(this);

        rescatado = true;
        timer->stop();
        moveTimer->stop();
    }
}

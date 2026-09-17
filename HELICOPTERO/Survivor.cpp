#include "Survivor.h"
#include "GAME.h"
#include "ResourceLoader.h"
#include <QTimer>
#include <QBrush>
#include <QPen>

extern Game * game;

Survivor::Survivor(qreal xPos, qreal yPos, QGraphicsScene *scene, int nivel)
    : QObject(), QGraphicsPixmapItem()
{
    this->escena = scene;
    progreso = 0;
    rescatado = false;
    rescatadoPorProgreso = false;
    heliEncima = false;

    // cargar los 2 fotogramas según el nivel (brazo derecho / izquierdo
    // levantado). Se usa la caché del hilo de precarga; si el hilo aún no
    // terminó, se carga directo como respaldo.
    QVector<QPixmap> precargadas = ResourceLoader::survivorFrames(nivel);
    if(precargadas.size() == 2 && !precargadas[0].isNull()){
        frames[0] = precargadas[0];
        frames[1] = precargadas[1];
    }else{
        QString base;
        switch(nivel){
        case 2:
            base = "superviviente_desierto_frame";
            break;
        case 3:
            base = "superviviente_nieve_frame";
            break;
        default:
            base = "superviviente_frame";
        }
        frames[0] = QPixmap(":/Sprites/recursosh/" + base + "1.png");
        frames[1] = QPixmap(":/Sprites/recursosh/" + base + "2.png");
    }
    currentFrame = 0;
    setPixmap(frames[currentFrame]);
    setPos(xPos, yPos);

    //  fondo de la barra de progresion
    fondoBarra = new QGraphicsRectItem(0, 0, boundingRect().width(), 6, this);
    fondoBarra->setBrush(QBrush(QColor(60, 60, 60)));
    fondoBarra->setPen(QPen(Qt::black));
    fondoBarra->setPos(0, -10);  // Encima del superviviente

    // barra de progresion
    barra = new QGraphicsRectItem(0, 0, 0, 6, this);
    barra->setBrush(QBrush(Qt::yellow));
    barra->setPen(QPen(Qt::NoPen));
    barra->setPos(0, -10);  // Encima del superviviente, sobre el fondo

    // timer de progresion
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateProgress()));
    timer->start(50);

    // timer de movimiento (el movimiento con pantalla)
    moveTimer = new QTimer(this);
    connect(moveTimer, SIGNAL(timeout()), this, SLOT(move()));
    moveTimer->start(50);

    // timer de animacion (alterna los brazos, como si pidiera auxilio)
    animTimer = new QTimer(this);
    connect(animTimer, SIGNAL(timeout()), this, SLOT(updateAnimation()));
    animTimer->start(400);
}

Survivor::~Survivor(){

    if(timer != nullptr){
        timer->stop();
    }
    if(moveTimer != nullptr){
        moveTimer->stop();
    }
    if(animTimer != nullptr){
        animTimer->stop();
    }
    // La barra y el fondo (para que se dentengan y desaparezcan)
    // No hace falta delete manual aquí
}

bool Survivor::isRescued() const{
    return rescatado;
}

bool Survivor::fueRescatado() const{
    return rescatadoPorProgreso;
}

bool Survivor::isHeliOver(QRectF heliRect) const{
    // Verificar si el rectángulo del heli se superpone con el del superviviente
    return heliRect.intersects(boundingRect().translated(pos()));
}

double Survivor::getProgreso() const{
    return progreso;
}

void Survivor::setHeliEncima(bool encima){
    heliEncima = encima;
}

void Survivor::updateProgress(){
    // Si el juego terminó (ganó o perdió), el mundo se pausa
    if(game == nullptr || game->juegoTerminado){
        return;
    }

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
    int anchoBarra = static_cast<int>((progreso / 100.0) * boundingRect().width());
    barra->setRect(0, 0, anchoBarra, 6);

    // Si llego a rescatar
    if(progreso >= 100){
        rescatado = true;
        rescatadoPorProgreso = true;  // rescate REAL (completó la barra)
        timer->stop();
        moveTimer->stop();  // Detener también el movimiento
        animTimer->stop();
        escena->removeItem(this);
        // No hacemos delete this porque eso se controla en survivor manager
    }
}

void Survivor::move(){

    if(scene() == nullptr){
        return;
    }

    // Si el juego terminó (ganó o perdió), el mundo se pausa
    if(game == nullptr || game->juegoTerminado){
        return;
    }

    // Mover hacia la izquierda
    setPos(x() - 3, y());

    // Si salio por la izquierda, quitarlo de la escena
    if(pos().x() + boundingRect().width() < 0){
        escena->removeItem(this);

        rescatado = true;
        timer->stop();
        moveTimer->stop();
        animTimer->stop();
    }
}

void Survivor::updateAnimation(){
    if(rescatado){
        return;
    }
    currentFrame = (currentFrame + 1) % 2;
    setPixmap(frames[currentFrame]);
}

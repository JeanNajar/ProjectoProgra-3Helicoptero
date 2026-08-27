#include <QApplication>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QList>
#include <QDebug>
#include <QMediaPlayer>
#include <QAudioOutput>

#include "ObstacleH.h"
#include "MyHeli.h"
#include "GAME.h"

extern Game * game;

ObstacleH::ObstacleH(ObstacleType type): QObject(), QGraphicsRectItem(){

    // Guardar el tipo
    tipo = type;

    switch(tipo) {
    case ObstacleType::VERTICAL:
      //dibujar el obstaculo
        setRect(0,0,100,300);
        break;

    case ObstacleType::SMALL:

        setRect(0, 0, 60, 80);
        break;

    case ObstacleType::CEILING:
        setRect(0, 0, 200, 150);
        break;
    }

    //conectarlo
    QTimer * timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(move()));
    timer->start(50);

    CrashSound = new QMediaPlayer;
    CrashSound->setSource(QUrl("qrc:/Sounds/recursosh/CrashSound.mp3"));

    audioOutput = new QAudioOutput();

    CrashSound->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.3);
}


ObstacleH::~ObstacleH(){
    delete CrashSound;
    delete audioOutput;
}

ObstacleType ObstacleH::getType() const{
    return tipo;
}
void ObstacleH::move(){

    //colision
    QList<QGraphicsItem* > colliding_items = collidingItems();

    for (int i=0, n=colliding_items.size();i<n;i++){

        if (typeid(*(colliding_items[i])) == typeid(MyHeli)) {
            //quita vida
            game->health->decrease();

              //CrashSound->play();

            //suena el sonido de crash
            if(CrashSound->playbackState() == QMediaPlayer::PlayingState){
                CrashSound->setPosition(0);
            }else if(CrashSound->playbackState()== QMediaPlayer::StoppedState){
                CrashSound->play();
            }
            //si llega a 0 se destruye
            if(game->health->getHealth()<=0){
                scene()->removeItem(colliding_items[i]);
                delete colliding_items[i];
            }
            //si el heli pega con un objeto lo destruye
            scene()->removeItem(this);

            QTimer::singleShot(1000, this, [this]() {
                delete this;
            });
           //delete this;

            return;

        }
    }

    //mover el obstaculo
    setPos(x()-5,y());
    if(pos().x()+rect().width()< 0){
        scene()->removeItem(this);
        delete this;
    }
}
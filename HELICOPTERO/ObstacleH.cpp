#include <QApplication>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QList>
#include <QDebug>
#include <QAudioOutput>

#include "ObstacleH.h"
#include "MyHeli.h"
#include "GAME.h"

extern Game * game;

ObstacleH::ObstacleH(): QObject(), QGraphicsRectItem(){

    //dibujar el obstaculo
    setRect(0,0,100,300);

    //conectarlo
    QTimer * timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(move()));

    timer->start(50);

    CrashSound = new QMediaPlayer;
    CrashSound->setSource(QUrl("qrc:/Sounds/recursosh/CrashSound.mp3"));

    QAudioOutput *audio = new QAudioOutput();

    CrashSound->setAudioOutput(audio);
    audio->setVolume(0.3);
}


void ObstacleH::move(){

    //colision
    QList<QGraphicsItem* > colliding_items = collidingItems();

    for (int i=0, n=colliding_items.size();i<n;i++){

        if (typeid(*(colliding_items[i])) == typeid(MyHeli)) {
            //if(dynamic_cast<MyHeli*>(colliding_items[i])){
            //quita vida
            game->health->decrease();

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
            delete this;

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
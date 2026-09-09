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
#include "ObstacleManager.h"

extern Game * game;

ObstacleH::ObstacleH(ObstacleType type, ObstacleManager *manager): QObject(), QGraphicsPixmapItem(){

    // Guardar el tipo
    tipo = type;
    this->manager = manager;

    switch(tipo) {
    case ObstacleType::VERTICAL:
        //torre industrial (obstaculo alto)
        //se escala para que sea mas gruesa y alta y quede anclada al suelo
        //(el manager la coloca en y = altura - 300, asi la base toca el fondo)
        setPixmap(QPixmap(":/Sprites/recursosh/torre_industrial_obstaculo_40x140.png")
                      .scaled(80, 300, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        break;

    case ObstacleType::SMALL:
        //cajon / barrera industrial (obstaculo bajo)
        setPixmap(QPixmap(":/Sprites/recursosh/cajon_barrera_obstaculo_32x32.png"));
        break;

    case ObstacleType::CEILING:
        //tuberias colgando del techo
        setPixmap(QPixmap(":/Sprites/recursosh/tuberias_techo_obstaculo_160x64.png"));
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

    if(scene() == nullptr){
        return;
    }

    //colision
    QList<QGraphicsItem* > colliding_items = collidingItems();

    for (int i=0, n=colliding_items.size();i<n;i++){

        if (typeid(*(colliding_items[i])) == typeid(MyHeli)) {
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
                MyHeli *heli = dynamic_cast<MyHeli*>(colliding_items[i]);
                if(heli != nullptr){
                    heli->crash();
                }
            }
            //si el heli pega con un objeto lo destruye
            scene()->removeItem(this);
            manager->notifyObstacleDied(this);
            return;
        }
    }

    //mover el obstaculo
    setPos(x()-5,y());
    if(pos().x()+boundingRect().width()< 0){
        scene()->removeItem(this);
        manager->notifyObstacleDied(this);
    }
}
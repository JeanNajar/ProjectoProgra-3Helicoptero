#ifndef MYHELI_H
#define MYHELI_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QMediaPlayer>
#include <QPixmap>
#include "Physics.h"

class QTimer;

class MyHeli: public QObject, public QGraphicsPixmapItem{

    Q_OBJECT

public:

    MyHeli();
    ~MyHeli();

    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);

public slots:

    void updatePhysics();
    void crash();
    void updateRotorAnimation();

private:

    //fisicas
    Physics *physics;
    double velX;
    double velY;
    bool thrusting; //subiendo
    bool movingLeft;
    bool movingRight;

    //sonidos
    QMediaPlayer *crashSound;
    QAudioOutput *crashAudio;

    //animacion del rotor (4 fotogramas)
    QPixmap rotorFrames[4];
    int currentFrame;
    QTimer *rotorTimer;

    //funciones
    void checkLanding();
};
#endif // MYHELI_H

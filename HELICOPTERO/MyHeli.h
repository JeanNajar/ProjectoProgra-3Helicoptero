#ifndef MYHELI_H
#define MYHELI_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QMediaPlayer>
#include "Physics.h"

class MyHeli: public QObject, public QGraphicsRectItem{

  Q_OBJECT

public:

    MyHeli();
    ~MyHeli();

    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);

public slots:

    void updatePhysics();

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

    //funciones
    void checkLanding();
    void crash();

};



#endif // MYHELI_H

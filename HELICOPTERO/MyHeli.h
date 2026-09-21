#ifndef MYHELI_H
#define MYHELI_H

#include <QGraphicsPixmapItem>
#include <QObject>
#include <QMediaPlayer>
#include <QPixmap>
#include <QElapsedTimer>
#include <QGraphicsRectItem>
#include "Physics.h"

class QTimer;

class MyHeli: public QObject, public QGraphicsPixmapItem{

    Q_OBJECT

public:

    MyHeli(int nivel = 1);
    ~MyHeli();

    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);

    // Dispara una bala hacia la derecha (solo desde el nivel 2, con cooldown)
    void disparar();

    // Recarga una cantidad fija de combustible (pickup de bidón)
    void recargarCombustible(double cantidad);

    // Detiene todos los timers del heli (física, rotor, viento, explosión).
    // Se usa al salir del juego: el heli no debe seguir "vivo" en el menú.
    void detenerTimers();

    // Combustible actual (0–100)
    double getFuel() const;

public slots:

    void updatePhysics();
    void crash();
    void updateRotorAnimation();
    void updateExplosion();

private:

    //fisicas
    Physics *physics;
    double velX;
    double velY;
    bool thrusting; //subiendo
    bool movingLeft;
    bool movingRight;
    bool crashed; //true apenas explota, corta la fisica y evita crash() repetido

    //combustible (0–100)
    double fuel;

    //viento (empuja al helicóptero)
    double windX;
    double windY;
    // Aviso rojo "Corriente de viento pasando" (nivel 3, durante la ventisca)
    QGraphicsTextItem *textoViento;

    //sonidos (compartidos: un solo reproductor de crash para toda la app,
    // evita que Qt extraiga el .wav del qrc a un temporal en cada nivel)
    static QMediaPlayer *crashSound;
    static QAudioOutput *crashAudio;

    //animacion del rotor (4 fotogramas)
    QPixmap rotorFrames[4];
    int currentFrame;
    QTimer *rotorTimer;

    //animacion de la explosion
    QPixmap explosionFrames[5];
    int explosionFrame;
    QTimer *explosionTimer;

    //disparo de balas (solo nivel 2 y 3)
    int nivel;                  // nivel actual del juego
    bool puedeDisparar;         // true si nivel >= 2
    QElapsedTimer ultimoDisparo; // controla el cooldown entre balas

    //funciones
    void checkLanding();
    // Muestra/oculta el aviso rojo de viento (nivel 3, durante la ventisca)
    void mostrarAvisoViento(bool visible);
};
#endif // MYHELI_H

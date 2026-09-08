#include "MyHeli.h"
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QAudioOutput>
#include <QGraphicsItem>
#include <QTimer>
#include <QGraphicsTextItem>
#include <QFont>

MyHeli::MyHeli() : QObject(), QGraphicsPixmapItem(){

    //cargar los 4 fotogramas del rotor girando
    rotorFrames[0] = QPixmap(":/Sprites/recursosh/helicoptero_frame1.png");
    rotorFrames[1] = QPixmap(":/Sprites/recursosh/helicoptero_frame2.png");
    rotorFrames[2] = QPixmap(":/Sprites/recursosh/helicoptero_frame3.png");
    rotorFrames[3] = QPixmap(":/Sprites/recursosh/helicoptero_frame4.png");
    currentFrame = 0;
    setPixmap(rotorFrames[currentFrame]);

    //esto sirve para que el punto de en medio sea el origen del helicoptero
    setTransformOriginPoint(boundingRect().center());

    //inicializacion de fisicas
    physics = new Physics();
    velX = 0.0;
    velY = 0.0;
    thrusting = false;
    movingLeft = false;
    movingRight = false;

    //timer para las fisicas
    QTimer *physicsTimer = new QTimer(this);
    connect(physicsTimer, SIGNAL(timeout()), this, SLOT(updatePhysics()));
    physicsTimer->start(16);

    //timer aparte para la animacion del rotor
    rotorTimer = new QTimer(this);
    connect(rotorTimer, SIGNAL(timeout()), this, SLOT(updateRotorAnimation()));
    rotorTimer->start(90);

    //sonido de choque

    crashSound = new QMediaPlayer;
    crashSound->setSource(QUrl("qrc:/Sounds/recursosh/CrashSound.mp3"));
    crashAudio = new QAudioOutput();
    crashSound->setAudioOutput(crashAudio);
    crashAudio->setVolume(0.5);

}

//destructor aqui liberamos la memoria
MyHeli::~MyHeli(){

    delete physics;
    delete crashSound;
    delete crashAudio;

}

void MyHeli::keyPressEvent(QKeyEvent *event){
    //el if para poder usar las dos el espacio y la flecha para arriba
    if(event->key() == Qt::Key_Up|| event->key() == Qt::Key_Space){
        //subir
        thrusting = true;
    }else if(event->key()==Qt::Key_Left){
        //moverse a la izquierda
        movingLeft=true;
    }else if(event->key() == Qt::Key_Right){
        //moverse a la derecha
        movingRight=true;
    }

    //esto sirve para no se propague a otros elementos
    event->accept();
}

void MyHeli::keyReleaseEvent(QKeyEvent *event){
    //esta parte sirve para detectar cuando se suelta la tecla

    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Space){
        thrusting = false;
    }else if(event->key() == Qt::Key_Left){
        movingLeft = false;
    }else if(event->key() == Qt::Key_Right){
        movingRight = false;
    }

    event->accept();
}


//actualizar las fisicas con el qtimer
void MyHeli::updatePhysics(){

    //asignamos a null si el heli se estrella
    if(scene() == nullptr){
        return;
    }

    double dt = 0.016;

    if(thrusting){
        //aplicamos el empuje
        physics->applyThrust(velY,dt);
    }else{
        //aplicamos la gravedad
        physics->applyGravity(velY,dt);
    }

    if(movingLeft){
        //acelerar hacia la izquierda
        velX-=600.0 * dt;
        if(velX< -physics->getMaxHorizontalSpeed()){
            velX = -physics->getMaxHorizontalSpeed();
        }

    }else if(movingRight){
        //acelerar hacia la derecha
        velX+= 600.0 * dt;
        if(velX > physics->getMaxHorizontalSpeed()){
            velX = physics->getMaxHorizontalSpeed();
        }
    }else{
        //inercia si no se toca ninguna tecla
        physics->applyFriction(velX, dt);
    }

    //movimientos (la formula de posicion)

    double newX = x() + velX * dt;
    double newY = y() + velY * dt;

    //limite izquierdo
    if(newX<0){
        newX = 0;
        velX = 0;
    }

    //limite derecho
    if(newX + boundingRect().width() > scene()->width()){
        newX = scene()->width() - boundingRect().width();
        velX = 0;
    }

    if(newY < 0){
        newY = 0;
        velY = 0;
    }

    if(newY + boundingRect().height() >= scene()->height()){
        newY = scene()->height() - boundingRect().height();
        checkLanding();
        return;
    }

    setPos(newX,newY);

    //controla el angulo segun la velocidad
    double targetAngle = physics->calculateTilt(velY, velX);

    double currentAngle = rotation();
    double newAngle = currentAngle + (targetAngle - currentAngle) * 0.15;
    setRotation(newAngle);
}

void MyHeli::checkLanding(){
    //tolerancia de aterrizaje

    if(physics->SafeLanding(velY)){
        //si atterrizamos
        velY = 0.0;
        setRotation(0.0);
    }else{
        // destruir el helicóptero
        crash();
    }
}

//avanza al siguiente fotograma para simular el giro de la helice
void MyHeli::updateRotorAnimation(){

    currentFrame = (currentFrame + 1) % 4;
    setPixmap(rotorFrames[currentFrame]);
}

void MyHeli::crash(){

    // Sonido de choque
    if(crashSound->playbackState() == QMediaPlayer::StoppedState){
        crashSound->play();
    }

    // Quitar el heli de la escena
    scene()->removeItem(this);

    //para que suene el sonido antes de borrarlo
    QTimer::singleShot(1000, this, [this]() {
        delete this;
    });
}
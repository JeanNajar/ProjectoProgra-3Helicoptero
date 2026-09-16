#include "MyHeli.h"
#include "GAME.h"
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QAudioOutput>
#include <QGraphicsItem>
#include <QTimer>
#include <QGraphicsTextItem>
#include <QFont>

extern Game * game;

MyHeli::MyHeli() : QObject(), QGraphicsPixmapItem()
{

    //cargar los 4 fotogramas del rotor girando
    rotorFrames[0] = QPixmap(":/Sprites/recursosh/helicoptero_frame1.png");
    rotorFrames[1] = QPixmap(":/Sprites/recursosh/helicoptero_frame2.png");
    rotorFrames[2] = QPixmap(":/Sprites/recursosh/helicoptero_frame3.png");
    rotorFrames[3] = QPixmap(":/Sprites/recursosh/helicoptero_frame4.png");
    currentFrame = 0;
    setPixmap(rotorFrames[currentFrame]);

    //cargas los fotogramas de la explosion
    explosionFrames[0] = QPixmap(":/Sprites/recursosh/explosion_frame1.png");
    explosionFrames[1] = QPixmap(":/Sprites/recursosh/explosion_frame2.png");
    explosionFrames[2] = QPixmap(":/Sprites/recursosh/explosion_frame3.png");
    explosionFrames[3] = QPixmap(":/Sprites/recursosh/explosion_frame4.png");
    explosionFrames[4] = QPixmap(":/Sprites/recursosh/explosion_frame5.png");
    explosionFrame = 0;

    //esto sirve para que el punto de en medio sea el origen del helicoptero
    setTransformOriginPoint(boundingRect().center());

    //inicializacion de fisicas
    physics = new Physics();
    velX = 0.0;
    velY = 0.0;
    thrusting = false;
    movingLeft = false;
    movingRight = false;
    crashed = false;

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
MyHeli::~MyHeli()
{

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

    // Si el juego terminó (ganó o perdió), el heli se congela
    if(game->juegoTerminado){
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

        // ===== SCROLL DEL MUNDO (heli aterrizado) =====
        // Cuando el heli está en el suelo, el mundo se mueve hacia la
        // izquierda (los obstáculos y supervivientes avanzan). Para que el
        // heli se mueva EXACTAMENTE a la misma velocidad que el mundo,
        // usamos la posición actual x() (sin la inercia del jugador velX)
        // y le restamos la velocidad de scroll (60px/s = 3px por tick).
        double scrollVel = 60.0;
        if(game->mundoEnMovimiento){
            newX = x() - scrollVel * dt;
            if(newX < 0){
                newX = 0;
            }
        }else{
            // El mundo se detuvo (la meta apareció): el heli aterrizado
            // queda QUIETO para poder aterrizar en la zona sin deslizarse.
            newX = x();
        }

        setPos(newX, newY);
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

void MyHeli::crash()
{
    if(crashed)
    {
        return; // ya esta explotando, no reinicar la animacion
    }
    crashed = true;

    // Avisar al juego: el heli se destruyó → mostrar pantalla de derrota
    // (con porcentaje de rescate y barra). Esto también pausa el mundo.
    if(game != nullptr && !game->juegoTerminado){
        game->mostrarDerrota();
    }

    // Sonido de choque
    if(crashSound->playbackState() == QMediaPlayer::StoppedState){
        crashSound->play();
    }

    // Detener animacion del rotor
    rotorTimer->stop();

    // Centrar la explosion donde estaba el helicoptero
    QPointF centro = pos() + boundingRect().center();
    setPixmap(explosionFrames[0]);
    setOffset(0,0);
    setPos(centro.x() - boundingRect().width() / 2.0, centro.y() - boundingRect().height() / 2.0);

    // Animar la explosion (5 fotogramas cada 100ms = 500ms)
    explosionTimer = new QTimer(this);
    connect(explosionTimer, SIGNAL(timeout()), this, SLOT(updateExplosion()));
    explosionTimer->start(100);

    //para que suene el somnido y se vea la explosion antes de borrarlo
    QTimer::singleShot(1000, this, [this]() {
        scene()->removeItem(this);
        delete this;
    });
}

void MyHeli::updateExplosion()
{
    explosionFrame++;
    if(explosionFrame >= 5)
    {
        explosionTimer->stop();
        return;
    }
    QPointF centroViejo = pos() + boundingRect().center();
    setPixmap(explosionFrames[explosionFrame]);
    setPos(centroViejo.x() - boundingRect().width() / 2.0, centroViejo.y() - boundingRect().height() / 2.0);
}
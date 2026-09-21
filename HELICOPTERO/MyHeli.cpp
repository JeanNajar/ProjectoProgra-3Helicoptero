#include "MyHeli.h"
#include "GAME.h"
#include "Bala.h"
#include <QKeyEvent>
#include <QGraphicsScene>
#include <QAudioOutput>
#include <QGraphicsItem>
#include <QTimer>
#include <QGraphicsTextItem>
#include <QFont>
#include <QBrush>
#include <QPen>

extern Game * game;

// Definición de los estáticos compartidos: un solo reproductor de crash
// para toda la app (se crea la primera vez que se usa).
QMediaPlayer * MyHeli::crashSound = nullptr;
QAudioOutput * MyHeli::crashAudio = nullptr;

MyHeli::MyHeli(int nivel) : QObject(), QGraphicsPixmapItem()
{
    this->nivel = nivel;
    // Las balas solo están disponibles desde el nivel 2 (desierto)
    puedeDisparar = (nivel >= 2);
    ultimoDisparo.start();

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

    // ===== COMBUSTIBLE =====
    // Arranca lleno (100); la barra va en el HUD (gestionada por Game)
    fuel = 100.0;

    // ===== VIENTO =====
    windX = 0.0;
    windY = 0.0;
    textoViento = nullptr;

    if(nivel == 2){
        // Nivel 2 (desierto): viento constante que empuja hacia los obstáculos
        // (los obstáculos vienen de la derecha → windX positivo empuja a la derecha)
        windX = 40.0;
        windY = 0.0;
    }else if(nivel == 3){
        // Nivel 3 (nieve): 2 ventiscas de 4s (a los 20s y 40s) con empuje
        // fuerte (400 px/s²) y aviso rojo "Corriente de viento pasando".
        QTimer::singleShot(20000, this, [this]() {
            windX = 400.0;
            windY = 60.0;
            mostrarAvisoViento(true);
            QTimer::singleShot(4000, this, [this]() {
                windX = 0.0;
                windY = 0.0;
                mostrarAvisoViento(false);
            });
        });
        QTimer::singleShot(40000, this, [this]() {
            windX = 400.0;
            windY = 60.0;
            mostrarAvisoViento(true);
            QTimer::singleShot(4000, this, [this]() {
                windX = 0.0;
                windY = 0.0;
                mostrarAvisoViento(false);
            });
        });
    }
    // Nivel 1: sin viento (windX = windY = 0)

    //timer para las fisicas
    QTimer *physicsTimer = new QTimer(this);
    connect(physicsTimer, SIGNAL(timeout()), this, SLOT(updatePhysics()));
    physicsTimer->start(16);

    //timer aparte para la animacion del rotor
    rotorTimer = new QTimer(this);
    connect(rotorTimer, SIGNAL(timeout()), this, SLOT(updateRotorAnimation()));
    rotorTimer->start(90);

    // Sonido de choque compartido (se crea una sola vez; evita que Qt
    // extraiga el .wav del qrc a un temporal en cada nivel)
    if(MyHeli::crashSound == nullptr){
        MyHeli::crashSound = new QMediaPlayer;
        MyHeli::crashSound->setSource(QUrl("qrc:/Sounds/recursosh/CrashSound.wav"));
        MyHeli::crashAudio = new QAudioOutput();
        MyHeli::crashSound->setAudioOutput(MyHeli::crashAudio);
        MyHeli::crashAudio->setVolume(0.5);
    }

}

//destructor aqui liberamos la memoria
MyHeli::~MyHeli()
{
    // Quitar el aviso de viento de la escena (si sigue visible) y borrarlo
    delete physics;

    if(textoViento != nullptr){
        if(textoViento->scene() != nullptr){
            textoViento->scene()->removeItem(textoViento);
        }
        delete textoViento;
        textoViento = nullptr;
    }
}

void MyHeli::detenerTimers(){
    // Detiene TODOS los timers del heli (fisica, rotor, viento, explosion)
    const QList<QTimer*> timers = findChildren<QTimer*>();
    for(QTimer *t : timers){
        t->stop();
    }
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
    }else if(event->key() == Qt::Key_X){
        //disparar una bala (solo nivel 2 y 3)
        disparar();
    }

    //esto sirve para no se propague a otros elementos
    event->accept();
}

void MyHeli::recargarCombustible(double cantidad){
    fuel += cantidad;
    if(fuel > 100.0){
        fuel = 100.0;
    }
}

double MyHeli::getFuel() const{
    return fuel;
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

void MyHeli::disparar(){

    // solo desde el nivel 2, y no cuando el juego terminó
    if(!puedeDisparar){
        return;
    }
    if(game == nullptr || game->juegoTerminado){
        return;
    }
    if(scene() == nullptr){
        return;
    }

    // cooldown de 400ms entre disparos
    if(ultimoDisparo.elapsed() < 400){
        return;
    }
    ultimoDisparo.restart();

    // la bala sale del lado derecho del heli, centrada verticalmente
    qreal bx = x() + boundingRect().width();
    qreal by = y() + boundingRect().height() / 2.0 - 4.0; // -4 para centrar la bala (8px de alto)

    Bala *bala = new Bala(bx, by, scene());
    scene()->addItem(bala);
}

//actualizar las fisicas con el qtimer
void MyHeli::updatePhysics(){

    //asignamos a null si el heli se estrella
    if(scene() == nullptr){
        return;
    }

    // Si el juego terminó (ganó o perdió), el heli se congela
    if(game == nullptr || game->juegoTerminado){
        return;
    }

    double dt = 0.016;

    // ===== COMBUSTIBLE =====
    // Consumo base (~1.2/s) + extra mientras thrusting (~3.0/s)
    double consumoBase = 1.2 * dt;
    double consumoExtra = 0.0;
    if(thrusting && fuel > 0.0){
        consumoExtra = 3.0 * dt;
    }
    fuel -= (consumoBase + consumoExtra);
    if(fuel < 0.0){
        fuel = 0.0;
    }

    // ===== EMPUJE / GRAVEDAD =====
    // Si no hay combustible, thrusting se ignora → solo gravedad
    if(thrusting && fuel > 0.0){
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

    // ===== VIENTO =====
    // Empuja al helicóptero en la dirección del viento (se suma a la velocidad).
    velX += windX * dt;
    velY += windY * dt;

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
        // Se mueve con el mundo usando x() y la velocidad de scroll (60px/s)
        double scrollVel = 60.0;
        if(game->mundoEnMovimiento){
            newX = x() - scrollVel * dt;
            if(newX < 0){
                newX = 0;
            }
        }else{
            // Mundo detenido (meta visible): el heli queda quieto
            newX = x();
        }

        setPos(newX, newY);
        checkLanding();

        // ===== GASOLINA OBLIGATORIA =====
        // En el suelo sin gasolina: pierde
        if(!crashed && fuel <= 0.0){
            crash();
        }
        return;
    }

    setPos(newX,newY);

    //controla el angulo segun la velocidad
    double targetAngle = physics->calculateTilt(velY, velX);

    double currentAngle = rotation();
    double newAngle = currentAngle + (targetAngle - currentAngle) * 0.15;
    setRotation(newAngle);
}

void MyHeli::mostrarAvisoViento(bool visible){
    // Aviso rojo de ventisca; se crea bajo demanda (en el constructor
    // el heli aún no está en la escena)
    if(scene() == nullptr){
        return;
    }
    if(textoViento == nullptr){
        textoViento = new QGraphicsTextItem("Corriente de viento pasando");
        textoViento->setFont(QFont("Arial", 20, QFont::Bold));
        textoViento->setDefaultTextColor(Qt::red);
        textoViento->setZValue(300);
        scene()->addItem(textoViento);
    }
    textoViento->setPos(scene()->width() / 2 - textoViento->boundingRect().width() / 2, 100);
    textoViento->setVisible(visible);
}

void MyHeli::checkLanding(){
    //tolerancia de aterrizaje

    // Sin gasolina el heli cae y explota al tocar el suelo (la gasolina
    // es obligatoria: no se puede volar sin combustible).
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
        // Evitar puntero colgante: si el juego sigue abierto, avisarle que
        // el heli ya no existe (así ~Game no lo borra dos veces).
        if(game != nullptr && game->heli == this){
            game->heli = nullptr;
        }
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
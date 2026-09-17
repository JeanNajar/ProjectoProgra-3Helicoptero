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

ObstacleH::ObstacleH(ObstacleType type, ObstacleManager *manager, int nivel): QObject(), QGraphicsPixmapItem(){

    // Guardar el tipo
    tipo = type;
    this->manager = manager;
    this->nivel = nivel;

    // Pixmap según el nivel (ciudad / desierto / nieve) y el tipo.
    // Los obstáculos de desierto y nieve usan sus imágenes propias;
    // el de techo (CEILING) no tiene variante temática y usa el genérico.
    QPixmap pix;
    switch(nivel){
    case 2: // desierto
        switch(tipo) {
        case ObstacleType::VERTICAL:
            pix = QPixmap(":/Sprites/recursosh/desierto_obstaculo_roca_vertical_60x200.png");
            break;
        case ObstacleType::SMALL:
            pix = QPixmap(":/Sprites/recursosh/desierto_obstaculo_roca_pequena_48x48.png");
            break;
        case ObstacleType::FALLING:
            pix = QPixmap(":/Sprites/recursosh/desierto_obstaculo_roca_cayendo_40x40.png");
            break;
        default: // CEILING
            pix = QPixmap(":/Sprites/recursosh/tuberias_techo_obstaculo_160x64.png");
        }
        break;
    case 3: // nieve
        switch(tipo) {
        case ObstacleType::VERTICAL:
            pix = QPixmap(":/Sprites/recursosh/nieve_obstaculo_espiga_hielo_60x200.png");
            break;
        case ObstacleType::SMALL:
            pix = QPixmap(":/Sprites/recursosh/nieve_obstaculo_roca_hielo_48x48.png");
            break;
        case ObstacleType::FALLING:
            pix = QPixmap(":/Sprites/recursosh/nieve_obstaculo_carambano_cayendo_28x70.png");
            break;
        default: // CEILING
            pix = QPixmap(":/Sprites/recursosh/tuberias_techo_obstaculo_160x64.png");
        }
        break;
    default: // ciudad (nivel 1)
        switch(tipo) {
        case ObstacleType::VERTICAL:
            //torre industrial (obstaculo alto)
            //se escala para que sea mas gruesa y alta y quede anclada al suelo
            //(el manager la coloca en y = altura - 300, asi la base toca el fondo)
            pix = QPixmap(":/Sprites/recursosh/torre_industrial_obstaculo_40x140.png")
                      .scaled(80, 300, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            break;
        case ObstacleType::SMALL:
            //cajon / barrera industrial (obstaculo bajo)
            pix = QPixmap(":/Sprites/recursosh/cajon_barrera_obstaculo_32x32.png");
            break;
        default: // CEILING y FALLING (el nivel 1 no usa FALLING, por si acaso)
            //tuberias colgando del techo
            pix = QPixmap(":/Sprites/recursosh/tuberias_techo_obstaculo_160x64.png");
        }
    }
    setPixmap(pix);

    // Vida según el tamaño del obstáculo (impactos de bala que aguanta)
    if(tipo == ObstacleType::SMALL || tipo == ObstacleType::FALLING){
        vida = 2;
    }else{
        vida = 3;
    }
    vidaMaxima = vida;
    barraVisible = false;

    // Barra de vida estilo Survivor: se dibuja encima del obstáculo pero
    // OCULTA hasta que una bala le pegue por primera vez.
    barraFondo = new QGraphicsRectItem(0, 0, boundingRect().width(), 6, this);
    barraFondo->setBrush(QBrush(QColor(60, 60, 60)));
    barraFondo->setPen(QPen(Qt::black));
    barraFondo->setPos(0, -10);
    barraFondo->setVisible(false);

    barra = new QGraphicsRectItem(0, 0, boundingRect().width(), 6, this);
    barra->setBrush(QBrush(Qt::yellow));
    barra->setPen(QPen(Qt::NoPen));
    barra->setPos(0, -10);
    barra->setVisible(false);

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

    // Si el juego terminó (ganó o perdió), los obstáculos se congelan
    if(game == nullptr || game->juegoTerminado){
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
    if(tipo == ObstacleType::FALLING){
        // cae de arriba hacia abajo (3px por tick = 60px/s)
        setPos(x(), y()+3);
        if(pos().y() > scene()->height()){
            scene()->removeItem(this);
            manager->notifyObstacleDied(this);
        }
        return;
    }

    //mover el obstaculo (3px por tick = 60px/s, igual que el scroll del mundo)
    setPos(x()-3,y());
    if(pos().x()+boundingRect().width()< 0){
        scene()->removeItem(this);
        manager->notifyObstacleDied(this);
    }
}

void ObstacleH::recibirImpacto(){

    if(scene() == nullptr){
        return;
    }

    // Si el juego terminó, las balas ya no afectan
    if(game == nullptr || game->juegoTerminado){
        return;
    }

    vida--;
    if(vida < 0){
        vida = 0;
    }

    // En el primer impacto la barra aparece arriba del obstáculo
    if(!barraVisible){
        barraVisible = true;
        barraFondo->setVisible(true);
        barra->setVisible(true);
    }

    // La barra se va LLENANDO con cada bala: ancho = daño acumulado / vida total
    int danio = vidaMaxima - vida;
    int ancho = static_cast<int>(boundingRect().width() * (danio / (double)vidaMaxima));
    barra->setRect(0, 0, ancho, 6);

    if(vida <= 0){
        // Destruido por las balas
        scene()->removeItem(this);
        manager->notifyObstacleDied(this);
    }
}
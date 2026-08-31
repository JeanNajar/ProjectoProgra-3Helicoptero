#include "Game.h"
#include "ObstacleType.h"

#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>

Game::Game(QWidget *parent) : QGraphicsView(parent) {


    //create a scene
    scene = new QGraphicsScene();
    scene->setSceneRect(0,0,800,600);

    setScene(scene);
    //turnoff horizontal and vertical bars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(800, 600);
    //create an item to add to the scene

    heli = new MyHeli();

    // Posicionar el helicoptero
    heli->setPos(20, height() - heli->rect().height());

    //make the heli focusable
    heli->setFlag(QGraphicsItem::ItemIsFocusable);
    heli->setFocus();

    // Añadirlo a la escena
    scene->addItem(heli);

    //crear el puntaje
    score =new Score();
    scene->addItem(score);

    //crear la vida
    health = new Health();
    health->setPos(health->x(),health->y()+25);
    scene->addItem(health);

    obstacleManager = new ObstacleManager(scene, this);

    QTimer * spawnTimer = new QTimer();
    QObject::connect(spawnTimer, SIGNAL(timeout()), this, SLOT(spawnObstacles()));
    spawnTimer->start(3000);

    //play background music
   /*
    QMediaPlayer *heliSound = new QMediaPlayer();
    QAudioOutput *audio = new QAudioOutput();

    heliSound->setAudioOutput(audio);
    audio->setVolume(0.3);
    heliSound->setSource(QUrl("qrc:/Sounds/recursosh/HelicopteroSound.mp3"));
    heliSound->play();*/
}

Game::~Game(){

    delete obstacleManager;

}

void Game::spawnObstacles(){

    static int contador = 0;

    ObstacleType tipo;
    int yPos = 0;

    switch(contador % 3){
    case 0:
        tipo = ObstacleType::VERTICAL;
        // yPos se queda en 0, spawnObstacle usa la posición por defecto (parte baja)
        break;
    case 1:
        tipo = ObstacleType::SMALL;
        break;
        // yPos se queda en 0, spawnObstacle
    case 2:
        tipo = ObstacleType::CEILING;
        // yPos se queda en 0, spawnObstacle siempre pone arriba (y=0)
        break;
    }

    obstacleManager->spawnObstacle(tipo, yPos);

    contador++;
}

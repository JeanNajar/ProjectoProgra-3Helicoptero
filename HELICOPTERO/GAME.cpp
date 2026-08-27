#include "Game.h"
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
heli->setRect(0,0,100,100);

// Posicionarlo en el centro inferior
heli->setPos(width() / 2, height() - heli->rect().height());

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

QTimer * timer=new QTimer();
QObject::connect(timer,SIGNAL(timeout()),heli,SLOT(spawn()));
timer->start(5000);

//play background music
QMediaPlayer *heliSound = new QMediaPlayer();
QAudioOutput *audio = new QAudioOutput();

heliSound->setAudioOutput(audio);
audio->setVolume(0.3);

heliSound->setSource(QUrl("qrc:/Sounds/recursosh/HelicopteroSound.mp3"));

heliSound->play();
}
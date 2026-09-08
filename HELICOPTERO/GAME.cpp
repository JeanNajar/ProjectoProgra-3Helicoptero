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
    //manager del nivel
    finishLine = nullptr;
    heliEnZona = false;
    tiempoEnZona = 0;
    nivelGanado = false;


     levelManager = new LevelManager(this);

    // Timer principal
    QTimer *finishCheckTimer = new QTimer(this);
    connect(finishCheckTimer, SIGNAL(timeout()), this, SLOT(checkFinishLine()));
    finishCheckTimer->start(50);


    // Conectar el spawn de obstáculos al timer del nivel
    connect(levelManager->spawnTimer, SIGNAL(timeout()),
        this, SLOT(spawnObstacles()));

    levelManager->startLevel(30, 3000);

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

     // Cuando el tiempo llega a 0 LevelManager pone active=false,
    if(!levelManager->isActive()){
        return;
    }

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


void Game::checkFinishLine(){


    // Solo actualizar el texto si el tiempo cambió
    static int ultimoTiempo = -1;
    int tiempoActual = levelManager->getTimeRemaining();
    if(tiempoActual != ultimoTiempo){
        ultimoTiempo = tiempoActual;
        score->setPlainText("Tiempo: " + QString::number(tiempoActual)
                            + "s  |  Score: " + QString::number(score->getScore()));
    }

     //DETECTAR FIN DE NIVEL

    static bool instruccionMostrada = false;
    if(levelManager->isFinished() && !instruccionMostrada){
        instruccionMostrada = true;

        QGraphicsTextItem *instruccion = new QGraphicsTextItem("Tiempo terminado! Aterriza en la zona VERDE");
        instruccion->setDefaultTextColor(Qt::yellow);
        instruccion->setFont(QFont("times", 18));
        instruccion->setPos(scene->width() / 2 - 200, 60);
        scene->addItem(instruccion);

        //evita puntero colgante
        if(heli->scene() != nullptr){
            heli->setFocus();
        }
    }

    // CREAR LA ZONA CUANDO LA PANTALLA ESTÉ LIMPIA

    if(finishLine == nullptr && levelManager->isFinished() && !nivelGanado){
        // si ya no hay obstáculos en pantalla
        if(obstacleManager->countVisible() == 0){
            // Crear la zona de aterrizaje
            finishLine = new FinishLine(scene);
        }else{
            // Todavía hay obstáculos, esperar al siguiente tick
            return;
        }
    }

    //VERIFICAR ATERRIZAJE EN LA ZONA
    // Solo verificar si la zona ya existe y el nivel terminó
    if(finishLine == nullptr || !levelManager->isFinished() || nivelGanado){
        return;
    }
    //evita segmentation fault
    if(heli->scene() == nullptr){
        return;
    }
    // Verificar si el helicóptero está ATERRIZADO y sobre la zona
    bool heliEnSuelo = (heli->y() + heli->rect().height() >= scene->height() - 2);

    // Verificar si el heli colisiona con la zona
    QList<QGraphicsItem*> colliding = heli->collidingItems();
    bool sobreZona = false;
    for(int i = 0; i < colliding.size(); i++){
        if(colliding[i] == finishLine){
            sobreZona = true;
            break;
        }
    }

    if(heliEnSuelo && sobreZona){
        // El heli está aterrizado sobre la zona → sumar tiempo
        tiempoEnZona++;

        // 40 ticks = 2 segundos
        if(tiempoEnZona >= 40){
            nivelGanado = true;
            mostrarVictoria();
        }
    }else{
        // El heli no está en la zona reinicia el contador
        tiempoEnZona = 0;
    }
}


void Game::mostrarVictoria(){

    // Texto de victoria
    QGraphicsTextItem *victoria = new QGraphicsTextItem("NIVEL COMPLETADO!");
    victoria->setDefaultTextColor(Qt::green);
    victoria->setFont(QFont("times", 36));
    victoria->setPos(scene->width() / 2 - 150, scene->height() / 2 - 20);
    scene->addItem(victoria);

    // Texto secundario
    QGraphicsTextItem *sub = new QGraphicsTextItem("Has aterrizado con exito");
    sub->setDefaultTextColor(Qt::white);
    sub->setFont(QFont("times", 16));
    sub->setPos(scene->width() / 2 - 100, scene->height() / 2 + 30);
    scene->addItem(sub);
}
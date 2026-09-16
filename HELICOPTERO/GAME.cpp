#include "Game.h"
#include "ObstacleType.h"
#include "Menu.h"

#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsPixmapItem>
#include <QColor>
#include <QPixmap>
#include <QPainter>
#include <QShowEvent>
#include <QResizeEvent>

//puntero global del juego (definido en main.cpp)
extern Game * game;
//puntero global del menú (definido en main.cpp): se usa para volver al menú
extern Menu * menu;

Game::Game(QWidget *parent) : QGraphicsView(parent) {


    //create a scene
    scene = new QGraphicsScene();
    scene->setSceneRect(0,0,800,600);

    setScene(scene);
    //turnoff horizontal and vertical bars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    resize(800, 600);
    setMinimumSize(640, 480);
    //color de fondo de la vista (igual al cielo del fondo, evita franjas blancas)
    setBackgroundBrush(QColor(21, 10, 43));

    //fondo de la ciudad cyberpunk.
    // YA NO es un item de la escena: se dibuja en drawBackground() estirado
    // a TODA la ventana, así al agrandar la ventana no quedan franjas
    // oscuras en los lados (el fondo crece con la ventana).
    fondoCiudad = QPixmap(":/Sprites/recursosh/fondo_ciudad_cyberpunk_800x600.png");

    //create an item to add to the scene

    heli = new MyHeli();

    // Posicionar el helicoptero
    heli->setPos(20, height() - heli->boundingRect().height());

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

    // ===== MANAGER DE SUPERVIVIENTES =====
    // Maneja a los supervivientes en el suelo. Recibe el obstacleManager
    // para no generar supervivientes dentro de un obstáculo.
    survivorManager = new SurvivorManager(scene, obstacleManager, this);

    //manager del nivel
    finishLine = nullptr;
    heliEnZona = false;
    tiempoEnZona = 0;
    nivelGanado = false;
    mundoEnMovimiento = true;  // El mundo avanza hasta que aparezca la meta
    juegoTerminado = false;    // false: el juego sigue en curso

    // Estado del HUD (reiniciable al reintentar el nivel)
    ultimoTiempo = -1;
    instruccionMostrada = false;

    // Panel de resultados (se crea la primera vez que se gana o pierde)
    panelFondo = nullptr;
    panelTitulo = nullptr;
    panelNota = nullptr;
    panelSub = nullptr;
    barraFondo = nullptr;
    barraRelleno = nullptr;
    btnReintentar = nullptr;
    btnMenu = nullptr;
    proxyReintentar = nullptr;
    proxyMenu = nullptr;


    levelManager = new LevelManager(this);

    // Timer principal
    QTimer *finishCheckTimer = new QTimer(this);
    connect(finishCheckTimer, SIGNAL(timeout()), this, SLOT(checkFinishLine()));
    finishCheckTimer->start(50);

    // Timer de actualización de supervivientes (cada 50ms).
    // Le dice a cada superviviente si el heli está encima (para el rescate)
    // y elimina los que ya fueron rescatados.
    QTimer *survivorTimer = new QTimer(this);
    connect(survivorTimer, SIGNAL(timeout()), this, SLOT(updateSurvivors()));
    survivorTimer->start(50);


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

    delete survivorManager;
    delete obstacleManager;
    // La escena se borra al final: elimina el heli, el score, la salud,
    // la zona de aterrizaje y el panel de resultados. Sin esto, cada
    // reintento filtraría una escena completa en memoria.
    delete scene;

}

void Game::spawnObstacles(){

    // Si el juego terminó (ganó o perdió), no generar más obstáculos
    if(juegoTerminado){
        return;
    }

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

    // Generar un superviviente DETRÁS del obstáculo recién creado.
    // Solo se generan 2 por nivel. Así el superviviente aparece "en el mapa"
    // detrás de un obstáculo y se mueve junto con él (efecto scroll).
    if(!survivorManager->allSpawned()){
        survivorManager->spawnSurvivorBehindObstacle(
            obstacleManager->getObstacle(obstacleManager->getCantidad() - 1));
    }
}

void Game::updateSurvivors(){
    // Si el juego terminó, no actualizar supervivientes
    // (evita tocar el heli si ya fue destruido)
    if(juegoTerminado){
        return;
    }

    // Le dice a cada superviviente si el heli está encima (para el rescate)
    // y elimina los que ya fueron rescatados.
    if(heli == nullptr || heli->scene() == nullptr){
        return;
    }
    QRectF heliRect(heli->pos(), heli->boundingRect().size());
    survivorManager->updateAll(heliRect);
}


void Game::checkFinishLine(){

    // Si el juego terminó (ganó o perdió), no hacer nada más
    if(juegoTerminado){
        return;
    }

    // Solo actualizar el texto si el tiempo cambió
    int tiempoActual = levelManager->getTimeRemaining();
    if(tiempoActual != ultimoTiempo){
        ultimoTiempo = tiempoActual;
        score->setPlainText("Tiempo: " + QString::number(tiempoActual)
                            + "s  |  Score: " + QString::number(score->getScore()));
    }

    //DETECTAR FIN DE NIVEL

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
            // El mapa deja de avanzar: el heli aterrizado ya no se desliza,
            // da la sensación de que el mundo se detuvo y solo falta aterrizar.
            mundoEnMovimiento = false;
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
    bool heliEnSuelo = (heli->y() + heli->boundingRect().height() >= scene->height() - 2);

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

    if(juegoTerminado){
        return;  // ya se mostró un panel
    }
    juegoTerminado = true;  // pausa todo el mundo

    crearPanel();

    panelTitulo->setPlainText("NIVEL COMPLETADO");
    panelTitulo->setDefaultTextColor(Qt::green);

    // Nota estilo Cuphead: A si vidas completas y todos los supervivientes
    // rescatados; cada vida perdida o superviviente no rescatado baja un
    // escalón (A, A-, B+, B, ...).
    int vidas = health->getHealth();
    int rescatados = survivorManager->getTotalRescatados();
    QString nota = calcularNota(vidas, rescatados);

    panelNota->setPlainText(nota);
    panelNota->setDefaultTextColor(QColor(255, 215, 0));  // dorado

    panelSub->setPlainText("Vidas: " + QString::number(vidas) + "/3"
                           + "   Rescatados: " + QString::number(rescatados) + "/2");

    // Centrar los textos sobre el panel
    panelTitulo->setPos(scene->width() / 2 - panelTitulo->boundingRect().width() / 2, 140);
    panelNota->setPos(scene->width() / 2 - panelNota->boundingRect().width() / 2, 210);
    panelSub->setPos(scene->width() / 2 - panelSub->boundingRect().width() / 2, 330);

    // La barra de progreso solo se usa en la derrota
    barraFondo->setVisible(false);
    barraRelleno->setVisible(false);
}

void Game::mostrarDerrota(){

    if(juegoTerminado){
        return;  // ya se mostró un panel
    }
    juegoTerminado = true;  // pausa todo el mundo

    crearPanel();

    panelTitulo->setPlainText("NIVEL FALLIDO");
    panelTitulo->setDefaultTextColor(Qt::red);

    // Porcentaje de rescate: 2 supervivientes = 200 puntos → 0 a 100%
    int porcentaje = survivorManager->getProgresoTotal() / 2;
    if(porcentaje > 100){
        porcentaje = 100;
    }

    panelNota->setPlainText(QString::number(porcentaje) + "%");
    panelNota->setDefaultTextColor(porcentaje >= 50 ? Qt::yellow : Qt::red);

    int rescatados = survivorManager->getTotalRescatados();
    panelSub->setPlainText("Rescatados: " + QString::number(rescatados) + "/2");

    // Barra de progreso simple
    barraFondo->setVisible(true);
    barraRelleno->setVisible(true);
    int ancho = static_cast<int>(300.0 * porcentaje / 100.0);
    barraRelleno->setRect(250, 380, ancho, 24);
    QColor colorBarra;
    if(porcentaje >= 66){
        colorBarra = QColor(0, 200, 80);      // verde
    }else if(porcentaje >= 33){
        colorBarra = QColor(255, 200, 0);     // amarillo
    }else{
        colorBarra = QColor(220, 40, 40);     // rojo
    }
    barraRelleno->setBrush(colorBarra);

    // Centrar los textos sobre el panel
    panelTitulo->setPos(scene->width() / 2 - panelTitulo->boundingRect().width() / 2, 140);
    panelNota->setPos(scene->width() / 2 - panelNota->boundingRect().width() / 2, 210);
    panelSub->setPos(scene->width() / 2 - panelSub->boundingRect().width() / 2, 330);
}

void Game::crearPanel(){

    if(panelFondo != nullptr){
        return;  // ya creado
    }

    // Fondo oscuro semi-transparente que tapa toda la escena
    panelFondo = new QGraphicsRectItem(0, 0, scene->width(), scene->height());
    panelFondo->setBrush(QColor(0, 0, 0, 180));
    panelFondo->setPen(Qt::NoPen);
    panelFondo->setZValue(1000);
    scene->addItem(panelFondo);

    panelTitulo = new QGraphicsTextItem();
    panelTitulo->setFont(QFont("times", 34, QFont::Bold));
    panelTitulo->setZValue(1001);
    scene->addItem(panelTitulo);

    panelNota = new QGraphicsTextItem();
    panelNota->setFont(QFont("times", 72, QFont::Bold));
    panelNota->setZValue(1001);
    scene->addItem(panelNota);

    panelSub = new QGraphicsTextItem();
    panelSub->setDefaultTextColor(Qt::white);
    panelSub->setFont(QFont("times", 16));
    panelSub->setZValue(1001);
    scene->addItem(panelSub);

    // Barra de progreso (solo visible en la derrota)
    barraFondo = new QGraphicsRectItem(250, 380, 300, 24);
    barraFondo->setBrush(QColor(40, 40, 40));
    barraFondo->setPen(QPen(Qt::black));
    barraFondo->setZValue(1001);
    barraFondo->setVisible(false);
    scene->addItem(barraFondo);

    barraRelleno = new QGraphicsRectItem(250, 380, 0, 24);
    barraRelleno->setBrush(QColor(0, 200, 80));
    barraRelleno->setPen(Qt::NoPen);
    barraRelleno->setZValue(1002);
    barraRelleno->setVisible(false);
    scene->addItem(barraRelleno);

    // Botones (widgets Qt incrustados en la escena, escalan con fitInView)
    btnReintentar = new QPushButton("Reintentar");
    btnReintentar->setFixedSize(220, 50);
    btnReintentar->setCursor(Qt::PointingHandCursor);
    btnReintentar->setStyleSheet(
        "QPushButton { background-color: #1a1a2e; color: #4de8ff;"
        " border: 2px solid #4de8ff; border-radius: 6px;"
        " font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2a2a4e; }"
        "QPushButton:pressed { background-color: #0f0f1e; }");
    proxyReintentar = scene->addWidget(btnReintentar);
    proxyReintentar->setPos(290, 440);
    proxyReintentar->setZValue(1001);
    connect(btnReintentar, &QPushButton::clicked, this, &Game::reintentarNivel);

    btnMenu = new QPushButton("Volver al menú");
    btnMenu->setFixedSize(220, 50);
    btnMenu->setCursor(Qt::PointingHandCursor);
    btnMenu->setStyleSheet(
        "QPushButton { background-color: #1a1a2e; color: #ff6b6b;"
        " border: 2px solid #ff6b6b; border-radius: 6px;"
        " font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2a2a4e; }"
        "QPushButton:pressed { background-color: #0f0f1e; }");
    proxyMenu = scene->addWidget(btnMenu);
    proxyMenu->setPos(290, 500);
    proxyMenu->setZValue(1001);
    connect(btnMenu, &QPushButton::clicked, this, &Game::volverAlMenu);
}

QString Game::calcularNota(int vidas, int rescatados) const{
    // Nota estilo Cuphead: A si vidas completas (3) y todos los
    // supervivientes rescatados (2). Cada vida perdida o superviviente
    // no rescatado baja un escalón: A, A-, B+, B, B-, C+, C, C-, D, F.
    static const char *escalones[] = {"A", "A-", "B+", "B", "B-",
                                      "C+", "C", "C-", "D", "F"};
    int deducciones = (3 - vidas) + (2 - rescatados);
    if(deducciones < 0){
        deducciones = 0;
    }
    if(deducciones > 9){
        deducciones = 9;
    }
    return QString(escalones[deducciones]);
}

void Game::reintentarNivel(){
    // Crear un nivel nuevo y cerrar este. El puntero global `game` se
    // actualiza al nuevo juego para que el resto del código (MyHeli,
    // ObstacleH, Survivor) siga funcionando con el juego vigente.
    Game *nuevo = new Game();
    nuevo->setAttribute(Qt::WA_DeleteOnClose);
    this->close();   // este juego se borra solo (WA_DeleteOnClose)
    game = nuevo;
    nuevo->show();
}

void Game::volverAlMenu(){
    // Mostrar el menú ANTES de cerrar el juego: si el menú está oculto y
    // cerramos el juego, la app se cerraría (última ventana visible).
    menu->show();
    this->close();
}

void Game::showEvent(QShowEvent *event){
    QGraphicsView::showEvent(event);
    //escala la escena para que quepa completa en la ventana
    //(evita que el heli y el suelo se corten en el borde inferior)
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void Game::resizeEvent(QResizeEvent *event){
    QGraphicsView::resizeEvent(event);
    // Re-escalar la escena cada vez que se redimensiona la ventana.
    // KeepAspectRatio: la escena 800x600 se escala proporcionalmente y
    // las franjas sobrantes se rellenan con el fondo de la ciudad
    // (drawBackground), así nunca se ve deformada ni cortada.
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void Game::drawBackground(QPainter *painter, const QRectF &rect){
    // Dibujar el fondo de la ciudad estirado a TODA la ventana.
    // Al redimensionar, el fondo crece con la ventana: no quedan franjas
    // oscuras en los lados (antes el fondo era un item de la escena y solo
    // cubría los 800x600 de la escena).
    painter->save();
    painter->resetTransform();  // dibujar en coordenadas de la ventana
    painter->drawPixmap(viewport()->rect(), fondoCiudad);
    painter->restore();
}
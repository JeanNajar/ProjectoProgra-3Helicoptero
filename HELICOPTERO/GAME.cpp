#include "GAME.h"
#include "ObstacleType.h"
#include "VentanaPrincipal.h"
#include "ResourceLoader.h"

#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFont>
#include <QGraphicsPixmapItem>
#include <QColor>
#include <QPixmap>
#include <QPainter>
#include <QShowEvent>
#include <QResizeEvent>
#include <QSettings>
#include <cstdlib>

//puntero global del juego (definido en main.cpp)
extern Game * game;
//ventana única (definida en main.cpp): se usa para navegar entre páginas
extern VentanaPrincipal * ventanaPrincipal;
//usuario que inició sesión (definido en main.cpp): el progreso es por cuenta
extern QString usuarioActual;

Game::Game(int nivel, QWidget *parent) : QGraphicsView(parent) {

    nivelActual = nivel;

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

    //fondo según el nivel.
    // YA NO es un item de la escena: se dibuja en drawBackground() estirado
    // a TODA la ventana, así al agrandar la ventana no quedan franjas
    // oscuras en los lados (el fondo crece con la ventana).
    // Se usa la caché del hilo de precarga (ResourceLoader); si el hilo aún
    // no terminó, se carga directo como respaldo.
    fondoCiudad = ResourceLoader::background(nivel);
    if(fondoCiudad.isNull()){
        QString rutaFondo;
        switch(nivel){
        case 2:
            rutaFondo = ":/Sprites/recursosh/nivel2_desierto_800x600.png";
            break;
        case 3:
            rutaFondo = ":/Sprites/recursosh/nivel3_nieve_800x600.png";
            break;
        default:
            rutaFondo = ":/Sprites/recursosh/fondo_ciudad_cyberpunk_800x600.png";
        }
        fondoCiudad = QPixmap(rutaFondo);
    }

    //create an item to add to the scene

    heli = new MyHeli(nivel);

    // Posicionar el helicoptero
    heli->setPos(20, height() - heli->boundingRect().height());

    //make the heli focusable
    heli->setFlag(QGraphicsItem::ItemIsFocusable);
    heli->setFocus();

    // Añadirlo a la escena
    scene->addItem(heli);

    //crear el puntaje (posicionado en la esquina superior izquierda)
    score =new Score();
    score->setPos(20, 10);
    score->setZValue(100);
    scene->addItem(score);

    //crear la vida (posicionada debajo del puntaje, más grande y visible)
    health = new Health();
    health->setPos(20, 50);
    health->setZValue(100);
    scene->addItem(health);

    obstacleManager = new ObstacleManager(scene, nivel, this);

    // ===== MANAGER DE SUPERVIVIENTES =====
    // Maneja a los supervivientes en el suelo. Recibe el obstacleManager
    // para no generar supervivientes dentro de un obstáculo, y el nivel
    // para usar los sprites correctos (ciudad / desierto / nieve).
    survivorManager = new SurvivorManager(scene, obstacleManager, nivel, this);

    //manager del nivel
    finishLine = nullptr;
    heliEnZona = false;
    tiempoEnZona = 0;
    nivelGanado = false;
    mundoEnMovimiento = true;  // El mundo avanza hasta que aparezca la meta
    juegoTerminado = false;    // false: el juego sigue en curso
    contador = 0;  // Contador de patrón de obstáculos

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
    btnNiveles = nullptr;
    btnMenu = nullptr;
    proxyReintentar = nullptr;
    proxyNiveles = nullptr;
    proxyMenu = nullptr;


    levelManager = new LevelManager(this);

    // Timer principal de verificación de zona de aterrizaje
    finishCheckTimer = new QTimer(this);
    connect(finishCheckTimer, SIGNAL(timeout()), this, SLOT(checkFinishLine()));
    finishCheckTimer->start(50);

    // Timer de actualización de supervivientes (cada 50ms)
    survivorTimer = new QTimer(this);
    connect(survivorTimer, SIGNAL(timeout()), this, SLOT(updateSurvivors()));
    survivorTimer->start(50);

    // Timer para la navegación automática después de victoria/derrota
    resultTimer = new QTimer(this);
    resultTimer->setSingleShot(true);

    // ===== MÚSICA DE FONDO =====
    bgMusic = new QMediaPlayer(this);
    bgAudio = new QAudioOutput(this);
    bgMusic->setAudioOutput(bgAudio);
    bgAudio->setVolume(0.3);
    bgMusic->setSource(QUrl("qrc:/Sounds/recursosh/HelicopteroSound.mp3"));
    bgMusic->play();
}

Game::~Game(){

    // Cancelar timers pendientes
    if(finishCheckTimer != nullptr){
        finishCheckTimer->stop();
        finishCheckTimer->deleteLater();
    }
    if(survivorTimer != nullptr){
        survivorTimer->stop();
        survivorTimer->deleteLater();
    }
    if(resultTimer != nullptr){
        resultTimer->stop();
        resultTimer->deleteLater();
    }

    // Detener y liberar música de fondo
    if(bgMusic != nullptr){
        bgMusic->stop();
        bgMusic->deleteLater();
    }
    if(bgAudio != nullptr){
        bgAudio->deleteLater();
    }

    // Limpiar elementos del popup de logros
    if(popupFondo != nullptr){ delete popupFondo; popupFondo = nullptr; }
    if(popupBarra != nullptr){ delete popupBarra; popupBarra = nullptr; }
    if(popupNota != nullptr){ delete popupNota; popupNota = nullptr; }
    if(popupMensaje != nullptr){ delete popupMensaje; popupMensaje = nullptr; }
    if(popupLogro != nullptr){ delete popupLogro; popupLogro = nullptr; }

    delete survivorManager;
    delete obstacleManager;

    // El heli tiene parte QObject (timers de física, rotor y explosión):
    // hay que borrarlo ANTES de la escena para detener sus timers y evitar
    // punteros colgantes al cambiar de nivel. Si ya se destruyó solo
    // (crash()), el puntero global quedó en nullptr (ver MyHeli::crash).
    if(heli != nullptr){
        delete heli;
        heli = nullptr;
    }

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

    ObstacleType tipo;
    int yPos = 0;

    // Patrón de obstáculos según el nivel:
    // - Nivel 1 (ciudad): los 3 clásicos (vertical, pequeño, techo).
    // - Niveles 2 y 3 (desierto/nieve): SOLO rocas/hielo del bioma
    //   (vertical, pequeño y el que CAE). El de techo (CEILING) es
    //   exclusivo de la ciudad y NO se genera en desierto/nieve.
    if(nivelActual >= 2){
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
            tipo = ObstacleType::FALLING;
            // cae desde arriba en una posición aleatoria (lo coloca spawnObstacle)
            break;
        }
    }else{
        switch(contador % 3){
        case 0:
            tipo = ObstacleType::VERTICAL;
            break;
        case 1:
            tipo = ObstacleType::SMALL;
            break;
        case 2:
            tipo = ObstacleType::CEILING;
            break;
        }
    }

    obstacleManager->spawnObstacle(tipo, yPos);

    contador++;

    // Generar un superviviente DETRÁS del obstáculo recién creado.
    // El máximo por nivel lo define SurvivorManager (5/10/15). Así el
    // superviviente aparece "en el mapa" detrás de un obstáculo y se mueve
    // junto con él (efecto scroll).
    if(!survivorManager->allSpawned()){
        survivorManager->spawnSurvivorBehindObstacle(
            obstacleManager->getObstacle(obstacleManager->getCantidad() - 1));
    }
}

void Game::spawnBidon(){

    // Si el juego terminó (ganó o perdió), no generar más bidones
    if(juegoTerminado){
        return;
    }

    // Cuando el tiempo llega a 0 LevelManager pone active=false
    if(!levelManager->isActive()){
        return;
    }

    // Aparece desde la derecha, a una altura aleatoria (evita el suelo y el techo)
    qreal xPos = scene->width() + 20;
    qreal yPos = 50 + (rand() % static_cast<int>(scene->height() - 150));

    BidonCombustible *bidon = new BidonCombustible(xPos, yPos, scene);
    scene->addItem(bidon);
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
        int rescatados = survivorManager->getTotalRescatados();
        int objetivo = survivorManager->getTotalObjetivo();
        score->mostrarRescates(rescatados, objetivo);
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
        return;
    }
    juegoTerminado = true;

    QSettings settings("HelicopterRescue", "Progreso");
    QString clave = QString("nivelDesbloqueado_%1").arg(usuarioActual);
    int desbloqueado = settings.value(clave, 1).toInt();
    if(nivelActual < 3 && nivelActual + 1 > desbloqueado){
        settings.setValue(clave, nivelActual + 1);
    }

    mostrarPopupVictoria();
}

void Game::mostrarDerrota(){

    if(juegoTerminado){
        return;  // ya se mostró un panel
    }
    juegoTerminado = true;  // pausa todo el mundo

    crearPanel();

    panelTitulo->setPlainText("NIVEL FALLIDO");
    panelTitulo->setDefaultTextColor(Qt::red);

    // Porcentaje de rescate: objetivo*100 puntos = 100%
    int objetivo = survivorManager->getTotalObjetivo();
    int porcentaje = survivorManager->getProgresoTotal() / objetivo;
    if(porcentaje > 100){
        porcentaje = 100;
    }

    panelNota->setPlainText(QString::number(porcentaje) + "%");
    panelNota->setDefaultTextColor(porcentaje >= 50 ? Qt::yellow : Qt::red);

    int rescatados = survivorManager->getTotalRescatados();
    panelSub->setPlainText("Rescatados: " + QString::number(rescatados) + "/" + QString::number(objetivo));

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

    // El selector de niveles se abre SOLO automáticamente al terminar la
    // partida (3 segundos para ver el resultado). Si el jugador pulsa
    // Reintentar / Seleccionar nivel / Volver al menú antes, este timer
    // se cancela solo.
    resultTimer->start(3000);
    connect(resultTimer, &QTimer::timeout, this, [this]() {
        seleccionarNivel();
    });
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
    proxyReintentar->setPos(290, 425);
    proxyReintentar->setZValue(1001);
    connect(btnReintentar, &QPushButton::clicked, this, &Game::reintentarNivel);

    btnNiveles = new QPushButton("Seleccionar nivel");
    btnNiveles->setFixedSize(220, 50);
    btnNiveles->setCursor(Qt::PointingHandCursor);
    btnNiveles->setStyleSheet(
        "QPushButton { background-color: #1a1a2e; color: #7dff9b;"
        " border: 2px solid #7dff9b; border-radius: 6px;"
        " font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2a2a4e; }"
        "QPushButton:pressed { background-color: #0f0f1e; }");
    proxyNiveles = scene->addWidget(btnNiveles);
    proxyNiveles->setPos(290, 475);
    proxyNiveles->setZValue(1001);
    connect(btnNiveles, &QPushButton::clicked, this, &Game::seleccionarNivel);

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
    proxyMenu->setPos(290, 525);
    proxyMenu->setZValue(1001);
    connect(btnMenu, &QPushButton::clicked, this, &Game::volverAlMenu);
}

QString Game::calcularNota(int vidas, int rescatados) const{
    // Nota estilo Cuphead: A si vidas completas (3) y todos los
    // supervivientes rescatados. Cada vida perdida o superviviente
    // no rescatado baja un escalón: A, A-, B+, B, B-, C+, C, C-, D, F.
    static const char *escalones[] = {"A", "A-", "B+", "B", "B-",
                                      "C+", "C", "C-", "D", "F"};
    int objetivo = survivorManager->getTotalObjetivo();
    int deducciones = (3 - vidas) + (objetivo - rescatados);
    if(deducciones < 0){
        deducciones = 0;
    }
    if(deducciones > 9){
        deducciones = 9;
    }
    return QString(escalones[deducciones]);
}

// ===== SISTEMA DE LOGROS =====

void Game::cargarLogros(){
    // No se necesita acciones adicionales; los logros se guardan en QSettings
}

void Game::guardarLogro(int nivel, const QString& nota){
    QSettings settings("HelicopterRescue", "Progreso");
    QString clave = QString("logro_nivel_%1").arg(nivel);
    settings.setValue(clave, nota);
}

bool Game::todosNivelesConA(){
    QSettings settings("HelicopterRescue", "Progreso");
    for(int i = 1; i <= 3; i++){
        QString clave = QString("logro_nivel_%1").arg(i);
        if(settings.value(clave).toString() != "A"){
            return false;
        }
    }
    return true;
}

void Game::actualizarLogros(){
    // Verificar si se desbloqueó el logro de todos los A
    if(todosNivelesConA()){
        // El logro ya está desbloqueado, no hacer nada extra
        // Se mostrará en el popup de la próxima victoria
    }
}

void Game::mostrarPopupVictoria(){
    // Limpiar popup anterior si existe
    if(popupFondo != nullptr){ scene->removeItem(popupFondo); delete popupFondo; popupFondo = nullptr; }
    if(popupBarra != nullptr){ scene->removeItem(popupBarra); delete popupBarra; popupBarra = nullptr; }
    if(popupNota != nullptr){ scene->removeItem(popupNota); delete popupNota; popupNota = nullptr; }
    if(popupMensaje != nullptr){ scene->removeItem(popupMensaje); delete popupMensaje; popupMensaje = nullptr; }
    if(popupLogro != nullptr){ scene->removeItem(popupLogro); delete popupLogro; popupLogro = nullptr; }

    // Obtener la nota
    int vidas = health->getHealth();
    int rescatados = survivorManager->getTotalRescatados();
    QString nota = calcularNota(vidas, rescatados);

    // Guardar el logro de este nivel
    guardarLogro(nivelActual, nota);
    actualizarLogros();

    // ===== COLORES POR NIVEL =====
    switch(nivelActual){
    case 1:
        popupColorFondo = QColor(10, 15, 40);
        popupColorBorde = QColor(77, 232, 255);
        break;
    case 2:
        popupColorFondo = QColor(40, 20, 5);
        popupColorBorde = QColor(255, 149, 0);
        break;
    case 3:
        popupColorFondo = QColor(15, 30, 50);
        popupColorBorde = QColor(170, 220, 255);
        break;
    default:
        popupColorFondo = QColor(10, 15, 40);
        popupColorBorde = QColor(77, 232, 255);
    }

    // ===== CREAR POPUP =====
    popupFondo = new QGraphicsRectItem(150, 100, 500, 350);
    popupFondo->setBrush(QBrush(popupColorFondo));
    popupFondo->setPen(QPen(popupColorBorde, 4));
    popupFondo->setZValue(2000);
    scene->addItem(popupFondo);

    popupBarra = new QGraphicsRectItem(150, 100, 500, 8);
    popupBarra->setBrush(QBrush(popupColorBorde));
    popupBarra->setPen(Qt::NoPen);
    popupBarra->setZValue(2001);
    scene->addItem(popupBarra);

    // Título "NIVEL COMPLETADO"
    QGraphicsTextItem *popupTitulo = new QGraphicsTextItem();
    popupTitulo->setFont(QFont("times", 20, QFont::Bold));
    popupTitulo->setDefaultTextColor(popupColorBorde);
    popupTitulo->setPlainText("NIVEL COMPLETADO");
    popupTitulo->setZValue(2002);
    popupTitulo->setPos(400 - popupTitulo->boundingRect().width() / 2, 150);
    scene->addItem(popupTitulo);

    // La letra grande (A, A-, B+, etc.)
    popupNota = new QGraphicsTextItem();
    popupNota->setFont(QFont("times", 96, QFont::Bold));
    popupNota->setDefaultTextColor(popupColorBorde);
    popupNota->setPlainText(nota);
    popupNota->setZValue(2003);
    popupNota->setPos(400 - popupNota->boundingRect().width() / 2, 190);
    scene->addItem(popupNota);

    // Mensaje debajo de la nota
    popupMensaje = new QGraphicsTextItem();
    popupMensaje->setFont(QFont("times", 18));
    if(nota == "A"){
        popupMensaje->setDefaultTextColor(QColor(0, 255, 150));
        popupMensaje->setPlainText("¡Excelente! ¡Sobresaliente!");
    }else if(nota.startsWith("B")){
        popupMensaje->setDefaultTextColor(QColor(255, 255, 100));
        popupMensaje->setPlainText("¡Buen trabajo! Sigue así");
    }else{
        popupMensaje->setDefaultTextColor(QColor(255, 150, 100));
        popupMensaje->setPlainText("¡Sigue intentando!");
    }
    popupMensaje->setZValue(2004);
    popupMensaje->setPos(400 - popupMensaje->boundingRect().width() / 2, 310);
    scene->addItem(popupMensaje);

    // ===== LOGRO ESPECIAL: TODOS LOS A =====
    if(todosNivelesConA()){
        popupLogro = new QGraphicsTextItem();
        popupLogro->setFont(QFont("times", 28, QFont::Bold));
        popupLogro->setDefaultTextColor(QColor(255, 215, 0));
        popupLogro->setPlainText("★ ¡Consigue todos los logros en A! ★");
        popupLogro->setZValue(2005);
        popupLogro->setPos(400 - popupLogro->boundingRect().width() / 2, 370);
        scene->addItem(popupLogro);

        QGraphicsRectItem *logroFondo = new QGraphicsRectItem(100, 355, 600, 50);
        logroFondo->setBrush(QBrush(QColor(255, 215, 0, 30)));
        logroFondo->setPen(QPen(QColor(255, 215, 0), 2));
        logroFondo->setZValue(2004);
        scene->addItem(logroFondo);
    }

    // ===== INFORMACIÓN ADICIONAL =====
    QGraphicsTextItem *infoRescatados = new QGraphicsTextItem();
    infoRescatados->setFont(QFont("times", 14));
    infoRescatados->setDefaultTextColor(Qt::white);
    infoRescatados->setPlainText(QString("Rescatados: %1/%2").arg(rescatados).arg(survivorManager->getTotalObjetivo()));
    infoRescatados->setZValue(2002);
    infoRescatados->setPos(400 - infoRescatados->boundingRect().width() / 2, 430);
    scene->addItem(infoRescatados);

    // Timer de cierre automático (5 segundos)
    resultTimer->start(5000);
    connect(resultTimer, &QTimer::timeout, this, [this]() {
        seleccionarNivel();
    });
}

void Game::reintentarNivel(){
    // Cancelar cualquier timer automático de victoria/derrota pendiente
    if(resultTimer != nullptr){
        resultTimer->stop();
    }
    // La ventana única recrea la página del juego con el mismo nivel
    ventanaPrincipal->reintentarNivel();
}

void Game::seleccionarNivel(){
    // Cancelar el timer automático antes de navegar
    if(resultTimer != nullptr){
        resultTimer->stop();
    }
    // Cambiar a la página del selector de niveles (ventana única)
    ventanaPrincipal->mostrarSelector();
}

void Game::volverAlMenu(){
    // Cancelar el timer automático antes de navegar
    if(resultTimer != nullptr){
        resultTimer->stop();
    }
    // Cambiar a la página del menú principal (ventana única)
    ventanaPrincipal->mostrarMenu();
}

void Game::reset(int nivel){
    // 0. Cancelar cualquier timer automático de victoria/derrota pendiente
    if(resultTimer != nullptr){
        resultTimer->stop();
    }

    // 1. Resetear estado
    juegoTerminado = false;
    nivelActual = nivel;
    mundoEnMovimiento = true;
    nivelGanado = false;
    heliEnZona = false;
    tiempoEnZona = 0;
    ultimoTiempo = -1;
    instruccionMostrada = false;
    finishLine = nullptr;
    contador = 0;

    // 2. Detener bidonTimer
    if(bidonTimer != nullptr){
        bidonTimer->stop();
    }

    // 3. Eliminar managers (son hijos de Game, no de la escena)
    if(survivorManager != nullptr){
        delete survivorManager;
        survivorManager = nullptr;
    }
    if(obstacleManager != nullptr){
        delete obstacleManager;
        obstacleManager = nullptr;
    }
    if(levelManager != nullptr){
        delete levelManager;
        levelManager = nullptr;
    }

    // 3.5 Eliminar objetos antiguos de la escena para evitar fugas de memoria
    if(heli != nullptr){
        scene->removeItem(heli);
        delete heli;
        heli = nullptr;
    }
    if(score != nullptr){
        scene->removeItem(score);
        delete score;
        score = nullptr;
    }
    if(health != nullptr){
        scene->removeItem(health);
        delete health;
        health = nullptr;
    }

    // 4. Limpiar la escena (elimina heli, score, health, finishLine,
    //    panel de resultados y bidones)
    scene->clear();

    // 5. Resetear punteros de items de escena (ahora dangling)
    heli = nullptr;
    score = nullptr;
    health = nullptr;
    panelFondo = nullptr;
    panelTitulo = nullptr;
    panelNota = nullptr;
    panelSub = nullptr;
    barraFondo = nullptr;
    barraRelleno = nullptr;
    btnReintentar = nullptr;
    btnNiveles = nullptr;
    btnMenu = nullptr;
    proxyReintentar = nullptr;
    proxyNiveles = nullptr;
    proxyMenu = nullptr;

    // 6. Re-crear managers
    obstacleManager = new ObstacleManager(scene, nivel, this);
    survivorManager = new SurvivorManager(scene, obstacleManager, nivel, this);
    levelManager = new LevelManager(this);
    // Re-conectar el spawn de obstáculos al timer del nivel
    connect(levelManager->spawnTimer, SIGNAL(timeout()),
            this, SLOT(spawnObstacles()));

    // 7. Re-crear heli
    heli = new MyHeli(nivel);
    heli->setPos(20, height() - heli->boundingRect().height());
    heli->setFlag(QGraphicsItem::ItemIsFocusable);
    heli->setFocus();
    scene->addItem(heli);

    // 8. Re-crear score y health
    score = new Score();
    score->setPos(20, 10);
    score->setZValue(100);
    scene->addItem(score);
    health = new Health();
    health->setPos(20, 50);
    health->setZValue(100);
    scene->addItem(health);
    // Actualizar el HUD inmediatamente
    health->updateDisplay();
    {
        int rescatados = survivorManager->getTotalRescatados();
        int objetivo = survivorManager->getTotalObjetivo();
        score->mostrarRescates(rescatados, objetivo);
    }

    // 9. Re-conectar bidonTimer (ya existe, solo reiniciarlo)
    if(bidonTimer != nullptr){
        bidonTimer->stop();
        bidonTimer->start(4000);
    }

    // 10. REINICIAR TIMERS PRINCIPALES (finishCheckTimer y survivorTimer)
    finishCheckTimer->stop();
    finishCheckTimer->start(50);
    survivorTimer->stop();
    survivorTimer->start(50);

    // 11. REINICIAR MÚSICA DE FONDO
    if(bgMusic != nullptr){
        bgMusic->stop();
        bgMusic->setPosition(0);
        bgMusic->play();
    }

    // 12. Iniciar nivel
    int tiempoLimite = 30;
    int spawnMs = 3000;
    switch(nivel){
    case 2:
        tiempoLimite = 30;
        spawnMs = 2500;
        break;
    case 3:
        tiempoLimite = 35;
        spawnMs = 2000;
        break;
    }
    levelManager->setLevelNumber(nivel);
    levelManager->startLevel(tiempoLimite, spawnMs);
}

void Game::showEvent(QShowEvent *event){
    QGraphicsView::showEvent(event);
    //escala la escena para que quepa completa en la ventana
    //(evita que el heli y el suelo se corten en el borde inferior)
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    // Al mostrarse la página del juego, darle el foco al heli para que
    // las teclas (subir, moverse, disparar) funcionen de inmediato.
    if(heli != nullptr && heli->scene() != nullptr){
        heli->setFocus();
    }
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
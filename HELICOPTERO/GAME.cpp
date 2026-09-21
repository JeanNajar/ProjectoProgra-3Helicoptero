#include "GAME.h"
#include "ObstacleType.h"
#include "VentanaPrincipal.h"
#include "ResourceLoader.h"
#include "AdminMusica.h"

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

//Puntero global del juego (definido en main.cpp)
extern Game * game;
//Ventana Ãºnica: navega entre pÃ¡ginas
extern VentanaPrincipal * ventanaPrincipal;
//Usuario actual: el progreso es por cuenta
extern QString usuarioActual;

Game::Game(int nivel, QWidget *parent) : QGraphicsView(parent) {

    nivelActual = nivel;

    //Crear la escena
    scene = new QGraphicsScene();
    scene->setSceneRect(0,0,800,600);

    setScene(scene);
    //Sin barras de scroll
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    resize(800, 600);
    setMinimumSize(640, 480);
    //Fondo de la vista (igual al cielo, evita franjas blancas)
    setBackgroundBrush(QColor(21, 10, 43));

    //Fondo segÃºn el nivel (se dibuja estirado en drawBackground)
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

    heli = new MyHeli(nivel);

    // Posicionar el helicoptero
    heli->setPos(20, height() - heli->boundingRect().height());

    //El heli recibe el foco (teclas)
    heli->setFlag(QGraphicsItem::ItemIsFocusable);
    heli->setFocus();

    // AÃ±adirlo a la escena
    scene->addItem(heli);

    //Puntaje arriba a la izquierda
    score =new Score();
    score->setPos(20, 10);
    score->setZValue(100);
    scene->addItem(score);

    //Vida debajo del puntaje
    health = new Health();
    health->setPos(20, 50);
    health->setZValue(100);
    scene->addItem(health);

    obstacleManager = new ObstacleManager(scene, nivel, this);

    //Manager de supervivientes (suelo)
    survivorManager = new SurvivorManager(scene, obstacleManager, nivel, this);

    //manager del nivel
    finishLine = nullptr;
    heliEnZona = false;
    tiempoEnZona = 0;
    nivelGanado = false;
    mundoEnMovimiento = true;  // El mundo avanza hasta que aparezca la meta
    juegoTerminado = false;    // false: el juego sigue en curso
    contador = 0;  // Contador de patrÃ³n de obstÃ¡culos

    //Estado del HUD
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
    btnOpciones = nullptr;
    proxyOpciones = nullptr;

    // Pantallita de pausa (se crea bajo demanda con el botÃ³n â‹®)
    pausaActiva = false;
    pausaFondo = nullptr;
    pausaTitulo = nullptr;
    btnReanudar = nullptr;
    btnPausaReiniciar = nullptr;
    btnPausaMenu = nullptr;
    proxyReanudar = nullptr;
    proxyPausaReiniciar = nullptr;
    proxyPausaMenu = nullptr;

    // Punteros que se crean bajo demanda: SIEMPRE inicializados a nullptr
    // (si no, son basura y las guardas if(x != nullptr) crashean).
    bidonTimer = nullptr;
    popupFondo = nullptr;
    popupBarra = nullptr;
    popupNota = nullptr;
    popupMensaje = nullptr;
    popupLogro = nullptr;

    // Barra de progreso del nivel (se crea en reset())
    barraTiempoFondo = nullptr;
    barraTiempoRelleno = nullptr;
    textoTiempo = nullptr;
    tiempoLimiteTotal = 0;

    // Barra de gasolina del HUD (se crea en reset())
    barraGasFondo = nullptr;
    barraGasRelleno = nullptr;
    textoGas = nullptr;
    textoGasCritico = nullptr;


    levelManager = new LevelManager(this);

    // Timer principal de verificaciÃ³n de zona de aterrizaje
    finishCheckTimer = new QTimer(this);
    connect(finishCheckTimer, SIGNAL(timeout()), this, SLOT(checkFinishLine()));
    finishCheckTimer->start(50);

    // Timer de actualizaciÃ³n de supervivientes (cada 50ms)
    survivorTimer = new QTimer(this);
    connect(survivorTimer, SIGNAL(timeout()), this, SLOT(updateSurvivors()));
    survivorTimer->start(50);

    // Timer de generaciÃ³n de bidones de combustible.
    // IMPORTANTE: antes este timer nunca se creaba (solo se inicializaba a
    // nullptr), por eso los bidones dejaron de aparecer. reset() lo reinicia.
    bidonTimer = new QTimer(this);
    connect(bidonTimer, SIGNAL(timeout()), this, SLOT(spawnBidon()));

    // Timer para la navegaciÃ³n automÃ¡tica despuÃ©s de victoria/derrota
    resultTimer = new QTimer(this);
    resultTimer->setSingleShot(true);
    // ConexiÃ³n ÃšNICA (en el constructor): evita conexiones acumuladas
    // cada vez que se gana o pierde.
    connect(resultTimer, &QTimer::timeout, this, [this]() {
        seleccionarNivel();
    });

    // ===== MUSICA DEL NIVEL =====
    // El nivel tiene SU propio tema (Level 1/2/3.mp3), que reproduce el
    // singleton AdminMusica en bucle. Al entrar a un nivel se detiene la
    // mÃºsica de las pantallas (login/menÃº) para que solo suene este tema.
    AdminMusica::instancia()->reproducirNivel(nivelActual);
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

    // Detener y liberar mÃºsica de fondo
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

    // El heli tiene parte QObject (timers de fÃ­sica, rotor y explosiÃ³n):
    // hay que borrarlo ANTES de la escena para detener sus timers y evitar
    // punteros colgantes al cambiar de nivel. Si ya se destruyÃ³ solo
    // (crash()), el puntero global quedÃ³ en nullptr (ver MyHeli::crash).
    if(heli != nullptr){
        delete heli;
        heli = nullptr;
    }

    // La escena se borra al final: elimina el heli, el score, la salud,
    // la zona de aterrizaje y el panel de resultados. Sin esto, cada
    // reintento filtrarÃ­a una escena completa en memoria.
    delete scene;

}

void Game::detenerMusica(){
    // Detiene la mÃºsica de fondo (se usa al salir del juego hacia el
    // menÃº/selector: el juego no debe seguir sonando si no se estÃ¡ jugando).
    if(bgMusic != nullptr){
        bgMusic->stop();
    }
}

void Game::pausar(){
    // Detiene TODO el juego (timers + mÃºsica) cuando se navega fuera.
    // Sin esto el mundo sigue corriendo en el menÃº: el nivel termina solo,
    // el heli cae y puede chocar, y hasta se navega solo al selector.
    if(finishCheckTimer != nullptr){
        finishCheckTimer->stop();
    }
    if(survivorTimer != nullptr){
        survivorTimer->stop();
    }
    if(bidonTimer != nullptr){
        bidonTimer->stop();
    }
    if(levelManager != nullptr){
        levelManager->detenerTimers();
    }
    if(heli != nullptr){
        heli->detenerTimers();
    }
    detenerMusica();
}

void Game::spawnObstacles(){

    // Si el juego terminÃ³ (ganÃ³ o perdiÃ³), no generar mÃ¡s obstÃ¡culos
    if(juegoTerminado){
        return;
    }

    // Cuando el tiempo llega a 0 LevelManager pone active=false,
    if(!levelManager->isActive()){
        return;
    }

    ObstacleType tipo;
    int yPos = 0;

    // PatrÃ³n de obstÃ¡culos segÃºn el nivel:
    // - Nivel 1 (ciudad): los 3 clÃ¡sicos (vertical, pequeÃ±o, techo).
    // - Niveles 2 y 3 (desierto/nieve): SOLO rocas/hielo del bioma
    //   (vertical, pequeÃ±o y el que CAE). El de techo (CEILING) es
    //   exclusivo de la ciudad y NO se genera en desierto/nieve.
    if(nivelActual >= 2){
        switch(contador % 3){
        case 0:
            tipo = ObstacleType::VERTICAL;
            // yPos se queda en 0, spawnObstacle usa la posiciÃ³n por defecto (parte baja)
            break;
        case 1:
            tipo = ObstacleType::SMALL;
            break;
            // yPos se queda en 0, spawnObstacle
        case 2:
            tipo = ObstacleType::FALLING;
            // cae desde arriba en una posiciÃ³n aleatoria (lo coloca spawnObstacle)
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

    // Generar un superviviente DETRÃS del obstÃ¡culo reciÃ©n creado.
    // El mÃ¡ximo por nivel lo define SurvivorManager (5/10/15). AsÃ­ el
    // superviviente aparece "en el mapa" detrÃ¡s de un obstÃ¡culo y se mueve
    // junto con Ã©l (efecto scroll).
    if(!survivorManager->allSpawned()){
        survivorManager->spawnSurvivorBehindObstacle(
            obstacleManager->getObstacle(obstacleManager->getCantidad() - 1));
    }
}

void Game::spawnBidon(){

    // Si el juego terminÃ³ (ganÃ³ o perdiÃ³), no generar mÃ¡s bidones
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
    // Si el juego terminÃ³, no actualizar supervivientes
    // (evita tocar el heli si ya fue destruido)
    if(juegoTerminado){
        return;
    }

    // Le dice a cada superviviente si el heli estÃ¡ encima (para el rescate)
    // y elimina los que ya fueron rescatados.
    if(heli == nullptr || heli->scene() == nullptr){
        return;
    }
    QRectF heliRect(heli->pos(), heli->boundingRect().size());
    survivorManager->updateAll(heliRect);
}


void Game::checkFinishLine(){

    // Si el juego terminÃ³ (ganÃ³ o perdiÃ³), no hacer nada mÃ¡s
    if(juegoTerminado){
        return;
    }

    // Actualizar la barra de gasolina del HUD (cada 50ms)
    actualizarBarraGas();

    // Solo actualizar el texto si el tiempo cambiÃ³
    int tiempoActual = levelManager->getTimeRemaining();
    if(tiempoActual != ultimoTiempo){
        ultimoTiempo = tiempoActual;
        int rescatados = survivorManager->getTotalRescatados();
        int objetivo = survivorManager->getTotalObjetivo();
        score->mostrarRescates(rescatados, objetivo);
        // Actualizar la barra de progreso del nivel (tiempo restante)
        actualizarBarraTiempo(tiempoActual);
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

    // CREAR LA ZONA CUANDO LA PANTALLA ESTÃ‰ LIMPIA

    if(finishLine == nullptr && levelManager->isFinished() && !nivelGanado){
        // si ya no hay obstÃ¡culos en pantalla
        if(obstacleManager->countVisible() == 0){
            // Crear la zona de aterrizaje
            finishLine = new FinishLine(scene);
            // El mapa deja de avanzar: el heli aterrizado ya no se desliza,
            // da la sensaciÃ³n de que el mundo se detuvo y solo falta aterrizar.
            mundoEnMovimiento = false;
        }else{
            // TodavÃ­a hay obstÃ¡culos, esperar al siguiente tick
            return;
        }
    }

    //VERIFICAR ATERRIZAJE EN LA ZONA
    // Solo verificar si la zona ya existe y el nivel terminÃ³
    if(finishLine == nullptr || !levelManager->isFinished() || nivelGanado){
        return;
    }
    //evita segmentation fault
    if(heli->scene() == nullptr){
        return;
    }
    // Verificar si el helicÃ³ptero estÃ¡ ATERRIZADO y sobre la zona
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
        // El heli estÃ¡ aterrizado sobre la zona â†’ sumar tiempo
        tiempoEnZona++;

        // 40 ticks = 2 segundos
        if(tiempoEnZona >= 40){
            nivelGanado = true;
            mostrarVictoria();
        }
    }else{
        // El heli no estÃ¡ en la zona reinicia el contador
        tiempoEnZona = 0;
    }
}


void Game::actualizarBarraGas(){
    // Barra de gasolina del HUD: se vacÃ­a segÃºn el combustible del heli.
    // Muestra la alerta "Gasolina en critico" cuando queda <= 30%.
    if(heli == nullptr || barraGasRelleno == nullptr){
        return;
    }
    double fuel = heli->getFuel();
    int ancho = static_cast<int>(150.0 * fuel / 100.0);
    if(ancho < 0) ancho = 0;
    if(ancho > 150) ancho = 150;
    barraGasRelleno->setRect(70, 103, ancho, 14);

    if(textoGasCritico != nullptr){
        textoGasCritico->setVisible(fuel <= 30.0);
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

    // El juego terminÃ³: detener la mÃºsica de fondo
    if(bgMusic != nullptr){
        bgMusic->stop();
    }

    mostrarPopupVictoria();
}

void Game::mostrarDerrota(){

    if(juegoTerminado){
        return;  // ya se mostrÃ³ un panel
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

    // El panel se queda en pantalla hasta que el jugador decida
    // (Reintentar / Seleccionar nivel / Volver al menÃº). NO navega solo.
    // Detener la mÃºsica: el juego terminÃ³, no debe seguir sonando.
    if(bgMusic != nullptr){
        bgMusic->stop();
    }
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

    btnMenu = new QPushButton("Volver al menÃº");
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
    // no rescatado baja un escalÃ³n: A, A-, B+, B, B-, C+, C, C-, D, F.
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
    // Verificar si se desbloqueÃ³ el logro de todos los A
    if(todosNivelesConA()){
        // El logro ya estÃ¡ desbloqueado, no hacer nada extra
        // Se mostrarÃ¡ en el popup de la prÃ³xima victoria
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

    // TÃ­tulo "NIVEL COMPLETADO"
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

    // ===== BOTONES =====
    // Reiniciar y salir al menÃº: el jugador decide (sin cierre automÃ¡tico).
    QPushButton *btnVictoriaReiniciar = new QPushButton("Reiniciar");
    btnVictoriaReiniciar->setFixedSize(220, 50);
    btnVictoriaReiniciar->setCursor(Qt::PointingHandCursor);
    btnVictoriaReiniciar->setStyleSheet(
        "QPushButton { background-color: #1a1a2e; color: #4de8ff;"
        " border: 2px solid #4de8ff; border-radius: 6px;"
        " font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2a2a4e; }"
        "QPushButton:pressed { background-color: #0f0f1e; }");
    QGraphicsProxyWidget *proxyVictoriaReiniciar = scene->addWidget(btnVictoriaReiniciar);
    proxyVictoriaReiniciar->setPos(290, 330);
    proxyVictoriaReiniciar->setZValue(2005);
    connect(btnVictoriaReiniciar, &QPushButton::clicked, this, &Game::reintentarNivel);

    QPushButton *btnVictoriaMenu = new QPushButton("Salir al menÃº");
    btnVictoriaMenu->setFixedSize(220, 50);
    btnVictoriaMenu->setCursor(Qt::PointingHandCursor);
    btnVictoriaMenu->setStyleSheet(
        "QPushButton { background-color: #1a1a2e; color: #ff6b6b;"
        " border: 2px solid #ff6b6b; border-radius: 6px;"
        " font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2a2a4e; }"
        "QPushButton:pressed { background-color: #0f0f1e; }");
    QGraphicsProxyWidget *proxyVictoriaMenu = scene->addWidget(btnVictoriaMenu);
    proxyVictoriaMenu->setPos(290, 390);
    proxyVictoriaMenu->setZValue(2005);
    connect(btnVictoriaMenu, &QPushButton::clicked, this, &Game::volverAlMenu);
}

void Game::reintentarNivel(){
    // Cancelar cualquier timer automÃ¡tico de victoria/derrota pendiente
    if(resultTimer != nullptr){
        resultTimer->stop();
    }
    // Diferir la navegaciÃ³n un tick: el botÃ³n que disparÃ³ este slot sigue
    // procesando su evento de mouse; si reset() lo borra ahora (vÃ­a
    // scene->clear()) el botÃ³n usarÃ­a memoria liberada al terminar el
    // evento â†’ crash. Con singleShot(0) el reset corre despuÃ©s.
    QTimer::singleShot(0, this, [this]() {
        ventanaPrincipal->reintentarNivel();
    });
}

void Game::seleccionarNivel(){
    // Cancelar el timer automÃ¡tico antes de navegar
    if(resultTimer != nullptr){
        resultTimer->stop();
    }
    // Diferir la navegaciÃ³n un tick (ver reintentarNivel)
    QTimer::singleShot(0, this, [this]() {
        ventanaPrincipal->mostrarSelector();
    });
}

void Game::volverAlMenu(){
    // Cancelar el timer automÃ¡tico antes de navegar
    if(resultTimer != nullptr){
        resultTimer->stop();
    }
    // Diferir la navegaciÃ³n un tick (ver reintentarNivel)
    QTimer::singleShot(0, this, [this]() {
        ventanaPrincipal->mostrarMenu();
    });
}

void Game::mostrarMenuOpciones(){
    // Pantallita de pausa (estilo panel de derrota, pero simple):
    // fondo oscuro + tÃ­tulo "PAUSA" + botones Reanudar/Reiniciar/Volver al menÃº.
    if(pausaActiva || juegoTerminado){
        return;  // ya estÃ¡ abierta, o el juego ya ganÃ³/perdiÃ³
    }
    pausaActiva = true;
    juegoTerminado = true;  // congela el mundo (heli, obstÃ¡culos, supervivientes)

    // Detener la cuenta regresiva del nivel y el spawn de obstÃ¡culos:
    // si no, el nivel termina solo mientras estÃ¡ en pausa.
    if(levelManager != nullptr){
        levelManager->detenerTimers();
    }

    // Pausar la mÃºsica de fondo
    if(bgMusic != nullptr){
        bgMusic->pause();
    }

    // Fondo oscuro semi-transparente que tapa toda la escena
    pausaFondo = new QGraphicsRectItem(0, 0, scene->width(), scene->height());
    pausaFondo->setBrush(QColor(0, 0, 0, 180));
    pausaFondo->setPen(Qt::NoPen);
    pausaFondo->setZValue(1000);
    scene->addItem(pausaFondo);

    // TÃ­tulo "PAUSA"
    pausaTitulo = new QGraphicsTextItem();
    pausaTitulo->setFont(QFont("times", 34, QFont::Bold));
    pausaTitulo->setDefaultTextColor(Qt::white);
    pausaTitulo->setPlainText("PAUSA");
    pausaTitulo->setZValue(1001);
    pausaTitulo->setPos(scene->width() / 2 - pausaTitulo->boundingRect().width() / 2, 150);
    scene->addItem(pausaTitulo);

    // BotÃ³n Reanudar (verde): cierra la pantallita y sigue jugando
    btnReanudar = new QPushButton("Reanudar");
    btnReanudar->setFixedSize(220, 50);
    btnReanudar->setCursor(Qt::PointingHandCursor);
    btnReanudar->setStyleSheet(
        "QPushButton { background-color: #1a1a2e; color: #7dff9b;"
        " border: 2px solid #7dff9b; border-radius: 6px;"
        " font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2a2a4e; }"
        "QPushButton:pressed { background-color: #0f0f1e; }");
    proxyReanudar = scene->addWidget(btnReanudar);
    proxyReanudar->setPos(290, 300);
    proxyReanudar->setZValue(1001);
    connect(btnReanudar, &QPushButton::clicked, this, &Game::cerrarMenuOpciones);

    // BotÃ³n Reiniciar (cian): reinicia el nivel
    btnPausaReiniciar = new QPushButton("Reiniciar");
    btnPausaReiniciar->setFixedSize(220, 50);
    btnPausaReiniciar->setCursor(Qt::PointingHandCursor);
    btnPausaReiniciar->setStyleSheet(
        "QPushButton { background-color: #1a1a2e; color: #4de8ff;"
        " border: 2px solid #4de8ff; border-radius: 6px;"
        " font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2a2a4e; }"
        "QPushButton:pressed { background-color: #0f0f1e; }");
    proxyPausaReiniciar = scene->addWidget(btnPausaReiniciar);
    proxyPausaReiniciar->setPos(290, 360);
    proxyPausaReiniciar->setZValue(1001);
    connect(btnPausaReiniciar, &QPushButton::clicked, this, &Game::reintentarNivel);

    // BotÃ³n Volver al menÃº (rojo)
    btnPausaMenu = new QPushButton("Volver al menÃº");
    btnPausaMenu->setFixedSize(220, 50);
    btnPausaMenu->setCursor(Qt::PointingHandCursor);
    btnPausaMenu->setStyleSheet(
        "QPushButton { background-color: #1a1a2e; color: #ff6b6b;"
        " border: 2px solid #ff6b6b; border-radius: 6px;"
        " font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2a2a4e; }"
        "QPushButton:pressed { background-color: #0f0f1e; }");
    proxyPausaMenu = scene->addWidget(btnPausaMenu);
    proxyPausaMenu->setPos(290, 420);
    proxyPausaMenu->setZValue(1001);
    connect(btnPausaMenu, &QPushButton::clicked, this, &Game::volverAlMenu);
}

void Game::cerrarMenuOpciones(){
    // Reanudar: cierra la pantallita de pausa y sigue el juego.
    // Diferir un tick: el botÃ³n que disparÃ³ el slot sigue procesando su
    // evento de mouse; borrarlo ahora crashearÃ­a (ver reintentarNivel).
    QTimer::singleShot(0, this, [this]() {
        if(!pausaActiva){
            return;
        }
        pausaActiva = false;
        juegoTerminado = false;  // reanuda el mundo

        // Quitar el panel de pausa
        if(pausaFondo != nullptr){ scene->removeItem(pausaFondo); delete pausaFondo; pausaFondo = nullptr; }
        if(pausaTitulo != nullptr){ scene->removeItem(pausaTitulo); delete pausaTitulo; pausaTitulo = nullptr; }
        if(proxyReanudar != nullptr){ scene->removeItem(proxyReanudar); delete proxyReanudar; proxyReanudar = nullptr; }
        if(proxyPausaReiniciar != nullptr){ scene->removeItem(proxyPausaReiniciar); delete proxyPausaReiniciar; proxyPausaReiniciar = nullptr; }
        if(proxyPausaMenu != nullptr){ scene->removeItem(proxyPausaMenu); delete proxyPausaMenu; proxyPausaMenu = nullptr; }
        btnReanudar = nullptr;
        btnPausaReiniciar = nullptr;
        btnPausaMenu = nullptr;

        // Reanudar la cuenta regresiva del nivel y el spawn de obstÃ¡culos
        if(levelManager != nullptr){
            int spawnMs = 3000;
            switch(nivelActual){
            case 2:
                spawnMs = 2500;
                break;
            case 3:
                spawnMs = 2000;
                break;
            }
            levelManager->startLevel(levelManager->getTimeRemaining(), spawnMs);
        }

        // Reanudar la mÃºsica de fondo
        if(bgMusic != nullptr){
            bgMusic->play();
        }

        // Devolver el foco al heli para que las teclas funcionen
        if(heli != nullptr && heli->scene() != nullptr){
            heli->setFocus();
        }
    });
}

void Game::reset(int nivel){
    // 0. Cancelar cualquier timer automÃ¡tico de victoria/derrota pendiente
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

    // 1.5 Actualizar el fondo segun el nuevo nivel (bug: faltaba esto,
    // por eso el fondo quedaba pegado al del primer nivel jugado)
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
    btnOpciones = nullptr;
    proxyOpciones = nullptr;
    // La pantallita de pausa tambiÃ©n la borrÃ³ scene->clear()
    pausaActiva = false;
    pausaFondo = nullptr;
    pausaTitulo = nullptr;
    btnReanudar = nullptr;
    btnPausaReiniciar = nullptr;
    btnPausaMenu = nullptr;
    proxyReanudar = nullptr;
    proxyPausaReiniciar = nullptr;
    proxyPausaMenu = nullptr;
    // Los popups de victoria tambiÃ©n los borrÃ³ scene->clear():
    // sin esto quedan dangling y la prÃ³xima victoria crashea.
    popupFondo = nullptr;
    popupBarra = nullptr;
    popupNota = nullptr;
    popupMensaje = nullptr;
    popupLogro = nullptr;
    // La barra de tiempo del nivel tambiÃ©n la borrÃ³ scene->clear()
    barraTiempoFondo = nullptr;
    barraTiempoRelleno = nullptr;
    textoTiempo = nullptr;
    // La barra de gasolina del HUD tambiÃ©n la borrÃ³ scene->clear()
    barraGasFondo = nullptr;
    barraGasRelleno = nullptr;
    textoGas = nullptr;
    textoGasCritico = nullptr;

    // 6. Re-crear managers
    obstacleManager = new ObstacleManager(scene, nivel, this);
    survivorManager = new SurvivorManager(scene, obstacleManager, nivel, this);
    levelManager = new LevelManager(this);
    // Re-conectar el spawn de obstÃ¡culos al timer del nivel
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

    // 8.5 Crear la barra de progreso del nivel (tiempo restante).
    // Arriba al centro: se vacÃ­a conforme pasa el tiempo del nivel.
    barraTiempoFondo = new QGraphicsRectItem(250, 15, 300, 20);
    barraTiempoFondo->setBrush(QBrush(QColor(30, 30, 60)));
    barraTiempoFondo->setPen(QPen(QColor(255, 255, 255, 120), 1));
    barraTiempoFondo->setZValue(100);
    scene->addItem(barraTiempoFondo);

    barraTiempoRelleno = new QGraphicsRectItem(250, 15, 300, 20);
    barraTiempoRelleno->setBrush(QBrush(QColor(0, 200, 80)));
    barraTiempoRelleno->setPen(Qt::NoPen);
    barraTiempoRelleno->setZValue(101);
    scene->addItem(barraTiempoRelleno);

    textoTiempo = new QGraphicsTextItem();
    textoTiempo->setFont(QFont("Arial", 16, QFont::Bold));
    textoTiempo->setDefaultTextColor(Qt::white);
    textoTiempo->setZValue(102);
    textoTiempo->setPos(560, 13);
    scene->addItem(textoTiempo);

    // 8.6 Barra de gasolina (HUD): debajo de la vida, estilo "GAS".
    // La barra ya no va encima del heli: se muestra fija en el HUD.
    textoGas = new QGraphicsTextItem();
    textoGas->setFont(QFont("Arial", 16, QFont::Bold));
    textoGas->setDefaultTextColor(Qt::white);
    textoGas->setPlainText("GAS");
    textoGas->setZValue(100);
    textoGas->setPos(20, 100);
    scene->addItem(textoGas);

    barraGasFondo = new QGraphicsRectItem(70, 103, 150, 14);
    barraGasFondo->setBrush(QBrush(QColor(60, 60, 60)));
    barraGasFondo->setPen(QPen(QColor(255, 255, 255, 120), 1));
    barraGasFondo->setZValue(100);
    scene->addItem(barraGasFondo);

    barraGasRelleno = new QGraphicsRectItem(70, 103, 150, 14);
    barraGasRelleno->setBrush(QBrush(QColor(255, 140, 0))); // naranja
    barraGasRelleno->setPen(Qt::NoPen);
    barraGasRelleno->setZValue(101);
    scene->addItem(barraGasRelleno);

    // Alerta de gasolina crÃ­tica (visible solo cuando fuel <= 30%)
    textoGasCritico = new QGraphicsTextItem();
    textoGasCritico->setFont(QFont("Arial", 14, QFont::Bold));
    textoGasCritico->setDefaultTextColor(Qt::red);
    textoGasCritico->setPlainText("Gasolina en critico");
    textoGasCritico->setZValue(102);
    textoGasCritico->setPos(20, 123);
    textoGasCritico->setVisible(false);
    scene->addItem(textoGasCritico);

    // 8.7 BotÃ³n de opciones (â‹®) arriba a la derecha: menÃº para reiniciar
    // o volver al menÃº en medio de un nivel (sin tener que perder).
    btnOpciones = new QPushButton("â‹®");
    btnOpciones->setFixedSize(40, 40);
    btnOpciones->setCursor(Qt::PointingHandCursor);
    btnOpciones->setToolTip("Opciones");
    btnOpciones->setStyleSheet(
        "QPushButton { background-color: #3a3a3a; color: #ffffff;"
        " border: 1px solid #666666; border-radius: 6px;"
        " font-size: 22px; font-weight: bold; }"
        "QPushButton:hover { background-color: #4a4a4a; }"
        "QPushButton:pressed { background-color: #2a2a2a; }");
    proxyOpciones = scene->addWidget(btnOpciones);
    proxyOpciones->setPos(scene->width() - 50, 10);
    proxyOpciones->setZValue(500);
    connect(btnOpciones, &QPushButton::clicked, this, &Game::mostrarMenuOpciones);

    // 9. Re-conectar bidonTimer (ya existe, solo reiniciarlo).
    // Los bidones aparecen cada 6s (antes 4s: menos frecuentes).
    if(bidonTimer != nullptr){
        bidonTimer->stop();
        bidonTimer->start(6000);
    }

    // 10. REINICIAR TIMERS PRINCIPALES (finishCheckTimer y survivorTimer)
    finishCheckTimer->stop();
    finishCheckTimer->start(50);
    survivorTimer->stop();
    survivorTimer->start(50);

    // 11. REINICIAR MÃšSICA DE FONDO
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

    // Guardar el tiempo total del nivel y llenar la barra de progreso
    tiempoLimiteTotal = tiempoLimite;
    actualizarBarraTiempo(tiempoLimite);
}

void Game::actualizarBarraTiempo(int tiempoRestante){
    // Barra de progreso del nivel: muestra cuÃ¡nto falta para que termine
    // (se vacÃ­a de 30s/35s hasta 0). Verde -> amarillo -> rojo.
    if(barraTiempoRelleno == nullptr || tiempoLimiteTotal <= 0){
        return;
    }
    int ancho = static_cast<int>(300.0 * tiempoRestante / tiempoLimiteTotal);
    if(ancho < 0) ancho = 0;
    if(ancho > 300) ancho = 300;
    barraTiempoRelleno->setRect(250, 15, ancho, 20);

    double fraccion = static_cast<double>(tiempoRestante) / tiempoLimiteTotal;
    QColor color;
    if(fraccion > 0.5){
        color = QColor(0, 200, 80);      // verde: queda mucho tiempo
    }else if(fraccion > 0.25){
        color = QColor(255, 200, 0);     // amarillo: la mitad
    }else{
        color = QColor(220, 40, 40);     // rojo: queda poco
    }
    barraTiempoRelleno->setBrush(color);

    if(textoTiempo != nullptr){
        textoTiempo->setPlainText(QString("Tiempo: %1s").arg(tiempoRestante));
    }
}

void Game::showEvent(QShowEvent *event){
    QGraphicsView::showEvent(event);
    //escala la escena para que quepa completa en la ventana
    //(evita que el heli y el suelo se corten en el borde inferior)
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    // Al mostrarse la pÃ¡gina del juego, darle el foco al heli para que
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
    // (drawBackground), asÃ­ nunca se ve deformada ni cortada.
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void Game::drawBackground(QPainter *painter, const QRectF &rect){
    // Dibujar el fondo de la ciudad estirado a TODA la ventana.
    // Al redimensionar, el fondo crece con la ventana: no quedan franjas
    // oscuras en los lados (antes el fondo era un item de la escena y solo
    // cubrÃ­a los 800x600 de la escena).
    painter->save();
    painter->resetTransform();  // dibujar en coordenadas de la ventana
    painter->drawPixmap(viewport()->rect(), fondoCiudad);
    painter->restore();
}
#ifndef GAME_H
#define GAME_H

#include <QGraphicsView>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <QPixmap>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "MyHeli.h"
#include "Score.h"
#include "Health.h"
#include "ObstacleManager.h"
#include "LevelManager.h"
#include "FinishLine.h"
#include "SurvivorManager.h"
#include "BidonCombustible.h"

class Game : public QGraphicsView {
    Q_OBJECT //

public:
    // Constructor: recibe el nivel a jugar (1, 2 o 3)
    Game(int nivel = 1, QWidget *parent = nullptr);

    // Destructor
    ~Game();

    // Atributos principales del juego
    QGraphicsScene * scene;
    MyHeli * heli;
    Score * score;
    Health * health;
    ObstacleManager * obstacleManager;
    LevelManager * levelManager;
    FinishLine * finishLine;
    SurvivorManager * survivorManager;  // Maneja a los supervivientes
    int nivelActual;         // Nivel que se está jugando (1, 2 o 3)
    bool mundoEnMovimiento;  // true: el mundo avanza (scroll). false: la meta apareció y todo se detiene
    bool juegoTerminado;     // true cuando se gana o pierde: pausa todo el mundo
    QTimer *bidonTimer;     // Timer que genera bidones de combustible

    // Pantallas de resultados (estilo Cuphead)
    void mostrarVictoria();  // Nota A/A-/B+... según vidas y rescatados
    void mostrarDerrota();   // Porcentaje de rescate + barra

    // Sistema de logros
    void mostrarPopupVictoria();  // Popup de nota coloreado por nivel
    void cargarLogros();
    void guardarLogro(int nivel, const QString& nota);
    bool todosNivelesConA();

public slots:
    void spawnObstacles();
    void checkFinishLine();
    void updateSurvivors();
    void reintentarNivel();
    void seleccionarNivel();
    void volverAlMenu();
    void spawnBidon();  // Genera un bidón de combustible
    void reset(int nivel);  // Reinicia el juego en su lugar (sin crear objetos nuevos)

    //logica de zona de aterrizaje
private:

    bool heliEnZona;
    int tiempoEnZona;
    bool nivelGanado;

    // Contador de patrón de obstáculos (se reinicia al reintentar)
    int contador;

    // Estado del HUD (antes eran variables static locales: se reinician
    // correctamente al reintentar el nivel)
    int ultimoTiempo;
    bool instruccionMostrada;

    // Panel de resultados (se crea la primera vez que se gana o pierde)
    QGraphicsRectItem *panelFondo;
    QGraphicsTextItem *panelTitulo;
    QGraphicsTextItem *panelNota;
    QGraphicsTextItem *panelSub;
    QGraphicsRectItem *barraFondo;
    QGraphicsRectItem *barraRelleno;
    QPushButton *btnReintentar;
    QPushButton *btnNiveles;
    QPushButton *btnMenu;
    QGraphicsProxyWidget *proxyReintentar;
    QGraphicsProxyWidget *proxyNiveles;
    QGraphicsProxyWidget *proxyMenu;

    // Timers principales del juego (ahora son miembros para poder reiniciarse en reset())
    QTimer *finishCheckTimer;
    QTimer *survivorTimer;

    // Timer para cancelar el singleShot de victoria/derrota automática
    QTimer *resultTimer;

    // Música de fondo
    QMediaPlayer *bgMusic;
    QAudioOutput *bgAudio;

    // Sistema de logros
    QGraphicsRectItem *popupFondo;
    QGraphicsTextItem *popupNota;
    QGraphicsTextItem *popupMensaje;
    QGraphicsTextItem *popupLogro;
    QGraphicsRectItem *popupBarra;
    QColor popupColorFondo;
    QColor popupColorBorde;

    void crearPanel();
    QString calcularNota(int vidas, int rescatados) const;
    void actualizarLogros();

    QPixmap fondoCiudad;  // Fondo de la ciudad (se dibuja a toda la ventana)

protected:
    //ajusta la vista para que toda la escena siempre sea visible
    void showEvent(QShowEvent *event) override;
    //re-escala la escena cuando se redimensiona la ventana
    void resizeEvent(QResizeEvent *event) override;
    //dibuja el fondo de la ciudad estirado a TODA la ventana
    void drawBackground(QPainter *painter, const QRectF &rect) override;
};

#endif // GAME_H

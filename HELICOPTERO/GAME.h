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
    void detenerMusica();    // Detiene la música de fondo (al salir del juego)
    void pausar();           // Detiene TODO el juego (timers + música) al salir al menú

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
    void mostrarMenuOpciones();  // Pantallita de pausa (⋮): Reanudar / Reiniciar / Volver al menú
    void cerrarMenuOpciones();   // Reanudar: cierra la pantallita y sigue el juego

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

    // Botón de opciones (⋮) arriba a la derecha: abre la pantallita de
    // pausa para reiniciar, volver al menú o reanudar en medio de un nivel.
    QPushButton *btnOpciones;
    QGraphicsProxyWidget *proxyOpciones;

    // Pantallita de pausa (estilo panel de derrota, pero simple):
    // fondo oscuro + título "PAUSA" + botones Reanudar/Reiniciar/Volver al menú.
    bool pausaActiva;              // true mientras la pantallita está abierta
    QGraphicsRectItem *pausaFondo;
    QGraphicsTextItem *pausaTitulo;
    QPushButton *btnReanudar;
    QPushButton *btnPausaReiniciar;
    QPushButton *btnPausaMenu;
    QPushButton *btnPausaMute;
    QGraphicsProxyWidget *proxyReanudar;
    QGraphicsProxyWidget *proxyPausaReiniciar;
    QGraphicsProxyWidget *proxyPausaMenu;
    QGraphicsProxyWidget *proxyPausaMute;

    // Timers principales del juego (ahora son miembros para poder reiniciarse en reset())
    QTimer *finishCheckTimer;
    QTimer *survivorTimer;

    // Barra de progreso del nivel: tiempo restante (no es la vida)
    QGraphicsRectItem *barraTiempoFondo;
    QGraphicsRectItem *barraTiempoRelleno;
    QGraphicsTextItem *textoTiempo;
    int tiempoLimiteTotal;   // segundos totales del nivel (para el porcentaje)

    // Barra de gasolina (HUD): debajo de la vida, con alerta de crítico.
    QGraphicsRectItem *barraGasFondo;
    QGraphicsRectItem *barraGasRelleno;
    QGraphicsTextItem *textoGas;
    QGraphicsTextItem *textoGasCritico;

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
    void actualizarBarraTiempo(int tiempoRestante);
    void actualizarBarraGas();

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

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
#include "MyHeli.h"
#include "Score.h"
#include "Health.h"
#include "ObstacleManager.h"
#include "LevelManager.h"
#include "FinishLine.h"
#include "SurvivorManager.h"

class Game : public QGraphicsView {
    Q_OBJECT //

public:
    // Constructor
    Game(QWidget *parent = nullptr);

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
    bool mundoEnMovimiento;  // true: el mundo avanza (scroll). false: la meta apareció y todo se detiene
    bool juegoTerminado;     // true cuando se gana o pierde: pausa todo el mundo

    // Pantallas de resultados (estilo Cuphead)
    void mostrarVictoria();  // Nota A/A-/B+... según vidas y rescatados
    void mostrarDerrota();   // Porcentaje de rescate + barra

public slots:
    void spawnObstacles();
    void checkFinishLine();
    void updateSurvivors();
    void reintentarNivel();
    void volverAlMenu();

    //logica de zona de aterrizaje
private:

    bool heliEnZona;
    int tiempoEnZona;
    bool nivelGanado;

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
    QPushButton *btnMenu;
    QGraphicsProxyWidget *proxyReintentar;
    QGraphicsProxyWidget *proxyMenu;

    void crearPanel();
    QString calcularNota(int vidas, int rescatados) const;

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
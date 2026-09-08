#ifndef GAME_H
#define GAME_H

#include <QGraphicsView>
#include <QWidget>
#include <QGraphicsScene>
#include "MyHeli.h"
#include "Score.h"
#include "Health.h"
#include "ObstacleManager.h"
#include "LevelManager.h"
#include "FinishLine.h"

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

    public slots:
   void spawnObstacles();
   void checkFinishLine();

//logica de zona de aterrizaje
    private:

   bool heliEnZona;
   int tiempoEnZona;
   bool nivelGanado;

    void mostrarVictoria();//temporal

protected:
    //ajusta la vista para que toda la escena siempre sea visible
    void showEvent(QShowEvent *event) override;
};

#endif // GAME_H